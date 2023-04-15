WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;
Ticker mqttReconnectTimer;
int publish_discovery_state_machine = 0;

void mqtt_start() {
  Serial.print("\nStarting FullyFeature_ESP8266 on ");
  Serial.println(ARDUINO_BOARD);
  Serial.println(ASYNC_MQTT_GENERIC_VERSION);

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(mqtt_host, mqtt_port);
  mqttClient.setCredentials(mqtt_username, mqtt_password);

  connectToWifi();
}

void onWifiConnect(const WiFiEventStationModeGotIP& event)
{
  (void) event;

  Serial.print("Connected to Wi-Fi. IP address: ");
  Serial.println(WiFi.localIP());
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event)
{
  (void) event;

  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToWifi()
{
  Serial.printf("Connecting to Wi-Fi [%s] ...\n", wifi_ssid);
	WiFi.mode(WIFI_STA);
	WiFi.begin(wifi_ssid, wifi_pass);
}

void connectToMqtt()
{
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void printSeparationLine()
{
  Serial.println("************************************************");
}

const char* device_entity_topic_format = "device/power/%s/%s";
const char* device_common_topic_format = "device/power/%s";
//const char* rssi_message_format = "{\"rssi\":\"%d\",\"prssi\":\"%u\"}";
const char* discovery_sensor_topic_format = "homeassistant/sensor/%s/%s/config";
const char* discovery_binary_sensor_topic_format = "homeassistant/binary_sensor/%s/%s/config";
const char* subscribe_topic_format = "device/power/%s/%s";

const char* discovery_power_channel_message_format = "{\"unit_of_measurement\":\"W\",\"name\":\"power-%d\",\"device_class\":\"power\",\"value_template\":\"{{ value_json.p%d | is_defined }}\",\"state_topic\":\"device/power/%s\",\"unique_id\":\"%s-power-%d\",\"device\":{\"identifiers\":[\"%s\"],\"manufacturer\":\"vitska000@gmail.com\",\"model\":\"PM7803\",\"name\":\"PM7803 PowerMeter\"}}";

void mqtt_publish_power_channel_discovery(int channel_number){
  char message_buffer[] = "{\"unit_of_measurement\":\"W\",\"name\":\"power-000\",\"device_class\":\"power\",\"value_template\":\"{{ value_json.p000 | is_defined }}\",\"state_topic\":\"device/power/500291D66C79\",\"unique_id\":\"500291D66C79-power-000\",\"device\":{\"identifiers\":[\"500291D66C79\"],\"manufacturer\":\"vitska000@gmail.com\",\"model\":\"PM7803\",\"name\":\"PM7803 PowerMeter\"}}";
  sprintf(
    message_buffer,
    discovery_power_channel_message_format,
    channel_number, //name
    channel_number, //value_template
    device_serial, //state_topic
    device_serial,
    channel_number, //unique_id
    device_serial //identifiers
    );

  char entity_name_buffer[] = "power-000";
  sprintf(entity_name_buffer, "power-%d", channel_number);
  char topic_buffer[] = "homeassistant/sensor/500291D66C79/500291D66C79/config";
  sprintf(topic_buffer, discovery_sensor_topic_format, device_serial, entity_name_buffer);
  
  Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, message_buffer);
  mqttClient.publish(topic_buffer, 1, true, message_buffer);
}


const char* discovery_binary_channel_message_format = "{\"name\":\"%s\",\"device_class\":\"power\",\"value_template\":\"{{ value_json.state }}\",\"state_topic\":\"device/power/%s/%s\",\"payload_on\":1,\"payload_off\":0,\"unique_id\":\"%s-power-binary-%s\",\"device\":{\"identifiers\":[\"%s\"],\"manufacturer\":\"vitska000@gmail.com\",\"model\":\"PM7803\",\"name\":\"PM7803 PowerMeter\"}}";

void mqtt_publish_binary_sensor_discovery(const char* entity_name){
  char message_buffer[] = "{\"name\":\"500291D66C79\",\"device_class\":\"power\",\"value_template\":\"{{ value_json.state }}\",\"state_topic\":\"device/power/500291D66C79/500291D66C79\",\"payload_on\":1,\"payload_off\":0,\"unique_id\":\"500291D66C79-power-binary-500291D66C79\",\"device\":{\"identifiers\":[\"500291D66C79\"],\"manufacturer\":\"vitska000@gmail.com\",\"model\":\"PM7803\",\"name\":\"PM7803 PowerMeter\"}}";
  sprintf(
    message_buffer,
    discovery_binary_channel_message_format,
    entity_name, //name
    device_serial, //state_topic
    entity_name,
    device_serial, //unique_id
    entity_name,
    device_serial //identifiers
    );

  char topic_buffer[] = "homeassistant/binary_sensor/500291D66C79/500291D66C79/config";
  sprintf(topic_buffer, discovery_binary_sensor_topic_format, device_serial, entity_name);
  
  Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, message_buffer);
  mqttClient.publish(topic_buffer, 1, true, message_buffer);
}

