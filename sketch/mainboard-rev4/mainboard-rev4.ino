/* ------------------------------------------------- *
 * Suntracker2  mainboard-rev4  Oct 2019 @ FM4DD     *
 *                                                   * 
 * This code controls Suntracker2 mainboard v2.1 and *
 * displayboard v2.1 for dual angle zenith + azimuth *
 * tracking. Requires suncalc v1.2 generated solar   *
 * position data stored on the MKRZERO MicroSD card. *
 * ------------------------------------------------- */
#include "Arduino_MKRGPS.h" // Arduino GPS library
#include <avr/dtostrf.h>    // dtostrf float conversion
#include "globals.h"        // global variable definitions
#include "demo.h"           // demo mode function

// write board settings and serial to sdcard file
struct s2r4_settings {
  byte serial;
  byte field2;
  char name[10];
};

/* ------------------------------------------------- */
/* Object class instances                            */
/* ------------------------------------------------- */
ledRow row;               // 16-LED eleveation row
ledRing ring;             // 48-LED azimuth ring
AMotor amotor;            // azimuth motor
ZMotor zmotor;            // azimuth motor
Demo demo;                // demo mode
Display display;          // TFT display
Sd2Card card;             // SD card
SdVolume volume;          // SD partition
SdFile root;              // SD filesystem
LSM303 compass;           // Magnetic field Sensor
uRTCLib rtc;              // DS3231 Precision RTC clock
XBee xbee;                // XBee S2C wireless comms
/* ------------------------------------------------- */
/* DEBUG enables debug output to the serial monitor  */
/* ------------------------------------------------- */
//#define DEBUG

