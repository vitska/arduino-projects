/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/
#include <Esp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ADS1X15.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <AsyncMqtt_Generic.h>

  // 102 - 17watt
  // 4370 - 700watt 128 sps
  // 8540 - 700watt 250 sps

#define CALIBRATION_700W (float)4400
const float digital_ac_current_to_watt = CALIBRATION_700W / 700;

char wifi_ssid[] = "Xiaomi_VSA1";
char wifi_pass[] = "sv@rkas52";
char mqtt_username[] = "cfg-mqtt";
char mqtt_password[] = "Zu1aeCheeshaelaiNuuG2Ooca1vooh6fiegeek4eeboong3uatoh0ahx9eud3phu";
IPAddress mqtt_host(192, 168, 3, 20);
const uint16_t mqtt_port = 1883;
const int timezone = 3;  // hours
const int dst = 0;
const char* device_serial = "500291D66C9M";

#define NUMBER_OF_CHANNELS 4
#define EEPROM_WRITE_PERIOD 3000 // ~ 5 minutes
#define MQTT_PUBLISH_PERIOD 15 //

bool mqtt_connected = false;
ADS1115 adc0(0x48);
LiquidCrystal_I2C lcd(0x3F, 20, 4);
AsyncMqttClient mqttClient;

struct t_channel_data{
  uint16_t min;
  uint16_t max;
  uint16_t pp;
  float watt;
  float watt_prev;
  uint64_t runMilis;
  uint64_t last_millis;
  double period_watt_h;
  double cumulative_kwatt_h;
  uint64_t cumulative_milis;
  uint8_t changed;
};

t_channel_data channel[NUMBER_OF_CHANNELS];

void listNetworksToSerial(int n){
  Serial.print(n);
  Serial.println(" WiFi networks found");
  for (int i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    Serial.print(i + 1);
    Serial.print(") ");
    Serial.print(WiFi.SSID(i));// SSID

    Serial.print(WiFi.RSSI(i));//Signal strength in dBm  
    Serial.print("dBm (");
    
    Serial.print(dBmtoPercentage(WiFi.RSSI(i)));//Signal strength in %  
    Serial.print("% )"); 
    if(WiFi.encryptionType(i) == ENC_TYPE_NONE)
    {
        Serial.println(" <<***OPEN***>>");        
    }else{
        Serial.println();        
    }

    delay(10);
  }
}

const int RSSI_MAX =-50;// define maximum strength of signal in dBm
const int RSSI_MIN =-100;// define minimum strength of signal in dBm

int dBmtoPercentage(int dBm)
{
  int quality;
    if(dBm <= RSSI_MIN)
    {
        quality = 0;
    }
    else if(dBm >= RSSI_MAX)
    {  
        quality = 100;
    }
    else
    {
        quality = 2 * (dBm + 100);
   }

     return quality;
}//dBmtoPercentage 

