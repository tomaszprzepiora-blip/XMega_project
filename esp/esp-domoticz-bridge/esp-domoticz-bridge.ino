#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#else
#include <WiFi.h>
#include <HTTPClient.h>
#endif

/*
 * Bridge UART <-> Domoticz HTTP JSON API.
 *
 * XMEGA wysyla linie, np.:
 *   BTN,1,TOGGLE
 *   BTN,6,OFF_ALL
 *   GET,TEMP,1
 *   GET,ALL
 *
 * ESP odpisuje liniami, np.:
 *   STATE,1,ON
 *   TEMP,1,21.6
 *   OK
 *   ERR,HTTP
 */

#if __has_include("domoticz_config.h")
#include "domoticz_config.h"
#else
static const char WIFI_SSID[] = "TWOJE_WIFI";
static const char WIFI_PASS[] = "TWOJE_HASLO";

static const char DOMOTICZ_HOST[] = "192.168.1.100";
static const uint16_t DOMOTICZ_PORT = 8080;

static const uint16_t LIGHT_IDX[5] = {
  101, /* 1 Salon */
  102, /* 2 Kuchnia */
  103, /* 3 Korytarz */
  104, /* 4 Biurko */
  105  /* 5 Noc */
};

static const uint16_t TEMP_IDX[3] = {
  201, /* 1 Salon */
  202, /* 2 Kuchnia */
  203  /* 3 Zewn. */
};
#endif

static String rxLine;

static String domoticzUrl(const String &path)
{
  return String("http://") + DOMOTICZ_HOST + ":" + DOMOTICZ_PORT + path;
}

static bool httpGet(const String &path, String *body)
{
  WiFiClient client;
  HTTPClient http;
  String url = domoticzUrl(path);

  if (!http.begin(client, url)) {
    return false;
  }

  int code = http.GET();
  if (code < 200 || code >= 300) {
    http.end();
    return false;
  }

  if (body != nullptr) {
    *body = http.getString();
  }

  http.end();
  return true;
}

static bool switchLight(uint8_t buttonId, const char *command)
{
  if (buttonId < 1 || buttonId > 5) {
    return false;
  }

  uint16_t idx = LIGHT_IDX[buttonId - 1];
  String path = String("/json.htm?type=command&param=switchlight&idx=") +
                idx + "&switchcmd=" + command;

  return httpGet(path, nullptr);
}

static bool sendTemperature(uint8_t tempId)
{
  if (tempId < 1 || tempId > 3) {
    return false;
  }

  String body;
  uint16_t idx = TEMP_IDX[tempId - 1];
  String path = String("/json.htm?type=command&param=getdevices&rid=") + idx;

  if (!httpGet(path, &body)) {
    return false;
  }

  int key = body.indexOf("\"Temp\"");
  if (key < 0) {
    key = body.indexOf("\"Data\"");
  }
  if (key < 0) {
    return false;
  }

  int colon = body.indexOf(':', key);
  if (colon < 0) {
    return false;
  }

  int start = colon + 1;
  while (start < (int)body.length() && (body[start] == ' ' || body[start] == '\"')) {
    start++;
  }

  int end = start;
  while (end < (int)body.length() &&
         (isDigit(body[end]) || body[end] == '-' || body[end] == '.')) {
    end++;
  }

  if (end <= start) {
    return false;
  }

  Serial.print("TEMP,");
  Serial.print(tempId);
  Serial.print(",");
  Serial.println(body.substring(start, end));
  return true;
}

static void sendAllTemperatures()
{
  for (uint8_t i = 1; i <= 3; i++) {
    if (!sendTemperature(i)) {
      Serial.print("ERR,TEMP,");
      Serial.println(i);
    }
    delay(60);
  }
}

static void handleButtonCommand(uint8_t id, const String &action)
{
  bool ok = false;

  if (id >= 1 && id <= 5) {
    ok = switchLight(id, action == "TOGGLE" ? "Toggle" : action.c_str());
  } else if (id == 6 && action == "OFF_ALL") {
    ok = true;
    for (uint8_t i = 1; i <= 5; i++) {
      if (!switchLight(i, "Off")) {
        ok = false;
      }
      delay(80);
    }
  }

  Serial.println(ok ? "OK" : "ERR,SWITCH");
}

static void handleLine(String line)
{
  line.trim();
  if (line.length() == 0) {
    return;
  }

  if (line.startsWith("BTN,")) {
    int first = line.indexOf(',');
    int second = line.indexOf(',', first + 1);
    if (second < 0) {
      Serial.println("ERR,BTN");
      return;
    }

    uint8_t id = (uint8_t)line.substring(first + 1, second).toInt();
    String action = line.substring(second + 1);
    handleButtonCommand(id, action);
    return;
  }

  if (line.startsWith("GET,TEMP,")) {
    uint8_t id = (uint8_t)line.substring(9).toInt();
    Serial.println(sendTemperature(id) ? "OK" : "ERR,TEMP");
    return;
  }

  if (line == "GET,ALL") {
    sendAllTemperatures();
    Serial.println("OK");
    return;
  }

  Serial.println("ERR,UNKNOWN");
}

void setup()
{
  Serial.begin(115200);
  rxLine.reserve(80);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }

  Serial.println("OK,WIFI");
}

void loop()
{
  while (Serial.available() > 0) {
    char ch = (char)Serial.read();
    if (ch == '\n') {
      handleLine(rxLine);
      rxLine = "";
    } else if (ch != '\r' && rxLine.length() < 79) {
      rxLine += ch;
    }
  }
}