void setup() {
#ifdef DEBUG
  /* ------------------------------------------------- */
  /* Enable Serial Debug output                        */
  /* ------------------------------------------------- */
  Serial.begin(9600);
  while (!Serial);   // wait for serial port to connect.
  Serial.println("Serial Debug Start");
#endif
  /* ------------------------------------------------- */
  /* Enable TFT display, clear screen, set black bgnd  */
  /* ------------------------------------------------- */
  digitalWrite(TFT_BL, LOW);  // Set LOW before OUTPUT
  pinMode(TFT_BL, OUTPUT);    // to prevent screen on
  display.enable();
  digitalWrite(TFT_BL, HIGH);
  display.bmpDraw("S2R4IMG.BMP", 0, 0);
  display.show_ver();
  delay(8000);
  display.header();
  display.opmode(opmode);
  delay(1000);
  /* ------------------------------------------------- */
  /* Set dipswitch GPIO ports as input                 */
  /* ------------------------------------------------- */
  pinMode(DIP1, INPUT);
  pinMode(DIP2, INPUT);
  dippos1 = digitalRead(DIP1);
  dippos2 = digitalRead(DIP2);    
  /* ------------------------------------------------- */
  /* Set pushbutton GPIO ports as input                */
  /* ------------------------------------------------- */
  pinMode(PUSH1, INPUT);
  pinMode(PUSH2, INPUT);
  /* ------------------------------------------------- */
  /* Pushbutton 1 (-) at startup triggers demo LOW=ON  */
  /* demo mode switches main loop from min to sec exec */
  /* ------------------------------------------------- */
  if(digitalRead(PUSH1) == LOW) opmode = 2; // demo mode
  /* ------------------------------------------------- */
  /* Pushbutton 2 (-) at startup for extended selftest */
  /* LOW=ON. Ext. Selftest runs extra checks at bootup */
  /* ------------------------------------------------- */
  if(digitalRead(PUSH2) == LOW) testmode = 1; // extended
  /* ------------------------------------------------- */
  /* Enable I2C bus                                    */
  /* ------------------------------------------------- */
  Wire.begin();
  /* ------------------------------------------------- */
  /* Pushbutton 2 at boot sets testmode. 1 = extended  */
  /* ------------------------------------------------- */
  if(testmode == 1) {
    u8g2_tft.drawStr(0, 100, "Modules Test:");
    /* ------------------------------------------------- */
    /* Identify I2C devices                              */
    /* 0x19 = LSM303DLHC Accel (Adafruit product: 1120)  */
    /* 0x1D = LSM303D (Sunhayato MM-TXS05 module)        */
    /* 0x1E = LSM303DLHC Mag (Adafruit product: 1120)    */
    /* 0x20 .. 0x27 = MCP23017 1-8                       */
    /* 0x42 = Arduio MKR GPS Shield on I2C               */
    /* 0x68 = DS3231 RTC                                 */
    /* ------------------------------------------------- */
    const int size = 12; // # of I2C devices to check for
    byte addr[size] = { 0x1D, 0x1E, 0x20, 0x21, 0x22, 0x23, 
                        0x24, 0x25, 0x26, 0x27, 0x42, 0x68 };
    int i;
    int line = 100;
    byte error;
    for(i = 0; i<size; i++ ) {
      /* ------------------------------------------------- */
      /* The i2c_scanner uses the Write.endTransmisstion   */
      /* return value to see if device exists at the addr  */
      /* ------------------------------------------------- */
      snprintf(lineStr, sizeof(lineStr), "I2C check %02x: ", addr[i]);
      line = line + 20;
      if (line > 239) {
        display.wipe();
        line = 100; 
      }
      u8g2_tft.drawStr(0, line, lineStr);
      Wire.beginTransmission(addr[i]);
      error = Wire.endTransmission();

      if (error == 0){
        snprintf(lineStr, sizeof(lineStr), "%02x Response OK", addr[i]);
        u8g2_tft.drawStr(150, line, lineStr);
      }
      else if (error==4) {
        snprintf(lineStr, sizeof(lineStr), "%02x Error      ", addr[i]);
        u8g2_tft.drawStr(150, line, lineStr);
      }
      else {
        snprintf(lineStr, sizeof(lineStr), "%02x Not Found  ", addr[i]);
        u8g2_tft.drawStr(150, line, lineStr);
      }
      delay(1000);
    }
    delay(2000);
    display.wipe();
  } // end testmode = 1, skip extended selftest
  /* ------------------------------------------------- */
  /* Enable the SD card module                         */
  /* ------------------------------------------------- */
  u8g2_tft.drawStr(0, 93, "Init SD card:");
  if (!card.init(SPI_HALF_SPEED, SDCARD_SS_PIN)) {
    u8g2_tft.drawStr(150, 93, "SD card FAIL  ");
    while(1);
  } else {
    switch (card.type()) {
      case SD_CARD_TYPE_SD1:
        Serial.println("SD1");
        u8g2_tft.drawStr(150, 93, "SD1 card OK   ");
        break;
      case SD_CARD_TYPE_SD2:
        u8g2_tft.drawStr(150, 93, "SD2 card OK   ");
        break;
      case SD_CARD_TYPE_SDHC:
        u8g2_tft.drawStr(150, 93, "SDHC card OK  ");
        break;
      default:
        u8g2_tft.drawStr(150, 93, "Unknown card  ");
    }
  }
  SD.begin(SDCARD_SS_PIN); 
  delay(500);
  u8g2_tft.drawStr(150, 93, "Read file dset.txt.");
  File dset = SD.open("DSET.TXT");
  int i = 0;
  if(dset) {
    unsigned long fsize = dset.size();
    snprintf(lineStr, sizeof(lineStr), "DSET.TXT %ld bytes OK", fsize);
    u8g2_tft.drawStr(150, 93, lineStr);
    delay(2000);
//while (dset.available()) {
//        dset.readStringUntil(':').toCharArray(lineStr,15);
//        u8g2_tft.drawStr(0, 100+(i*20), lineStr);
//        dset.read(lineStr, 1); // skip over one space
//        dset.readStringUntil('\n').toCharArray(lineStr,15);
//        u8g2_tft.drawStr(150, 100+(i*20), lineStr);
//        delay(1000);
//        i++;
//      }
    } else {
      u8g2_tft.drawStr(150, 93, "Fail open dset.txt");
    }
    dset.close();
  /* ------------------------------------------------- */
  /* Enable the DS3231 RTC clock module                */
  /* ------------------------------------------------- */
  u8g2_tft.drawStr(0, 111, "Init DS3231:");
  Wire.beginTransmission(0x68);
  if (Wire.endTransmission() == 0) {
    rtc.set_rtc_address(0x68);
    rtc.set_model(URTCLIB_MODEL_DS3231);
    /* ------------------------------------------------- */
    /* To set the initial time, enable the line below    */
    /* set(sec,min,hour,dayOfWeek,dayOfMonth,month,year) */
    /* ------------------------------------------------- */
    //rtc.set(0, 3, 18, 1, 4, 11, 19);
    rtc.refresh();
    snprintf(lineStr, sizeof(lineStr), "20%02d/%02d/%02d %02d:%02d",
    rtc.year(),rtc.month(),rtc.day(),rtc.hour(),rtc.minute());
  }
  else {
    snprintf(lineStr, sizeof(lineStr), "No RTC I2C 0x68");
  }
  u8g2_tft.drawStr(150, 111, lineStr);
  delay(1500);
  /* ------------------------------------------------- */
  /* Enable the LSM303 Compass sensor module           */
  /* 1. Sunhayato MM-TXS05 SA0 default I2C addr (0x1D) */
  /* compass.init(LSM303::device_D,LSM303::sa0_high);  */
  /* 2. Adafruit LSM303DLHC module (PROD-ID 1120) use: */
  /* compass.init(LSM303::device_DLHC,LSM303::sa0_high);*/
  /* init() without args tries to determine chip type  */
  /* ------------------------------------------------- */
  boolean mag_found = false;
  u8g2_tft.drawStr(0, 129, "Init Compass:");
  mag_found = compass.init();
  if(mag_found) {
    if(compass.getDeviceType() == 3) u8g2_tft.drawStr(150, 129, "SH MM-TXS05");
    else 
      if(compass.getDeviceType() == 2) u8g2_tft.drawStr(150, 129, "ADA-LSM303");
    compass.enableDefault();
    /* ------------------------------------------------- */
    /* LSM303D Calibration values, see Calibrate example */
    /* ------------------------------------------------- */
    //compass.m_min = (LSM303::vector<int16_t>){-2246, -1776, -4034};
    //compass.m_max = (LSM303::vector<int16_t>){+2463, +2870,  +822};
    // Board 2 - Adafruit SLM303DLHC
    compass.m_min = (LSM303::vector<int16_t>){-595, -511, -445};
    compass.m_max = (LSM303::vector<int16_t>){+589, +597, +573};
    u8g2_tft.drawStr(257, 129, "Init OK");
    /* ------------------------------------------------- */
    /* Aquire compass sensor data                        */
    /* ------------------------------------------------- */
    compass.read();
    if(compass.getDeviceType() == 3) heading = compass.heading() + mdecl;
    if(compass.getDeviceType() == 2) heading = compass.heading((LSM303::vector<int>){1,0,0}) + mdecl;
    if(heading < 0) heading = 360.0 + heading;
    /* ------------------------------------------------- */
    /* print North heading to TFT                        */
    /* ------------------------------------------------- */
    dtostrf(heading,6,4,lineStr);
    u8g2_tft.drawStr(257,129, lineStr);
  }
  else {
    u8g2_tft.drawStr(150, 129, "LSM303 Not Found!");
  }
  delay(1500);
  /* ------------------------------------------------- */
  /* Enable MKRGPS with I2C DDC protocol               */
  /* ------------------------------------------------- */
  u8g2_tft.drawStr(0, 147, "Init GPS I2C:");
  Wire.beginTransmission(0x42);
  if (Wire.endTransmission() == 0) {
    GPS.begin(GPS_MODE_I2C);
    /* ------------------------------------------------- */
    /* Aquire GPS sensor data                            */
    /* ------------------------------------------------- */
    if (GPS.available()) {
      latitude   = GPS.latitude();
      longitude  = GPS.longitude();
      altitude   = GPS.altitude();
      speed      = GPS.speed();
      satellites = GPS.satellites();
    }
    char latStr[16];
    String(latitude, 8).toCharArray(latStr, 8);
    char lonStr[16];
    String(longitude, 8).toCharArray(lonStr, 8);
    snprintf(lineStr, sizeof(lineStr), "%d Satellites", satellites);
    u8g2_tft.drawStr(150, 147, lineStr);
    snprintf(lineStr, sizeof(lineStr), "LAT: %s", latStr);
    u8g2_tft.drawStr(0, 165, lineStr);
    snprintf(lineStr, sizeof(lineStr), "LON: %s", lonStr);
    u8g2_tft.drawStr(150, 165, lineStr);
  }
  else {
    u8g2_tft.drawStr(150, 147, "No GPS Shield 0x42");
  }
  delay(500);
  /* ------------------------------------------------- */
  /* Test XBee S2C wireless commuication module        */
  /* ------------------------------------------------- */
  u8g2_tft.drawStr(0, 183, "Init XBee S2C:");
  if(xbee.enable() == true) {
    u8g2_tft.drawStr(150, 183, "Module OK ");
    if(xbee.getstatus() == true) {
      if(xbee.getassociation() == 0)
        snprintf(lineStr, sizeof(lineStr),
          "Network %d ONLINE", xbee.getoper_panid());
      u8g2_tft.drawStr(150, 183, lineStr);
    }
  }
  else
    u8g2_tft.drawStr(150, 183, "Module N/A");
  /* ------------------------------------------------- */
  /* Test mainboard zenith row IO expanders            */
  /* ------------------------------------------------- */
  u8g2_tft.drawStr(0, 201, "Init LED Row:");
  if(row.enable() == 0) {
    u8g2_tft.drawStr(150, 201, "I2C 0x26/27 OK ");
    /* ----------------------------------------------- */
    /* Test LED row                                    */
    /* ----------------------------------------------- */
    u8g2_tft.drawStr(0, 219, "Test LED Row: ");
    delay(500);
    u8g2_tft.drawStr(150, 219, "Zenith Row RED  ");
    row.stepled_red(50);
    u8g2_tft.drawStr(150, 219, "Zenith Row GREEN");
    row.stepled_green(50);
    u8g2_tft.drawStr(150, 219, "Zenith Row ALL  ");
    row.lightcheck();
    u8g2_tft.drawStr(150, 219, "Zenit Row done  ");
  }
  else
    u8g2_tft.drawStr(150, 201, "I2C 0x26/27 ERR");
  delay(500);
  
  /* ------------------------------------------------- */
  /* Test if displayboard is connected                 */
  /* ------------------------------------------------- */
  u8g2_tft.drawStr(0, 237, "Init LED Ring:");
  if(ring.enable() == 0) {
    u8g2_tft.drawStr(150, 237, "I2C 0x20-25 OK ");
    /* ----------------------------------------------- */
    /* Test LED ring                                   */
    /* ----------------------------------------------- */
    u8g2_tft.drawStr(0, 237, "Test LED Ring:");
    u8g2_tft.drawStr(150, 237, "                ");
    u8g2_tft.drawStr(150, 237, "Lightshow ON"); 
    ring.lightshow(40);
  }
  else
    u8g2_tft.drawStr(150, 237, "I2C 0x20-25 ERR");
  delay(3500);
  display.wipe();
  /* ------------------------------------------------- */
  /* 2nd dip switch enables the motor function, LOW=ON */
  /* ------------------------------------------------- */
  if(dippos2 == LOW) {
    amotor.enable();
    zmotor.enable();
    /* ----------------------------------------------- */
    /* Motor1 testing                                  */
    /* ----------------------------------------------- */
    u8g2_tft.drawStr(0, 100, "Test Stepper-1:");
    amotor.sethome(SLOW); /* bring motor home if reqrd */
    delay(500);
    zmotor.demoturn(2);   /* test motor for 2x times   */
    u8g2_tft.drawStr(150, 100, "Complete");
    delay(1000);
    m1cpos = 0;    /* Set motor1 current position = D1 */
    m1tpos = 0;    /* Set motor1 target position = D1  */
    m2cpos = 0;    /* Set motor1 current position = D1 */
    m2tpos = 0;    /* Set motor1 target position = D1  */
  }
  /* ------------------------------------------------- */
  /* Read sunrise sunset time from SRS-<YYYY>.BIN file */
  /* ------------------------------------------------- */
  snprintf(srsfile, sizeof(srsfile), "SRS-20%02d.BIN", rtc.year());
  get_suntime(srsfile);
  /* ------------------------------------------------- */
  /* Set daily data file name per RTC current date     */
  /* ------------------------------------------------- */
  snprintf(binfile, sizeof(binfile), "20%02d%02d%02d.BIN", 
             rtc.year(),rtc.month(),rtc.day());
  /* ------------------------------------------------- */
  /* Check for demo mode                               */
  /* ------------------------------------------------- */
  if(opmode == 2) {
    /* ----------------------------------------------- */
    /* If demo mode was requested, set TFT mode state  */
    /* ----------------------------------------------- */
    display.wipe();
    display.opmode(opmode);
    demo.run();
  }
  delay(3000);
  /* ------------------------------------------------- */
  /* Intitialize azimuth/elevation position from file  */
  /* ------------------------------------------------- */
  rtc.refresh();
  solar_position(binfile);
  /* ------------------------------------------------- */
  /* Clear TFT section below header                    */
  /* ------------------------------------------------- */
  display.wipe();
  opmode = 1;
  display.opmode(opmode);
  display.show_date();     // show date
  display.show_rise_set(); // show sunrise/sunset
  display.show_transit();  // show transit
  if(olddaylight == daylight) olddaylight = !olddaylight;
  display.daysymbol();
} // end setup

