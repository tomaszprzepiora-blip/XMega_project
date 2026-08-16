#ifndef DOMOTICZ_CONFIG_H
#define DOMOTICZ_CONFIG_H

#include <stdint.h>

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

