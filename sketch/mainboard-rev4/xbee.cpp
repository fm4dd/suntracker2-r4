/* ---------------------------------------------------- *
 * Suntracker2 mainboard-rev4 xbee.cpp 2020-04 @FM4DD   *
 *                                                      *
 * This file has the functions to control the XBee S2C  *
 * radio module. The module is coordinator/router.      *
 * ---------------------------------------------------- */
#include <Arduino.h>            // Arduino default library
#include "xbee.h"

#define DEBUG
/* ---------------------------------------------------- */
/* enable() XBee S2C module on HW port Serial1 pin13/14 */
/* returns true for success, false for errors           */
/* ---------------------------------------------------- */
bool XBee::enable() {
  Serial1.begin(sSpeed);                    // set Serial1 speed
  Serial1.setTimeout(sWait);                // set timeout, default is 1s
  for(int i=0; i<5; i++) {                  // wait 5s for serial1 to connect
    if (Serial1) break;                     // we got connected
#ifdef DEBUG
    else Serial.println("enable(): Wait for Serial1");
#endif
    delay(1000);                            // wait one second
  }
  if(!Serial1) return false;                // return failure
#ifdef DEBUG
  Serial.println("enable(): Serial1 connected");
#endif
  /* ------------------------------------------------- */
  /* Test Xbee S2C module by sending break signal +++  */
  /* ------------------------------------------------- */
  if(startcmdmode() == false)
    return false;                            // return failure
  if(endcmdmode() == false)
    return false;                            // return failure
  return true;                               // return success
} // end enable()

/* ---------------------------------------------------- */
/* getinfo() gets XBee S2C module HW information incl.  */
/* MAC, firmware version, hardware model, bus voltage   */
/* returns true for success, false for errors           */
/* ---------------------------------------------------- */
bool XBee::getinfo() {
  String response;
  if(!Serial1) XBee::enable();
  /* ------------------------------------------------- */
  /* Enter CMD mode                                    */
  /* ------------------------------------------------- */
  if(startcmdmode() == false)
    return false;                            // return failure
  /* ------------------------------------------------- */
  /* Get firmware version with ATVR, returns 4 bytes   */
  /* ------------------------------------------------- */
#ifdef DEBUG
  Serial.println("getinfo(): Start");
#endif
  sendstring("ATVR");                        // Request firmware version
  response = recvstring(4);                  // get serial response
  // assign response to FW string
  response.toCharArray(_info.firmware, sizeof(_info.firmware));
  /* ------------------------------------------------- */
  /* Get hardware version with ATHV, returns 4 bytes   */
  /* ------------------------------------------------- */
  sendstring("ATHV");                        // Request hardware version
  response = recvstring(4);                  // get serial response
  // assign response to HW string
  response.toCharArray(_info.hardware, sizeof(_info.hardware));
  /* ------------------------------------------------- */
  /* Get node identifier with ATNI, may return 0 bytes */
  /* ------------------------------------------------- */
  sendstring("ATNI");                        // Request node id
  response = recvstring(0);                  // get serial response
  // assign response to Node ID string
  response.toCharArray(_info.nodeid, sizeof(_info.nodeid));
  /* ------------------------------------------------- */
  /* Get MAC address with ATSH and ATSL, ret 6/8 bytes */
  /* ------------------------------------------------- */
  sendstring("ATSH");                        // Request mac upper
  delay(200);                                // add extra delay
  response = recvstring(6);                  // get serial response
  // assign response to MAC string e.g. 0013A200417D5111
  uint8_t leadzeros = 8 - response.length();
  while(leadzeros > 0) {
    response = "0" + response;
    leadzeros--;
  }
  String mac = response;

  sendstring("ATSL");                        // Request mac lower
  response = recvstring(8);                  // get serial response
  // add response to MAC string e.g. 0013A200417D5111
  leadzeros = 8 - response.length();
  while(leadzeros > 0) {
    response = "0" + response;
    leadzeros--;
  }
  mac = mac + response;
  mac.toCharArray(_info.mac, sizeof(_info.mac));
  /* ------------------------------------------------- */
  /* Get bus voltage in mV with AT%V, returns 3 bytes  */
  /* ------------------------------------------------- */
  sendstring("AT%V");                        // Request mac upper
  response = recvstring(3);                  // get serial response

  // convert string to float
  uint32_t millivolt = strtol(response.c_str(), NULL, 16);
  _info.volt = float(millivolt) / 1000.0;
#ifdef DEBUG
  Serial.print("Convert Volt: ");
  Serial.println(_info.volt,3);
#endif
  if(endcmdmode() == false)
    return false;                            // return failure
  return true;                               // return success
} // end getinfo()

