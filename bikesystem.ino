

// Includes
#include "bikesystem.h"
#include <SoftwareSerial.h>
#include "DHT.h"
#include <string.h>

#include "bt_func.h"

// Defines
#define MAX_STRING_LEN  128
#define DHTPIN 2     // what pin we're connected to
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


const char DELIM_NORMAL[] = " ";
const char DELIM_BTAT[] = ":=";

const uint8_t CMD_ASCII_START = 0;
const uint8_t CMD_ASCII_END = 127;

// General vars
bool has_sent_greeting = false;

// Declarations
void send_config(Stream &ser);
void send_sensors(Stream &ser);

// Command declerations
void cmd_send_config(uint8_t id, char* args, Stream &ser);
void cmd_send_sensors(uint8_t id, char* args, Stream &ser);

void cmd_general(uint8_t id, char* args, Stream &ser);


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

// Command Interpreter
void config_get()
{
	sensors[sensor_amount] = new Serial_info("DHT_H", "DHT Humidity", DHT_get_humidity);
	sensor_amount++;

	sensors[sensor_amount] = new Serial_info("DHT_T", "DHT Temp", DHT_get_temp);
	sensor_amount++;
}

void config_cmd()
{

	commands[command_amount] = new Command("CONFIG", "Send Config", 0, cmd_send_config, true);
	command_amount++;

	commands[command_amount] = new Command("SEND_SENSORS", "Send Sensor Data", 0, cmd_send_sensors, true);
	command_amount++;

	commands[command_amount] = new Command("OK", "BT OK", 0, cmd_bt_ok, false);
	command_amount++; 

	commands[command_amount] = new Command("+BTSTATE", "Bluetooth State", 1, cmd_bt_state, false);
	command_amount++;

	commands[command_amount] = new Command("+BTST", "Bluetooth St", 1, cmd_bt_st, false);
	command_amount++;

	commands[command_amount] = new Command("WORK", "Bluetooth Mode", 1, cmd_bt_work, false);
	command_amount++;

	commands[command_amount] = new Command("CONNECT", "Bluetooth Mode", 1, cmd_bt_connect, false);
	command_amount++;
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
	serout.print("\nParsing command: ");
	serout.println(command);

	char *ptr;
	char *sub = get_token(command, &ptr, delim);
	for (int i = 0; i < command_amount; i++)
	{
		int ret = strcmp(commands[i]->cmd, sub);

		if ( ret == 0 )
		{
			if (commands[i]->sendBT == true)
			{
				commands[i]->fn(i, ptr, blueToothSerial);
			} else {
				commands[i]->fn(i, ptr, serout);
			}
			return;
		}
	}
	serout.print("\nCommand not found: ");
	serout.println(command);
}

// General commands setup
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
	//serout.print("get_command: is_available=");
	//serout.println(ser.available());
	//delay(500);
	// wait for character to arrive
	if ( ser.available() )
	{
		if ( cmd_pos == 0 )
		{
			bt_msg_status = digitalRead(BT_STATUS_PIN);
		}

		char c = ser.read();
		serout.print(c);
		serout.print("[");
		serout.print((uint8_t)c);
		serout.print("]");

		if ( bt_msg_status == 1 && bt_state == 4 && bt_connect == 1)
		{
			//ser.write(c);
		}

		if ( c == '\n' ) {
			//serout.println("End of Line");
			return;
		}

		if ( (uint8_t)c < CMD_ASCII_START || (uint8_t)c > CMD_ASCII_END ) { 
			return; 
		}
		cmd_buf[cmd_pos++] = c;

		if (c == '\r') // is it the terminator byte?
		{
			cmd_buf[--cmd_pos] = 0;

			if ( cmd_pos > 1 )
			{

				serout.print("\nCommand: (");
				serout.print(cmd_pos);
				serout.print(")"); 
				serout.print(cmd_buf);
				//serout.print(" BTstate: "); 
				//serout.println(bt_msg_status ? "Active" : "Disconnected");
				parse_command(cmd_buf, serout, delim);
			}
			cmd_pos = 0;
			//blueToothSerial.println(cmd_buf);
			//You can write you BT communication logic here
		}
	}
	//  buf[i] = 0; // 0 string terminator just in case
}


// Sensor and Actuator Commands
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

// Main setup and loop

void setup_softwareserial()
{
	pinMode(RxD, INPUT);
	pinMode(TxD, OUTPUT);
	//bt_setup();
}

void setup() {
	//config_get();
	Serial.begin(38400); 
	Serial.println("Bike Computer Setup");

	//Serial.println("config_get");
	config_get();
	config_cmd();

	//delay(1000);
	setup_softwareserial();

	bt_setup();

	//Serial.println("DHT Begin");
	dht.begin();

	//Serial.println("Send Config");
	send_config(Serial);
} 

void loop() {
	bt_msg_status = digitalRead(BT_STATUS_PIN);

	if ( !has_sent_greeting )
	{
		if ( bt_state == 4 && bt_connect == 1)
		{
			delay(2000);
			Serial.println("BT Connected. Sending greeting");
			send_config(blueToothSerial);
			has_sent_greeting = true;
			blueToothSerial.flush();
		}

	}

	// get_command(blueToothSerial, Serial, (has_sent_greeting && bt_msg_status == 1) ? DELIM_NORMAL : DELIM_BTAT );
	get_command(blueToothSerial, Serial, DELIM_BTAT );
	
}



