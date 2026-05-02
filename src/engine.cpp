#include "engine.h"

struct WheelState {
    int  forwardPin;
    int  backwardPin;
    int  speed;      // logical speed 0–255; hardware minimum of 80 is applied at output only
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

// ---------------------------------------------------------------------------
// Scheduled stop
// ---------------------------------------------------------------------------

static unsigned long stopAt = 0;

void scheduleStop(int duration) {
    stopAt = millis() + (unsigned long)duration;
}

// ---------------------------------------------------------------------------
// Speed ramp
// ---------------------------------------------------------------------------

static int          rampTarget   = -1;
static int          rampStepMs   = 0;
static unsigned long rampNextStep = 0;

void rampTo(int targetSpeed, int stepMs) {
    rampTarget   = constrain(targetSpeed, 0, 255);
    rampStepMs   = stepMs;
    rampNextStep = millis();
}

// ---------------------------------------------------------------------------
// Low-level PWM output
// Hardware minimum running speed is 80; applied here only, not stored in state
// so the logical speed continues to track correctly (e.g. for ramp, adjustSpeed)
// ---------------------------------------------------------------------------

static void applyWheel(const WheelState& w) {
    int pwm = (w.speed == 0) ? 0 : constrain(w.speed, 80, 255);
    if (w.isForward) {
        Serial.printf("Forward:  pin %d  pwm %d\r\n", w.forwardPin, pwm);
        analogWrite(w.backwardPin, 0);
        analogWrite(w.forwardPin,  pwm);
    } else {
        Serial.printf("Backward: pin %d  pwm %d\r\n", w.backwardPin, pwm);
        analogWrite(w.forwardPin,  0);
        analogWrite(w.backwardPin, pwm);
    }
}

// ---------------------------------------------------------------------------
// engine_loop — call every iteration of loop()
// ---------------------------------------------------------------------------

void engine_loop() {
    if (stopAt > 0 && millis() >= stopAt) {
        stopAt = 0;
        stop();
    }

    if (rampTarget >= 0 && millis() >= rampNextStep) {
        rampNextStep = millis() + (unsigned long)rampStepMs;
        bool allDone = true;
        for (auto& w : wheels) {
            if      (w.speed < rampTarget) { w.speed++; allDone = false; }
            else if (w.speed > rampTarget) { w.speed--; allDone = false; }
            applyWheel(w);
        }
        if (allDone) rampTarget = -1;
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void setupEngine() {
    for (auto& w : wheels) {
        pinMode(w.forwardPin,  OUTPUT);
        pinMode(w.backwardPin, OUTPUT);
    }
}

// ---------------------------------------------------------------------------
// Per-wheel control
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Multi-wheel operations
// ---------------------------------------------------------------------------

void reverseAll() {
    rampTarget = -1;
    for (auto& w : wheels) {
        w.isForward = !w.isForward;
        applyWheel(w);
    }
}

void adjustSpeed(int delta) {
    rampTarget = -1;
    for (auto& w : wheels) {
        w.speed = constrain(w.speed + delta, 0, 255);
        applyWheel(w);
    }
}

// ---------------------------------------------------------------------------
// Speed / direction queries
// ---------------------------------------------------------------------------

int  wheelLeftBackSpeed()   { return wheels[LEFT_BACK].speed; }
int  wheelLeftFrontSpeed()  { return wheels[LEFT_FRONT].speed; }
int  wheelRightFrontSpeed() { return wheels[RIGHT_FRONT].speed; }
int  wheelRightBackSpeed()  { return wheels[RIGHT_BACK].speed; }

bool wheelLeftBackIsForward()   { return wheels[LEFT_BACK].isForward; }
bool wheelLeftFrontIsForward()  { return wheels[LEFT_FRONT].isForward; }
bool wheelRightFrontIsForward() { return wheels[RIGHT_FRONT].isForward; }
bool wheelRightBackIsForward()  { return wheels[RIGHT_BACK].isForward; }

// ---------------------------------------------------------------------------
// Turning
// ---------------------------------------------------------------------------

void pivotLeft(int speed) {
    stopAt = 0; rampTarget = -1;
    wheelLeftBackBackward(speed);
    wheelLeftFrontBackward(speed);
    wheelRightFrontForward(speed);
    wheelRightBackForward(speed);
}

void pivotRight(int speed) {
    stopAt = 0; rampTarget = -1;
    wheelLeftBackForward(speed);
    wheelLeftFrontForward(speed);
    wheelRightFrontBackward(speed);
    wheelRightBackBackward(speed);
}

void arcLeft(int innerSpeed, int outerSpeed) {
    stopAt = 0; rampTarget = -1;
    wheelLeftBackForward(innerSpeed);
    wheelLeftFrontForward(innerSpeed);
    wheelRightFrontForward(outerSpeed);
    wheelRightBackForward(outerSpeed);
}

void arcRight(int innerSpeed, int outerSpeed) {
    stopAt = 0; rampTarget = -1;
    wheelLeftBackForward(outerSpeed);
    wheelLeftFrontForward(outerSpeed);
    wheelRightFrontForward(innerSpeed);
    wheelRightBackForward(innerSpeed);
}

// ---------------------------------------------------------------------------
// All-wheel convenience
// ---------------------------------------------------------------------------

void moveForward(int speed, int time) {
    Serial.println("Moving Forward");
    stopAt = 0; rampTarget = -1;
    wheelLeftBackForward(speed);
    wheelLeftFrontForward(speed);
    wheelRightFrontForward(speed);
    wheelRightBackForward(speed);
    if (time > 0) scheduleStop(time);
}

void moveBackward(int speed, int time) {
    Serial.println("Moving Backward");
    stopAt = 0; rampTarget = -1;
    wheelLeftBackBackward(speed);
    wheelLeftFrontBackward(speed);
    wheelRightFrontBackward(speed);
    wheelRightBackBackward(speed);
    if (time > 0) scheduleStop(time);
}

void stop() {
    Serial.println("Stopping");
    stopAt = 0; rampTarget = -1;
    for (auto& w : wheels) {
        w.speed = 0;
        analogWrite(w.forwardPin,  0);
        analogWrite(w.backwardPin, 0);
    }
}
