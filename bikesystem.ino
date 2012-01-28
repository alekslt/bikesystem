

// Includes
#include "bikesystem.h"
#include <SoftwareSerial.h>

#include "DHT.h"

#include <string.h>

// Defines
#define MAX_STRING_LEN  128

#define DHTPIN 2     // what pin we're connected to

#define RxD 11
#define TxD 12

#define BT_STATUS_PIN 3
#define BT_RESET_PIN 4

#define DEBUG_ENABLED  0

// Attributes
DHT dht(DHTPIN, DHT11);

SoftwareSerial blueToothSerial(RxD,TxD);

struct Serial_info *sensors[10];
uint8_t sensor_amount = 0;

struct Command *commands[10];
uint8_t command_amount = 0;

char cmd_buf[128];
uint8_t cmd_pos = 0;
uint8_t bt_msg_status = 0;

uint8_t bt_initstate = 0;

const char DELIM_NORMAL[] = " ";
const char DELIM_BTAT[] = ":=";

const uint8_t CMD_ASCII_START = 0;
const uint8_t CMD_ASCII_END = 127;

// Declarations
void send_config(Stream &ser);
void send_sensors(Stream &ser);

char* get_token(char in_string[], char **ptr, const char delim[]);

void get_command(Stream &ser, Stream &serout, const char delim[]);

// Definitions

// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
void DHT_get_temp(uint8_t id, Stream &ser)
{
  float t = dht.readTemperature();

  if (isnan(t)) {
    ser.println("ERR");
  } 
  else {
    ser.println(t);
  }
}

void DHT_get_humidity(uint8_t id, Stream &ser)
{
  float h = dht.readHumidity();

  if (isnan(h)) {
    ser.println("ERR");
  } 
  else {
    ser.println(h);
  }  
}

void config_get()
{
	sensors[sensor_amount] = new Serial_info("DHT_H", "DHT Humidity", DHT_get_humidity);
  sensor_amount++;

	sensors[sensor_amount] = new Serial_info("DHT_T", "DHT Temp", DHT_get_temp);
  sensor_amount++;
}

void cmd_send_config(uint8_t id, char* args, Stream &ser);
void cmd_send_sensors(uint8_t id, char* args, Stream &ser);

void cmd_general(uint8_t id, char* args, Stream &ser);

void cmd_bt_ok(uint8_t id, char* args, Stream &ser);
void cmd_bt_state(uint8_t id, char* args, Stream &ser);
void cmd_bt_work(uint8_t id, char* args, Stream &ser);

void config_cmd()
{
	
	commands[command_amount] = new Command("CONFIG", "Send Config", 0, cmd_send_config);
  command_amount++;

  commands[command_amount] = new Command("+BTSTATE", "Bluetooth State", 1, cmd_bt_state);
  command_amount++;

  commands[command_amount] = new Command("WORK", "Bluetooth Mode", 1, cmd_bt_work);
  command_amount++;
	  
  commands[command_amount] = new Command("SEND_SENSORS", "Send Sensor Data", 0, cmd_send_sensors);
  command_amount++;

  commands[command_amount] = new Command("OK", "BT OK", 0, cmd_bt_ok);
  command_amount++; 
}

void send_config(Stream &ser)
{
  ser.println("Config Pairs");
  for (uint8_t i = 0; i < sensor_amount; i++)
  {
    sensors[i]->print_config(ser);
  }
}

void send_sensors(Stream &ser)
{
  ser.println("New DataGroup");
  for (uint8_t i = 0; i < sensor_amount; i++)
  {
  	sensors[i]->print_data(i, ser);
  }
}

char* get_token(char in_string[], char **ptr, const char delim[])
{
  static char copy[MAX_STRING_LEN];
  char *act = copy;
  char *sub;  

  // Since strtok consumes the first arg, make a copy
  if ( in_string != NULL )
  {
    strcpy(copy, in_string);
    sub = strtok_r(copy, delim, ptr);
  } 
  else {
    sub = strtok_r(NULL, delim, ptr);
  }

  return sub; 
}