void loop() {
  /* ------------------------------------------------- */
  /* Aquire time, every new minute read solar position */
  /* ------------------------------------------------- */
  rtc.refresh();
  if(rtc.second() == 0) {
    if(rtc.minute() == 0) {
      display.show_date();     // refresh date
      snprintf(binfile, sizeof(binfile), "20%d%02d%02d.BIN", 
                rtc.year(),rtc.month(),rtc.day());
      if(rtc.day() == 1) {
        snprintf(srsfile, sizeof(srsfile), "SRS-20%d.BIN", 
                rtc.year());
      }
      get_suntime(srsfile);
      display.show_rise_set(); // refresh sunrise/sunset
      display.show_transit();  // refresh transit
    }
    solar_position(binfile);
    display.daysymbol();
  }
  /* ------------------------------------------------- */
  /* Show Azimuth and sunrise/sunset on TFT            */
  /* ------------------------------------------------- */
  display.show_azimuth(azimuth);
  /* ------------------------------------------------- */
  /* Show Elevation on TFT, add leading + for daytime  */
  /* ------------------------------------------------- */
  display.show_elevation(zenith);
  /* ------------------------------------------------- */
  /* Show time on TFT                                  */
  /* ------------------------------------------------- */
  snprintf(lineStr, sizeof(lineStr), "Time: %02d:%02d:%02d",
  rtc.hour(),rtc.minute(),rtc.second());
  display.show_time(lineStr);
  /* ------------------------------------------------- */
  /* Aquire GPS sensor data                            */
  /* ------------------------------------------------- */
  // this hangs if there is no gps installed
  //if (GPS.available()) {
  //  latitude   = GPS.latitude();
  //  longitude  = GPS.longitude();
  // altitude   = GPS.altitude();
  //  speed      = GPS.speed();
  //  satellites = GPS.satellites();
  //}
  /* ------------------------------------------------- */
  /* Aquire compass sensor data                        */
  /* ------------------------------------------------- */
  compass.read();
  
  /* ------------------------------------------------- */
  /* If the sensor board orientation is unaligned, set */
  /* compass.heading((LSM303::vector<int>){0,1,0}) for */
  /* pointing along the Y-axis for example. Remove if  */
  /* the X-axis is the point of reference on a LSM303D */
  /* ------------------------------------------------- */
  /* Because magnetic north != real north, we adjust:  */
  /* Calculate magnetic declination and convergence    */
  /* to determine true North from magnetic North       */ 
  /* https://www.ngdc.noaa.gov/geomag/WMM/soft.shtml   */
  /* ------------------------------------------------- */
  if(compass.getDeviceType() == 3) heading = compass.heading() + mdecl;
  if(compass.getDeviceType() == 2) heading = compass.heading((LSM303::vector<int>){1,0,0}) + mdecl;
  if(heading < 0) heading = 360.0 + heading;
  /* ------------------------------------------------- */
  /* print North heading to TFT                        */
  /* ------------------------------------------------- */
  display.show_heading();
  /* ------------------------------------------------- */
  /* Convert heading angle to LED range value (0..48)  */   
  /* ------------------------------------------------- */
  hled = (int8_t)round((heading / 7.5) * 10 / 10.0);
  aled = (int8_t)round((azimuth / 7.5) * 10 / 10.0);
  rled = (int8_t)round((riseaz / 7.5) * 10 / 10.0);
  sled = (int8_t)round((setaz / 7.5) * 10 / 10.0);

  /* ------------------------------------------------- */
  /* If 48 is returned, it should roll over to 0       */   
  /* ------------------------------------------------- */
  if(hled > 47) hled = 0;
  if(aled > 47) aled = 0;
  if(rled > 47) rled = 0;
  if(sled > 47) sled = 0;
  /* ------------------------------------------------- */
  /* Because LED-1 is 90 degrees offset, we compensate */
  /* ------------------------------------------------- */
  hled = hled + 36; if(hled > 47) hled = hled - 48;
  /* ------------------------------------------------- */
  /* Reverse counting to keep the LED aligned to North */
  /* ------------------------------------------------- */
  hled = 48 - hled; if(hled > 47) hled = 0;
  /* ------------------------------------------------- */
  /* Remaining LED follow heading                      */
  /* ------------------------------------------------- */
  aled = hled + aled; if(aled > 47) aled = aled - 48;
  rled = hled + rled; if(rled > 47) rled = rled - 48;
  sled = hled + sled; if(sled > 47) sled = sled - 48;

#ifdef DEBUG
   var2serial();
#endif 

  /* ------------------------------------------------- */
  /* motor1 target position follows azimuth LED value  */
  /* if 2nd dip switch enabled the motor. DIP2: LOW=ON */
  /* ------------------------------------------------- */
  if(dippos2 == LOW) {
    if(daylight) {
      if(m1move == false) {
        m1tpos = round(aled * 16.65);
        m2tpos = (uint16_t) round(zenith * 7.12);
        if(m1tpos != m1cpos) {
          amotor.adjust(m1tpos, SLOW);
        }
        if(m2tpos != m2cpos) {
          zmotor.adjust(m2tpos, SLOW);
        }
      }
    }
    else {                /* nightfall, move motor-1 home */
      if(m1move == false) {
        if(m1tpos != 0) { /* are we already parked at D1? */
          m1tpos = 0;     /* set D1 as home until sunrise */
          amotor.adjust(m1tpos, SLOW);
        }
      }
    }
  }
  /* ------------------------------------------------- */
  /* motor1 home when push button 1 is pressed  LOW=ON */
  /* ------------------------------------------------- */
  if(dippos2 == LOW) {
    if(m1move == false) {
      if(digitalRead(PUSH1) == LOW) { amotor.sethome(SLOW); }
      if(digitalRead(PUSH2) == LOW) { amotor.oneled(SLOW); }
    }
  }

  ring.set_hdgled();
  ring.set_aziled();
  ring.set_riseled();
  ring.set_downled();
  row.set_eleled(zenith);
  row.set_transled();
  //display.location();   
  /* ------------------------------------------------- */
  /* 2nd display mode: light up all green LED starting */
  /* from sunriseLED+1 to current azimuth LED, until   */
  /* sunset. At sunset, delete all daytime green LED   */
  /* and show the sun azimuth angle in standard mode.  */
  /* ------------------------------------------------- */
  if(dippos1 == LOW) display_mode2();
  delay(50);
}

