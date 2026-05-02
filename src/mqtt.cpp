#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "wifiCommunication.h"
#include "mqtt.h"
#include "engine.h"

// ---------------------------------------------------------------------------
// MQTT connection state
// ---------------------------------------------------------------------------

unsigned long reconnectInterval      = 5000;
unsigned long lastReconnectAttempt   = millis() - reconnectInterval - 1;

WiFiClientSecure net;
MQTTClient       client;
char             clientId[23];

// ---------------------------------------------------------------------------
// Routine state
// ---------------------------------------------------------------------------

static const int MAX_ROUTINE_STEPS = 10;

struct RoutineStep {
    char command[32];
    int  speed;
    int  duration;
};

static RoutineStep   routineSteps[MAX_ROUTINE_STEPS];
static int           routineLength  = 0;
static int           routineIndex   = -1;   // -1 = not running
static unsigned long routineStepAt  = 0;

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------

bool checkMQTTconnection();
void subscribeTopics();
void messageReceived(String &topic, String &payload);
static void executeCommand(const String& command, int speed, int duration);
static void routine_loop();
static void publishStatus();

// ---------------------------------------------------------------------------
// MQTT setup / loop / reconnect
// ---------------------------------------------------------------------------

void mqtt_setup() {
  snprintf(clientId, 23, "MCUDEVICE-%llX", ESP.getEfuseMac());

  net.setCACert(rootCABuff);
  net.setCertificate(certificateBuff);
  net.setPrivateKey(privateKeyBuff);

  client.begin(mqtt_server, mqtt_server_port, net);
  client.onMessage(messageReceived);

  while (!client.connect(clientId)) {
    Serial.print(".");
    delay(100);
  }

  Serial.printf("  Successfully connected to MQTT broker\r\n");
  subscribeTopics();
}

void subscribeTopics() {
  client.subscribe(mqttCommandTopic);
  Serial.printf("  Subscribed to %s\r\n", mqttCommandTopic);
}

void mqtt_loop() {
  if (!client.connected()) {
    unsigned long now = millis();
    if ((now - lastReconnectAttempt) > reconnectInterval) {
      lastReconnectAttempt = now;
      checkMQTTconnection();
    }
  }

  if (client.connected()) {
    client.loop();
  }

  routine_loop();
}

bool checkMQTTconnection() {
  if (wifiIsDisabled) return false;

  if (WiFi.isConnected()) {
    if (client.connected()) {
      return true;
    } else {
      if (client.connect(clientId)) {
        Serial.printf("  Successfully connected to MQTT broker\r\n");
        subscribeTopics();
      } else {
        Serial.printf("  MQTT connection failed (but WiFi is available). Will try later ...\r\n");
      }
      return client.connected();
    }
  } else {
    Serial.printf("  No connection to MQTT server, because WiFi is not connected.\r\n");
    return false;
  }
}

void publishMQTTMessage(const char *topic, const char *payload) {
  if (wifiIsDisabled) return;

  if (checkMQTTconnection()) {
    if (!client.publish(topic, payload)) {
      Serial.printf("  Publish failed\r\n");
    }
  } else {
    Serial.printf("  Cannot publish: WiFi or MQTT not connected\r\n");
  }
}

// ---------------------------------------------------------------------------
// Telemetry
// ---------------------------------------------------------------------------

static void publishStatus() {
  JsonDocument doc;
  doc["device"] = clientId;

  JsonObject lb = doc["leftBack"].to<JsonObject>();
  lb["speed"]   = wheelLeftBackSpeed();
  lb["forward"] = wheelLeftBackIsForward();

  JsonObject lf = doc["leftFront"].to<JsonObject>();
  lf["speed"]   = wheelLeftFrontSpeed();
  lf["forward"] = wheelLeftFrontIsForward();

  JsonObject rf = doc["rightFront"].to<JsonObject>();
  rf["speed"]   = wheelRightFrontSpeed();
  rf["forward"] = wheelRightFrontIsForward();

  JsonObject rb = doc["rightBack"].to<JsonObject>();
  rb["speed"]   = wheelRightBackSpeed();
  rb["forward"] = wheelRightBackIsForward();

  char buf[256];
  serializeJson(doc, buf, sizeof(buf));
  publishMQTTMessage(mqttStatusTopic, buf);
}

