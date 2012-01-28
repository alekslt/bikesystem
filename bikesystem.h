#ifndef BIKESYSTEM_H
#define BIKESYSTEM_H

#include "Arduino.h"

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
  
  Command(char _cmd[], char _description[], uint8_t _argn, CommandFn _fn)
  {
		strcpy(cmd, _cmd);
  	strcpy(description, _description);
  	argn = _argn;
  	fn = _fn;
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

#endif // BIKESYSTEM_H
