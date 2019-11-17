/* ---------------------------------------------------- *
 * Suntracker2  mainboard-rev4 led.cpp   2019-10 @FM4DD *
 *                                                      *
 * This file has the functions to control the dualcolor *
 * zenith row on the mainboard, and the 48 LED ring on  *
 * the displayboard v2.1, connected to mainboard v2.1.  *
 * ---------------------------------------------------- */
#include <Arduino.h>           // Arduino default library
#include "Adafruit_MCP23017.h" // https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library
#include "globals.h"           // global variable definitions

/* ---------------------------------------------------- */
/* 8x IO Expanders for a total of 128 I/O ports         */
/* ---------------------------------------------------- */
Adafruit_MCP23017 mcp1;
Adafruit_MCP23017 mcp2;
Adafruit_MCP23017 mcp3;
Adafruit_MCP23017 mcp4;
Adafruit_MCP23017 mcp5;
Adafruit_MCP23017 mcp6;
Adafruit_MCP23017 mcp7;
Adafruit_MCP23017 mcp8;

/* ---------------------------------------------------- */
/* enable() initializes the 2 zenith row expanders, and */
/* set MCP23017 addr in mcp.begin(x) per addr pin value */
/* 0 = 0x20, 1 = 0x21, 2 = 0x22, 3 = 0x23, ... 7 = 0x27 */
/* Returns 0 on success, or I2C errcode on failure.     */
/* ---------------------------------------------------- */
uint8_t ledRow::enable() {
  uint8_t retcode = 0;
  Wire.beginTransmission(0x26);
  retcode = Wire.endTransmission();
  if (retcode != 0) return retcode;
  mcp7.begin(6);        /* Enable the MCP23017 module 6 */
  Wire.beginTransmission(0x27);
  retcode = Wire.endTransmission();
  if (retcode != 0) return retcode;
  mcp8.begin(7);        /* Enable the MCP23017 module 7 */
  /* Configure all MCP23017 ports in LED output mode    */
  for(int i=0; i<16; i++) {
    mcp7.pinMode(i, OUTPUT);
    mcp8.pinMode(i, OUTPUT);
    mcp7.digitalWrite(i, LOW);
    mcp8.digitalWrite(i, LOW);
  }
  return retcode;
}

/* ---------------------------------------------------- */
/* all_ledoff() turns off 16 LED, regardless of state   */
/* ---------------------------------------------------- */
void ledRow::all_ledoff() {
  mcp7.writeGPIOAB(0x00); /* expander6 all red LED off  */
  mcp8.writeGPIOAB(0x00); /* expander7 all green LED off*/
}

/* ----------------------------------------------------- */
/* Single LED walkthrough - Red: mcp6,7 port A,B         */
/* Runs a single red LED light around the circle.        */
/* ----------------------------------------------------- */
void ledRow::stepled_red(int millisec) {
  ledRow::all_ledoff();                  /* reset LEDs   */
  for(int i=0; i<16; i++) {              /* expander-6   */
    mcp7.digitalWrite(i, HIGH);          /* next LED on  */
    if(i>0) mcp7.digitalWrite(i-1, LOW); /* prev LED off */
    delay(millisec);                     /* speed in ms  */
  }
  mcp7.digitalWrite(15, LOW);            /* last LED off */
}
void ledRow::stepled_green(int millisec) {
  mcp8.digitalWrite(15, LOW);            /* last LED off */
  for(int i=0; i<16; i++) {              /* expander-7   */
    mcp8.digitalWrite(i, HIGH);          /* next LED on  */
    if(i>0) mcp8.digitalWrite(i-1, LOW); /* prev LED off */
    delay(millisec);                     /* speed in ms  */
  }
  mcp8.digitalWrite(15, LOW);            /* last LED off */
}

