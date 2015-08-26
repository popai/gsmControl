/*
 * pinDef.h
 *
 *  Created on: Aug 25, 2015
 *      Author: popai
 */

#include "gsmControl.h"
#include <MyGSM.h>
#include <EEPROM.h>
#include <avr/eeprom.h>

#include "cmd.h"
#include "pinDef.h"
//**************************************************************************
//char number[] = "+39123456789";  	//Destination number
//char text[] = "hello world";  	    //SMS to send

//**************************************************************************

extern GSM gsm;		//gsm handler class define in cmd.h

char sms_rx[122];   //Received text SMS
char number[20];	//sender phone number

int error;			//error from function

bool config = false, delEEPROM = false;	//define state of controller

byte Check_SMS();  //Check if there is SMS

//float Thermistor(int Tpin);

/**
 * @brief : The setup function is called once at startup of the sketch
 *
 * @param : no parameters
 * @return: no returns
 */
void setup()
{
// Add your initialization code here
	pinSetUp();			//set pins

	Serial.begin(115200);	//start hardwre serial
	delay(100);
	Serial.println("system startup");

	strcpy_P(sms_rx, PSTR("Comanda ne scrisa"));
	EEPROM.update((int)255, (uint8_t)sms_rx);
	//startup gsm module
	uint8_t tri = 0;
	while ((error == 0) && (tri < 3))  	//Check status
	{
		gsm.TurnOn(9600);          		//module power on
		gsm.InitParam(PARAM_SET_1);		//configure the module
		gsm.Echo(0); 				 	//enable/disable AT echo
		error = gsm.SendSMS(number, "Modul ON"); //TODO change to adder metode
		++tri;
	}
	if (error == 0)
		Serial.println("GSM init error");

	//if (digitalRead(jp2) == LOW)
	if ((PINC & (1 << PINC4)) == 0)
	{
		delEEPROM = true;
		DellEprom();
	}
	//if (digitalRead(jp3) == LOW)
	if ((PINC & (1 << PINC5)) == 0)
	{
		config = true;
	}

}

void loop()
{
// The loop function is called in an endless loop
	int id;
	byte i = 0;
	if (delEEPROM)
		return;

	if (config)
	{
		if (Serial.available() > 0)
		{
			while (Serial.available() > 0)
			{
				sms_rx[i] = (Serial.read()); //read data
				i++;
			}
			sms_rx[i] = '\0';

			delay(100);

			if (strstr_P(sms_rx, PSTR("citeste")) != 0)
			{
				for (int i = 0; i <= 512; i += 18)
					ReadEprom(sms_rx, i);
			}
			else if (strlen(sms_rx) != 0)
				CfgCmd(sms_rx);
			*sms_rx = 0x00;
		}

		else
		{
			id = Check_SMS();
			if (id == GETSMS_AUTH_SMS || id == GETSMS_NOT_AUTH_SMS)
				Config(number, sms_rx);
			*sms_rx = 0x00;
			id = 0;
		}
	}
	else
	{
		VerificIN();
		id = Check_SMS();
		if (id == GETSMS_AUTH_SMS)
			Comand(number, sms_rx);
		else if(id == GETSMS_NOT_AUTH_SMS)
		{
			//Check if receive a pas
			char buffer[24];
			ReadEprom(buffer, 18 * 21);
			if (strcmp(buffer, sms_rx) == 0)
			{
				uint8_t nr_pfonnr;
				nr_pfonnr = eeprom_read_byte((const uint8_t *) 18);

				//write number on sim
				if (nr_pfonnr < 6) //max 6 number
				{
					error = gsm.WritePhoneNumber(nr_pfonnr, number);
					if (error != 0)
					{
						sprintf_P(buffer,
								PSTR("Number %s writed in Phone Book position %c"),
								number, nr_pfonnr);
						Serial.println(buffer);
						++nr_pfonnr;
						eeprom_write_byte((uint8_t *) 18, nr_pfonnr);
						strcpy_P(buffer, PSTR("Acceptat"));
						gsm.SendSMS(number, buffer);

					}
					else
					{
						strcpy_P(buffer, PSTR("Writing error"));
						Serial.println(buffer);
						gsm.SendSMS(number, buffer);
					}
				}
				else
				{
					strcpy_P(buffer, PSTR("No free slot"));
					gsm.SendSMS(number, buffer);
				}
			}


		}
		*number = 0x00;
		*sms_rx = 0x00;
		id = 0;
	}
	delay(1000);

}

/**
 * @brief :Check if there is an sms
 *
 * @param : no parameters
 * @return: Received SMS position
 */

byte Check_SMS()
{
	char str[32];
	byte pos_sms_rx = -1;  //Received SMS position
	pos_sms_rx = gsm.IsSMSPresent(SMS_UNREAD);
	if (pos_sms_rx > 0)
	{
		//Read text/number/position of sms
		//gsm.GetSMS(pos_sms_rx, number, sms_rx, 120);
		error = gsm.GetAuthorizedSMS(pos_sms_rx, number, sms_rx, 122, 1, 6);
		if (error == GETSMS_AUTH_SMS)
		{
			sprintf_P(str, PSTR("Received SMS from %d (sim position: %d)"),
					number, pos_sms_rx);
			Serial.println(str);
			Serial.println(sms_rx);
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
			return GETSMS_AUTH_SMS;
		}
		else
			return	GETSMS_NOT_AUTH_SMS;
	}
	return pos_sms_rx;
}

