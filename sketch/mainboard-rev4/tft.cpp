/* ---------------------------------------------------- *
 * Suntracker2  mainboard-rev4 tft.cpp   2019-10 @FM4DD *
 *                                                      *
 * This file has the functions defs for the 320Z240 TFT *
 * breakout display from Adafruit 2.8", product ID 1770 *
 * connected to mainboard v2.1.                         *
 * ---------------------------------------------------- */
#include <Arduino.h>           // Arduino default library
#include <avr/dtostrf.h>       // dtostrf float conversion
#include "Adafruit_GFX.h"      // Adafruit graphics library
#include "Adafruit_ILI9341.h"  // driver for Adafruit 2.8 TFT display
#include "U8g2_for_Adafruit_GFX.h" // U8G2 font extension library
#include "img_mono.h"          // bitmap files for displays
#include "globals.h"           // global variable definitions

/* ---------------------------------------------------- */
/* 8x IO Expanders for a total of 128 I/O ports         */
/* ---------------------------------------------------- */
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
U8G2_FOR_ADAFRUIT_GFX u8g2_tft;   // u8g2 fonts

/* ---------------------------------------------------- */
/* enable() initializes the tft display and draws the   */
/* header logo+text covering pixels 0,0 until 320,70.   */
/* ---------------------------------------------------- */
void Display::enable() {
  /* ------------------------------------------------- */
  /* Enable TFT display, clear screen, set black bgnd  */
  /* ------------------------------------------------- */
  tft.begin(24000000);
  /* ------------------------------------------------- */
  /* Orientation: 0=vert, 1=hor, 2=vert inv, 3=hor inv */
  /* ------------------------------------------------- */
  tft.setRotation(3);
  u8g2_tft.begin(tft);
  tft.fillScreen(ILI9341_BLACK);
}

/* ---------------------------------------------------- */
/* header() draws logo+text from 0,0px until 320,70px   */
/* ---------------------------------------------------- */
void Display::header() {
  tft.fillScreen(ILI9341_BLACK);
  tft.drawBitmap(0, 0, arduinoLogo, 120, 60, 0x3536);
  tft.drawLine(0, 70, 319, 70, ILI9341_GREEN);
  u8g2_tft.setFont(u8g2_font_inr19_mr);
  u8g2_tft.setForegroundColor(0x3536); // same as logo
  u8g2_tft.drawStr(130,22, "Arduino");
  u8g2_tft.drawStr(130,48, "MKRZero");
  u8g2_tft.setFont(u8g2_font_6x12_mr);
  u8g2_tft.drawStr(130,62, prgver);
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
}

void Display::show_ver() {
  char *line = "Suntracker2 R4";
  u8g2_tft.setForegroundColor(0xfce0);     // amber
  u8g2_tft.setFont(u8g2_font_inb21_mr);
  //u8g2_tft.setFontMode(1); // transparent background
  tft.fillRect(0, 190, 320, 50, 0x0);
  u8g2_tft.drawStr(20,218, line);
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
  u8g2_tft.drawStr(70,236, prgver);
  //u8g2_tft.setFontMode(0);
}

/* ---------------------------------------------------- */
/* opmode() shows the operations mode (0=track, 1=demo, */
/* 2=setup)" in the top-right corner at 10,250 - 60,310 */
/* ---------------------------------------------------- */
void Display::opmode(uint8_t mode) {
  tft.fillRect(255, 3, 61, 29, 0x3536);
  u8g2_tft.setForegroundColor(0x0);        // black
  u8g2_tft.setBackgroundColor(0x3536);     // cyan
  u8g2_tft.setFont(u8g2_font_fub14_tr);
  u8g2_tft.drawStr(260, 25, "S2R4");
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
  u8g2_tft.setBackgroundColor(0x0);        // black
  u8g2_tft.setForegroundColor(0x3536);     // cyan
  tft.drawRect(255, 32, 61, 30, 0x3536);
  if(mode == 0) {                          // Init
    u8g2_tft.drawStr(263, 51, "Init");
  }
  if(mode == 1) {                          // Track
    u8g2_tft.drawStr(263, 51, "Track");
    /* ----------------------------------------------- */
    /* Draw second screen separator line at 160px down */
    /* ----------------------------------------------- */
    tft.drawLine(0, 160, 319, 160, ILI9341_GREEN);
    tft.fillCircle(9, 160, 8, ILI9341_BLACK);
    u8g2_tft.drawStr(5,165, "E");
    tft.drawCircle(9, 160, 9, ILI9341_GREEN);
  }
  if(mode == 2) {                 /* Demo             */
    u8g2_tft.drawStr(263, 51, "Demo");
    /* ----------------------------------------------- */
    /* Draw second screen separator line at 160px down */
    /* ----------------------------------------------- */
    tft.drawLine(0, 160, 319, 160, ILI9341_GREEN);
  }
  if(mode == 3) {                 /* Setup            */
    u8g2_tft.drawStr(263, 51, "Setup");
  }
}

