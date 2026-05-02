#include "engine.h"

struct WheelState {
    int forwardPin;
    int backwardPin;
    int speed;
    bool isForward;
};

static WheelState wheels[] = {
    { WheelLeftBackMovingForward,    WheelLeftBackMovingBackward,   0, true },
    { WheelLeftFrontMovingForward,   WheelLeftFrontMovingBackward,  0, true },
    { WheelRightFrontMovingForward,  WheelRightFrontMovingBackward, 0, true },
    { WheelRightBackMovingForward,   WheelRightBackMovingBackward,  0, true },
};

static const int LEFT_BACK   = 0;
static const int LEFT_FRONT  = 1;
static const int RIGHT_FRONT = 2;
static const int RIGHT_BACK  = 3;

static unsigned long stopAt = 0;  // millis() deadline for auto-stop; 0 = none

void engine_loop() {
    if (stopAt > 0 && millis() >= stopAt) {
        stopAt = 0;
        stop();
    }
}

void scheduleStop(int duration) {
    stopAt = millis() + (unsigned long)duration;
}

static void applyWheel(WheelState& w) {
    // Actually, it can't run at speed lower than 80
    if (w.speed < 80) {
        w.speed = 80;
    } else if (w.speed > 255) {
        w.speed = 255;
    }
    if (w.isForward) {
        Serial.printf("Forward:  Wheel %d: %d\r\n", w.forwardPin, w.speed);
        analogWrite(w.backwardPin, 0);
        analogWrite(w.forwardPin,  w.speed);
    } else {
        Serial.printf("Backward:  Wheel %d: %d\r\n", w.backwardPin, w.speed);
        analogWrite(w.forwardPin,  0);
        analogWrite(w.backwardPin, w.speed);
    }
}

static void setForward(int idx, int speed) {
    wheels[idx].speed     = speed;
    wheels[idx].isForward = true;
    applyWheel(wheels[idx]);
}

static void setBackward(int idx, int speed) {
    wheels[idx].speed     = speed;
    wheels[idx].isForward = false;
    applyWheel(wheels[idx]);
}

static void reverseWheel(int idx) {
    wheels[idx].isForward = !wheels[idx].isForward;
    applyWheel(wheels[idx]);
}

void setupEngine() {
    for (auto& w : wheels) {
        pinMode(w.forwardPin,  OUTPUT);
        pinMode(w.backwardPin, OUTPUT);
    }
}

void wheelLeftBackForward(int speed)    { setForward (LEFT_BACK,   speed); }
void wheelLeftBackBackward(int speed)   { setBackward(LEFT_BACK,   speed); }
void wheelLeftFrontForward(int speed)   { setForward (LEFT_FRONT,  speed); }
void wheelLeftFrontBackward(int speed)  { setBackward(LEFT_FRONT,  speed); }
void wheelRightFrontForward(int speed)  { setForward (RIGHT_FRONT, speed); }
void wheelRightFrontBackward(int speed) { setBackward(RIGHT_FRONT, speed); }
void wheelRightBackForward(int speed)   { setForward (RIGHT_BACK,  speed); }
void wheelRightBackBackward(int speed)  { setBackward(RIGHT_BACK,  speed); }

void wheelLeftBackReverse()   { reverseWheel(LEFT_BACK); }
void wheelLeftFrontReverse()  { reverseWheel(LEFT_FRONT); }
void wheelRightFrontReverse() { reverseWheel(RIGHT_FRONT); }
void wheelRightBackReverse()  { reverseWheel(RIGHT_BACK); }

void reverseAll() {
    for (auto& w : wheels) {
        w.isForward = !w.isForward;
        applyWheel(w);
    }
}

void adjustSpeed(int delta) {
    for (auto& w : wheels) {
        w.speed = constrain(w.speed + delta, 0, 255);
        applyWheel(w);
    }
}

int wheelLeftBackSpeed()   { return wheels[LEFT_BACK].speed; }
int wheelLeftFrontSpeed()  { return wheels[LEFT_FRONT].speed; }
int wheelRightFrontSpeed() { return wheels[RIGHT_FRONT].speed; }
int wheelRightBackSpeed()  { return wheels[RIGHT_BACK].speed; }

void pivotLeft(int speed) {
    stopAt = 0;
    wheelLeftBackBackward(speed);
    wheelLeftFrontBackward(speed);
    wheelRightFrontForward(speed);
    wheelRightBackForward(speed);
}

void pivotRight(int speed) {
    stopAt = 0;
    wheelLeftBackForward(speed);
    wheelLeftFrontForward(speed);
    wheelRightFrontBackward(speed);
    wheelRightBackBackward(speed);
}

void arcLeft(int innerSpeed, int outerSpeed) {
    stopAt = 0;
    wheelLeftBackForward(innerSpeed);
    wheelLeftFrontForward(innerSpeed);
    wheelRightFrontForward(outerSpeed);
    wheelRightBackForward(outerSpeed);
}

void arcRight(int innerSpeed, int outerSpeed) {
    stopAt = 0;
    wheelLeftBackForward(outerSpeed);
    wheelLeftFrontForward(outerSpeed);
    wheelRightFrontForward(innerSpeed);
    wheelRightBackForward(innerSpeed);
}

void moveForward(int speed, int time) {
    Serial.println("Moving Forward");
    stopAt = 0;
    wheelLeftBackForward(speed);
    wheelLeftFrontForward(speed);
    wheelRightFrontForward(speed);
    wheelRightBackForward(speed);
    if (time > 0) scheduleStop(time);
}

void moveBackward(int speed, int time) {
    Serial.println("Moving Backward");
    stopAt = 0;
    wheelLeftBackBackward(speed);
    wheelLeftFrontBackward(speed);
    wheelRightFrontBackward(speed);
    wheelRightBackBackward(speed);
    if (time > 0) scheduleStop(time);
}

void stop() {
    Serial.println("Stopping");
    stopAt = 0;
    for (auto& w : wheels) {
        w.speed = 0;
        analogWrite(w.forwardPin,  0);
        analogWrite(w.backwardPin, 0);
    }
}
