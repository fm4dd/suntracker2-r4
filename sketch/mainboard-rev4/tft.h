/* ---------------------------------------------------- *
 * Suntracker2  mainboard-rev4     tft.h 2019-10 @FM4DD *
 *                                                      *
 * This file has the functions defs for the 320Z240 TFT *
 * breakout display from Adafruit 2.8", product ID 1770 *
 * connected to mainboard v2.1.                         *
 * ---------------------------------------------------- */

/* ---------------------------------------------------- */
/* Adafruit 2.8 TFT display use HW SPI + CS and DC pins */
/* ---------------------------------------------------- */
#define TFT_DC 0
#define TFT_CS 1
#define TFT_BL 18

class Display {
  public:
    void enable();
    void header();
    void show_ver();
    void opmode(uint8_t);
    void wipe();
    void daysymbol();
    void hdgled(int, int);
    void aziled(int, int);
    void eleled(int, int);
    void show_time(char*);
    void show_heading();
    void show_azimuth(float);
    void show_elevation(float);
    void location(float, float, float);
    void show_date();
    void show_rise_set();
    void show_transit();
    void bmpDraw(char*, uint8_t, uint16_t);
  private:
    void fmtNumber(float, int, int, char*);
    uint16_t read16(File&);
    uint32_t read32(File&);
};
