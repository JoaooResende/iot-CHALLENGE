#include "feeder.h"
#include "alerts.h"
#include <time.h>

static Servo         servo;
static unsigned long lastBtnOpenAt  = 0;
static unsigned long lastBtnCloseAt = 0;

static void _log(const char* msg) {
    int top = min(state.logCount, LOG_MAX - 1);
    for (int i = top; i > 0; i--) state.logs[i] = state.logs[i - 1];

    unsigned long s = millis() / 1000;
    snprintf(state.logs[0].time, LOG_TIME_LEN, "%02lu:%02lu:%02lu",
             (s / 3600) % 24, (s / 60) % 60, s % 60);
    strncpy(state.logs[0].msg, msg, LOG_MSG_LEN - 1);
    state.logs[0].msg[LOG_MSG_LEN - 1] = '\0';

    if (state.logCount < LOG_MAX) state.logCount++;

    Serial.printf("[%s] %s\n", state.logs[0].time, msg);
}

static void _servo_set_target(int angle) {
    state.servoTargetAngle = angle;
}

static void _servo_step() {
    if (state.servoCurrentAngle == state.servoTargetAngle) return;

    unsigned long now = millis();
    if (now - state.servoLastStepMs < SERVO_STEP_MS) return;
    state.servoLastStepMs = now;

    if (state.servoCurrentAngle < state.servoTargetAngle)
        state.servoCurrentAngle++;
    else
        state.servoCurrentAngle--;

    servo.write(state.servoCurrentAngle);
}

static bool _servo_at_target() {
    return state.servoCurrentAngle == state.servoTargetAngle;
}

void feeder_begin() {
    servo.setPeriodHertz(50);
    servo.attach(PIN_SERVO, 500, 2400);
    servo.write(SERVO_ANGLE_CLOSED);
    state.servoCurrentAngle = SERVO_ANGLE_CLOSED;
    state.servoTargetAngle  = SERVO_ANGLE_CLOSED;

    pinMode(PIN_BTN_OPEN,  INPUT_PULLUP);
    pinMode(PIN_BTN_CLOSE, INPUT_PULLUP);

    state.lastAutoFeedAt = millis();
    _log("Sistema iniciado");
}

void feeder_trigger_open(const char* reason) {
    if (state.doorState != DOOR_CLOSED) return;
    _log(reason);
    state.doorState = DOOR_OPENING;
    _servo_set_target(SERVO_ANGLE_OPEN);
}

void feeder_trigger_close(const char* reason) {
    if (state.doorState == DOOR_CLOSED || state.doorState == DOOR_CLOSING) return;
    _log(reason);
    noTone(PIN_BUZZER);
    state.doorState = DOOR_CLOSING;
    _servo_set_target(SERVO_ANGLE_CLOSED);
}

bool feeder_is_open() {
    return state.doorState == DOOR_OPEN || state.doorState == DOOR_ALARMING;
}

static void _handle_state_machine() {
    unsigned long now = millis();

    switch (state.doorState) {

        case DOOR_OPENING:
            if (_servo_at_target()) {
                state.doorState    = DOOR_OPEN;
                state.doorOpenedAt = now;
                state.foodLevel    = 0;
                alerts_play_feed_open();
                _log("Porta aberta - distribuindo racao");
            }
            break;

        case DOOR_OPEN:
            if (now - state.doorOpenedAt >= INTERVAL_DOOR_OPEN - INTERVAL_ALARM) {
                state.doorState    = DOOR_ALARMING;
                state.alarmStartAt = now;
                _log("Alarme: encerrando ciclo de alimentacao");
            }
            break;

        case DOOR_ALARMING:
            if (now - state.alarmStartAt >= INTERVAL_ALARM) {
                feeder_trigger_close("Fechamento automatico pos-alarme");
            }
            break;

        case DOOR_CLOSING:
            if (_servo_at_target()) {
                state.doorState      = DOOR_CLOSED;
                state.lastAutoFeedAt = now;
                state.feedCount++;

                unsigned long s = now / 1000;
                snprintf(state.lastFeedTime, LOG_TIME_LEN, "%02lu:%02lu:%02lu",
                         (s / 3600) % 24, (s / 60) % 60, s % 60);

                alerts_play_feed_close();
                _log("AVISO: nivel de racao baixo - reabastecer!");
            }
            break;

        case DOOR_CLOSED:
            if (state.autoFeedEnabled) {
                if (state.useSchedule) {
                    struct tm t;
                    if (getLocalTime(&t)) {
                        bool inSchedMin = (t.tm_hour == state.schedHour &&
                                          t.tm_min  == state.schedMin);
                        if (inSchedMin && !state.schedFiredThisMinute) {
                            state.schedFiredThisMinute = true;
                            feeder_trigger_open("Alimentacao agendada por horario");
                        } else if (!inSchedMin) {
                            state.schedFiredThisMinute = false;
                        }
                    }
                } else {
                    if (now - state.lastAutoFeedAt >= state.feedIntervalMs) {
                        feeder_trigger_open("Alimentacao automatica agendada");
                    }
                }
            }
            break;
    }
}

static void _handle_buttons() {
    unsigned long now = millis();

    if (digitalRead(PIN_BTN_OPEN) == LOW && now - lastBtnOpenAt > INTERVAL_BTN_DEBOUNCE) {
        lastBtnOpenAt = now;
        feeder_trigger_open("Botao fisico: abrir");
    }

    if (digitalRead(PIN_BTN_CLOSE) == LOW && now - lastBtnCloseAt > INTERVAL_BTN_DEBOUNCE) {
        lastBtnCloseAt = now;
        feeder_trigger_close("Botao fisico: fechar");
    }
}

void feeder_update() {
    _servo_step();
    _handle_state_machine();
    _handle_buttons();
}
