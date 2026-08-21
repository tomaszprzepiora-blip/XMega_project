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
 * XMEGA nie zna nic poza idx urzadzenia Domoticz - caly mapping
 * przycisk/ekran -> idx zyje w firmware XMEGA (firmware/protocol/domoticz_map.h).
 * ESP jest czystym tlumaczem "idx + wartosc" <-> Domoticz HTTP API.
 *
 * XMEGA wysyla linie, np.:
 *   idx:32:1     ustaw przelacznik idx=32 na On
 *   idx:32:0     ustaw przelacznik idx=32 na Off
 *   idx:40:?     zapytaj o biezaca wartosc idx=40 (stan albo temperatura)
 *
 * ESP odpisuje liniami, np.:
 *   RCV,32       natychmiastowe potwierdzenie odbioru linii (przed HTTP)
 *   idx:32:1     potwierdzenie/odczyt stanu przelacznika (0/1) - wykonano
 *   idx:40:216   odczyt temperatury w deci-stopniach (216 = 21.6 C) - wykonano
 *   ERR,IDX,40   blad HTTP/Domoticz dla danego idx - wykonano z bledem
 *   ERR,WIFI     brak polaczenia WiFi
 */

#if __has_include("domoticz_config.h")
#include "domoticz_config.h"
#else
static const char WIFI_SSID[] = "TWOJE_WIFI";
static const char WIFI_PASS[] = "TWOJE_HASLO";

static const char DOMOTICZ_HOST[] = "192.168.1.100";
static const uint16_t DOMOTICZ_PORT = 8080;
#endif

static String rxLine;

static bool wifiReady()
{
  return WiFi.status() == WL_CONNECTED;
}

static void updateWifiLed()
{
  /* Built-in LED (GPIO2 on D1 mini) is active-low: LOW turns it on. */
  digitalWrite(LED_BUILTIN, wifiReady() ? LOW : HIGH);
}

static String domoticzUrl(const String &path)
{
  return String("http://") + DOMOTICZ_HOST + ":" + DOMOTICZ_PORT + path;
}

static bool httpGetOnce(const String &url, String *body)
{
  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, url)) {
    return false;
  }

  http.setTimeout(8000);
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

static bool httpGet(const String &path, String *body)
{
  String url;

  if (!wifiReady()) {
    return false;
  }

  url = domoticzUrl(path);

  /*
   * Observed on hardware: Domoticz sometimes applies the change correctly
   * (verified independently) even though ESP8266HTTPClient reports a
   * failure here - a transient TCP/connection hiccup on the ESP8266 side,
   * not a real request failure. A couple of quick retries clears it up
   * without XMEGA ever seeing the false error.
   */
  for (uint8_t attempt = 0; attempt < 3; attempt++) {
    if (httpGetOnce(url, body)) {
      return true;
    }
    if (attempt < 2) {
      delay(200);
    }
  }

  return false;
}

static bool switchLight(uint16_t idx, const char *command)
{
  String path = String("/json.htm?type=command&param=switchlight&idx=") +
                idx + "&switchcmd=" + command;

  return httpGet(path, nullptr);
}

static bool queryIdx(uint16_t idx)
{
  String body;
  String path = String("/json.htm?type=command&param=getdevices&rid=") + idx;

  if (!httpGet(path, &body)) {
    return false;
  }

  int statusKey = body.indexOf("\"Status\"");
  if (statusKey >= 0) {
    int colon = body.indexOf(':', statusKey);
    int quote1 = colon < 0 ? -1 : body.indexOf('"', colon + 1);
    int quote2 = quote1 < 0 ? -1 : body.indexOf('"', quote1 + 1);
    if (colon < 0 || quote1 < 0 || quote2 < 0) {
      return false;
    }

    String status = body.substring(quote1 + 1, quote2);
    Serial.print("idx:");
    Serial.print(idx);
    Serial.print(":");
    Serial.println(status == "On" ? 1 : 0);
    return true;
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

  float temp = body.substring(start, end).toFloat();
  int deciTemp = (int)(temp * 10.0f + (temp >= 0 ? 0.5f : -0.5f));

  Serial.print("idx:");
  Serial.print(idx);
  Serial.print(":");
  Serial.println(deciTemp);
  return true;
}

static void handleLine(String line)
{
  line.trim();
  if (line.length() == 0) {
    return;
  }

  if (line == "PING") {
    Serial.println(wifiReady() ? "OK,PONG,WIFI" : "OK,PONG,NOWIFI");
    return;
  }

  if (!wifiReady()) {
    Serial.println("ERR,WIFI");
    return;
  }

  if (line.startsWith("idx:")) {
    int firstColon = line.indexOf(':', 4);
    if (firstColon < 0) {
      Serial.println("ERR,UNKNOWN");
      return;
    }

    uint16_t idx = (uint16_t)line.substring(4, firstColon).toInt();
    String token = line.substring(firstColon + 1);

    if (token == "?") {
      /*
       * Ack receipt immediately, before the (possibly slow, up to 8s) HTTP
       * call - lets XMEGA tell "ESP got the line" apart from "Domoticz is
       * slow to answer" and retry the send if this never arrives.
       */
      Serial.print("RCV,");
      Serial.println(idx);
      if (!queryIdx(idx)) {
        Serial.print("ERR,IDX,");
        Serial.println(idx);
      }
      /*
       * XMEGA can send several idx lines back to back (All Off, Sync) with
       * no gap on the UART side. Without a small pause here, back-to-back
       * HTTP requests occasionally fail because the ESP8266 has not yet
       * released the previous connection - the old per-idx loops on this
       * side used the same defensive delay (80-250 ms) for that reason.
       */
      delay(150);
      return;
    }

    if (token == "0" || token == "1") {
      Serial.print("RCV,");
      Serial.println(idx);
      if (switchLight(idx, token == "1" ? "On" : "Off")) {
        Serial.print("idx:");
        Serial.print(idx);
        Serial.print(":");
        Serial.println(token);
      } else {
        Serial.print("ERR,IDX,");
        Serial.println(idx);
      }
      delay(150);
      return;
    }
  }

  Serial.println("ERR,UNKNOWN");
}

void setup()
{
  Serial.begin(115200);
  rxLine.reserve(80);
  delay(100);
  Serial.println("BOOT");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); /* off until WiFi connects */

  WiFi.mode(WIFI_STA);
#if defined(ESP8266)
  /*
   * Modem sleep (default) delays/drops packets between requests, which
   * shows up as the first HTTP call after boot succeeding and every
   * following call intermittently timing out (ERR,IDX) even though the
   * same request works instantly from another host on the same network.
   */
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
#else
  WiFi.setSleep(false);
#endif
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t started = millis();
  while (!wifiReady() && millis() - started < 20000UL) {
    delay(250);
  }

  updateWifiLed();

  if (wifiReady()) {
    Serial.print("OK,WIFI,");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("ERR,WIFI");
  }
}

void loop()
{
  updateWifiLed();

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
