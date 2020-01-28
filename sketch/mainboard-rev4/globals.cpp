#include <Arduino.h>          // Arduino default library
#include "globals.h"

const PROGMEM char prgver[] = "Version 20191130-01";
int8_t eled = 0;             // elevation LED (green)
int8_t oldeled = 17;         // 17 > max value 15, indicates "init"
int8_t tled = 0;             // max elevation (transit) LED (red)
int8_t oldtled = 17;         // 17 > max value 15, indicates "init"
int8_t hled = 0;             // heading LED (red)
int8_t oldhled = 49;         // 49 > max value 48, indicates "init"
int8_t aled = 0;             // azimuth LED (green)
int8_t oldaled = 49;         // 49 > max value 48, indicates "init"
int8_t rled = 0;             // sunrise LED (red)
int8_t oldrled = 49;         // 49 > max value 48, indicates "init"
int8_t sled = 0;             // sunset LED (red)
int8_t oldsled = 49;         // 49 > max value 48, indicates "init"
uint8_t risehour = 0;        // hour of sunrise
uint8_t risemin = 0;         // minute of sunrise
uint8_t transithour = 0;     // peak altitude hour
uint8_t transitmin = 0;      // peak altitude minute
uint8_t sethour = 0;         // sunset hour
uint8_t setmin = 0;          // sunset minute
float heading;               // North heading
float latitude;              // GPS latitude
float longitude;             // GPS longitude
float altitude;              // GPS altitude
float speed;                 // GPS speed
uint8_t satellites;          // GPS satellites
int16_t hdg;                 // North heading, rounded to 0..360
double azimuth;              // sun azimuth angle, north->eastward
double zenith;               // zenith angle, substract from 90 to get altitude
uint16_t riseaz;             // sunrise azimuth, rounded to full degrees
int16_t transalt;            // transit altitude - solar noon peak altitude
int16_t oldtransalt = 91;    // detect changes to transit altitude above
uint16_t setaz;              // sunset azimuth, rounded to full degrees
boolean daylight;            // daylight flag, false = night
boolean olddaylight;         // flag to detect moment of sunrise/set change
char lineStr[41];            // display output buffer, 80-chars + \0
char aledStr[5];             // display azimuth LED control "-C01"
char hledStr[5];             // display heading LED control "-B01"
char eledStr[5];             // display elevation LED control "-G01"
char binfile[13];            // daily sun data file yyyymmdd.bin
char srsfile[13];            // sunrise/sunset file srs-yyyy.bin
const double mdecl = -7.583; // local magnetic declination
uint8_t dippos1 = 0;         // dipswitch 1 position (extra tests)
uint8_t dippos2 = 0;         // dipswitch 2 position (motor enable)
uint16_t m1cpos = 0;         // stepper motor-1 current step position (0..1599)
uint16_t m1tpos = 0;         // stepper motor-1 target step position (0..1599)
uint16_t m2cpos = 0;         // stepper motor-2 current step position (0..640)
uint16_t m2tpos = 0;         // stepper motor-2 target step position (0..640)
boolean m1move = false;      // stepper motor-1 "move in progress" flag
boolean m2move = false;      // stepper motor-2 "move in progress" flag
uint8_t opmode = 0;          // operations mode: 0=init, 1=track, 2=demo, 3=setup
uint8_t testmode = 0;        // selftest mode: 0 = normal, 2 = extended
float day_azi[1440];         // daily azimuth angle 0..360, 1min interval, 6KB
float day_ele[1440];         // daily elevation angle -90 ..90, 1min interval 6KB