const char* discovery_rssi_message_format = "{\"unit_of_measurement\":\"dbm\",\"name\":\"rssi\",\"device_class\":\"signal_strength\",\"value_template\":\"{{ value_json.rssi | is_defined }}\",\"state_topic\":\"device/power/%s\",\"unique_id\":\"%s-%s\",\"device\":{\"identifiers\":[\"%s\"],\"manufacturer\":\"vitska000@gmail.com\",\"model\":\"PM7803\",\"name\":\"PM7803 PowerMeter\"}}";
void mqtt_publish_rssi_discovery(){
  const char* entity_name = "rssi";

  char message_buffer[] = "{\"unit_of_measurement\":\"dbm\",\"name\":\"rssi\",\"device_class\":\"signal_strength\",\"value_template\":\"{{ value_json.rssi | is_defined }}\",\"state_topic\":\"device/power/500291D66C79\",\"unique_id\":\"500291D66C79-500291D66C79\",\"device\":{\"identifiers\":[\"500291D66C79\"],\"manufacturer\":\"vitska000@gmail.com\",\"model\":\"PM7803\",\"name\":\"PM7803 PowerMeter\"}}";
  sprintf(
    message_buffer,
    discovery_rssi_message_format,
    device_serial,
    device_serial,
    entity_name,
    device_serial);

  char topic_buffer[] = "homeassistant/sensor/500291D66C79/500291D66C79/config";
  sprintf(topic_buffer, discovery_sensor_topic_format, device_serial, entity_name);

  Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, message_buffer);
  mqttClient.publish(topic_buffer, 1, true, message_buffer);
}

const char* discovery_rssi_percent_topic_format = "homeassistant/sensor/%s/rssi-percent/config";
const char* discovery_rssi_percent_message_format = "{\"unit_of_measurement\":\"%%\",\"name\":\"rssi-percent\",\"device_class\":\"signal_strength\",\"value_template\":\"{{ value_json.prssi | is_defined }}\",\"state_topic\":\"device/power/%s\",\"unique_id\":\"%s-rssi-percent\",\"device\":{\"identifiers\":[\"%s\"],\"manufacturer\":\"vitska000@gmail.com\",\"model\":\"PM7803\",\"name\":\"PM7803 PowerMeter\"}}";
void mqtt_publish_rssi_percent_discovery(){
  const char* entity_name = "rssi-percent";

  char message_buffer[] = "{\"unit_of_measurement\":\"%\",\"name\":\"rssi-percent\",\"device_class\":\"signal_strength\",\"value_template\":\"{{ value_json.prssi | is_defined }}\",\"state_topic\":\"device/power/500291D66C79\",\"unique_id\":\"500291D66C79-rssi-percent\",\"device\":{\"identifiers\":[\"500291D66C79\"],\"manufacturer\":\"vitska000@gmail.com\",\"model\":\"PM7803\",\"name\":\"PM7803 PowerMeter\"}}";
  sprintf(
    message_buffer,
    discovery_rssi_percent_message_format,
    device_serial,
    device_serial,
    device_serial);

  char topic_buffer[] = "homeassistant/sensor/500291D66C79/500291D66C79/config";
  sprintf(topic_buffer, discovery_rssi_percent_topic_format, device_serial);

  Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, message_buffer);
  mqttClient.publish(topic_buffer, 1, true, message_buffer);
}

// void mqtt_publish_state(int channel_number, const char* units, const char* value){

