/*
 * pinDef.h
 *
 *  Created on: Aug 25, 2015
 *      Author: popai
 */

#include "gsmControl.h"
#include <GSM_Shield.h>

//void Check_SMS();  //Check if there is SMS
//void Check_Protocol(String str);

//**************************************************************************
char number[] = "+39123456789";  	//Destination number
char text[] = "hello world";  	    //SMS to send
byte type_sms = SMS_ALL;      	//Type of SMS
byte del_sms = 1;               	//0: No deleting sms - 1: Deleting SMS
//**************************************************************************

GSM gsm;
char sms_rx[122]; //Received text SMS
//int inByte=0;    //Number of byte received on serial port
char number_incoming[20];
int call;
int error;
bool config = false, delEEPROM = false;

//The setup function is called once at startup of the sketch
void setup()
{
// Add your initialization code here
	Serial.begin(115200);
	Serial.println("system startup");
	gsm.TurnOn(9600);          			//module power on
	gsm.InitParam(PARAM_SET_1);         //configure the module
	gsm.Echo(0);               			//enable AT echo

	//if (digitalRead(jp2) == LOW)
	if ((PINC & (1 << PORTC4)) == 0)
	{
		delEEPROM = true;
		//DellEprom();
	}
	//if (digitalRead(jp3) == LOW)
	if ((PINC & (1 << PORTC5)) == 0)
	{
		config = true;
	}


}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
}
