/*
https://github.com/vitska/arduino-projects/tree/master/pm7801-powermeter
*/
#include <Esp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ADS1X15.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <Ticker.h>
#include "config.h"
#include "wifi_ap_scan.h"
#include "sensors.hpp"
#include <AsyncMqtt_Generic.h>

  // 102 - 17watt
  // 4370 - 700watt 128 sps
  // 8540 - 700watt 250 sps

#define CALIBRATION_700W (float)4400
const float digital_ac_current_to_watt = CALIBRATION_700W / 700;

// #define NUMBER_OF_POWER_CHANNELS 4
// #define NUMBER_OF_ENERGY_CHANNELS 4
#define EEPROM_WRITE_PERIOD 3000 // ~ 5 minutes
#define MQTT_PUBLISH_PERIOD 15 //

bool mqtt_connected = false;
ADS1115 adc0(0x48);
LiquidCrystal_I2C lcd(0x3F, 20, 4);
AsyncMqttClient mqttClient;

t_channel_data channel[IN_CHANNELS_SZ];

SensorValue* publish_values[] = { 
  new PowerValue("in-f1", "p0", 0), /* 0 */
  new PowerValue("in-f2", "p1", 1), /* 1 */
  new PowerValue("in-f3", "p2", 2), /* 2 */
  new PowerValue("power-3", "p3", 3), /* 3 */
  new SumPowerValue("in-total", "pintt"), /* 4 */
  new EnergyValue   ("energy-0", "f0e", 0), /* 5 */
  new EnergyValue   ("energy-1", "f1e", 1), /* 6 */
  new EnergyValue   ("energy-2", "f2e", 2), /* 7 */
  new EnergyValue   ("energy-3", "f3e", 3), /* 8 */
  new SumEnergyValue("energy-itt", "eintt"), /* 9 */
  new PowerBinaryValue("lamp-1-shape", "state"), /* 10 */
  new RssiValue("rssi", "rssi"), /* 11 */
  new RssiPercentValue("rssi-percent", "prssi") /* 12 */
};
#define PUBLISH_VALUES_SZ sizeof(publish_values) / sizeof(publish_values[0])

// backward compatibility
SensorValue* power_sensor[] = { publish_values[0], publish_values[1], publish_values[2], publish_values[3], publish_values[4] };
#define POWER_SENSOR_SZ sizeof(power_sensor) / sizeof(power_sensor[0])

SensorValue* energy_sensor[] = { publish_values[5], publish_values[6], publish_values[7], publish_values[8], publish_values[9] };
#define ENERGY_SENSOR_SZ sizeof(energy_sensor) / sizeof(energy_sensor[0])

#define EEPROM_INIT_MARKER 0x1237 // change marker to reset cumulative EEPROM

void init_from_eeprom(){
  int eepromAddress = 0;
  int marker = 0;
  EEPROM.get(eepromAddress, marker); eepromAddress += sizeof(marker);
  Serial.printf("EEPROM marker:%X\n", marker);
  if(marker == EEPROM_INIT_MARKER){
    for(int i=0;i<IN_CHANNELS_SZ;i++){
      EEPROM.get(eepromAddress, channel[i].cumulative_kwatt_h);
      Serial.printf("Reading channel %d [Address:%d] cumulative_kwatt_h:%f\n", i, eepromAddress, channel[i].cumulative_kwatt_h);
      eepromAddress += sizeof(channel[i].cumulative_kwatt_h);
    }
  }  
}

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_RED1, OUTPUT); digitalWrite(LED_RED1, LOW);
  pinMode(LED_GREEN1, OUTPUT); digitalWrite(LED_GREEN1, LOW);
  pinMode(LED_GREEN2, OUTPUT); digitalWrite(LED_GREEN2, LOW);
  pinMode(LED_YELLOW1, OUTPUT); digitalWrite(LED_YELLOW1, LOW);

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

  init_wifi_ap();
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
  lcd.setCursor(0, channel);lcd.print(data->pp);lcd.print("    ");
  lcd.setCursor(5, channel);lcd.printf("%3.1f", data->watt);lcd.print("    ");
  lcd.setCursor(11, channel);lcd.printf("%4.6f", data->cumulative_kwatt_h);lcd.print(" ");
}

void shape_matching(struct t_channel_data *data, int channel){
}

void write_eeprom(){
  int eepromAddress = 0;
  int marker = EEPROM_INIT_MARKER;
  EEPROM.put(eepromAddress, marker); eepromAddress += sizeof(marker);
  Serial.printf("Writting EEPROM marker:%X\n", marker);
  for(int i=0;i<IN_CHANNELS_SZ;i++){
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
unsigned long time_pin = 0;
int state_machine = 0;
int publish_state_machine = 0;

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
      case 1: mqtt_publish_binary(publish_values[10]); break;
      case 2: mqtt_publish_energy_all(); break;
      case 3: publish_state_machine=0; return;
    }
    publish_state_machine++;
  }
}
