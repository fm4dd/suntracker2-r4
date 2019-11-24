/* ---------------------------------------------------- *
 * Suntracker2  mainboard-rev4 led.cpp   2019-10 @FM4DD *
 *                                                      *
 * This file has the functions to control the stepper   *
 * motors. one=azimuth = improve naming!!!              *
 * ---------------------------------------------------- */
#include <Arduino.h>           // Arduino default library
#include "globals.h"           // global variable definitions

void AMotor::enable() {
  /* ------------------------------------------------- */
  /* Set azimuth motor control pins as output          */
  /* ------------------------------------------------- */
  pinMode(STEP1,OUTPUT); 
  pinMode(DIR1,OUTPUT);
  pinMode(ENA1,OUTPUT);
  pinMode(ESWI1,INPUT);
  digitalWrite(ENA1,HIGH); // Disable stepstick1
}

/* ---------------------------------------------------- */
/* demoturn demonstrates a 360 degree stepper turn      */
/* clockwise, and then returns back counterclockwise    */
/* Red LED will follow the indicator aligned with D1    */
/* ---------------------------------------------------- */
void AMotor::demoturn(uint8_t count) {
  int mled;               /* To move one red LED clockwise  */
  int oldmled;            /* To turn off the previous LED   */
  mcp1.writeGPIOAB(0x00); /* Turn off all red LED 1..16     */
  mcp2.writeGPIOAB(0x00); /* Turn off all red LED 17..32    */
  mcp3.writeGPIOAB(0x00); /* Turn off all red LED 33..48    */
  digitalWrite(ENA1,LOW); /* Enable stepstick1 motor driver */
  for(uint8_t turn = 0; turn < count; turn++) {
    digitalWrite(DIR1,HIGH); /* clockwise motor direction    */
    for(float x = 0; x < 1600; x++) { /* 1600 steps = 1 turn */
      digitalWrite(STEP1,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(STEP1,LOW); 
      delayMicroseconds(500);
      mled = (int) round(x / 34); /* 1600/47=34, round to int */ 
      if (mled != oldmled) {
        if(oldmled < 16) mcp1.digitalWrite(oldmled, LOW);
        else if(oldmled < 32) mcp2.digitalWrite((oldmled-16), LOW);
        else if(oldmled < 48) mcp3.digitalWrite((oldmled-32), LOW);
        if(mled > 31) mcp3.digitalWrite((mled-32), HIGH);
        else if(mled > 15) mcp2.digitalWrite((mled-16), HIGH);
        else if(mled < 16) mcp1.digitalWrite(mled, HIGH);
        oldmled = mled;
      }
    }
    mcp1.digitalWrite(0, HIGH); /* Last value is 48, turns 1 */
    mcp3.digitalWrite(15, LOW);  /* and delete previous LED  */
    
    delay(1000);                /* Wait for one second delay */
    oldmled = 0;                /* Reset the old LED value   */
    digitalWrite(DIR1 ,LOW);     /* Set dir counterclockwise */
    for(double x = 1599; x >= 0; x--) { /* 1599..0 steps 1 turn */
      digitalWrite(STEP1, HIGH);
      delayMicroseconds(500);
      digitalWrite(STEP1, LOW);
      delayMicroseconds(500);
      mled = round(x / 34); /* 1600/48=33.3, round to int  */
      if (mled != oldmled) {
        if(oldmled < 16) mcp1.digitalWrite(oldmled, LOW);
        else if(oldmled < 32) mcp2.digitalWrite((oldmled-16), LOW);
        else if(oldmled < 48) mcp3.digitalWrite((oldmled-32), LOW);
        if(mled > 31) mcp3.digitalWrite((mled-32), HIGH);
        else if(mled > 15) mcp2.digitalWrite((mled-16), HIGH);
        else if(mled < 16) mcp1.digitalWrite(mled, HIGH);
        oldmled = mled;
      }
    }
    mcp1.digitalWrite(0, HIGH); /* Turn the last LED 2 to 1  */
    mcp3.digitalWrite(15, LOW);  /* and delete previous LED  */
    delay(1000);                /* Wait for one second delay */
  }
  digitalWrite(ENA1, HIGH); /* Disable stepstick1 motordriver */
  mcp1.writeGPIOAB(0x00);   /* Turn off all red LED 1..16     */
  mcp2.writeGPIOAB(0x00);   /* Turn off all red LED 17..32    */
  mcp3.writeGPIOAB(0x00);   /* Turn off all red LED 17..32    */
}

/* ---------------------------------------------------- */
/* oneturn turns 360 degrees, argument gives direction  */
/* ---------------------------------------------------- */
void AMotor::oneturn(boolean direction, int speed) {
  digitalWrite(ENA1,LOW); /* Enable stepstick1 motor driver */
  digitalWrite(DIR1,direction); /* HIGH=clockwise, LOW=CCW  */
  for(float x = 0; x < 1600; x++) { /* 1600 steps = 1turn */
    digitalWrite(STEP1,HIGH); 
    delayMicroseconds(speed); 
    digitalWrite(STEP1,LOW); 
    delayMicroseconds(speed);
  }
  digitalWrite(ENA1, HIGH); /* Disable stepstick1 motordriver */
}

/* ---------------------------------------------------- */
/* oneadjust moves from the current position            */
/* to the target position. position range 0..1600       */
/* ---------------------------------------------------- */
void AMotor::oneadjust(uint16_t target, int speed) {
  uint16_t steps = 0;
  m1move = true;
  digitalWrite(ENA1,LOW); /* Enable stepstick1 motor driver */
  if(m1cpos < m1tpos) {
    if(abs(m1cpos - m1tpos) < 800) {
      digitalWrite(DIR1,HIGH); /* run clockwise motor direction */
      steps = m1tpos - m1cpos; /* get required steps to target  */
    }
    else {                    /* the other way around is shorter */
      digitalWrite(DIR1,LOW); /* move counterclockwise direction */
      steps = 1600 - m1tpos + m1cpos;
    }
  }
  else {
    if((m1cpos - m1tpos) < 800) {
      digitalWrite(DIR1,LOW); /* counterclockwise  direction  */
      steps = m1cpos - m1tpos;
    }
    else {                     /* the other way around is shorter */
      digitalWrite(DIR1,HIGH); /* run clockwise motor direction   */
      steps = 1600 - m1cpos + m1tpos;      
    }
  }
  for(int x = 0; x < steps; x++) {
      digitalWrite(STEP1,HIGH);
      delayMicroseconds(speed);
      digitalWrite(STEP1,LOW); 
      delayMicroseconds(speed);
  }
  digitalWrite(ENA1,HIGH); /* Disable stepstick1 motordriver */
  m1move = false;
  m1cpos = m1tpos;
}

/* ---------------------------------------------------- */
/* oneled moves from the current motor position         */
/* one single LED back. The speed defines step time,    */
/* 500 = fast, 5000 = slow                              */
/* ---------------------------------------------------- */
void AMotor::oneled(int speed) {
  uint16_t steps = 34;
  m1move = true;
  digitalWrite(ENA1,LOW); /* Enable stepstick1 motordriver */
  digitalWrite(DIR1,HIGH); /* clockwise motor direction    */
  for(int x = 0; x < steps; x++) {
      digitalWrite(STEP1,HIGH);
      delayMicroseconds(speed);
      digitalWrite(STEP1,LOW); 
      delayMicroseconds(speed);
  }
  digitalWrite(ENA1,HIGH); /* Disable stepstick1 motordriver */
  m1move = false;
  m1cpos = m1cpos + 34;
}

/* ---------------------------------------------------- */
/* motor1home moves from the current motor position     */
/* back to starting point aligned with LED D1.          */
/* ---------------------------------------------------- */
void AMotor::sethome(int speed) {
  if(digitalRead(ESWI1) == 1) { /* motor is in home position */
    m1cpos = 0;                 /* reset the motor position  */
    exit;                       /* all done, exit function   */
  }

  m1move = true;
  digitalWrite(ENA1,LOW); /* Enable stepstick1 motor driver */

  if(m1cpos > 0) {          /* position is known, go back   */
    digitalWrite(DIR1,LOW); /* counterclockwise  direction  */
    uint16_t steps = m1cpos+10; /* +10 to really hit eswi1  */
    for(int x = 0; x < steps; x++) {
      digitalWrite(STEP1,HIGH);
      delayMicroseconds(speed);
      digitalWrite(STEP1,LOW); 
      delayMicroseconds(speed);
      if(digitalRead(ESWI1) == 1) { /* motor is now at home */
        m1cpos = 0;                 /* reset motor position */
        digitalWrite(ENA1,HIGH);    /* disable motor driver */
        m1move = false;
        exit;                       /* done, exit function  */
      }
    }
  }
  else {                  /* position is not known, go back */
    digitalWrite(DIR1, LOW);  /* counterclockwise direction */
    for(int x = 0; x < 1610; x++) {
      digitalWrite(STEP1,HIGH);
      delayMicroseconds(speed);
      digitalWrite(STEP1,LOW); 
      delayMicroseconds(speed);
      if(digitalRead(ESWI1) == 1) { /* if it found home pos */
        m1cpos = 0;                 /* reset motor position */
        digitalWrite(ENA1,HIGH);    /* disable motor driver */
        m1move = false;
        exit;                       /* done, exit function  */
      }
    }
  }
}

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
