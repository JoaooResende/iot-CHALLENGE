#pragma once
#include "config.h"
#include "state.h"

void display_begin();
void display_update();
void display_show(const char* row1, const char* row2);
void display_show_ip(const String& ip);
