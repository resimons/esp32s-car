#pragma once

#include <Arduino.h>

#define WheelLeftBackMovingForward    GPIO_NUM_12  // Motor A INA
#define WheelLeftBackMovingBackward   GPIO_NUM_13  // Motor A INB

#define WheelLeftFrontMovingForward   GPIO_NUM_14  // Motor B INA
#define WheelLeftFrontMovingBackward  GPIO_NUM_15  // Motor B INB

#define WheelRightFrontMovingForward  GPIO_NUM_25  // Motor C INA
#define WheelRightFrontMovingBackward GPIO_NUM_26  // Motor C INB

#define WheelRightBackMovingForward   GPIO_NUM_32  // Motor D INA
#define WheelRightBackMovingBackward  GPIO_NUM_33  // Motor D INB

void setupEngine();

// Per-wheel direction control (speed: 0–255)
void wheelLeftBackForward(int speed);
void wheelLeftBackBackward(int speed);
void wheelLeftFrontForward(int speed);
void wheelLeftFrontBackward(int speed);
void wheelRightFrontForward(int speed);
void wheelRightFrontBackward(int speed);
void wheelRightBackForward(int speed);
void wheelRightBackBackward(int speed);

// Flip direction of a single wheel keeping its current speed
void wheelLeftBackReverse();
void wheelLeftFrontReverse();
void wheelRightFrontReverse();
void wheelRightBackReverse();

// Flip direction of all wheels keeping their individual speeds
void reverseAll();

// Query the tracked speed of a wheel (0–255)
int wheelLeftBackSpeed();
int wheelLeftFrontSpeed();
int wheelRightFrontSpeed();
int wheelRightBackSpeed();

// Pivot in place: left side backward, right side forward (or vice versa) at equal speed
void pivotLeft(int speed);
void pivotRight(int speed);

// Arc turn while moving forward: inner side slower, outer side faster
void arcLeft(int innerSpeed, int outerSpeed);
void arcRight(int innerSpeed, int outerSpeed);

// All-wheel convenience functions
void moveForward(int speed, int time);
void moveBackward(int speed, int time);
void stop();