/* ------------------------------------------------- */
/* get solar postion record for current time.        */
/* ------------------------------------------------- */
void solar_position(char *file) {
  uint8_t linebuf[19];
  File bin = SD.open(file);
  while (bin.available()) {
    bin.read(linebuf, sizeof(linebuf));
    if(linebuf[0] == rtc.hour() && linebuf[1] == rtc.minute()) {
      memcpy(&azimuth, &linebuf[3], sizeof(double));
      memcpy(&zenith, &linebuf[11], sizeof(double));
      if(linebuf[2] == 1) daylight = 1;
      else daylight = 0;
      break;
    }
  }
  bin.close();
}

/* ------------------------------------------------- */
/* get sunrise/transit/sunset record for current day */
/* ------------------------------------------------- */
void get_suntime(char *file) {
  uint8_t linebuf[14];
  File bin = SD.open(file);
  while (bin.available()) {
    bin.read(linebuf, sizeof(linebuf));
    if(linebuf[0] == rtc.month() && linebuf[1] == rtc.day()) {
      risehour=linebuf[2];
      risemin=linebuf[3];
      memcpy(&riseaz, &linebuf[4], sizeof(uint16_t));
      transithour=linebuf[6];
      transitmin=linebuf[7];
      memcpy(&transalt, &linebuf[8], sizeof(int16_t));
      sethour=linebuf[10];
      setmin=linebuf[11];
      memcpy(&setaz, &linebuf[12], sizeof(uint16_t));
      break;
    }
  }
  bin.close();
}

