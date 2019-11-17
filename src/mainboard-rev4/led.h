/* ---------------------------------------------------- *
 * Suntracker2  mainboard-rev4     led.h 2019-10 @FM4DD *
 *                                                      *
 * This file has the functions defs for the dualcolor   *
 * zenith row on the mainboard, and the 48 LED ring on  *
 * the displayboard v2.1, connected to mainboard v2.1.  *
 * ---------------------------------------------------- */

class ledRow {
  public:
    uint8_t enable();
    void all_ledoff();
    void lightcheck();
    void stepled_red(int);
    void stepled_green(int);
    void set_eleled(float);
    void set_transled();
};

class ledRing {
  public:
    uint8_t enable();
    void all_ledoff();
    void lightcheck();
    void stepled(uint8_t, int);
    void lightshow(int);
    void set_hdgled();
    void set_aziled();
    void set_riseled();
    void set_downled();
};
