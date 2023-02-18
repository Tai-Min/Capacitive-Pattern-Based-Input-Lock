#pragma once

constexpr uint32_t SLEEP_STANDBY_MS = 100;
constexpr uint32_t SLEEP_NORMAL_MS = 50;
constexpr uint32_t SLEEP_PATTERN_WIP_MS = 10;

constexpr uint32_t TIMER_PROXIMITY_THRESHOLD_MS = 30000;
constexpr uint32_t TIMER_TOUCHPAD_THRESHOLD_MS = 5000;
constexpr uint32_t TIMER_CANCEL_THRESHOLD_MS = 2000;
constexpr uint32_t TIMER_PATTERN_BLINKER_THRESHOLD_MS = 250;

/*
* 1 for proximity
* 1 for touchpad
* 1 for cancel
* 1 for pattern blink
* 4 for relays
*/
constexpr uint8_t TIMER_COUNT = 8;

constexpr uint8_t CAPSENSE_NUM_ROWS = 20;
constexpr uint8_t CAPSENSE_NUM_COLS = 20;
constexpr uint8_t CAPSENSE_ROWS_DIVIDER = 100 / CAPSENSE_NUM_ROWS;
constexpr uint8_t CAPSENSE_COLS_DIVIDER = 100 / CAPSENSE_NUM_COLS;
constexpr uint8_t CAPSENSE_BUFFER_SIZE = CAPSENSE_NUM_ROWS * CAPSENSE_NUM_COLS / 8;
constexpr uint32_t CAPSENSE_BAD_WIDGET_ID = 1000;

constexpr uint8_t PATTERN_MAX_LENGTH = 8;
constexpr uint32_t PATTERN_ERR_BLINK_DELAY_MS = 500;

constexpr uint8_t RELAYS_COUNT = 4;

constexpr uint32_t WDT_IRQ_NUMBER = 6;