//   char entity_name_buffer[] = "000";
//   sprintf(entity_name_buffer, "%d", channel_number);

//   char topic_buffer[] = "device/power/500291D66C79/500291D66C79";
//   sprintf(topic_buffer, state_topic_format, device_serial, entity_name_buffer);

//   char message_buffer[] = "{\"p0\":\"0000.000000\",\"p1\":\"0000.000000\",\"p2\":\"0000.000000\",\"p3\":\"0000.000000\",\"p4\":\"0000.000000\",\"p5\":\"0000.000000\",\"p6\":\"0000.000000\",\"p7\":\"0000.000000\",\"p8\":\"0000.000000\",\"p9\":\"0000.000000\",\"p10\":\"0000.000000\",\"p11\":\"0000.000000\",\"p12\":\"0000.000000\",\"p13\":\"0000.000000\",\"p14\":\"0000.000000\",\"p15\":\"0000.000000\"}";

//   Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, value);
//   mqttClient.publish(topic_buffer, 2, true, value);
// }

// void mqtt_publish_rssi(long value){

//   char topic_buffer[] = "device/power/500291D66C79";
//   sprintf(topic_buffer, rssi_topic_format, device_serial);

//   char message_buffer[] = "{\"rssi\":\"0000.000000\",\"prssi\":\"0000.000000\"}";
//   sprintf(
//     message_buffer,
//     rssi_message_format,
//     value,
//     dBmtoPercentage(value));

//   Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, message_buffer);
//   mqttClient.publish(topic_buffer, 2, true, message_buffer);
// }

const char* all_json_begining_format = "{\"rssi\":\"%d\",\"prssi\":\"%u\"";
const char* all_json_power_channel_format = ",\"p%d\":\"%.1f\"";
const char* all_json_power_delta_format = ",\"pd%d\":\"%.1f\"";
const char* all_json_ending_format = ",\"id\":\"%s\"}";
void mqtt_publish_power_all(long rssi){

  char topic_buffer[] = "device/power/500291D66C79";
  sprintf(topic_buffer, device_common_topic_format, device_serial);

  char message_buffer[] = "{\"rssi\":\"-000\",\"prssi\":\"000\",\"p0\":\"0000.0\",\"pd0\":\"-0000.0\",\"p1\":\"0000.0\",\"pd1\":\"-0000.0\",\"p2\":\"0000.0\",\"pd2\":\"-0000.0\",\"p3\":\"0000.0\",\"pd3\":\"-0000.0\",\"p4\":\"0000.0\",\"pd4\":\"-0000.0\",\"p5\":\"0000.0\",\"pd5\":\"-0000.0\",\"p6\":\"0000.0\",\"pd6\":\"-0000.0\",\"p7\":\"0000.0\",\"pd7\":\"-0000.0\",\"p8\":\"0000.0\",\"pd8\":\"-0000.0\",\"p9\":\"0000.0\",\"pd9\":\"-0000.0\",\"p10\":\"0000.0\",\"pd10\":\"-0000.0\",\"p11\":\"0000.0\",\"pd11\":\"-0000.0\",\"p12\":\"0000.0\",\"pd12\":\"-0000.0\",\"p13\":\"0000.0\",\"pd13\":\"-0000.0\",\"p14\":\"0000.0\",\"pd14\":\"-0000.0\",\"p15\":\"0000.0\",\"pd15\":\"-0000.0\"}";
  char* concat_message_ptr = message_buffer;

  // Assemble json
  int printed = sprintf(concat_message_ptr, all_json_begining_format, rssi, dBmtoPercentage(rssi));  concat_message_ptr += printed > 0 ? printed : 0;

  for(int i=0;i<NUMBER_OF_CHANNELS;i++){
    printed = sprintf(concat_message_ptr, all_json_power_channel_format, i, channel[i].watt);  concat_message_ptr += printed > 0 ? printed : 0;
    float diff = channel[i].watt - channel[i].watt_prev;
    if(abs(diff) > 5){
      printed = sprintf(concat_message_ptr, all_json_power_delta_format, i, diff);  concat_message_ptr += printed > 0 ? printed : 0;
    }
  }
  printed = sprintf(concat_message_ptr, all_json_ending_format, device_serial);  concat_message_ptr += printed > 0 ? printed : 0;

  Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, message_buffer);
  mqttClient.publish(topic_buffer, 2, true, message_buffer);
}

