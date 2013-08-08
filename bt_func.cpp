// 
// 
// 

#include "bt_func.h"
#include "bikesystem.h"

#include <SoftwareSerial.h>
#include <string.h>

uint8_t bt_msg_status = 0;
uint8_t bt_initstate = 0;
uint8_t bt_ok = 0;
uint8_t bt_work = 0;
uint8_t bt_state = 1;
uint8_t bt_connect = 0;

// Bluetooth commands
void cmd_bt_ok(uint8_t id, char* args, Stream &ser)
{
	commands[id]->print_info(args, ser);

	bt_ok = 1;
}

void cmd_bt_state(uint8_t id, char* args, Stream &ser)
{
	commands[id]->print_info(args, ser);

	char *ptr = args;
	char *sub = get_token(NULL, &ptr, DELIM_BTAT);

	//ser.print("Token 1: ");
	//ser.println(sub);

	uint8_t t1 = atoi(sub);
	bt_state = t1;

	if ( bt_state == 1 )
	{
		bt_connect = 0;
		has_sent_greeting = false;
	}
	//ser.print("Token 1-converted: ");
	//ser.println(bt_state);
}

void cmd_bt_st(uint8_t id, char* args, Stream &ser)
{
}

void cmd_bt_work(uint8_t id, char* args, Stream &ser)
{
	commands[id]->print_info(args, ser);

	bt_work = 1;
}

void cmd_bt_connect(uint8_t id, char* args, Stream &ser)
{
	bt_connect = 1;
}

// Bluetooth Setup Code

void bt_reset()
{
 digitalWrite(BT_RESET_PIN, LOW);
 delay(2000);
 digitalWrite(BT_RESET_PIN, HIGH);
}
 
void bt_enter_commode()
{
 Serial.println("Enter ComMode"); 
 blueToothSerial.flush();
 delay(500);
 //digitalWrite(PIO11, LOW);
 digitalWrite(BT_RESET_PIN, LOW);
 delay(500);
 digitalWrite(BT_RESET_PIN, HIGH);
 delay(500);
 blueToothSerial.begin(38400);
}
 
void bt_enter_atmode()
{
 Serial.println("Resetting bluetooth"); 
 blueToothSerial.flush();
 delay(500);
 //digitalWrite(PIO11, HIGH);
 bt_reset();
 delay(500);
 blueToothSerial.begin(38400);
 
}

void bt_disc()
{
	digitalWrite(BT_DISCO_PIN, LOW);
	delay(100);
	digitalWrite(BT_DISCO_PIN, HIGH);
	delay(10);
}

//Checks if the response "OK" is received.
void bt_check_ok()
{
	char a,b;
	uint16_t watch_counter = 0;

	bt_ok = 0;
	Serial.print("Waiting for reply: bt_ok=");
	Serial.println(bt_ok);

	while( ! bt_ok )
	{
		get_command(blueToothSerial, Serial, DELIM_BTAT);
		delay(10);
		if (watch_counter++ > 5000)
		{
			bt_reset();
			bt_disc();
			bt_initstate = 0;
			Serial.println("Timed out. Restarting");
			return;
		}
	}
}

void bt_wait_for(uint8_t &value, uint8_t equals, char desc[])
{
	if ( value != equals )
	{
		Serial.println(desc);
		while ( value != equals )
		{
			get_command(blueToothSerial, Serial, DELIM_BTAT);
		}
	}
}

void bt_setup()
{
	char* cmd;
	boolean execute;

	pinMode(BT_STATUS_PIN, INPUT);
	pinMode(BT_DISCO_PIN, OUTPUT);
	pinMode(BT_RESET_PIN, OUTPUT);
	digitalWrite(BT_RESET_PIN, HIGH);
	
	blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400

	uint8_t bt_status = digitalRead(BT_STATUS_PIN);
	Serial.print("Bluetooth state: "); 
	Serial.println(bt_status ? "Active" : "Disconnected");

	if ( bt_status == 1 )
	{
		Serial.println("Device already connected. Skipping bt init");
		bt_state = 4;
		bt_connect = 1;

		return; 
	}

	bt_enter_atmode();

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
			execute = 0;
			break;
		case 5:
			bt_wait_for(bt_work, 1, "Waiting for module to switch mode");    
			execute = 0;
			//delay(1000);
			break;
		case 6:
			digitalWrite(BT_DISCO_PIN, LOW);    
			delay(1000); // This delay is required.
			cmd = "+INQ=1";
			break;
		case 7:
			bt_status = digitalRead(BT_STATUS_PIN);
			Serial.print("Bluetooth state: "); 
			Serial.println(bt_status ? "Active" : "Disconnected");
			execute = 0;
			break;

		case 8:
			delay(1000);
			blueToothSerial.flush();
			blueToothSerial.begin(38400);

			//bt_enter_commode();

			Serial.println("BT Setup done");
			Serial.println("--\n");
			return;    
			break;
		}

		if ( execute == 1 )
		{
			bt_ok = 0;
			bt_wait_for(bt_state, 1, "Waiting for ready state");

			blueToothSerial.print("\r\n");
			blueToothSerial.print(cmd);
			blueToothSerial.print("\r\n");
			Serial.println(cmd);
			delay(1500);
			bt_check_ok();
			delay(100);
		}
		Serial.println("--\n");
		bt_initstate++;
	}
}