/* ---------------------------------------------------- */
/* getXXX returns the XBee detail collected w. getinfo  */
/* MAC, firmware version, hardware model, bus voltage   */
/* ---------------------------------------------------- */
const char *XBee::getfirmware() {
  if(strlen(_info.firmware) == 0) XBee::getinfo();
  return _info.firmware;
}
const char *XBee::gethardware() {
  if(strlen(_info.hardware) == 0) XBee::getinfo();
  return _info.hardware;
}
const char *XBee::getnodeid() {
  return _info.nodeid;
}
const char *XBee::getmac() {
  if(strlen(_info.mac) == 0) XBee::getinfo();
  return _info.mac;
}
float XBee::getvolt() {
  if(_info.volt == 0.0) XBee::getinfo();
  return _info.volt;
}

/* ---------------------------------------------------- */
/* sendstring() sends a string over / to the XBee radio */
/* args: String to send, flag to send AT cmds to module */
/* ---------------------------------------------------- */
void XBee::sendstring(const char * sendstr) {
  uint8_t sendbytes = strlen(sendstr);
  while(1) {
    if(Serial1.availableForWrite() > sendbytes) break;
    delay(1);
  }
#ifdef DEBUG
  Serial.print("Serial1 send: ");
  Serial.print(sendstr);
  Serial.print(" ");
  Serial.println(sendbytes);
#endif
  Serial1.println(sendstr);
  return;
}

/* ---------------------------------------------------- */
/* startcmdmode() enters command mode to send AT cmds   */
/* returns true for success, false for errors           */
/* ---------------------------------------------------- */
bool XBee::startcmdmode() {
  /* ------------------------------------------------- */
  /* Enter CMD mode by sending break signal +++        */
  /* ------------------------------------------------- */
  delay(sGuard);                         // wait guard time before CMD
  while(1) {                             // check transmit buffer
    if(Serial1.availableForWrite() > 3) break;
    delay(1);
  }
#ifdef DEBUG
  Serial.println("Serial1 send: +++");
#endif
  Serial1.print("+++");                  // send break signal
  /* ------------------------------------------------- */
  /* Verify module responds with 'OK'                  */
  /* ------------------------------------------------- */
  while(1) {                             // wait for serial
    if(Serial1.available() > 2) break;
    delay(1);
  }
  String ret = Serial1.readStringUntil('\r');
#ifdef DEBUG
  Serial.print("Serial1 recv: ");
  Serial.println(ret);
#endif
  if(ret != String("OK"))                // check respose matches "OK"
    return false;
  return true;
}

/* ---------------------------------------------------- */
/* endcmdmode() close command mode after sending AT cmd */
/* returns true for success, false for errors           */
/* ---------------------------------------------------- */
bool XBee::endcmdmode() {
  /* ------------------------------------------------- */
  /* Send ATCN command to leave CMD mode               */
  /* ------------------------------------------------- */
  while(1) {                             // check transmit buffer
    if(Serial1.availableForWrite() > 5) break;
    delay(1);
  }
#ifdef DEBUG
  Serial.println("Serial1 send: ATCN");
#endif
  Serial1.println("ATCN");               // send ATCN command
  String ret = Serial1.readStringUntil('\r');
#ifdef DEBUG
  Serial.print("Serial1 recv: ");
  Serial.println(ret);
#endif
  if(ret != String("OK"))                // check respose matches "OK"
    return false;
  return true;
}