void parse_command(char command[], Stream &serout, const char delim[])
{
  char *ptr;
  char *sub = get_token(command, &ptr, delim);
  for (int i = 0; i < command_amount; i++)
  {
    /*
    Serial.print("Comparing: ");
     Serial.print(commands[i].cmd);
     Serial.print("(");
     Serial.print(strlen(commands[i].cmd));
     
     Serial.print(") with ");
     Serial.print(sub);
     Serial.print("(");
     Serial.print(strlen(sub));
     Serial.println(")");
     */
    int ret = strcmp(commands[i]->cmd, sub);

    //Serial.print(" Ret: ");
    //Serial.println(ret);

    if ( ret == 0 )
    {
      //sub = strtok_r(NULL, delim, &ptr);
      commands[i]->fn(i, ptr, serout);
    }
  }
}

uint8_t bt_ok = 0;

void cmd_bt_ok(uint8_t id, char* args, Stream &ser)
{
  commands[id]->print_info(args, ser);

  //ser.print("Args: ");
  //ser.println(args);  

  bt_ok = 1;
}

uint8_t bt_state = 1;

void cmd_bt_state(uint8_t id, char* args, Stream &ser)
{
  commands[id]->print_info(args, ser);

  char *ptr = args;
  char *sub = get_token(NULL, &ptr, DELIM_BTAT);

  //ser.print("Token 1: ");
  //ser.println(sub);

  uint8_t t1 = atoi(sub);
  bt_state = t1;

  //ser.print("Token 1-converted: ");
  //ser.println(bt_state);
}

uint8_t bt_work = 0;

void cmd_bt_work(uint8_t id, char* args, Stream &ser)
{
	commands[id]->print_info(args, ser);

  bt_work = 1;
}


void BT_Disc()
{
  digitalWrite(BT_RESET_PIN, LOW);
  delay(100);
  digitalWrite(BT_RESET_PIN, HIGH);
  delay(10);
}

//Checks if the response "OK" is received.
void CheckOK()
{
  char a,b;
  uint16_t watch_counter = 0;
  
  bt_ok = 0;
  Serial.println("Waiting for reply");
  while( ! bt_ok )
  {
    get_command(blueToothSerial, Serial, DELIM_BTAT);

    /*
    if(int len = blueToothSerial.available())
     {
     a = blueToothSerial.read();
     //Serial.print("DEBUG: ");
     Serial.print(a);
     
     if('O' == a)
     {
     b = blueToothSerial.read();
     //Serial.print("DEBU2: ");
     Serial.print(b);
     if('K' == b)
     {
     //Serial.println("BREAKING");
     break;
     }
     
     }
     }
     */
     if (watch_counter++ > 10000)
     {
       BT_Disc();
       bt_initstate = 0;
       return;
     }
  }

  //Serial.println("DEBU2: START");
  //while( (a = blueToothSerial.read()) != -1)
  //{

  //   Serial.print(b);
  //Wait until all response chars are received
  //}
  //Serial.println("DEBU2: END");
}

void wait_for(uint8_t &value, uint8_t equals, char desc[])
{
  if ( value != equals )
  {
    Serial.println("Waiting for module to switch mode");
    while ( value != equals )
    {
      get_command(blueToothSerial, Serial, DELIM_BTAT);
    }
  }
}