/* ---------------------------------------------------- */
/* clear() cleans the display below logo line at 70px   */
/* ---------------------------------------------------- */
void Display::wipe() {
    tft.fillRect(0, 80, 320, 160, 0x0);
}

void Display::daysymbol() {
  if(olddaylight != daylight && daylight == true) {
    tft.fillRect(238, 80, 32, 32, 0x0);      // clear old symbol
    tft.drawBitmap(238, 80, dayImg, 32, 32, 0x3536);
    olddaylight = daylight;
  }
  if (olddaylight != daylight && daylight == false){
    tft.fillRect(238, 80, 32, 32, 0x0);      // clear old symbol
    tft.drawBitmap(238, 80, nightImg, 32, 32, 0x3536);
    olddaylight = daylight;
  }
  if (rtc.hour() == 23 && rtc.minute() == 0) {
    tft.fillRect(238, 80, 32, 32, 0x0);       // clear old symbol
    tft.drawBitmap(238, 80, ghostImg, 32, 32, 0x3536);
  }
  if (rtc.hour() == 0 && rtc.minute() == 0) { // back to night
    tft.fillRect(238, 80, 32, 32, 0x0);       // clear old symbol
    tft.drawBitmap(238, 80, nightImg, 32, 32, 0x3536);
  }
}

/* ------------------------------------------------- */
/* Show north heading LED control data on TFT        */
/* ------------------------------------------------- */
void Display::hdgled(int old, int now) {
  if(old < 16)
    snprintf(hledStr, sizeof(hledStr), "-A%02d", old);
  else if(old < 32)
    snprintf(hledStr, sizeof(hledStr), "-B%02d", old-16);
  else if(old < 48)
    snprintf(hledStr, sizeof(hledStr), "-C%02d", old-32);
  else snprintf(hledStr, sizeof(hledStr), "init");
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
  u8g2_tft.drawStr(277,94, hledStr);
  if(now < 16)
    snprintf(hledStr, sizeof(hledStr), "+A%02d", now);
  if(now > 15)
    snprintf(hledStr, sizeof(hledStr), "+B%02d", (now-16));
  if(now > 31)
    snprintf(hledStr, sizeof(hledStr), "+C%02d", (now-32));
  u8g2_tft.drawStr(277,110, hledStr);
}

/* ------------------------------------------------- */
/* Show azimuth LED control data on TFT              */
/* ------------------------------------------------- */
void Display::aziled(int old, int now) {
  if(old < 16)
    snprintf(aledStr, sizeof(aledStr), "-D%02d", old);
  else if(old < 32)
    snprintf(aledStr, sizeof(aledStr), "-E%02d", old-16);
  else if(old < 48)
    snprintf(aledStr, sizeof(aledStr), "-F%02d", old-32);
  else snprintf(aledStr, sizeof(aledStr), "init");
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
  u8g2_tft.drawStr(116,94, aledStr);
  if(now < 16)
    snprintf(aledStr, sizeof(aledStr), "+D%02d", now);
  if(now > 15)
    snprintf(aledStr, sizeof(aledStr), "+E%02d", (now-16));
  if(now > 31)
    snprintf(aledStr, sizeof(aledStr), "+F%02d", (now-32));
  u8g2_tft.drawStr(116,110, aledStr);
}
/* ------------------------------------------------- */
/* Show elevation LED control data on TFT            */
/* ------------------------------------------------- */
void Display::eleled(int old, int now) {
  if(old > 16) snprintf(eledStr, sizeof(eledStr), "init");
  else if(old < 0) snprintf(eledStr, sizeof(eledStr), " off");
  else snprintf(eledStr, sizeof(eledStr), "-G%02d", old);
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
  u8g2_tft.drawStr(116,184, eledStr);
  if(now < 0) snprintf(eledStr, sizeof(eledStr), " off");
  else snprintf(eledStr, sizeof(eledStr), "+G%02d", now);
  u8g2_tft.drawStr(116,200, eledStr);
}

void Display::show_heading() {
  char dirStr[7] = "H"; // display "H263"
  fmtNumber(heading, 3, 0, &dirStr[1]);
  u8g2_tft.setFont(u8g2_font_inr19_mr);
  u8g2_tft.drawStr(167,106, dirStr);
}

void Display::show_time(char *timeStr) {
  /* ------------------------------------------------- */
  /* Show time on TFT                                  */
  /* ------------------------------------------------- */
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
  u8g2_tft.setCursor(167, 143);
  u8g2_tft.print(timeStr);
}

void Display::show_azimuth(float az) {
  char aziStr[7] = "A"; // display "A134.8"
  fmtNumber(az, 5, 1, &aziStr[1]);
  u8g2_tft.setFont(u8g2_font_inr19_mr);
  u8g2_tft.drawStr(0,106, aziStr);
}