/* ---------------------------------------------------- */
/* recvstring() receives a string over XBee radio       */
/* returns the received String object                   */
/* ---------------------------------------------------- */
String XBee::recvstring(uint8_t size) {
  while(0) {                            // wait for serial
    if(Serial1.available() > size) break;
    delay(1);
  }
  String received;
  received = Serial1.readStringUntil('\r');
#ifdef DEBUG
  Serial.print("Serial1 recv: ");
  Serial.println(received);
#endif
  return received;
}

/* ---------------------------------------------------- */
/* setconfig() configures XBee radio as device / coord. */
/* args: array of config commands, number of commands   */
/* returns true for success, false for errors           */
/* ---------------------------------------------------- */
bool XBee::setconfig(const char **conf, uint8_t entries) {
  uint8_t sendbytes = 0;
  String response;
#ifdef DEBUG
  Serial.println("setconfig(): Start");
#endif
  if(startcmdmode() == false)            // begin CMD mode 
    return false;
    
  while(entries){                        // write config data
    sendstring(conf[entries-1]);
    response = recvstring(2);            // get 'OK' response
    entries--;  
  }
  sendstring("ATWR");                    // write current config to flash
  response = recvstring(2);              // get 'OK' response
  sendstring("ATAC");                    // apply changes to module
  response = recvstring(2);              // get 'OK' response
  if(endcmdmode() == false)              // end CMD mode
    return false;
  return true;
}

/* ---------------------------------------------------- */
/* getstatus() gets XBee S2C module read-only status,   */
/* incl. assaciation, signal strength, free device, etc */
/* returns true for success, false for errors           */
/* ---------------------------------------------------- */
bool XBee::getstatus() {
  String response;
  if(!Serial1) XBee::enable();
  /* ------------------------------------------------- */
  /* Enter CMD mode                                    */
  /* ------------------------------------------------- */
  if(startcmdmode() == false)
    return false;                            // return failure
  /* ------------------------------------------------- */
  /* Get device_free with ATNC, returns 0..14 (1 byte) */
  /* ------------------------------------------------- */
#ifdef DEBUG
  Serial.println("getinfo(): Start");
#endif
  sendstring("ATNC");                        // Request firmware version
  response = recvstring(2);                  // get serial response
  _status.device_free = response.toInt();    // assign to device_free
  /* ------------------------------------------------- */
  /* Get association with ATAI, returns 1 byte, 0 = OK */
  /* if non-zero return, it will be error codes in hex */
  /* ------------------------------------------------- */
  sendstring("ATAI");                        // Request hardware version
  response = recvstring(2);                  // get serial response
  // convert from hex to dec, and assign to association
  _status.association = strtol(response.c_str(), NULL, 16);
  /* ------------------------------------------------- */
  /* Get operational PAN ID with ATOP, returns 8 bytes */
  /* ------------------------------------------------- */
  sendstring("ATOP");                        // Request node id
  response = recvstring(16);                 // get serial response
  _status.oper_panid = response.toInt();      // assign to oper_panid

  if(endcmdmode() == false)
    return false;                            // return failure
  return true;                               // return success
} // end getstatus()

/* ---------------------------------------------------- */
/* getXXX returns the XBee detail collected w getstatus */
/* ---------------------------------------------------- */
uint8_t XBee::getdevice_free() { return _status.device_free; }
uint8_t XBee::getassociation() { return _status.association; }
uint32_t XBee::getoper_panid() { return _status.oper_panid; }
uint16_t XBee::getoper_chan() { return _status.oper_chan; }
uint8_t XBee::getpwr_level() { return _status.pwr_level; }
uint8_t XBee::getlast_rssi() { return _status.last_rssi; }
uint8_t XBee::getmax_packets() { return _status.max_packets; }
uint16_t XBee::getnw_address() { return _status.nw_address; }
uint16_t XBee::getpt_address() { return _status.pt_address; }