/* ---------------------------------------------------- */
/* lightcheck() LED on/off tests for all LED together.  */
/* Blink all ports. 0xFFFF lights both GPIOA and GPIOB  */
/* Setting 0xFF only lights A, 0x00FF lights up only B  */
/* ---------------------------------------------------- */
void ledRow::lightcheck() {
  mcp7.writeGPIOAB(0xFFFF);  /* expander1 red LED on    */
  delay(500);
  mcp7.writeGPIOAB(0x00);    /* expander1 red LED off   */
  mcp8.writeGPIOAB(0xFFFF);  /* expander2 green LED on  */
  delay(500);
  mcp8.writeGPIOAB(0x00);    /* expander2 green LED off */
}

/* ---------------------------------------------------- */
/* Set elevation LED. Calculates the sun elevation from */
/* zenith value, and adjusts down to 16-LED row value.  */
/* ---------------------------------------------------- */
void ledRow::set_eleled(float ze) {
  eled = (int8_t)round(((90-ze) / 5.625) * 10 / 10.0);
  if (eled != oldeled) {
    if (oldeled >= 0 && oldeled > eled) mcp8.digitalWrite(oldeled, LOW);
    if (eled >= 0) mcp8.digitalWrite(eled, HIGH);
    if (eled >= 1 && (mcp8.digitalRead(eled-1) == 0)) {
      for (uint8_t i = 0; i<eled; i++) mcp8.digitalWrite(i, HIGH);
    }
    display.eleled(oldeled, eled);
    oldeled = eled;
  }
}

/* ---------------------------------------------------- */
/* Set transit LED. Check if the daily transit altitude */
/* has changed. Since its rounded to the nearest degree */
/* it changes only every 2-3 days. The 16-LED row also  */
/* reduces resolution which further lowers LED changes. */
/* ---------------------------------------------------- */
void ledRow::set_transled() {
  if(transalt != oldtransalt) {
    tled = (int8_t)round(transalt / 5.625);
    if(oldtled != tled) {
      mcp7.digitalWrite(oldtled, LOW);
      mcp7.digitalWrite(tled, HIGH);
      oldtled = tled;
    }
    oldtransalt = transalt;
  }
}

/* ---------------------------------------------------- */
/* led_enable() initializes the 6 GPIO port expanders   */
/* Set MCP23017 addr in mcp.begin(x) per addr pin value */
/* 0 = 0x20, 1 = 0x21, 2 = 0x22, 3 = 0x23, ... 7 = 0x27 */
/* Returns 0 on success, or I2C errcode on failure.     */
/* ---------------------------------------------------- */
uint8_t ledRing::enable() {
  uint8_t retcode = 0;
  for (uint8_t i=0; i<7; i++) {
    Wire.beginTransmission(0x20 + i);
    retcode = Wire.endTransmission();
    if (retcode != 0) return retcode;
  }
  mcp1.begin(0);     /* Enable the MCP23017 module 1 */
  mcp2.begin(1);     /* Enable the MCP23017 module 2 */
  mcp3.begin(2);     /* Enable the MCP23017 module 3 */
  mcp4.begin(3);     /* Enable the MCP23017 module 4 */
  mcp5.begin(4);     /* Enable the MCP23017 module 5 */
  mcp6.begin(5);     /* Enable the MCP23017 module 6 */
  /* Configure all MCP23017 ports in LED output mode */
  for(int i=0; i<16; i++) {
    mcp1.pinMode(i, OUTPUT);
    mcp2.pinMode(i, OUTPUT);
    mcp3.pinMode(i, OUTPUT);
    mcp4.pinMode(i, OUTPUT);
    mcp5.pinMode(i, OUTPUT);
    mcp6.pinMode(i, OUTPUT);
    mcp1.digitalWrite(i, LOW);
    mcp2.digitalWrite(i, LOW);
    mcp3.digitalWrite(i, LOW);
    mcp4.digitalWrite(i, LOW);
    mcp5.digitalWrite(i, LOW);
    mcp6.digitalWrite(i, LOW);
  }
  return retcode;
}