void setupBlueToothConnection()
{
  char* cmd;
  boolean execute;
  
  pinMode(BT_STATUS_PIN, INPUT);
  pinMode(BT_RESET_PIN, OUTPUT);   

  uint8_t bt_status = digitalRead(BT_STATUS_PIN);
  Serial.print("Bluetooth state: "); 
  Serial.println(bt_status ? "Active" : "Disconnected");

  //digitalWrite(BT_RESET_PIN, LOW); 
  BT_Disc();
  
  if ( bt_status == 1 )
  {
     return; 
  }
  
  while ( 1 )
  {
    execute = 1;
    
    Serial.print("Step: ");
    Serial.println(bt_initstate);
    
    switch ( bt_initstate )
    {
    case 0:
      Serial.println("setupBlueToothConnection");
      cmd = "+STWMOD=0";
      break;
    case 1:
      cmd = "+STNA=BikeComputer";
      break;
    case 2:
      cmd = "+STAUTO=0";
      break;
    case 3:
      cmd = "+STOAUT=1";
      break;
    case 4:    
      //delay(100); // This delay is required.
      //sendBlueToothCommand("\r\n +STPIN=2222\r\n");
      break;
    case 5:
			wait_for(bt_work, 1, "Waiting for module to switch mode");    
      execute = 0;
      //delay(1000);
      break;
    case 6:
      digitalWrite(BT_RESET_PIN, LOW);    
      delay(1000); // This delay is required.
      cmd = "+INQ=1";
      break;
    case 7:
      bt_status = digitalRead(BT_STATUS_PIN);
      Serial.print("Bluetooth state: "); 
      Serial.println(bt_status ? "Active" : "Disconnected");
      return;    
      break;

    }
    
    if ( execute == 1 )
    {
      bt_ok = 0;
      wait_for(bt_state, 1, "Waiting for ready state");
      
      blueToothSerial.print("\r\n");
      blueToothSerial.print(cmd);
      blueToothSerial.print("\r\n");
      Serial.print(cmd);
      //delay(1500);
      CheckOK();
      //delay(1000);
    }
    Serial.println("\n--\n");
    bt_initstate++;
  }
}


void setup_softwareserial()
{
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  setupBlueToothConnection();
}

void setup() {
  //config_get();
  Serial.begin(38400); 
  Serial.println("Bike Computer Setup");

  //Serial.println("config_get");
  config_get();
  config_cmd();

  blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  delay(1000);
  setup_softwareserial();

  //Serial.println("DHT Begin");
  dht.begin();

  //Serial.println("Send Config");
  send_config(Serial);
}

void cmd_send_config(uint8_t id, char* args, Stream &ser)
{
  send_config(ser); 
}

void cmd_send_sensors(uint8_t id, char* args, Stream &ser)
{
  send_sensors(ser);  
}

void cmd_general(uint8_t id, char* args, Stream &ser)
{
  commands[id]->print_info(args, ser);
}

void get_command(Stream &ser, Stream &serout, const char delim[])
{
  // wait for character to arrive
  if (ser.available() != 0)
  {
    if ( cmd_pos == 0 )
    {
      bt_msg_status = digitalRead(BT_STATUS_PIN);
    }

    char c = ser.read();
    if ( c == '\n' ) { 
      return; 
    }

    if ( (uint8_t)c < CMD_ASCII_START || (uint8_t)c > CMD_ASCII_END ) { 
      return; 
    }
    cmd_buf[cmd_pos++] = c;

    /*
    serout.print(c);
     serout.print("[");
     serout.print((uint8_t)c);
     serout.print("]");
     */
    if (c == '\r') // is it the terminator byte?
    {
      cmd_buf[--cmd_pos] = 0;

      if ( cmd_pos > 1 )
      {

        serout.print("Command: (");
        serout.print(cmd_pos);
        serout.print(")"); 
        serout.print(cmd_buf);
        serout.print(" BTstate: "); 
        serout.println(bt_msg_status ? "Active" : "Disconnected");
        parse_command(cmd_buf, serout, delim);
      }
      cmd_pos = 0;
      //blueToothSerial.println(cmd_buf);
      //You can write you BT communication logic here
    }
  }
  //  buf[i] = 0; // 0 string terminator just in case
}  

void loop() {
  //send_sensors();
  //blueToothSerial.println("Test");
  bt_msg_status = digitalRead(BT_STATUS_PIN);
  get_command(blueToothSerial, Serial, bt_msg_status ? DELIM_NORMAL : DELIM_BTAT );
}



