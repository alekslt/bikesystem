// bt_func.h

#ifndef _BT_FUNC_h
#define _BT_FUNC_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define RxD 11
#define TxD 12

#define BT_STATUS_PIN 3
#define BT_DISCO_PIN 4
#define BT_RESET_PIN 5

// Forwards
class Stream;

// Bluetooth vars
extern uint8_t bt_msg_status;
extern uint8_t bt_initstate;
extern uint8_t bt_ok;
extern uint8_t bt_work;
extern uint8_t bt_state;
extern uint8_t bt_connect;

void get_command(Stream &ser, Stream &serout, const char delim[]);

void cmd_bt_ok(uint8_t id, char* args, Stream &ser);
void cmd_bt_state(uint8_t id, char* args, Stream &ser);
void cmd_bt_st(uint8_t id, char* args, Stream &ser);
void cmd_bt_connect(uint8_t id, char* args, Stream &ser);
void cmd_bt_work(uint8_t id, char* args, Stream &ser);

void bt_setup();

#endif

