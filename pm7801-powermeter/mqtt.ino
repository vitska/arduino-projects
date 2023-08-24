WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;
Ticker mqttReconnectTimer;
int publish_discovery_state_machine = 0;

void mqtt_start() {
  Serial.print("\nStarting FullyFeature_ESP8266 on ");
  Serial.println(ARDUINO_BOARD);
  Serial.println(ASYNC_MQTT_GENERIC_VERSION);
  Serial.println(__DATE__ " " __TIME__);

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

  digitalWrite(LED_GREEN1, HIGH);
  Serial.print("Connected to Wi-Fi. IP address: ");
  Serial.println(WiFi.localIP());
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event)
{
  (void) event;

  digitalWrite(LED_GREEN1, LOW);
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToWifi()
{
  Serial.printf("Connecting to Wi-Fi [%s] ...\n", selected_wifi_ap->ssid);
	WiFi.mode(WIFI_STA);
	WiFi.begin(selected_wifi_ap->ssid, selected_wifi_ap->pass);
}

void connectToMqtt()
{
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

const char* device_topic_format = "device/%s/%s";

uint32_t power_id = 0;
const char* all_json_begining_format = "{\"id\":\"p-%d\",\"rssi\":\"%d\",\"prssi\":\"%u\"";
const char* all_json_power_channel_format = ",\"%s\":\"%.1f\"";
const char* all_json_power_delta_format = ",\"pd%d\":\"%.1f\"";
const char* all_json_ending_format = "}";
void mqtt_publish_power_all(long rssi){

  char topic_buffer[] = "device/power/500291D66C79";
  sprintf(topic_buffer, device_topic_format, "power", device_serial);

  char message_buffer[] = "{\"rssi\":\"-000\",\"prssi\":\"000\",\"p0\":\"0000.0\",\"pd0\":\"-0000.0\",\"p1\":\"0000.0\",\"pd1\":\"-0000.0\",\"p2\":\"0000.0\",\"pd2\":\"-0000.0\",\"p3\":\"0000.0\",\"pd3\":\"-0000.0\",\"p4\":\"0000.0\",\"pd4\":\"-0000.0\",\"p5\":\"0000.0\",\"pd5\":\"-0000.0\",\"p6\":\"0000.0\",\"pd6\":\"-0000.0\",\"p7\":\"0000.0\",\"pd7\":\"-0000.0\",\"p8\":\"0000.0\",\"pd8\":\"-0000.0\",\"p9\":\"0000.0\",\"pd9\":\"-0000.0\",\"p10\":\"0000.0\",\"pd10\":\"-0000.0\",\"p11\":\"0000.0\",\"pd11\":\"-0000.0\",\"p12\":\"0000.0\",\"pd12\":\"-0000.0\",\"p13\":\"0000.0\",\"pd13\":\"-0000.0\",\"p14\":\"0000.0\",\"pd14\":\"-0000.0\",\"p15\":\"0000.0\",\"pd15\":\"-0000.0\"}";
  char* concat_message_ptr = message_buffer;

  // Assemble json
  int printed = sprintf(concat_message_ptr, all_json_begining_format, power_id++, rssi, dBmtoPercentage(rssi));  concat_message_ptr += printed > 0 ? printed : 0;

  for(int power_sensor_i=0;power_sensor_i<POWER_SENSOR_SZ;power_sensor_i++){
    printed = sprintf(concat_message_ptr, all_json_power_channel_format, power_sensor[power_sensor_i]->json_property, power_sensor[power_sensor_i]->getValue());  concat_message_ptr += printed > 0 ? printed : 0;
    // float diff = channel[i].watt - channel[i].watt_prev;
    // if(abs(diff) > 5){
    //   printed = sprintf(concat_message_ptr, all_json_power_delta_format, i, diff);  concat_message_ptr += printed > 0 ? printed : 0;
    // }
  }
  printed = sprintf(concat_message_ptr, all_json_ending_format);  concat_message_ptr += printed > 0 ? printed : 0;

  Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, message_buffer);
  digitalWrite(LED_RED1, HIGH);
  mqttClient.publish(topic_buffer, 2, true, message_buffer);
}

uint32_t energy_id = 0;
const char* all_json_energy_begining_format = "{\"id\":\"e-%d\"";
const char* all_json_energy_channel_format = ",\"%s\":\"%.6f\"";
const char* all_json_energy_ending_format = "}";
void mqtt_publish_energy_all(){

  char topic_buffer[] = "device/energy/500291D66C79";
  sprintf(topic_buffer, device_topic_format, "energy", device_serial);

  char message_buffer[] = "{\"id\":\"e-0000000000\",\"500291D66C79\":\"000.000000\",\"500291D66C79\":\"000.000000\",\"500291D66C79\":\"000.000000\",\"500291D66C79\":\"000.000000\"}";
  char* concat_message_ptr = message_buffer;

  // Assemble json
  int printed = sprintf(concat_message_ptr, all_json_energy_begining_format, energy_id++);  concat_message_ptr += printed > 0 ? printed : 0;

  for(int energy_sensor_i=0;energy_sensor_i<ENERGY_SENSOR_SZ;energy_sensor_i++){
    printed = sprintf(concat_message_ptr, all_json_energy_channel_format, energy_sensor[energy_sensor_i]->json_property, energy_sensor[energy_sensor_i]->getValue());  concat_message_ptr += printed > 0 ? printed : 0;
  }
  printed = sprintf(concat_message_ptr, all_json_energy_ending_format);  concat_message_ptr += printed > 0 ? printed : 0;

  Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, message_buffer);
  digitalWrite(LED_RED1, HIGH);
  mqttClient.publish(topic_buffer, 2, true, message_buffer);
}

