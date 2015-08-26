/*
 * cmd.h
 *
 *  Created on: Apr 15, 2013
 *      Author: popai
 */

#ifndef CMD_H_
#define CMD_H_


int8_t CfgCmd(char *buffer);
void ReadEprom(char* str_citit, int const adresa);
void DellEprom();
void Config(char *nrtel, char *inmsg);
void Comand(char *nrtel, char *inmsg);
float Thermistor(int RawADC);
void StareOUT(char *nrtel);
void StareTMP(char *nrtel);
void VerificIN();

#endif /* CMD_H_ */
