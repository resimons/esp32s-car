#include <driver/gpio.h>
#include <esp32-hal-gpio.h>

// --- wifi ---------------------------------------------------------------------------------------------------------------------------------
const char* const wifi_ssid              = "my ssid";
const char* const wifi_password          = "my password";

// --- mqtt ---------------------------------------------------------------------------------------------------------------------------------
const char* const mqtt_server            = "server";
const int mqtt_server_port               = 12345;

const char* const mqttSensorTopic        = "mytopic";
const char* const mqqtTopicAlive         = "topic/iamalive";

// https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
#define myTimezone                 "Europe/Amsterdam"

const char* const rootCABuff = \
"-----BEGIN CERTIFICATE-----\n" \
"-----END CERTIFICATE-----\n";

// Fill with your certificate.pem.crt with LINE ENDING
const char* const certificateBuff = \
"-----BEGIN CERTIFICATE-----\n" \
"-----END CERTIFICATE-----\n";

// Fill with your private.pem.key with LINE ENDING
const char* const privateKeyBuff = \
"-----BEGIN PRIVATE KEY-----\n" \
"-----END PRIVATE KEY-----\n";

