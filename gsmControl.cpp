/*
 * pinDef.h
 *
 *  Created on: Aug 25, 2015
 *      Author: popai
 */

#include "gsmControl.h"
#include <MyGSM.h>

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

byte Check_SMS();  //Check if there is SMS
void Check_Protocol(String str);
float Thermistor(int Tpin);

//The setup function is called once at startup of the sketch
void setup()
{
// Add your initialization code here
	Serial.begin(115200);
	Serial.println("system startup");

	uint8_t tri = 0;
	while ((error == 0) && (tri < 3))  //Check status
	{
		gsm.TurnOn(9600);          //module power on
		gsm.InitParam(PARAM_SET_1);          //configure the module
		gsm.Echo(0); 				 //enable/disable AT echo
		error = gsm.SendSMS(number, "Modul ON");
		++tri;
	}
	if (error == 0)
		Serial.println("GSM init error");

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

void loop()
{
// The loop function is called in an endless loop
	char inSerial[20];
	byte i = 0;
	byte time_s = 0;
	byte time = 0;
	delay(20);

	if (delEEPROM)
		return;

	if (config)
	{
		//Check data serial com
		if (Serial.available() > 0)
		{
			while (Serial.available() > 0)
			{
				inSerial[i] = (Serial.read()); //read data
				i++;
			}
			inSerial[i] = '\0';
			Check_Protocol(inSerial);
		}
		else if (Check_SMS())
		{
			config = true;
			if (strlen(sms_rx) != 0)
				Check_Protocol(sms_rx);
		}
	}
	else if (Check_SMS())
		; //Check if there is SMS

}

/**
 * Check_Protocol
 * @brief : take command from serial or sms interface and execute them.
 *
 * @param : String = command from serial or sms
 * @return: no return value
 */
void Check_Protocol(String inStr)
{
	char str[32];
	sprintf_P(str, PSTR("Command %s"), inStr);
	Serial.println(str);

	strcpy_P(str, PSTR("Check_Protocol"));
	Serial.println(str);

	if (!config && inStr.length() > 3)
		inStr.toCharArray(number, 20, 4);

	Serial.println(number);
	switch (inStr[0])
	{
	case 'a':  //Answer
		//Serial.println("No incoming call");
		break;

	case 's': //S //Send SMS
		sprintf_P(str, PSTR("Send SMS to %d"), number);
		Serial.println(str);
		error = gsm.SendSMS(number, text);
		if (error == 0)  //Check status
		{
			strcpy_P(str, PSTR("SMS ERROR \n"));
			Serial.println(str);
		}
		else
		{
			strcpy_P(str, PSTR("SMS OK \n"));
			Serial.println(str);
		}
		break;

	case 'p':  //Read-Write Phone Book
		if (inStr.length() > 2)
		{

			switch (inStr[1])
			{
			case 'd':  //Delete number in specified position  pd2
				error = gsm.DelPhoneNumber(inStr[2]);
				if (error != 0)
				{
					Serial.print("Phone number position ");
					Serial.print(inStr[2]);
					Serial.println(" deleted");
				}
				break;

			case 'g':  //Read from Phone Book position      pg2
				error = gsm.GetPhoneNumber(inStr[2], number);
				if (error != 0)  //Find number in specified position
				{
					Serial.print("Phone Book position ");
					Serial.print(inStr[2]);
					Serial.print(": ");
					Serial.println(number);
				}
				else  //Not find number in specified position
				{
					Serial.print("No Phone number in position ");
					Serial.println(inStr[2]);
				}
				break;
			case 'w':  //Write from Phone Book Position    pw2
				//error = gsm.WritePhoneNumber(inStr[2], number);
				if (error != 0)
				{
					Serial.print("Number ");
					Serial.print(number);
					Serial.print(" writed in Phone Book position ");
					Serial.println(inStr[2]);
				}
				else
					Serial.println("Writing error");
				break;

			}

		}
		else
			Serial.println("Writing error");
		break;

	}

	delay(15);

	return;
}

byte Check_SMS()  //Check if there is an sms 'type_sms'
{
	char str[32];
	byte pos_sms_rx;  //Received SMS position
	pos_sms_rx = gsm.IsSMSPresent(type_sms);
	if (pos_sms_rx != 0)
	{
		//Read text/number/position of sms
		//gsm.GetSMS(pos_sms_rx, number, sms_rx, 120);
		gsm.GetAuthorizedSMS(pos_sms_rx, number, sms_rx, 122, 1, 6);
		sprintf_P(str, PSTR("Received SMS from %d (sim position: %d)"), number,
				pos_sms_rx);
		Serial.println(str);
		Serial.println(sms_rx);
		if (del_sms == 1)  //If 'del_sms' is 1, i delete sms
		{
			error = gsm.DeleteSMS(pos_sms_rx);
			if (error == 1)
			{
				strcpy_P(str, PSTR("SMS deleted"));
				Serial.println(str);
			}
			else
			{
				strcpy_P(str, PSTR("SMS not deleted"));
				Serial.println(str);
			}
		}
	}
	return pos_sms_rx;
}

/**
 * @brief : Thermistor calculate the temperature in grades Celsius dead by a 10K thermistor
 *
 * @param : analog pin where connect the thermistor
 * @return: temperature in grade Celsius
 */
float Thermistor(int Tpin)
{
	float Vo;
	float logRt, Rt, T;
	float R = 9870; // fixed resistance, measured with multimeter
// c1, c2, c3 are calibration coefficients for a particular thermistor
	float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
	Vo = analogRead(Tpin);
//Vo = ReadADC(Tpin);
	Rt = R * (1024.0 / Vo - 1.0);
	logRt = log(Rt);
	T = (1.0 / (c1 + c2 * logRt + c3 * logRt * logRt * logRt)) - 273.15;
	return T;
}
