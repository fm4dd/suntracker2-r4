/* ---------------------------------------------------- *
 * Suntracker2  mainboard-rev4   xbee.h 2020-04 @FM4DD  *
 *                                                      *
 * This file has the functions defs for the XBee radio  *
 * S2C module XB24CZ7WIT-004 on HW serial 13-RX/14-TX.  *
 * ---------------------------------------------------- */
#include <Arduino.h>      // Arduino default library

const int sSpeed = 9600;  // Serial line speed for XBee
const int sWait  = 1000;  // wait 1s for serial response
const int sGuard = 1000;  // Guard timer for CMD mode

struct XBee_Info {
  char firmware[5];       // ATVR, returns 4 digits firmware
  char hardware[5];       // ATHV, returns 4 bytes HW
  char nodeid[21];        // ATNI, returns 20 bytes Node name
  char mac[17];           // ATDH+ATDL, returns 16 bytes MAC
  float volt;             // AT%V, hex converted to Volt
};

struct XBee_Status {
  uint8_t device_free;    // ATNC, 0...14
  uint8_t association;    // ATAI, connected = 0
  uint32_t oper_panid;    // ATOP, 0 = not connected
  uint16_t oper_chan;     // ATCH, 0 = not connected
  uint8_t last_rssi;      // ATDB, recv signal strength 0..FF
  uint8_t pwr_level;      // ATPP, power level for pwr mode 4
  uint8_t max_packets;    // ATNP, max. bytes for unicasts
  uint16_t nw_address;    // ATMY, NW addr, FFFE if disconnected
  uint16_t pt_address;    // ATMY, Parent addr, FFFE if disconnected
};

struct XBee_Config {}; // TBD

class XBee {
  public:
    // Coordinator configuration (default)
    const char *coord_conf[7] = {
      "ATID24",           // PAN ID = 24
      "ATCE1",            // Enable Coordinator role
      "ATJV0",            // Dont try to join NW
      "ATDH0",            // Dest addr high & low = 0/FFFF
      "ATDLFFFF",         // We talk to everyone (broadbast)
      "ATNIS2R4-master",  // Device name
      "ATAP0" };          // Set transparent mode
    // End Device configuration (optional if coord exists)
    const char *device_conf[7] = {
      "ATID24",           // PAN ID = 24
      "ATCE0",            // Disable Coordinator role
      "ATJV1",            // Join network at boot
      "ATDH0",            // Dest addr high & low = 0/0
      "ATDL0",            // we only talk to coordinator
      "ATNIS2R4-node1",   // Device name
      "ATAP0" };          // Set transparent mode
    bool enable();
    // make getinfo()-collected data available
    bool getinfo();
    const char *gethardware();
    const char *getfirmware();
    const char *getnodeid();
    const char *getmac();
    float getvolt();
    // make getstatus()-collected data available
    bool getstatus();
    uint8_t getdevice_free();
    uint8_t getassociation();
    uint32_t getoper_panid();
    uint16_t getoper_chan();
    uint8_t getpwr_level();
    uint8_t getlast_rssi();
    uint8_t getmax_packets();
    uint16_t getnw_address();
    uint16_t getpt_address();
    // bool getconfig();
    bool setconfig(const char **, uint8_t);
    void sendstring(const char *);
    bool startcmdmode();
    bool endcmdmode();
    String recvstring(uint8_t);
  private:
    struct XBee_Info _info;
    struct XBee_Status _status;
};