void init_wifi_time() // sets time
{
	WiFi.mode(WIFI_STA);
  int nw_cnt = WiFi.scanNetworks();
  if (nw_cnt == 0) {
    Serial.println("no WiFi networks found");
    return;
  } else {
    listNetworksToSerial(nw_cnt);
  }
  Serial.printf("Init time from WiFi [%s] ...\n", wifi_ssid);
  lcd.setCursor(0, 0);lcd.printf("WiFi:%s", wifi_ssid);
	WiFi.begin(wifi_ssid, wifi_pass);

	while (WiFi.status() != WL_CONNECTED)
	  {
		  delay(1);
	  }

  Serial.print("IP Address: "); Serial.println(WiFi.localIP());
  lcd.setCursor(0, 1);lcd.printf("IP:%s", WiFi.localIP().toString().c_str());

	configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
	delay(500);

  time_t rawtime; 
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  Serial.printf("DateTime:[%04d-%02d-%02dT%02d:%02d:%02d]\n", 1900 + timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  lcd.setCursor(0, 2);lcd.printf("%04d-%02d-%02dT%02d:%02d:%02d", 1900 + timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  delay (2000);

//12345678901234567890
//2023-03-01T08:20:20
	WiFi.disconnect();
}

#define EEPROM_INIT_MARKER 0x1237 // change marker to reset cumulative EEPROM

void init_from_eeprom(){
  int eepromAddress = 0;
  int marker = 0;
  EEPROM.get(eepromAddress, marker); eepromAddress += sizeof(marker);
  Serial.printf("EEPROM marker:%X\n", marker);
  if(marker == EEPROM_INIT_MARKER){
    for(int i=0;i<NUMBER_OF_CHANNELS;i++){
      EEPROM.get(eepromAddress, channel[i].cumulative_kwatt_h);
      Serial.printf("Reading channel %d [Address:%d] cumulative_kwatt_h:%f\n", i, eepromAddress, channel[i].cumulative_kwatt_h);
      eepromAddress += sizeof(channel[i].cumulative_kwatt_h);
    }
  }  
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(500000);
	EEPROM.begin(64);
  memset(channel, 0, sizeof(channel));
  init_from_eeprom();
  //  0  =  CONTINUOUS
  //  1  =  SINGLE      default
  adc0.setMode(0);
/*
differs for different devices, check datasheet or readme.md
|  data rate  |  ADS101x  |  ADS111x  |   Notes   |
|:-----------:|----------:|----------:|:---------:|
|     0       |   128     |    8      |  slowest  |
|     1       |   250     |    16     |           |
|     2       |   490     |    32     |           |
|     3       |   920     |    64     |           |
|     4       |   1600    |    128    |  default  |
|     5       |   2400    |    250    |           |
|     6       |   3300    |    475    |           |
|     7       |   3300    |    860    |  fastest  |
*/
  // adc0.setDataRate(3); // actual speed depends on device
  //           GAIN
  //  0  =  +- 6.144V  default
  //  1  =  +- 4.096V
  //  2  =  +- 2.048V
  //  4  =  +- 1.024V
  //  8  =  +- 0.512V
  //  16 =  +- 0.256V
  adc0.setGain(4);
  
  Wire.begin(2,0);
  lcd.init();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight 

  init_wifi_time();
  mqtt_start();
}

// 1000W * h = 1kWh
// 1000 / 3600 000 = kWms
// 1000 * runtime / 3600 000

// 860 / 50 = 17.2
#define ADC_READINGS_NUMBER 14 // 25 max, with bigger loop delay WiFi fails
#define ADC_READINGS_DELAY_MS 7

void getAndRecalculateChannel(struct t_channel_data *data, int channel){
  data->min=0xFFFF,data->max=0;
  for(int i=0;i<ADC_READINGS_NUMBER;i++){
    delay(ADC_READINGS_DELAY_MS);
    uint16_t current = adc0.getValue();
//    Serial.printf("ADC:%u\n", current);
    if(current > data->max){
      data->max = current;      
    }
    if(current < data->min){
      data->min = current;
    }
  }
  data->pp = data->max-data->min;  
//  Serial.printf("min:%u;max:%u;pp:%u\n", data->min, data->max, data->pp);
  // Cut off zero noice
  if(data->pp < 3){
    data->pp = 0;
  }

  // calibration, assume linear
  // 102 - 17watt
  // 4370 - 700watt 128 sps
  // 5150 - 700watt 250 sps
  data->watt_prev = data->watt;  
  data->watt = data->pp;
  data->watt /= digital_ac_current_to_watt;

  data->changed = data->watt_prev != data->watt;

  uint64_t time = millis();
  data->runMilis = time - data->last_millis;
  data->last_millis = time;
  data->cumulative_milis += data->runMilis;

  data->period_watt_h = data->watt;
  data->period_watt_h *= data->runMilis;
  data->period_watt_h /= 3600000;
  data->cumulative_kwatt_h += data->period_watt_h / 1000;
}

void printChannelBusy(int channel){
  lcd.setCursor(0, channel);lcd.print(" ");
}

void printChannel(struct t_channel_data *data, int channel){
//  lcd.setCursor(0, channel);lcd.print(channel);lcd.print(":");
  lcd.setCursor(0, channel);lcd.print(data->pp);lcd.print("    ");
  lcd.setCursor(5, channel);lcd.printf("%3.1f", data->watt);lcd.print("    ");
//  lcd.setCursor(6, channel);lcd.print(data->runMilis);
  lcd.setCursor(11, channel);lcd.printf("%4.6f", data->cumulative_kwatt_h);lcd.print(" ");
}

void shape_matching(struct t_channel_data *data, int channel){
}

void write_eeprom(){
  int eepromAddress = 0;
  int marker = EEPROM_INIT_MARKER;
  EEPROM.put(eepromAddress, marker); eepromAddress += sizeof(marker);
  Serial.printf("Writting EEPROM marker:%X\n", marker);
  for(int i=0;i<NUMBER_OF_CHANNELS;i++){
    Serial.printf("Writting channel %d [Address:%d] cumulative_kwatt_h:%f\n", i, eepromAddress, channel[i].cumulative_kwatt_h);
    EEPROM.put(eepromAddress, channel[i].cumulative_kwatt_h);
    eepromAddress += sizeof(channel[i].cumulative_kwatt_h);
  }
  ESP.wdtFeed(); // service the WDT here
  EEPROM.commit();
}

int eeprom_cnt = EEPROM_WRITE_PERIOD;
int mqtt_publish_cnt = MQTT_PUBLISH_PERIOD;
int mqtt_publish_channel_num = 0;
//int channel_num = 0;
unsigned long time_pin = 0;
int state_machine = 0;
int publish_state_machine = 0;

// void mqtt_publish_channel(struct t_channel_data *data, int channel){
//   if(!mqtt_connected){
//     return;
//   }

//   char value_buf[] = "0000.000000";
//   sprintf(value_buf, "%.1f", data->watt);

//   mqtt_publish_state(channel, "W", value_buf);
//   // sprintf(mqtt_topic_buffer, mqtt_power_topic_format, channel);
//   // Serial.printf("Publish:[%s] value:[%s]\n", mqtt_topic_buffer, value_buf);
//   // mqttClient.publish(mqtt_topic_buffer, 2, true, value_buf);

//   // sprintf(mqtt_topic_buffer, mqtt_cumulative_power_topic_format, channel);
//   // sprintf(value_buf, "%4.6f", data->cumulative_kwatt_h);
//   // Serial.printf("Publish:[%s] value:[%s]\n", mqtt_topic_buffer, value_buf);
//   // mqttClient.publish(mqtt_topic_buffer, 2, true, value_buf);
// }

void loop() {
  time_pin = millis();

  switch(state_machine){
    case 0: getAndRecalculateChannel(channel+0, 0); adc0.requestADC(1); break;
    case 1: printChannel(channel+0, 0); break;
    case 2: shape_matching(channel+0, 0); break;

    case 3: getAndRecalculateChannel(channel+1, 1); adc0.requestADC(2); break;
    case 4: printChannel(channel+1, 1); break;

    case 5: getAndRecalculateChannel(channel+2, 2); adc0.requestADC(3); break;
    case 6: printChannel(channel+2, 2); break;

    case 7: getAndRecalculateChannel(channel+3, 3); adc0.requestADC(0); break;
    case 8: printChannel(channel+3, 3); break;

    case 9: state_machine = -1; break;
  }

  state_machine++;

  if(eeprom_cnt-- < 1){
    eeprom_cnt = EEPROM_WRITE_PERIOD;
    write_eeprom();

    return;
  }

  if((mqtt_publish_cnt-- < 1) && mqtt_connected){
    mqtt_publish_cnt = MQTT_PUBLISH_PERIOD;

    switch(publish_state_machine){
      case 0: mqtt_publish_power_all(WiFi.RSSI()); break;
      case 1: mqtt_publish_binary("lamp-1-shape", false); break;
      case 2: publish_state_machine=0; return;
    }
    publish_state_machine++;
  }


  // getAndRecalculateChannel(channel+channel_num, channel_num);
  // printChannel(channel+channel_num, channel_num);
  // if(++channel_num > 3){
  //   channel_num = 0;
  // }

  // Serial.printf("%d: Loop time:[%ul]\n", state_machine, millis() - time_pin);
}
