/* ---------------------------------------------------- *
 * Suntracker2  mainboard-rev4 demo.h    2019-10 @FM4DD *
 *                                                      *
 * This file has the class and function definitions for *
 * the suntracker demo mode, which runs through one day *
 * in highspeed mode.                                   *
 * ---------------------------------------------------- */

class Demo {
  public:
    void run();
  private:
    void fill_dayarray(char *file);
    void fmtNumber(float num, int len, int prec, char *buf);
};