const char* binary_json_begining_format = "{\"name\":\"%s\"";
const char* binary_json_power_channel_format = ",\"state\":%d";
const char* binary_json_ending_format = "}";
void mqtt_publish_binary(SensorValue* psensor){
  char topic_buffer[] = "device/power/500291D66C79/500291D66C79";
  sprintf(topic_buffer, device_topic_format, psensor->getDeviceClass(), device_serial, psensor->name);

  char message_buffer[] = "{\"name\":\"500291D66C79\",\"state\":\"0\"}";
  char* concat_message_ptr = message_buffer;

  // Assemble json
  int printed = sprintf(concat_message_ptr, binary_json_begining_format, psensor->name);  concat_message_ptr += printed > 0 ? printed : 0;
  printed = sprintf(concat_message_ptr, binary_json_power_channel_format, psensor->getValue() > 0 ? 1 : 0);  concat_message_ptr += printed > 0 ? printed : 0;
  printed = sprintf(concat_message_ptr, binary_json_ending_format);  concat_message_ptr += printed > 0 ? printed : 0;

  Serial.printf("Publish:[%s] Message:[%s]\n", topic_buffer, message_buffer);
  digitalWrite(LED_RED1, HIGH);
  mqttClient.publish(topic_buffer, 2, true, message_buffer);
}


void onMqttConnect(bool sessionPresent)
{
  digitalWrite(LED_GREEN2, HIGH);
  Serial.print("Connected to MQTT broker: ");
  Serial.print(mqtt_host);
  Serial.print(", port: ");
  Serial.println(mqtt_port);

  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  discovery_state_machine_start();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  (void) reason;

  digitalWrite(LED_GREEN2, LOW);

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
  digitalWrite(LED_RED1, LOW);
  // Serial.print("Publish acknowledged. packetId: ");
  // Serial.println(packetId);

  discovery_state_machine();
}

#define SM_POWER_CHANNEL_SECTION_BEGIN 1
#define SM_POWER_CHANNEL_SECTION_END (SM_POWER_CHANNEL_SECTION_BEGIN + 32)
#define SM_BINARY_CHANNEL_SECTION_BEGIN (SM_ENERGY_CHANNEL_SECTION_END + 1)
#define SM_BINARY_CHANNEL_SECTION_END (SM_BINARY_CHANNEL_SECTION_BEGIN + 1)

void discovery_state_machine_start(){
  publish_values[0]->mqttDiscoveryPublish(&mqttClient);
  publish_discovery_state_machine = 1;
}

void discovery_state_machine(){
  if(publish_discovery_state_machine == INT_MAX){
    return;
  }

  Serial.printf("publish_discovery_state_machine:[%d]\n", publish_discovery_state_machine);

  if(publish_discovery_state_machine < PUBLISH_VALUES_SZ){
    publish_values[publish_discovery_state_machine]->mqttDiscoveryPublish(&mqttClient);
  }
  else{
    //---------- Finish discovery publishing -------------
    publish_discovery_state_machine = INT_MAX;
    mqtt_connected = true;
    return;
  }

  publish_discovery_state_machine++;
}