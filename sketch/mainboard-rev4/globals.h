#include "Adafruit_MCP23017.h" // https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library
#include "Adafruit_GFX.h"      // Adafruit graphics library
#include "Adafruit_ILI9341.h"  // driver for Adafruit 2.8 TFT display
#include "U8g2_for_Adafruit_GFX.h" // U8G2 font extension library
#include "LSM303.h"            // https://github.com/pololu/lsm303-arduino
#include "uRTCLib.h"           // https://github.com/Naguissa/uRTCLib
#include <Wire.h>              // Arduino default 12C libary
#include <Arduino.h>           // Arduino default library
#include <SPI.h>               // Arduino default SPI library
#include <SD.h>                // Arduino default SD library
#include "led.h"               // LED code Zenith row and displayboard
#include "tft.h"               // TFT display functions
#include "motor.h"             // Stepper motor control
#include "xbee.h"              // XBee radio module control
/* ------------------------------------------------- */
/* mainboard pin assignments                         */
/* ------------------------------------------------- */
#define DIP1  5     // GPIO pin dipswitch-1 on GPIO-5
#define DIP2  4     // GPIO pin dipswitch-2 on GPIO-4
#define PUSH1 2     // GPIO pin pushbutton-1 on GPIO-2
#define PUSH2 3     // GPIO pin pushbutton-2 on GPIO-3
#define ENA1  19    // GPIO pin stepper motor-1 enable
#define STEP1 20    // GPIO pin stepper motor-1 step
#define DIR1  21    // GPIO pin stepper motor-1 direction
#define ESWI1 7     // GPIO pin stepper motor-1 endswich
#define ENA2  15    // GPIO pin stepper motor-2 enable
#define STEP2 16    // GPIO pin stepper motor-2 step
#define DIR2  17    // GPIO pin stepper motor-2 direction
#define ESWI2 6     // GPIO pin stepper motor-2 endswich

/* ------------------------------------------------- */
/* SLOW and FAST set motor speed if stepper is used  */
/* ------------------------------------------------- */
#define SLOW 5000
#define FAST 500

/* ------------------------------------------------- */
/* Mainboard 16-Led Row and DisplayBoard 48-Led Ring */
/* ------------------------------------------------- */
extern ledRow row;
extern ledRing ring;
extern AMotor amotor;
extern ZMotor zmotor;
extern Adafruit_ILI9341 tft;
extern U8G2_FOR_ADAFRUIT_GFX u8g2_tft;
extern Display display;
extern Sd2Card card;
extern SdVolume volume;
extern SdFile root;
/* ------------------------------------------------- */
/* Magnetic field Sensor                             */
/* ------------------------------------------------- */
extern LSM303 compass;
/* ------------------------------------------------- */
/* DS3231 Precision RTC clock                        */
/* ------------------------------------------------- */
extern uRTCLib rtc;
/* ---------------------------------------------------- */
/* 8x IO Expanders for a total of 128 I/O ports         */
/* ---------------------------------------------------- */
extern Adafruit_MCP23017 mcp1;
extern Adafruit_MCP23017 mcp2;
extern Adafruit_MCP23017 mcp3;
extern Adafruit_MCP23017 mcp4;
extern Adafruit_MCP23017 mcp5;
extern Adafruit_MCP23017 mcp6;
extern Adafruit_MCP23017 mcp7;
extern Adafruit_MCP23017 mcp8;

extern const PROGMEM char prgver[];
extern int8_t eled;
extern int8_t oldeled;
extern int8_t tled;
extern int8_t oldtled;
extern int8_t hled;
extern int8_t oldhled;
extern int8_t aled;
extern int8_t oldaled;
extern int8_t rled;
extern int8_t oldrled;
extern int8_t sled;
extern int8_t oldsled;
extern uint8_t risehour;
extern uint8_t risemin;
extern uint8_t transithour;
extern uint8_t transitmin;
extern uint8_t sethour;
extern uint8_t setmin;
extern float heading;
extern float latitude;
extern float longitude;
extern float altitude;
extern float speed;
extern uint8_t satellites;
extern char gpsStr[15];
extern int16_t hdg;
extern double azimuth;
extern double zenith;
extern uint16_t riseaz;
extern int16_t transalt;
extern int16_t oldtransalt;
extern uint16_t setaz;
extern boolean daylight;
extern boolean olddaylight;     // flag to detect moment of sunrise/set change
extern char lineStr[41];        // display output buffer, 80-chars + \0
extern char aledStr[5];         // display azimuth LED control "-C01"
extern char hledStr[5];         // display heading LED control "-B01"
extern char eledStr[5];         // display elevation LED control "-G01"
extern char riseStr[18];        // display string "Sunrise 05:40 096"
extern char setStr[18];         // display string "Sunset  17:19 264"
extern char binfile[13];        // daily sun data file yyyymmdd.bin
extern char srsfile[13];        // sunrise/sunset file srs-yyyy.bin
extern const double mdecl;      // local magnetic declination
extern uint8_t dippos1;         // dipswitch 1 position (extra tests)
extern uint8_t dippos2;         // dipswitch 2 position (motor enable)
extern uint16_t m1cpos;         // stepper motor-1 current step position (0..1599)
extern uint16_t m1tpos;         // stepper motor-1 target step position (0..1599)
extern uint16_t m2cpos;         // stepper motor-1 current step position (0..640)
extern uint16_t m2tpos;         // stepper motor-1 target step position (0..640)
extern boolean m1move;          // stepper motor-1 "move in progress" flag
extern boolean m2move;          // stepper motor-2 "move in progress" flag
extern uint8_t opmode;          // operations mode: 0 = normal, 1 = demo
extern uint8_t testmode;        // selftest mode: 0 = normal, 2 = extended
//extern uint16_t day_azi[1440];  // daily azimuth angle 0..360, 1min interval, 2.88KB
//extern int16_t day_ele[1440];   // daily elevation angle -90 ..90, 1min interval 2.88KB
extern float day_azi[1440];     // daily azimuth angle 0..360, 1min interval, 6KB
extern float day_ele[1440];     // daily elevation angle -90 ..90, 1min interval 6KB
