#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
//Board = Arduino Uno
#define __AVR_ATmega328P__
#define ARDUINO 105
#define __AVR__
#define F_CPU 16000000L
#define __cplusplus
#define __attribute__(x)
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__
#define __builtin_va_list
#define __builtin_va_start
#define __builtin_va_end
#define __DOXYGEN__
#define prog_void
#define PGM_VOID_P int
#define NOINLINE __attribute__((noinline))

typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

void DHT_get_temp(uint8_t id, Stream &ser);
void DHT_get_humidity(uint8_t id, Stream &ser);
void config_get();
void config_cmd();
char* get_token(char in_string[], char **ptr, const char delim[]);
void parse_command(char command[], Stream &serout, const char delim[]);
void cmd_send_config(uint8_t id, char* args, Stream &ser);
void cmd_send_sensors(uint8_t id, char* args, Stream &ser);
void cmd_general(uint8_t id, char* args, Stream &ser);
void get_command(Stream &ser, Stream &serout, const char delim[]);
void cmd_bt_ok(uint8_t id, char* args, Stream &ser);
void cmd_bt_state(uint8_t id, char* args, Stream &ser);
void cmd_bt_work(uint8_t id, char* args, Stream &ser);
void resetBT();
void enterComMode();
void enterATMode();
void BT_Disc();
void CheckOK();
void wait_for(uint8_t &value, uint8_t equals, char desc[]);
void setupBlueToothConnection();
void send_config(Stream &ser);
void send_sensors(Stream &ser);
void setup_softwareserial();
//
//

#include "D:\Development\arduino-1.0.5\hardware\arduino\variants\standard\pins_arduino.h" 
#include "D:\Development\arduino-1.0.5\hardware\arduino\cores\arduino\arduino.h"
#include "D:\Development\arduinoprojects\bikesystem\bikesystem.ino"
#include "D:\Development\arduinoprojects\bikesystem\bikesystem.h"
#endif
