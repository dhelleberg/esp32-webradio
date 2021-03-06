#include <Arduino.h>
#include <HTTPClient.h>
#include <TimeLib.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <configuration.h>
#include <radio.h>
#include <NtpClientLib.h>

#define WIFI_TIMEOUT_CONFIGURATION_S 60

Configuration configuration;
ConfigurationData *configuration_data;
bool configuration_mode_enabled = false;

Radio radio;

const char *ntp_server = "europe.pool.ntp.org";
const long gmt_offset_sec = 3600;
const int daylight_offset_sec = 0;

int station_index = 0;
int32_t old_count = 0;

void setup() {
  // Setup Serial on 115200 Baud
  Serial.begin(115200);

  radio.init_encoder();
  radio.init_display();
  radio.display_connecting();

  // Load configuration and check validity
  configuration_data = configuration.get_configuration();
  if (!(configuration_data->eeprom_check[0] == 'O' && configuration_data->eeprom_check[1] == 'K') ||
      configuration_data->configuration_version != CONFIGURATION_VERSION) {
    configuration.init();
    configuration_mode_enabled = true;
    radio.display_configuration();
    return;
  }

  // Start Wifi connection
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(configuration_data->ssid, configuration_data->password);
  Serial.print("[WIFI] Connecting to WiFi ");
  int wifi_connection_time = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    wifi_connection_time++;
    if (wifi_connection_time > WIFI_TIMEOUT_CONFIGURATION_S) {
      Serial.println("[WIFI] Run into Wifi Timeout, configuration");
      Serial.flush();
      WiFi.disconnect();
      radio.display_configuration();
      configuration.init();
      configuration_mode_enabled = true;
      return;
    }
  }
  Serial.println("\n[WIFI] Connected");

  // Load remote NTP time
  NTP.begin(ntp_server, 1, true, 0);
  Serial.printf("[Timer] %s\n", NTP.getTimeDateString().c_str());

  // Player
  radio.init_player();
  radio.display_station();
}

void loop() {
  if (configuration_mode_enabled) {
    delay(100);
    return;
  }
  radio.loop();
}
