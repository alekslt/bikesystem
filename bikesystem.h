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
};

struct Command
{
  char cmd[16];
  char description[32];
  uint8_t argn;
  CommandFn fn;
};

#endif // BIKESYSTEM_H
