#include "display.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

static LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
static char              prevRow1[17] = "";
static char              prevRow2[17] = "";
static unsigned long     lastUpdateMs = 0;


static void _draw(const char* r1, const char* r2) {
    if (strncmp(r1, prevRow1, 16) == 0 && strncmp(r2, prevRow2, 16) == 0) return;

    strncpy(prevRow1, r1, 16); prevRow1[16] = '\0';
    strncpy(prevRow2, r2, 16); prevRow2[16] = '\0';

    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(r1);
    lcd.setCursor(0, 1); lcd.print(r2);
}

void display_begin() {
    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();
    _draw("Pet Smart Feeder", "Iniciando...");
}

void display_show(const char* row1, const char* row2) {
    _draw(row1, row2);
}

void display_show_ip(const String& ip) {
    _draw("IP:", ip.c_str());
}

void display_update() {
    unsigned long now = millis();
    if (now - lastUpdateMs < 500) return;
    lastUpdateMs = now;

    char r1[17], r2[17];

    switch (state.doorState) {
        case DOOR_CLOSED: {
            if (state.useSchedule) {
                snprintf(r1, 17, "Hora Person.");
                snprintf(r2, 17, "Racao: %d%%", state.foodLevel);
            } else {
                unsigned long elapsed = (state.lastAutoFeedAt == 0) ? 0 : (now - state.lastAutoFeedAt);
                unsigned long nextIn  = (elapsed < state.feedIntervalMs)
                                      ? (state.feedIntervalMs - elapsed) / 1000
                                      : 0;
                snprintf(r1, 17, "Prox: %lus", nextIn);
                snprintf(r2, 17, "Racao: %d%%", state.foodLevel);
            }
            break;
        }
        case DOOR_OPENING:
            _draw("Abrindo porta", "Aguarde...");
            return;

        case DOOR_OPEN: {
            unsigned long left = 0;
            unsigned long elapsed = now - state.doorOpenedAt;
            unsigned long threshold = INTERVAL_DOOR_OPEN - INTERVAL_ALARM;
            if (elapsed < threshold) left = (threshold - elapsed) / 1000;
            snprintf(r1, 17, "Porta: ABERTA");
            snprintf(r2, 17, "Fecha em: %lus", left);
            break;
        }
        case DOOR_ALARMING:
            _draw("!! ATENCAO !!", "Fechando...");
            return;

        case DOOR_CLOSING:
            _draw("Fechando porta", "Aguarde...");
            return;

        default:
            return;
    }

    _draw(r1, r2);
}