/* ---------------------------------------------------- */
/* all_ledoff() turns off 48 LED, regardless of state   */
/* ---------------------------------------------------- */
void ledRing::all_ledoff() {
  mcp1.writeGPIOAB(0x00); /* expander1 all red   LED off */
  mcp2.writeGPIOAB(0x00); /* expander2 all red   LED off */
  mcp3.writeGPIOAB(0x00); /* expander3 all red   LED off */
  mcp4.writeGPIOAB(0x00); /* expander4 all green LED off */
  mcp5.writeGPIOAB(0x00); /* expander5 all green LED off */
  mcp6.writeGPIOAB(0x00); /* expander6 all green LED off */
}

/* ---------------------------------------------------- */
/* led_lightcheck() LED on/off tests for all 6 units:   */
/* Blink all ports. 0xFFFF lights both GPIOA and GPIOB  */
/* Setting 0xFF only lights A, 0x00FF lights up only B  */
/* ---------------------------------------------------- */
void ledRing::lightcheck() {
  mcp1.writeGPIOAB(0xFFFF);   /* MCP-1  1..16 red  on   */
  delay(500);
  mcp1.writeGPIOAB(0x00);     /* MCP-1  1..16 red  off  */
  mcp2.writeGPIOAB(0xFFFF);   /* MCP-2 17..32 red  on   */
  delay(500);
  mcp2.writeGPIOAB(0x00);     /* MCP-2 17..32 red  off  */
  mcp3.writeGPIOAB(0xFFFF);   /* MCP-3 33..48 red  on   */
  delay(500);
  mcp3.writeGPIOAB(0x00);     /* MCP-3 33..48 red  off  */
  mcp4.writeGPIOAB(0xFFFF);   /* MCP-4  1..16 green on  */
  delay(500);    
  mcp4.writeGPIOAB(0x00);     /* MCP-4  1..16 green off */
  mcp5.writeGPIOAB(0xFFFF);   /* MCP-5 17..32 green on  */
  delay(500);
  mcp5.writeGPIOAB(0x00);     /* MCP-5 17..32 green off */
  mcp6.writeGPIOAB(0xFFFF);   /* MCP-6 33..48 green  on */
  delay(500);
  mcp6.writeGPIOAB(0x00);     /* MCP-6 33..48 green off */
}

/* ----------------------------------------------------- */
/* Single LED walkthrough.Red: MCP-1..3 Green: MCP-4..6  */
/* Runs a single LED light around the 48-LED circle.     */
/* ----------------------------------------------------- */
void ledRing::stepled(uint8_t color, int millisec) {
  Adafruit_MCP23017 mcp_red[3]   = { mcp1, mcp2, mcp3 };
  Adafruit_MCP23017 mcp_green[3] = { mcp4, mcp5, mcp6 };
  
  ledRing::all_ledoff();                   /* reset LEDs */
  for(int i=0; i<3; i++) {                   /* MCP-1..3 */
    for(int j=0; j<16; j++) {               /* LED 1..16 */
      if(color == 1 ) {
        mcp_red[i].digitalWrite(j, HIGH);      /* LED on */
        if(j>0) 
          mcp_red[i].digitalWrite(j-1, LOW);  /* LED off */
      }
      else {
        mcp_green[i].digitalWrite(j, HIGH);   /* LED on  */
        if(j>0)
          mcp_green[i].digitalWrite(j-1, LOW); /* LED off */
      }
      delay(millisec);                    /* speed in ms  */
    } // end 0..15 LED
    if(color == 1 ) {
      mcp_red[i].digitalWrite(15, LOW);   /* last LED off */
    }
    else {
      mcp_green[i].digitalWrite(15, LOW); /* last LED off */
    } // end 0..15 LED
    delay(millisec);
  } // end MCP group
}