const char* binary_json_begining_format = "{\"name\":\"%s\"";
const char* binary_json_power_channel_format = ",\"state\":%d";
const char* binary_json_ending_format = "}";
void mqtt_publish_binary(const char* name, bool state){
  char topic_buffer[] = "device/power/500291D66C79/500291D66C79";
  sprintf(topic_buffer, device_entity_topic_format, device_serial, name);

  char message_buffer[] = "{\"name\":\"500291D66C79\",\"state\":\"0\"}";
  char* concat_message_ptr = message_buffer;

  // Assemble json
  int printed = sprintf(concat_message_ptr, binary_json_begining_format, name);  concat_message_ptr += printed > 0 ? printed : 0;
  printed = sprintf(concat_message_ptr, binary_json_power_channel_format, state ? 1 : 0);  concat_message_ptr += printed > 0 ? printed : 0;
  printed = sprintf(concat_message_ptr, binary_json_ending_format);  concat_message_ptr += printed > 0 ? printed : 0;

  Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, message_buffer);
  mqttClient.publish(topic_buffer, 2, true, message_buffer);
}


void onMqttConnect(bool sessionPresent)
{
  Serial.print("Connected to MQTT broker: ");
  Serial.print(mqtt_host);
  Serial.print(", port: ");
  Serial.println(mqtt_port);
  // Serial.print("Topic: ");
  // Serial.println(mqtt_topic);

  printSeparationLine();
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  // char subscribe_topic[] = "device/power/500291D66C79";
  // sprintf(subscribe_topic, subscribe_topic_format, device_serial);
  // uint16_t packetIdSub = mqttClient.subscribe(subscribe_topic, 2);
  // Serial.print("Subscribing at QoS 2, packetId: ");
  // Serial.println(packetIdSub);

  // mqttClient.publish(mqtt_topic, 0, true, "ESP8266 Test1");
  // Serial.println("Publishing at QoS 0");

  // uint16_t packetIdPub1 = mqttClient.publish(mqtt_topic, 1, true, "ESP8266 Test2");
  // Serial.print("Publishing at QoS 1, packetId: ");
  // Serial.println(packetIdPub1);

  // uint16_t packetIdPub2 = mqttClient.publish(mqtt_topic, 2, true, "ESP8266 Test3");
  // Serial.print("Publishing at QoS 2, packetId: ");
  // Serial.println(packetIdPub2);

  // printSeparationLine();

  mqtt_publish_rssi_discovery();
//  mqtt_publish_power_channel_discovery(discovery_channel);// start sequence
//  mqtt_publish_power_channel_discovery(2);
//  mqtt_publish_power_channel_discovery(3);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  (void) reason;

  Serial.println("Disconnected from MQTT.");

  mqtt_connected = false;

  if (WiFi.isConnected())
  {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(const uint16_t& packetId, const uint8_t& qos)
{
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(const uint16_t& packetId)
{
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties,
                   const size_t& len, const size_t& index, const size_t& total)
{
  (void) payload;

  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

void onMqttPublish(const uint16_t& packetId)
{
  Serial.print("Publish acknowledged. packetId: ");
  Serial.println(packetId);

  if(publish_discovery_state_machine == INT_MAX){
    return;
  }

//    Serial.printf("publish_discovery_state_machine:%d", publish_discovery_state_machine);

  if(publish_discovery_state_machine == 0){
    mqtt_publish_rssi_percent_discovery();
  }
  else if((publish_discovery_state_machine > 0) && (publish_discovery_state_machine < 5)){
    mqtt_publish_power_channel_discovery(publish_discovery_state_machine-1);// 1..4
  }
  else if((publish_discovery_state_machine >= 5) && (publish_discovery_state_machine < 6)){
    mqtt_publish_binary_sensor_discovery("lamp-1-shape"); //5
  }
  else{
    //---------- Finish discovery publishing -------------
    publish_discovery_state_machine = INT_MAX;
    mqtt_connected = true;
    return;
  }

  publish_discovery_state_machine++;
}
