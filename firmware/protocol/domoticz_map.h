#ifndef DOMOTICZ_MAP_H
#define DOMOTICZ_MAP_H

#include <stdint.h>

/*
 * idx urzadzen Domoticz wedlug docs/project/domoticz-devices-2026-08-21.md.
 * idx 44 "Temperatura kuchnia" jest martwe (Used=0 od 2025-01-04), dlatego
 * slot temperatury 3 pokazuje Salon (idx 39) zamiast Kuchni.
 */

#define UI_SWITCH_COUNT 5
#define UI_TEMP_COUNT 3

static const uint16_t UI_SWITCH_IDX[UI_SWITCH_COUNT] = {
    32, /* 1 Lozko Aga */
    33, /* 2 Lozko Tomek */
    34, /* 3 Biurko */
    76, /* 4 Dock Station */
    57  /* 5 Salon Spr. */
};

static const uint16_t UI_TEMP_IDX[UI_TEMP_COUNT] = {
    39, /* 1 Salon */
    40, /* 2 Sypialnia */
    42  /* 3 Zewn. */
};

#endif