// ---------------------------------------------------------------------------
// Command interpreter
// executeCommand: pure dispatch, no routine cancellation — safe to call
//                 from both external commands and routine steps.
// handleCommand:  public entry point; cancels any running routine first.
// ---------------------------------------------------------------------------

static void executeCommand(const String& cmd, int speed, int duration) {
  if (cmd == "forward") {
    wheelLeftBackForward(speed);
    wheelLeftFrontForward(speed);
    wheelRightFrontForward(speed);
    wheelRightBackForward(speed);
  } else if (cmd == "backward") {
    wheelLeftBackBackward(speed);
    wheelLeftFrontBackward(speed);
    wheelRightFrontBackward(speed);
    wheelRightBackBackward(speed);
  } else if (cmd == "pivotLeft") {
    pivotLeft(speed);
  } else if (cmd == "pivotRight") {
    pivotRight(speed);
  } else if (cmd == "arcLeft") {
    arcLeft(speed / 2, speed);
  } else if (cmd == "arcRight") {
    arcRight(speed / 2, speed);
  } else if (cmd == "rampTo") {
    // speed = target; duration = step interval in ms
    rampTo(speed, duration > 0 ? duration : 10);
    return;
  } else if (cmd == "adjustSpeed") {
    adjustSpeed(speed);
    return;
  } else if (cmd == "reverseAll") {
    reverseAll();
    return;
  } else if (cmd == "status") {
    publishStatus();
    return;
  } else if (cmd == "stop") {
    stop();
    return;
  } else {
    Serial.printf("  Unknown command: %s\r\n", cmd.c_str());
    return;
  }

  if (duration > 0) scheduleStop(duration);
}

void handleCommand(const String& command, int speed, int duration) {
  routineIndex  = -1;
  routineLength = 0;
  executeCommand(command, speed, duration);
}

// ---------------------------------------------------------------------------
// Routine
// Each step runs for step.duration ms, then the next step is dispatched.
// Calls executeCommand (not handleCommand) to avoid cancelling the routine.
// ---------------------------------------------------------------------------

static void routine_loop() {
  if (routineIndex < 0) return;
  if (millis() < routineStepAt) return;

  if (routineIndex >= routineLength) {
    routineIndex = -1;
    Serial.printf("  Routine complete\r\n");
    return;
  }

  const RoutineStep& step = routineSteps[routineIndex];
  Serial.printf("  Routine step %d: %s  speed=%d  duration=%d\r\n",
                routineIndex, step.command, step.speed, step.duration);
  executeCommand(String(step.command), step.speed, 0);
  routineStepAt = millis() + (unsigned long)step.duration;
  routineIndex++;
}

// ---------------------------------------------------------------------------
// MQTT message handler
// ---------------------------------------------------------------------------

void messageReceived(String &topic, String &payload) {
  Serial.printf("  MQTT [%s]: %s\r\n", topic.c_str(), payload.c_str());

  if (topic != mqttCommandTopic) {
    Serial.printf("  Ignoring topic %s\r\n", topic.c_str());
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.printf("  JSON parse error: %s\r\n", error.c_str());
    return;
  }

  const char* device  = doc["device"]   | "";
  const char* command = doc["command"]  | "";
  int         speed   = doc["speed"]    | 0;
  int         duration = doc["duration"] | 0;

  if (strlen(device) > 0 && strcmp(device, clientId) != 0) {
    Serial.printf("  Ignoring command for device %s\r\n", device);
    return;
  }

  if (strlen(command) == 0) {
    Serial.printf("  Missing 'command' field\r\n");
    return;
  }

  // Routine is parsed here (not via handleCommand) because it needs the full doc
  if (strcmp(command, "routine") == 0) {
    JsonArray steps = doc["steps"].as<JsonArray>();
    routineLength = 0;
    for (JsonObject step : steps) {
      if (routineLength >= MAX_ROUTINE_STEPS) break;
      strlcpy(routineSteps[routineLength].command, step["command"] | "", 32);
      routineSteps[routineLength].speed    = step["speed"]    | 0;
      routineSteps[routineLength].duration = step["duration"] | 0;
      routineLength++;
    }
    routineIndex  = 0;
    routineStepAt = millis();
    Serial.printf("  Starting routine: %d steps\r\n", routineLength);
    return;
  }

  Serial.printf("  Executing: %s  speed=%d  duration=%d\r\n", command, speed, duration);
  handleCommand(String(command), speed, duration);
}