#ifdef DEBUG
/* ------------------------------------------------- */
/* Debug LED and motor values to serial              */
/* ------------------------------------------------- */
void var2serial() {
  Serial.print("Heading LED: ");
  Serial.print(hled);
  Serial.print(" old: ");
  Serial.print(oldhled);
  Serial.print(" Azimuth LED: ");
  Serial.print(aled);
  Serial.print(" old: ");
  Serial.print(oldaled);
  Serial.print(" M1 target: ");
  Serial.print(m1tpos);
  Serial.print(" M1 current: ");
  Serial.print(m1cpos);
  Serial.print(" Push-1: ");
  Serial.print(digitalRead(PUSH1));
  Serial.print(" Push-2: ");
  Serial.println(digitalRead(PUSH2));
}
/* ------------------------------------------------- */
/* Debug DIP switch and button values to serial      */
/* ------------------------------------------------- */
void swi2serial() {
  Serial.print("DIP-1: ");
  Serial.print(dippos1);
  Serial.print(" DIP-2: ");
  Serial.print(dippos2);
  Serial.print(" Push-1: ");
  Serial.print(digitalRead(PUSH1));
  Serial.print(" Push-2: ");
  Serial.println(digitalRead(PUSH2));
}
#endif


/* ------------------------------------------------- */
/* 2nd display mode: light up all green LED starting */
/* from sunriseLED+1 to current azimuth LED, until   */
/* sunset. At sunset, delete all daytime green LED   */
/* and show the sun azimuth angle in standard mode.  */
/* ------------------------------------------------- */
void display_mode2() {
  
 // delete all daytime LED when sunset occurs,
 // continue with single azimuth  LED at night
  if(daylight == false && olddaylight == true) {
 #ifdef DEBUG
    Serial.println("==========================");
    Serial.println("NIGHTTIME SWITCH DETECTED!");
    Serial.println("==========================");
#endif
    mcp3.writeGPIOAB(0x00);
    mcp4.writeGPIOAB(0x00);
    if(aled < 16) mcp3.digitalWrite(aled, HIGH);
    if(aled > 15) mcp4.digitalWrite((aled-16), HIGH);
  }
  olddaylight = daylight;
    
#ifdef DEBUG
    Serial.print("rled: ");
    Serial.print(rled);
    Serial.print(" aled: ");
    Serial.println(aled);
#endif
  // example: rled = 23, aled = 8
  if(daylight) {
    uint8_t i = 0;
    if(rled == 31) i = 0;
    else i = rled + 1;
    if(rled < aled) {
      while(i<=aled) {
        if(i < 16) mcp3.digitalWrite(i, HIGH);
        if(i > 15) mcp4.digitalWrite((i-16), HIGH);
        i++;
      }
    }
    if(rled > aled) {
      if(rled == 31) i = 0;
      else i = rled + 1;
      while(i<32) {
        if(i < 16) mcp3.digitalWrite(i, HIGH);
        if(i > 15) mcp4.digitalWrite((i-16), HIGH);
        i++;
      }
      i = 0;
      while(i<aled) {
        if(i < 16) mcp3.digitalWrite(i, HIGH);
        if(i > 15) mcp4.digitalWrite((i-16), HIGH);
        i++;
      }
    }
  }
}
