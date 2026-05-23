#pragma once
#include <Arduino.h>
#include "config.h"

enum DoorState {
    DOOR_CLOSED,
    DOOR_OPENING,
    DOOR_OPEN,
    DOOR_ALARMING,
    DOOR_CLOSING
};

struct LogEntry {
    char time[LOG_TIME_LEN];
    char msg[LOG_MSG_LEN];
};

struct SystemState {
    // Porta / servo
    DoorState     doorState        = DOOR_CLOSED;
    int           servoCurrentAngle = SERVO_ANGLE_CLOSED;
    int           servoTargetAngle  = SERVO_ANGLE_CLOSED;
    unsigned long servoLastStepMs   = 0;

    unsigned long doorOpenedAt  = 0;
    unsigned long alarmStartAt  = 0;

    
    bool          autoFeedEnabled = true;
    unsigned long lastAutoFeedAt  = 0;
    char          lastFeedTime[LOG_TIME_LEN] = "--:--:--";
    int           feedCount  = 0;
    int           foodLevel  = FOOD_LEVEL_INITIAL;

    
    bool          useSchedule          = false;
    unsigned long feedIntervalMs       = INTERVAL_AUTO_FEED;
    int           schedHour            = 8;
    int           schedMin             = 0;
    bool          schedFiredThisMinute = false;

    
    float temperature = 0.0f;
    float humidity    = 0.0f;

    
    unsigned long bootMs = 0;

   
    LogEntry logs[LOG_MAX];
    int      logCount = 0;
};


extern SystemState state;
