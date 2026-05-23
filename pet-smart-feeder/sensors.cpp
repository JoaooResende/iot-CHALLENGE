#include "sensors.h"
#include <DHTesp.h>

static DHTesp        dht;
static unsigned long lastReadAt = 0;

void sensors_begin() {
    dht.setup(PIN_DHT, DHTesp::DHT22);
    
    lastReadAt = millis();
}

void sensors_update() {
    unsigned long now = millis();
    if (now - lastReadAt < INTERVAL_DHT_READ) return;
    lastReadAt = now;

    TempAndHumidity th = dht.getTempAndHumidity();

    
    if (dht.getStatus() == 0) {
        state.temperature = th.temperature;
        state.humidity    = th.humidity;
    }
}
