#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

cc -std=c99 -Wall -Wextra -pedantic \
  -I"$ROOT/firmware/ui" \
  -I"$ROOT/firmware/protocol" \
  "$ROOT/firmware/ui/xmega_ui.c" \
  "$ROOT/firmware/protocol/esp_protocol.c" \
  "$ROOT/tests/test_xmega_ui.c" \
  -o /tmp/test_xmega_ui

/tmp/test_xmega_ui

cc -std=c99 -Wall -Wextra -pedantic \
  -I"$ROOT/firmware/protocol" \
  "$ROOT/firmware/protocol/esp_protocol.c" \
  "$ROOT/tests/test_esp_protocol.c" \
  -o /tmp/test_esp_protocol

/tmp/test_esp_protocol

