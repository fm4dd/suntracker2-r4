/* ---------------------------------------------------- *
 * Suntracker2  mainboard-rev4 demo.cpp  2019-10 @FM4DD *
 *                                                      *
 * This file has function code for the suntracker demo  *
 * mode, which runs through 24 hours in highspeed mode. *
 * ---------------------------------------------------- */
#include <Arduino.h>           // Arduino default library
#include "globals.h"           // global variable definitions
#include "demo.h"

/* ---------------------------------------------------- */
/* led_enable() initializes the 2 zenith row expanders  */
/* ---------------------------------------------------- */
void Demo::run() {
  uint16_t i = 0;
  uint8_t hr = 0;
  uint8_t mi = 0;
  fill_dayarray(binfile);
  olddaylight = true;
  daylight = false;
  display.show_date();
  display.show_rise_set();
  display.show_transit();
  while(i < 1440) {
    /* ------------------------------------------------- */
    /* generate and show simulated time                  */
    /* ------------------------------------------------- */
    mi++;
    if(mi == 60) { hr++; mi = 0; }
    if(hr == 24) break;
    snprintf(lineStr, sizeof(lineStr), "Time: %02d:%02d:00",
    hr, mi);
    display.show_time(lineStr);
    /* ------------------------------------------------- */
    /* Show Azimuth and sunrise/sunset on TFT            */
    /* ------------------------------------------------- */
    display.show_azimuth(day_azi[i]);
    /* ------------------------------------------------- */
    /* Show Elevation on TFT, add leading + for daytime  */
    /* ------------------------------------------------- */
    display.show_elevation(day_ele[i]);
    if(day_ele[i] <=90) daylight = true;
    else daylight = false;
    display.daysymbol();
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
    display.show_heading();
    /* ------------------------------------------------- */
    /* Convert heading angle to LED range value (0..48)  */   
    /* ------------------------------------------------- */
    hled = (int8_t)round((heading / 7.5) * 10 / 10.0);
    aled = (int8_t)round((day_azi[i] / 7.5) * 10 / 10.0);
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
    row.set_eleled(day_ele[i]);
    row.set_transled();
    /* ------------------------------------------------- */
    /* loop activities complete, increment loop counter  */     
    /* Set the loop speed. 60000 = 60s = normal time     */
    /* 100 = 1/10s intervals, 1440*0.1sec/60 = 2:24mins  */
    /* ------------------------------------------------- */
    //delay(10); // MKRZero gets so slow we can disable delay
    i++;
  }
  /* ------------------------------------------------- */
  /* After the DEMO turn off all LEDs and reset values */
  /* ------------------------------------------------- */
  display.wipe();
  row.all_ledoff();
  ring.all_ledoff();
  hled = 0;             // heading LED (red)
  oldhled = 49;         // 49 > max value 48, indicates "init"
  aled = 0;             // azimuth LED (green)
  oldaled = 49;         // 49 > max value 48, indicates "init"
  rled = 0;             // sunrise LED (red)
  oldrled = 49;         // 49 > max value 48, indicates "init"
  sled = 0;             // sunset LED (red)
  oldsled = 49;         // 49 > max value 48, indicates "init"
  oldeled = 17;         // 17 > max value 15, indicates "init"
  oldtled = 17;         // 17 > max value 15, indicates "init"
  oldtransalt = 91;     // 91 > max value 90, inticates "init"
}

/* ------------------------------------------------- */
/* Read daily sun angles, store them in day arrays   */
/* ------------------------------------------------- */
void Demo::fill_dayarray(char *file) {
  uint8_t linebuf[19];
  uint16_t i = 0;
  File bin = SD.open(file);
  while (bin.available()) {
    /* safety check. dayfile should have max 1440 data points */
    if(i >= 1440) break;
    bin.read(linebuf, sizeof(linebuf));
    memcpy(&azimuth, &linebuf[3], sizeof(double));
    memcpy(&zenith, &linebuf[11], sizeof(double));
    day_azi[i] = azimuth;
    day_ele[i] = zenith;
    i++;
  }
  bin.close();
}