void Display::show_elevation(float ze) {
  char eleStr[7] = "E"; // display "E+55.2"
  if(ze <= 90.0) {
    eleStr[1] = '+';
    fmtNumber((90.0 - ze), 4, 1, &eleStr[2]);
  }
  else fmtNumber((90.0 - ze), 4, 1, &eleStr[1]);
  u8g2_tft.setFont(u8g2_font_inr19_mr);
  u8g2_tft.drawStr(0,196, eleStr);
}

/* ------------------------------------------------- */
/* Show location data on TFT                         */
/* ------------------------------------------------- */
void Display::location(float lat, float lon, float alt) {
  char locStr[15];             // GPS buffer string
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
  fmtNumber(lat, 9, 5, locStr);
  snprintf(lineStr, sizeof(lineStr), "LAT:  %s", locStr);
  u8g2_tft.drawStr(167,184, lineStr);
  fmtNumber(lon, 9, 5, gpsStr);
  snprintf(lineStr, sizeof(lineStr), "LONG: %s", locStr);
  u8g2_tft.drawStr(167,200, lineStr);
  fmtNumber(alt, 9, 5, gpsStr);
  snprintf(lineStr, sizeof(lineStr), "ALT:  %s", locStr);
  u8g2_tft.drawStr(167,216, lineStr);
}

void Display::show_rise_set() {
  char riseStr[18]; // display "Sunrise 05:40 096"
  char setStr[18];  // display "Sunset  17:19 264"
  snprintf(riseStr, sizeof(riseStr), "Sunrise %02d:%02d %03d",
           risehour, risemin, riseaz);
  snprintf(setStr, sizeof(setStr), "Sunset  %02d:%02d %03d",
           sethour, setmin, setaz);
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
  u8g2_tft.setCursor(0, 126);
  u8g2_tft.print(riseStr);
  u8g2_tft.setCursor(0, 143);
  u8g2_tft.print(setStr);
}

void Display::show_transit() {
  char elehStr[18]; // display "HiPoint 11:49 046"
  char eletStr[18]; // display "Sun Transit 11:49"
  snprintf(elehStr, sizeof(elehStr), "HiPoint %02d:%02d +%02d",
           transithour, transitmin, transalt);
  snprintf(eletStr, sizeof(eletStr), "LoPoint %02d:%02d -%02d",
           transithour, transitmin, transalt);
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
  u8g2_tft.setCursor(0, 216);
  u8g2_tft.print(elehStr);
  u8g2_tft.setCursor(0, 233);
  u8g2_tft.print(eletStr);
}

void Display::show_date() {
  u8g2_tft.setFont(u8g2_font_t0_17_mr);
  snprintf(lineStr, sizeof(lineStr), "Date: 20%d/%02d/%02d",
  rtc.year(),rtc.month(),rtc.day());
  u8g2_tft.setCursor(167, 126);
  u8g2_tft.print(lineStr);
}

/* ------------------------------------------------- */
/* fmtNumber() converts a float to string including  */
/* length, precision and leading zeroes.             */
/* https://forum.arduino.cc/index.php?topic=413850.0 */
/* ------------------------------------------------- */
void Display::fmtNumber(float num, int len, int prec, char *buf) {
  dtostrf(num,len,prec,buf);
  for (int i = 0; i < len+1; i++) {
    if (buf[i]==' ') buf[i]='0';
  }
}

// Drawing speed, 20 is meant to be the best,
// can go up to 60, just uses a lot of RAM.
#define BUFFPIXEL 40

void Display::bmpDraw(char *filename, uint8_t x, uint16_t y) {
  File     bmpFile;
  int      bmpWidth, bmpHeight;   
  uint8_t  bmpDepth;              
  uint32_t bmpImageoffset;        
  uint32_t rowSize;               
  uint8_t  sdbuffer[3*BUFFPIXEL]; 
  uint8_t  buffidx = sizeof(sdbuffer); 
  boolean  goodBmp = false;       
  boolean  flip    = true;        
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();
  SD.begin(SDCARD_SS_PIN);
  bmpFile = SD.open(filename);
  
  if(read16(bmpFile) == 0x4D42) { 
    read32(bmpFile);
    (void)read32(bmpFile); 
    bmpImageoffset = read32(bmpFile); 
    bmpImageoffset, DEC;
    read32(bmpFile);
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) {
      bmpDepth = read16(bmpFile);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) {

        goodBmp = true;
        rowSize = (bmpWidth * 3 + 3) & ~3;

        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;
        tft.setAddrWindow(x, y, x+w-1, y+h-1);
        for (row=0; row<h; row++) { 
          if(flip) 
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) {
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer);
          }

          for (col=0; col<w; col++) {
            if (buffidx >= sizeof(sdbuffer)) {
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; 
            }

       
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.color565(r,g,b));
          } 
        } 
      } 
    }
  }

  bmpFile.close();
}

uint16_t Display::read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); 
  ((uint8_t *)&result)[1] = f.read(); 
  return result;
}

uint32_t Display::read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); 
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); 
  return result;
}
