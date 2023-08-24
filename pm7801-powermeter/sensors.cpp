#include <ESP8266WiFi.h>
#include "config.h"
#include "sensors.hpp"

void SensorValue::mqttDiscoveryPublish(AsyncMqttClient* pmqttClient){
  char topic_buffer[] = "homeassistant/sensor/500291D66C79/500291D66C79/config";
  sprintf(topic_buffer, "homeassistant/%s/%s/%s/config", "sensor", device_serial, name);

  char state_topic[] = "device/power/500291D66C79";
  sprintf(state_topic, "device/%s/%s", getDeviceClass(), device_serial);

  char value_template[] = "{{value_json.500291D66C79|is_defined}}";
  sprintf(value_template, "{{value_json.%s|is_defined}}", json_property);

  mqtt_discovery_publish(pmqttClient, value_template, state_topic, topic_buffer);
}

void SensorValue::mqtt_discovery_publish(AsyncMqttClient* pmqttClient, const char* value_template, const char* state_topic, const char* topic){
  //                                                                                                                              1                                                                                                   2                                                                                                   3                                                                                                   4
  //                                    1         2         3         4         5         6         7         8         9         0         1         2         3         4         5         6         7         8         9         0         1         2         3         4         5         6         7         8         9         0         1         2         3         4         5         6         7         8         9         0
  //                           123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
  //  char message_buffer[] = "{"unit_of_measurement":"dbm","name":"rssi","device_class":"signal_strength","value_template":"{{ value_json.rssi | is_defined }}","state_topic":"device/power/500291D66C79","unique_id":"500291D66C79-500291D66C79","device":{"identifiers":["500291D66C79"],"manufacturer":"vitska000@gmail.com","model":"PM7803","name":"PM7803 PowerMeter"}}";
  //  char message_buffer[] = "{"unit_of_measurement":"%","name":"rssi-percent","device_class":"signal_strength","value_template":"{{ value_json.prssi | is_defined }}","state_topic":"device/power/500291D66C79","unique_id":"500291D66C79-rssi-percent","device":{"identifiers":["500291D66C79"],"manufacturer":"vitska000@gmail.com","model":"PM7803","name":"PM7803 PowerMeter"}}";
  //  char message_buffer[] = "{"unit_of_measurement":"W","name":"power-000","device_class":"power","value_template":"{{ value_json.p000 | is_defined }}","state_topic":"device/power/500291D66C79","unique_id":"500291D66C79-power-000","device":{"identifiers":["500291D66C79"],"manufacturer":"vitska000@gmail.com","model":"PM7803","name":"PM7803 PowerMeter"}}";

    const char* unit_of_measurement = getUnit();

    // char message_buffer[400];
    String message_buffer = "{\"name\":\"";
    message_buffer += name;
    message_buffer += "\",";

    if(unit_of_measurement != NULL){
      message_buffer += "\"unit_of_measurement\":\"";
      message_buffer += unit_of_measurement;
      message_buffer += "\",";
    }

    message_buffer += "\"device_class\":\"";
    message_buffer += getDeviceClass();
    message_buffer += "\",";

    message_buffer += "\"value_template\":\"";
    message_buffer += value_template;
    message_buffer += "\",";

    message_buffer += "\"state_topic\":\"";
    message_buffer += state_topic;
    message_buffer += "\",";

    message_buffer += getAdditionalDiscoveryProps();

    message_buffer += ",\"unique_id\":\"";
    message_buffer += device_serial;
    message_buffer += "-";
    message_buffer += name;
    message_buffer += "\",";

    message_buffer += "\"device\":{\"identifiers\":[\"";
    message_buffer += device_serial;
    message_buffer += "\"],";

    message_buffer += "\"manufacturer\":\"";
    message_buffer += ha_manufacturer;
    message_buffer += "\",\"model\":\"";
    message_buffer += ha_model;
    message_buffer += "\",\"name\":\"";
    message_buffer += ha_name;
    message_buffer += "\"}}";

    Serial.printf("Publish:[%s] Message:[%s]\n", topic, message_buffer.c_str());
    digitalWrite(LED_RED1, HIGH);
    pmqttClient->publish(topic, 1, true, message_buffer.c_str());
  }

void PowerBinaryValue::mqttDiscoveryPublish(AsyncMqttClient* pmqttClient){
  char topic_buffer[] = "homeassistant/binary_sensor/500291D66C79/500291D66C79/config";
  sprintf(topic_buffer, "homeassistant/%s/%s/%s/config", "binary_sensor", device_serial, name);

  char state_topic[] = "device/power/500291D66C79/500291D66C79";
  sprintf(state_topic, "device/power/%s/%s", device_serial, name);

  mqtt_discovery_publish(pmqttClient, "{{value_json.state}}", state_topic, topic_buffer);
}

double RssiValue::getValue() { return WiFi.RSSI(); }

void RssiValue::mqttDiscoveryPublish(AsyncMqttClient* pmqttClient){
  char topic_buffer[] = "homeassistant/sensor/500291D66C79/500291D66C79/config";
  sprintf(topic_buffer, "homeassistant/%s/%s/%s/config", "sensor", device_serial, "rssi");

  char state_topic[] = "device/power/500291D66C79";
  sprintf(state_topic, "device/power/%s", device_serial);

  mqtt_discovery_publish(pmqttClient, "{{value_json.rssi|is_defined}}", state_topic, topic_buffer);
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
}

double RssiPercentValue::getValue() { return dBmtoPercentage(WiFi.RSSI()); }

void RssiPercentValue::mqttDiscoveryPublish(AsyncMqttClient* pmqttClient){
  char topic_buffer[] = "homeassistant/sensor/500291D66C79/500291D66C79/config";
  sprintf(topic_buffer, "homeassistant/%s/%s/%s/config", "sensor", device_serial, name);

  char state_topic[] = "device/power/500291D66C79";
  sprintf(state_topic, "device/power/%s", device_serial);

  mqtt_discovery_publish(pmqttClient, "{{value_json.prssi|is_defined}}", state_topic, topic_buffer);
}
