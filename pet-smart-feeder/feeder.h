#pragma once
#include <ESP32Servo.h>
#include "config.h"
#include "state.h"

void feeder_begin();
void feeder_update();
void feeder_trigger_open(const char* reason);
void feeder_trigger_close(const char* reason);
bool feeder_is_open();
