/* ---------------------------------------------------- *
 * Suntracker2  mainboard-rev4   motor.h 2019-10 @FM4DD *
 *                                                      *
 * This file has the functions defs for the stepper     *
 * motors.                                              *
 * ---------------------------------------------------- */

class AMotor {
  public:
    void enable();
    void poweron();
    void poweroff();
    void demoturn(uint8_t);
    void oneturn(boolean, int);
    void adjust(uint16_t, int);
    void oneadjust(uint16_t, int);
    void oneled(int);
    void sethome(int);
};

class ZMotor {
  public:
    void enable();
    void demoturn(uint8_t);
    void poweron();
    void poweroff();
    void adjust(uint16_t, int);
    //void oneadjust(uint16_t, int);
    //void oneled(int);
    //void onehome(int);
};
