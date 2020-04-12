/* ---------------------------------------------------- *
 * Suntracker2 mainboard-rev4 motorz.cpp 2019-10 @FM4DD *
 *                                                      *
 * This file has the functions to control the Zenith    *
 * axis stepper motor.                                  *
 * ---------------------------------------------------- */
#include <Arduino.h>           // Arduino default library
#include "globals.h"           // global variable definitions

void ZMotor::enable() {
  /* ------------------------------------------------- */
  /* Set zenith motor control pins as output           */
  /* ------------------------------------------------- */
  pinMode(STEP2,OUTPUT); 
  pinMode(DIR2,OUTPUT);
  pinMode(ENA2,OUTPUT);
  pinMode(ESWI2,INPUT);
  digitalWrite(ENA2,HIGH); // Disable stepstick2
}

/* ---------------------------------------------------- */
/* poweron enables the stepstick controller and sets    */
/* the motor move variabe m1move to 'true'.             */
/* ---------------------------------------------------- */
void ZMotor::poweron() {
  if(digitalRead(ENA2) == HIGH) {
    digitalWrite(ENA2,LOW); /* Enable stepstick2 motor driver */
    m2move = true;
  }
}

/* ---------------------------------------------------- */
/* poweroff enables the stepstick controller and sets   */
/* the motor move variabe m1move to 'false'.            */
/* ---------------------------------------------------- */
void ZMotor::poweroff() {
  if(digitalRead(ENA2) == LOW) {
    digitalWrite(ENA2,HIGH); /* Disable stepstick2 motor driver */
    m2move = false;
  }
}

/* ---------------------------------------------------- */
/* demoturn demonstrates a 90 degree stepper turn       */
/* counterclockwise, and then returns back clockwise    */
/* Red LED will follow the indicator aligned with D1    */
/* ---------------------------------------------------- */
void ZMotor::demoturn(uint8_t count) {
  int mled = 0;           /* To move one red LED clockwise  */
  int oldmled = 41;       /* To turn off the previous LED   */
  mcp6.writeGPIOAB(0x00); /* Turn off all green LED 1..16   */
  mcp7.writeGPIOAB(0x00); /* Turn off all red LED 1..16     */
  digitalWrite(ENA2,LOW); /* Enable stepstick2 motor driver */
  for(uint8_t turn = 0; turn < count; turn++) {
    digitalWrite(DIR2,HIGH); /* clockwise motor direction    */
    for(int x = 0; x < 640; x++) { /* 0..639 = 90 degrees    */
      digitalWrite(STEP2,HIGH); 
      delayMicroseconds(SLOW); 
      digitalWrite(STEP2,LOW); 
      delayMicroseconds(SLOW);
      mled = x / 40; /* 800/16=50, round to int */ 
      if (mled != oldmled) {
        mcp7.digitalWrite(oldmled, LOW);
        mcp7.digitalWrite(mled, HIGH);
        oldmled = mled;
      }
    }
    digitalWrite(DIR2 ,LOW);     /* Set dir counterclockwise */
    delay(1000);                /* Wait for one second delay */
    for(int x = 639; x >= 0; x--) {  /* 639 ..0 = 90 degrees */
      digitalWrite(STEP2, HIGH);
      delayMicroseconds(SLOW);
      digitalWrite(STEP2, LOW);
      delayMicroseconds(SLOW);
      mled = x / 40; /* 640/16=40, round to int  */
      if (mled != oldmled) {
        mcp7.digitalWrite(oldmled, LOW);
        mcp7.digitalWrite(mled, HIGH);
        oldmled = mled;
      }
    }
    delay(1000);                /* Wait for one second delay */
  }
  digitalWrite(ENA2, HIGH); /* Disable stepstick1 motordriver */
  mcp7.writeGPIOAB(0x00);   /* Turn off all red LED 1..16     */
}

/* ---------------------------------------------------- */
/* adjust moves from the current position towards the   */
/* target position in one go. position range 0..1600    */
/* ---------------------------------------------------- */
void ZMotor::adjust(uint16_t target, int speed) {
  uint16_t steps = 0;
  m2move = true;
  digitalWrite(ENA2,LOW); /* Enable stepstick1 motor driver */
  if(m2cpos < m2tpos) {
    if(abs(m2cpos - m2tpos) < 320) {
      digitalWrite(DIR2,HIGH); /* run clockwise motor direction */
      steps = m2tpos - m2cpos; /* get required steps to target  */
    }
    else {                    /* the other way around is shorter */
      digitalWrite(DIR2,LOW); /* move counterclockwise direction */
      steps = 320 - m2tpos + m2cpos;
    }
  }
  else {
    if((m2cpos - m2tpos) < 320) {
      digitalWrite(DIR2,LOW); /* counterclockwise  direction  */
      steps = m2cpos - m2tpos;
    }
    else {                     /* the other way around is shorter */
      digitalWrite(DIR2,HIGH); /* run clockwise motor direction   */
      steps = 320 - m2cpos + m2tpos;      
    }
  }
  for(int x = 0; x < steps; x++) {
      digitalWrite(STEP2,HIGH);
      delayMicroseconds(speed);
      digitalWrite(STEP2,LOW); 
      delayMicroseconds(speed);
  }
  //digitalWrite(ENA1,HIGH); /* Disable stepstick1 motordriver */
  m2move = false;
  m2cpos = m2tpos;
}
