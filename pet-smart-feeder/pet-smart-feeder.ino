


#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "config.h"
#include "state.h"
#include "alerts.h"
#include "display.h"
#include "sensors.h"
#include "feeder.h"
#include "api.h"



SystemState state;       
WebServer   server(80);


static void wifi_connect() {
    display_show("Conectando", "ao WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Conectando");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    String ip = WiFi.localIP().toString();
    Serial.printf("\nConectado! IP: %s\n", ip.c_str());
    Serial.printf("Dashboard:    http://%s\n", ip.c_str());

    // UTC-3 (Brasilia) — sem horario de verao
    configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    display_show_ip(ip);
    delay(2000);
}



void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=== Pet Family Smart Feeder ===");

    state.bootMs = millis();

    alerts_begin();
    display_begin();
    sensors_begin();
    feeder_begin();

    wifi_connect();
    api_begin(server);

    alerts_play_startup();
    Serial.println("Sistema pronto.\n");
}



void loop() {
    server.handleClient();
    feeder_update();
    sensors_update();
    display_update();
    alerts_update();
}
