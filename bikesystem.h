#ifndef BIKESYSTEM_H
#define BIKESYSTEM_H

#include "Arduino.h"

// Defines
#define MAX_STRING_LEN  128
#define DEBUG_ENABLED  0

// Forwards
class Stream;
class SoftwareSerial;

typedef void (*SensorDataFn)(uint8_t id, Stream &ser);
typedef void (*CommandFn)(uint8_t id, char* args, Stream &ser);

struct Serial_info
{
  char key[6];
  char description[16];
  SensorDataFn fn;
	
	Serial_info(char _key[], char _description[], SensorDataFn _fn)
  {
		strcpy(key, _key);
  	strcpy(description, _description);
  	fn = _fn;
	}
	
	void print_config(Stream &ser)
	{
	  ser.print(key);
    ser.print(" ");
    ser.println(description);
	}
	
	void print_data(uint8_t i, Stream &ser)
	{
	  ser.print(key);
    ser.print(" ");
   	fn(i, ser);
	}
	 
};

struct Command
{
  char cmd[16];
  char description[32];
  uint8_t argn;
  CommandFn fn;
  bool sendBT;
  
  Command(char _cmd[], char _description[], uint8_t _argn, CommandFn _fn, bool _sendBT)
  {
		strcpy(cmd, _cmd);
  	strcpy(description, _description);
  	argn = _argn;
  	fn = _fn;
	sendBT = _sendBT;
	}
	
	void print_info(char* args, Stream &ser)
	{
	  ser.print("CMD: ");
	  ser.print(cmd);
	  ser.print(" Args: ");
	  ser.print(argn);
	  ser.print(" Desc: ");
	  ser.println(description);
	
	  ser.print("Args: ");
	  ser.println(args);
	}

};

extern struct Serial_info *sensors[10];
extern uint8_t sensor_amount;

extern struct Command *commands[10];
extern uint8_t command_amount;

extern const char DELIM_NORMAL[];
extern const char DELIM_BTAT[];

extern const uint8_t CMD_ASCII_START;
extern const uint8_t CMD_ASCII_END;

extern bool has_sent_greeting;

extern SoftwareSerial blueToothSerial;

char* get_token(char in_string[], char **ptr, const char delim[]);
void get_command(Stream &ser, Stream &serout, const char delim[]);

#endif // BIKESYSTEM_H