/* ------------------------------------------------------- */
/* Lightshow demonstration on a 48x 2-color LED ring       */
/* ------------------------------------------------------- */
void ledRing::lightshow(int millisec) {
  /* ------------------------------------------------------- */
  /* Pattern-1: light up one expander (16 LED), then remove  */
  /* one LED on either side until all LED are turned off.    */
  /* ------------------------------------------------------- */
  int decline[9] = { 0xFFFF, 0x7FFE, 0x3FFC, 0x1FF8, 0xFF0, 0x7E0, 0x3C0, 0x180, 0x0 };
  /* ------------------------------------------------------- */
  /* Pattern-2: Switch on every second LED on a expander and */
  /* and alternate the LED with it's neighbor LED.           */
  /* ------------------------------------------------------- */
  int alternate[2] = { 0xAAAA, 0x5555 };
  /* ------------------------------------------------------- */
  /* Pattern-3: In four steps light up 1, add 2, then add 3, */
  /* finally 4 LEDs: LED1-12-123-1234 on the 16 LED expander */
  /* ------------------------------------------------------- */
  int null2four[5] = { 0x0, 0x8888, 0xCCCC, 0xEEEE, 0xFFFF };
  /* ------------------------------------------------------- */
  /* Pattern-4: In eight steps light up 1, add 2, then add 3 */
  /* finally 8: 1-12-123-1234-12345-123456-1234567-12345678  */
  /* ------------------------------------------------------- */
  //int null2eight[9] = { 0x0, 0x8888, 0xCCCC, 0xEEEE, 0xFFFF };
  /* ------------------------------------------------------- */
  /* Pattern-5: Light up every 4th LED and move it forward.  */
  /* LED1-5-9-13, LED2-6-10-14, LED3-7-11-15, LED4-8-12-16   */
  /* ------------------------------------------------------- */
  int move4[4] = { 0x8888, 0x4444, 0x2222, 0x1111 };
  /* ------------------------------------------------------- */
  /* Pattern-6: Light up all 16 LED,turn one off, and move   */
  /* the one dark led from right to left side. We start with */
  /* 0xFFFF, loop (subtract), and stop at 0x7FFF.            */
  /* ------------------------------------------------------- */
 int move1off[17] = { 0xFFFF, 0xFFFE, 0xFFFD, 0xFFFB, 0xFFF7, 0xFFEF, 0xFFDF, 0xFFBF,
                      0xFF7F, 0xFEFF, 0xFDFF, 0xFBFF, 0xF7FF, 0xEFFF, 0xDFFF, 0xBFFF, 0x7FFF };
  /* ------------------------------------------------------- */
  /* Pattern7: In combination with Pattern-6, this moves an */
  /* alternate color in the dark spot, right to left side.   */
  /* ------------------------------------------------------- */
  int move1on[17] = { 0x0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100,
                    0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000 };

  all_ledoff();                             /* reset LEDs   */
  /* ------------------------------------------------------- */
  /* Pattern-1 on red LEDs                                   */
  /* ------------------------------------------------------- */
  for(uint8_t i = 0; i < 9; i++) {
    mcp1.writeGPIOAB(decline[i]);
    mcp2.writeGPIOAB(decline[i]);
    mcp3.writeGPIOAB(decline[i]);
    delay(millisec);
  }
  /* ------------------------------------------------------- */
  /* Reverse Pattern-1 on red LEDs, turn decline to increase */
  /* ------------------------------------------------------- */
  for(uint8_t i = 8; i > 0; i--) {
    mcp1.writeGPIOAB(decline[i]);
    mcp2.writeGPIOAB(decline[i]);
    mcp3.writeGPIOAB(decline[i]);
    delay(millisec);
  }
  /* ------------------------------------------------------- */
  /*  Pattern-1 on red LEDs                                  */
  /* ------------------------------------------------------- */
  for(uint8_t i = 0; i < 9; i++) {
    mcp1.writeGPIOAB(decline[i]);
    mcp2.writeGPIOAB(decline[i]);
    mcp3.writeGPIOAB(decline[i]);
    delay(millisec);
  }
  delay(millisec * 4);
  /* ------------------------------------------------------- */
  /*  Pattern-1 on green LEDs                                */
  /* ------------------------------------------------------- */
  for(uint8_t i = 0; i < 9; i++) {
    mcp4.writeGPIOAB(decline[i]);
    mcp5.writeGPIOAB(decline[i]);
    mcp6.writeGPIOAB(decline[i]);
    delay(millisec);
  }
  /* ------------------------------------------------------- */
  /* Reverse Pattern-1 on green LEDs, increase               */
  /* ------------------------------------------------------- */
  for(uint8_t i = 8; i > 0; i--) {
    mcp4.writeGPIOAB(decline[i]);
    mcp5.writeGPIOAB(decline[i]);
    mcp6.writeGPIOAB(decline[i]);
    delay(millisec);
  }
  /* ------------------------------------------------------- */
  /*  Pattern-1 on green LEDs                                */
  /* ------------------------------------------------------- */
  for(uint8_t i = 0; i < 9; i++) {
    mcp4.writeGPIOAB(decline[i]);
    mcp5.writeGPIOAB(decline[i]);
    mcp6.writeGPIOAB(decline[i]);
    delay(millisec);
  }
  delay(millisec * 4);
  /* ------------------------------------------------------- */
  /* Pattern-3: LED1-12-123-1234 for green LEDs              */
  /* ------------------------------------------------------- */
  for(uint8_t j = 0; j < 8; j++) {
    for(uint8_t i = 0; i < 5; i++) {
      mcp4.writeGPIOAB(null2four[i]);
      mcp5.writeGPIOAB(null2four[i]);
      mcp6.writeGPIOAB(null2four[i]);
      delay(millisec);
    }
  }
  all_ledoff();                           /* reset LEDs   */
  /* ------------------------------------------------------- */
  /* Pattern 5: move 1 in 4 LED for red LEDs               */
  /* ------------------------------------------------------- */
  for(uint8_t j = 0; j < 8; j++) {
      for(uint8_t i = 0; i < 5; i++) {
      mcp1.writeGPIOAB(move4[i]);
      mcp2.writeGPIOAB(move4[i]);
      mcp3.writeGPIOAB(move4[i]);
      delay(millisec);
    }
  }
  all_ledoff();
  /* ------------------------------------------------------- */
  /* Pattern 6 and 7: move 1 green LED with all red LEDs on  */
  /* ------------------------------------------------------- */
    for(uint8_t i = 0; i < 17; i++) {
      mcp1.writeGPIOAB(move1off[i]);
      mcp2.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1on[i]);
      mcp5.writeGPIOAB(move1on[i]);
      mcp6.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 0; i < 17; i++) {
      mcp1.writeGPIOAB(move1off[i]);
      mcp2.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1on[i]);
      mcp5.writeGPIOAB(move1on[i]);
      mcp6.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 0; i < 17; i++) {
      mcp1.writeGPIOAB(move1off[i]);
      mcp2.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1on[i]);
      mcp5.writeGPIOAB(move1on[i]);
      mcp6.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 16; i > 0; i--) {
      mcp1.writeGPIOAB(move1off[i]);
      mcp2.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1on[i]);
      mcp5.writeGPIOAB(move1on[i]);
      mcp6.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 16; i > 0; i--) {
      mcp1.writeGPIOAB(move1off[i]);
      mcp2.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1on[i]);
      mcp5.writeGPIOAB(move1on[i]);
      mcp6.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 16; i > 0; i--) {
      mcp1.writeGPIOAB(move1off[i]);
      mcp2.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1on[i]);
      mcp5.writeGPIOAB(move1on[i]);
      mcp6.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 16; i > 0; i--) {
      mcp6.writeGPIOAB(move1off[i]);
      mcp5.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1on[i]);
      mcp2.writeGPIOAB(move1on[i]);
      mcp1.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 16; i > 0; i--) {
      mcp6.writeGPIOAB(move1off[i]);
      mcp5.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1on[i]);
      mcp2.writeGPIOAB(move1on[i]);
      mcp1.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 16; i > 0; i--) {
      mcp6.writeGPIOAB(move1off[i]);
      mcp5.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1on[i]);
      mcp2.writeGPIOAB(move1on[i]);
      mcp1.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 0; i < 17; i++) {
      mcp6.writeGPIOAB(move1off[i]);
      mcp5.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1on[i]);
      mcp2.writeGPIOAB(move1on[i]);
      mcp1.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 0; i < 17; i++) {
      mcp6.writeGPIOAB(move1off[i]);
      mcp5.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1on[i]);
      mcp2.writeGPIOAB(move1on[i]);
      mcp1.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
    for(uint8_t i = 0; i < 17; i++) {
      mcp6.writeGPIOAB(move1off[i]);
      mcp5.writeGPIOAB(move1off[i]);
      mcp4.writeGPIOAB(move1off[i]);
      mcp3.writeGPIOAB(move1on[i]);
      mcp2.writeGPIOAB(move1on[i]);
      mcp1.writeGPIOAB(move1on[i]);
      delay(millisec);
    }
  all_ledoff();
}

