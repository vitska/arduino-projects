const char* ha_manufacturer = "vitska000@gmail.com";
const char* ha_model = "PM7803";
const char* ha_name = "PM7803 PowerMeter V0.2";

wifi_ap_t wifi_ap[] = {
  { "DALEKA_Wi-Fi5", "********" }, // Highest priority
  { "DALEKA", "********" },
  { "DALEKA-2", "********" } // Lowest priority
};

char mqtt_username[] = "cfg-mqtt";
char mqtt_password[] = "Zu1aeCheeshaelaiNuuG2Ooca1vooh6fiegeek4eeboong3uatoh0ahx9eud3phu";
IPAddress mqtt_host(192, 168, 3, 21);
const uint16_t mqtt_port = 1883;
const int timezone = 3;  // hours
const int dst = 0;
const char* device_serial = "500291D66C9X";
