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
#include <AsyncMqtt_Generic.h>

  // 102 - 17watt
  // 4370 - 700watt 128 sps
  // 8540 - 700watt 250 sps

#define CALIBRATION_700W (float)4400
const float digital_ac_current_to_watt = CALIBRATION_700W / 700;

char wifi_ssid[] = "DALEKA";
char wifi_pass[] = "*********************************";
char mqtt_username[] = "cfg-mqtt";
char mqtt_password[] = "Zu1aeCheeshaelaiNuuG2Ooca1vooh6fiegeek4eeboong3uatoh0ahx9eud3phu";
IPAddress mqtt_host(192, 168, 3, 21);
const uint16_t mqtt_port = 1883;
const int timezone = 3;  // hours
const int dst = 0;
const char* device_serial = "500291D66C9M";

#define LED_RED1 15
#define LED_GREEN1 13
#define LED_GREEN2 12
#define LED_YELLOW1 14

// #define NUMBER_OF_POWER_CHANNELS 4
// #define NUMBER_OF_ENERGY_CHANNELS 4
#define IN_CHANNELS_SZ 4
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

t_channel_data channel[IN_CHANNELS_SZ];

class SensorValue {
  public:
    SensorValue(const char *_name, const char *_json_property, int _ch_num){
      name = _name;
      json_property = _json_property;
      ch_num = _ch_num;
    }

    const char *name;
    const char *json_property;
    int ch_num;

    virtual const char* getDeviceClass() = 0;

    virtual const char* getAdditionalDiscoveryProps() { return ",\"state_class\":\"measurement\""; }

    virtual const char* getUnit() = 0;

    virtual double getValue() = 0;

    virtual void mqttDiscoveryPublish(AsyncMqttClient* pmqttClient){
      char topic_buffer[] = "homeassistant/sensor/500291D66C79/500291D66C79/config";
      sprintf(topic_buffer, "homeassistant/%s/%s/%s/config", "sensor", device_serial, name);

      char state_topic[] = "device/power/500291D66C79";
      sprintf(state_topic, "device/%s/%s", getDeviceClass(), device_serial);

      char value_template[] = "{{value_json.500291D66C79|is_defined}}";
      sprintf(value_template, "{{value_json.%s|is_defined}}", json_property);

      mqtt_discovery_publish(pmqttClient, value_template, state_topic, topic_buffer);
    }

  protected:
    void mqtt_discovery_publish(AsyncMqttClient* pmqttClient, const char* value_template, const char* state_topic, const char* topic){
    //                                                                                                                              1                                                                                                   2                                                                                                   3                                                                                                   4
    //                                    1         2         3         4         5         6         7         8         9         0         1         2         3         4         5         6         7         8         9         0         1         2         3         4         5         6         7         8         9         0         1         2         3         4         5         6         7         8         9         0
    //                           123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    //  char message_buffer[] = "{"unit_of_measurement":"dbm","name":"rssi","device_class":"signal_strength","value_template":"{{ value_json.rssi | is_defined }}","state_topic":"device/power/500291D66C79","unique_id":"500291D66C79-500291D66C79","device":{"identifiers":["500291D66C79"],"manufacturer":"vitska000@gmail.com","model":"PM7803","name":"PM7803 PowerMeter"}}";
    //  char message_buffer[] = "{"unit_of_measurement":"%","name":"rssi-percent","device_class":"signal_strength","value_template":"{{ value_json.prssi | is_defined }}","state_topic":"device/power/500291D66C79","unique_id":"500291D66C79-rssi-percent","device":{"identifiers":["500291D66C79"],"manufacturer":"vitska000@gmail.com","model":"PM7803","name":"PM7803 PowerMeter"}}";
    //  char message_buffer[] = "{"unit_of_measurement":"W","name":"power-000","device_class":"power","value_template":"{{ value_json.p000 | is_defined }}","state_topic":"device/power/500291D66C79","unique_id":"500291D66C79-power-000","device":{"identifiers":["500291D66C79"],"manufacturer":"vitska000@gmail.com","model":"PM7803","name":"PM7803 PowerMeter"}}";

      const char* unit_of_measurement = getUnit();
      char unit_of_measurement_buffer[] = "\"unit_of_measurement\":\"00000\",";
      unit_of_measurement_buffer[0] = '\0';
      if(unit_of_measurement != NULL){
        sprintf(unit_of_measurement_buffer, "\"unit_of_measurement\":\"%s\",", unit_of_measurement);
      }

      char message_buffer[400];
      const char* generic_discovery_message_format = "{\"name\":\"%s\",%s\"device_class\":\"%s\",\"value_template\":\"%s\",\"state_topic\":\"%s\"%s,\"unique_id\":\"%s-%s\",\"device\":{\"identifiers\":[\"%s\"],\"manufacturer\":\"vitska000@gmail.com\",\"model\":\"PM7803\",\"name\":\"PM7803 PowerMeter\"}}";
      sprintf(
        message_buffer,
        generic_discovery_message_format,
        name,
        unit_of_measurement_buffer,
        getDeviceClass(),
        value_template,
        state_topic,
        getAdditionalDiscoveryProps(),
        device_serial, //unique_id
        name,
        device_serial //identifiers
        );

      Serial.printf("Publish:[%s] Message:[%s]\n", topic, message_buffer);
      digitalWrite(LED_RED1, HIGH);
      pmqttClient->publish(topic, 1, true, message_buffer);
    }

};

