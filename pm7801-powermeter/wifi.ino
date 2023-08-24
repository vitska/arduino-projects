#include "wifi_ap_scan.h"

wifi_ap_t* selected_wifi_ap = &wifi_ap[1];

int wifi_ap_contains(const char* ssid){
  int size = sizeof(wifi_ap) / sizeof(wifi_ap_t);
  for(int i = 0;i < size; i++){
    if(strcmp(ssid, wifi_ap[i].ssid) == 0){
      return i;
    }
  }

  return -1;
}

void selectWifiAp(int n){
  Serial.print(n);
  Serial.println(" WiFi networks found");
  int32_t max_rssi = INT32_MIN;
  int networkItem = -1;
  for (int i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    String ssid = WiFi.SSID(i);
    int32_t rssi = WiFi.RSSI(i);

    Serial.print(i + 1);
    Serial.print(") ");

    int wifi_idx;
    if((wifi_idx = wifi_ap_contains(ssid.c_str())) != -1){
      Serial.print("---> ");
      if(max_rssi < rssi){
        max_rssi = rssi;
        selected_wifi_ap = &wifi_ap[wifi_idx];
      }
    }

    Serial.print(ssid);// SSID
    Serial.print(" ");
    Serial.print(rssi);//Signal strength in dBm  
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

  Serial.print("AP:[");
  Serial.print(selected_wifi_ap->ssid);
  Serial.println("] selected");
}

void init_wifi_ap() // sets time
{
	WiFi.mode(WIFI_STA);
  int nw_cnt = WiFi.scanNetworks();
  if (nw_cnt == 0) {
    Serial.println("no WiFi networks found");
    return;
  } else {
    selectWifiAp(nw_cnt);
  }
  Serial.printf("Init time from WiFi [%s] ...\n", selected_wifi_ap->ssid);
  lcd.setCursor(0, 0);lcd.printf("WiFi:%s", selected_wifi_ap->ssid);
	WiFi.begin(selected_wifi_ap->ssid, selected_wifi_ap->pass);

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