void ledRing::set_hdgled() {
  /* ------------------------------------------------- */
  /* Set heading LED                                   */
  /* ------------------------------------------------- */
  if (hled != oldhled) {
    if(oldhled < 16) mcp1.digitalWrite(oldhled, LOW);
    else if(oldhled < 32) mcp2.digitalWrite((oldhled-16), LOW);
    else if(oldhled < 48) mcp3.digitalWrite((oldhled-32), LOW);
    if(hled > 31) mcp3.digitalWrite((hled-32), HIGH);
    else if(hled > 15) mcp2.digitalWrite((hled-16), HIGH);
    else if(hled < 16) mcp1.digitalWrite(hled, HIGH);
    display.hdgled(oldhled, hled);
    oldhled = hled;
  }
}

void ledRing::set_aziled() {
  /* ------------------------------------------------- */
  /* Set azimuth LED                                   */
  /* ------------------------------------------------- */
  if (aled != oldaled) {
    if(oldaled < 16) mcp4.digitalWrite(oldaled, LOW);
    else if(oldaled < 32) mcp5.digitalWrite((oldaled-16), LOW);
    else if(oldaled < 48) mcp6.digitalWrite((oldaled-32), LOW);
    if(aled > 31) mcp6.digitalWrite((aled-32), HIGH);
    else if(aled > 15) mcp5.digitalWrite((aled-16), HIGH);
    else if(aled < 16) mcp4.digitalWrite(aled, HIGH);
    display.aziled(oldaled, aled);
    oldaled = aled;
  }
}
void ledRing::set_riseled() {
  /* ------------------------------------------------- */
  /* Set sunrise LED                                   */
  /* ------------------------------------------------- */
  if (rled != oldrled) {
    if(oldrled < 16) mcp1.digitalWrite(oldrled, LOW);
    else if(oldrled < 32) mcp2.digitalWrite((oldrled-16), LOW);
    else if(oldrled < 48) mcp3.digitalWrite((oldrled-32), LOW);
    if(rled > 31) mcp3.digitalWrite((rled-32), HIGH);
    else if(rled > 15) mcp2.digitalWrite((rled-16), HIGH);
    else if(rled < 16) mcp1.digitalWrite(rled, HIGH);
    oldrled = rled;
  }
}
void ledRing::set_downled() {
  /* ------------------------------------------------- */
  /* Set sunset LED                                    */
  /* ------------------------------------------------- */
  if (sled != oldsled) {
    if(oldsled < 16) mcp1.digitalWrite(oldsled, LOW);
    else if(oldsled < 32) mcp2.digitalWrite((oldsled-16), LOW);
    else if(oldsled < 48) mcp3.digitalWrite((oldsled-32), LOW);
    if(sled > 31) mcp3.digitalWrite((sled-32), HIGH);
    else if(sled > 15) mcp2.digitalWrite((sled-16), HIGH);
    else if(sled < 16) mcp1.digitalWrite(sled, HIGH);
    oldsled = sled;
  }
}