class PowerValue : public SensorValue {
  public:
    PowerValue(const char *_name, const char *_json_property, int _ch_num) : SensorValue(_name, _json_property, _ch_num){
    }

    const char* getDeviceClass() { return "power"; }

    const char* getUnit() { return "W"; }

    double getValue() { return channel[ch_num].watt; }
};

class PowerBinaryValue : public PowerValue {
  public:
    PowerBinaryValue(const char *_name, const char *_json_property) : PowerValue(_name, _json_property, -1){
    }

    const char* getUnit() { return NULL; }

    const char* getAdditionalDiscoveryProps() { return ",\"payload_on\":1,\"payload_off\":0"; }

    double getValue() { return 1; }

    void mqttDiscoveryPublish(AsyncMqttClient* pmqttClient){
      char topic_buffer[] = "homeassistant/binary_sensor/500291D66C79/500291D66C79/config";
      sprintf(topic_buffer, "homeassistant/%s/%s/%s/config", "binary_sensor", device_serial, name);

      char state_topic[] = "device/power/500291D66C79/500291D66C79";
      sprintf(state_topic, "device/power/%s/%s", device_serial, name);

      mqtt_discovery_publish(pmqttClient, "{{value_json.state}}", state_topic, topic_buffer);
    }
};

class EnergyValue : public SensorValue {
  public:
    EnergyValue(const char *_name, const char *_json_property, int _ch_num) : SensorValue(_name, _json_property, _ch_num){
    }

    virtual const char* getAdditionalDiscoveryProps() { return ",\"state_class\":\"total\""; }

    const char* getDeviceClass() { return "energy"; }

    const char* getUnit() { return "kWh"; }

    double getValue() { return channel[ch_num].cumulative_kwatt_h; }
};

class SumPowerValue : public PowerValue {
  public:
    SumPowerValue(const char *_name, const char *_json_property) : PowerValue(_name, _json_property, -1){
    }

    double getValue() { return channel[0].watt + channel[1].watt + channel[2].watt; }
};

class SumEnergyValue : public EnergyValue {
  public:
    SumEnergyValue(const char *_name, const char *_json_property) : EnergyValue(_name, _json_property, -1){
    }

    double getValue() { return channel[0].cumulative_kwatt_h + channel[1].cumulative_kwatt_h + channel[2].cumulative_kwatt_h; }
};

class RssiValue : public PowerValue {
  public:
    RssiValue(const char *_name, const char *_json_property) : PowerValue(_name, _json_property, -1){
    }

    const char* getAdditionalDiscoveryProps() { return ""; }

    const char* getDeviceClass() { return "signal_strength"; }

    const char* getUnit() { return "dbm"; }

    double getValue() { return WiFi.RSSI(); }

    void mqttDiscoveryPublish(AsyncMqttClient* pmqttClient){
      char topic_buffer[] = "homeassistant/sensor/500291D66C79/500291D66C79/config";
      sprintf(topic_buffer, "homeassistant/%s/%s/%s/config", "sensor", device_serial, "rssi");

      char state_topic[] = "device/power/500291D66C79";
      sprintf(state_topic, "device/power/%s", device_serial);

      mqtt_discovery_publish(pmqttClient, "{{value_json.rssi|is_defined}}", state_topic, topic_buffer);
    }
};

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

class RssiPercentValue : public RssiValue {
  public:
    RssiPercentValue(const char *_name, const char *_json_property) : RssiValue(_name, _json_property){
    }

    const char* getUnit() { return "%"; }

    double getValue() { return dBmtoPercentage(WiFi.RSSI()); }

    void mqttDiscoveryPublish(AsyncMqttClient* pmqttClient){
      char topic_buffer[] = "homeassistant/sensor/500291D66C79/500291D66C79/config";
      sprintf(topic_buffer, "homeassistant/%s/%s/%s/config", "sensor", device_serial, name);

      char state_topic[] = "device/power/500291D66C79";
      sprintf(state_topic, "device/power/%s", device_serial);

      mqtt_discovery_publish(pmqttClient, "{{value_json.prssi|is_defined}}", state_topic, topic_buffer);
    }
};

SensorValue* publish_values[] = { 
  new PowerValue("in-f1", "p0", 0), /* 0 */
  new PowerValue("in-f2", "p1", 1), /* 1 */
  new PowerValue("in-f3", "p2", 2), /* 2 */
  new PowerValue("power-3", "p3", 3), /* 3 */
  new SumPowerValue("in-total", "pintt"), /* 4 */
  //              500291D66C79
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

  digitalWrite(LED_GREEN1, HIGH);
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
  digitalWrite(LED_GREEN1, LOW);
}

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


  // getAndRecalculateChannel(channel+channel_num, channel_num);
  // printChannel(channel+channel_num, channel_num);
  // if(++channel_num > 3){
  //   channel_num = 0;
  // }

  // Serial.printf("%d: Loop time:[%ul]\n", state_machine, millis() - time_pin);
}
