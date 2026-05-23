#include "alerts.h"
#include <Arduino.h>

static unsigned long lastBeepAt = 0;
static bool          beepPhase  = false;

void alerts_begin() {
    pinMode(PIN_BUZZER,     OUTPUT);
    pinMode(PIN_LED_OPEN,   OUTPUT);
    pinMode(PIN_LED_CLOSED, OUTPUT);

    digitalWrite(PIN_LED_OPEN,   LOW);
    digitalWrite(PIN_LED_CLOSED, HIGH);
}

void alerts_play_startup() {
    tone(PIN_BUZZER, 880,  150); delay(200);
    tone(PIN_BUZZER, 1100, 150); delay(200);
    tone(PIN_BUZZER, 1320, 250);
}

void alerts_play_feed_open() {
    tone(PIN_BUZZER, 1200, 300);
}

void alerts_play_feed_close() {
    tone(PIN_BUZZER, 800, 200);
}

static void _update_leds() {
    bool open = (state.doorState == DOOR_OPEN || state.doorState == DOOR_ALARMING);
    digitalWrite(PIN_LED_OPEN,   open ? HIGH : LOW);
    digitalWrite(PIN_LED_CLOSED, open ? LOW  : HIGH);
}

static void _update_alarm_beep() {
    if (state.doorState != DOOR_ALARMING) return;

    unsigned long now = millis();
    if (now - lastBeepAt >= 500) {
        lastBeepAt = now;
        beepPhase  = !beepPhase;
        if (beepPhase) tone(PIN_BUZZER, 1500, 380);
    }
}

void alerts_update() {
    _update_leds();
    _update_alarm_beep();
}
