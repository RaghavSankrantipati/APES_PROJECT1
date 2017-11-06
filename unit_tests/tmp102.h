/*
 * tmp102.h
 *
 *  Created on: Nov 3, 2017
 *      Author: raghav
 */

#ifndef TMP102_H_
#define TMP102_H_

/**********************************************************************
*@Filename:tmp102.h
*
*@Description:This is a header for library for TMP102 sensor
*@Author:Sai Raghavendra Sankrantipati
*@Date:11/5/2017
*@compiler:arm-linux-gnueabihf-gcc
*@Usage : Connect TMP102 to I2C 2 and use any of the library function to read and write registers
 **********************************************************************/

#include<stdint.h>

/*Change to 1 to check library functions from main.c*/
#define CHECK_LIBRARY  	0x00

/*Address on I2C device*/
#define TMP102_ADDRESS	0x48

/*Address to pointer register*/
#define POINTER_ADDRESS	0x00
/*Address of different registers
 * Or it with pointer registers
 */
#define TEMPREG_ADDRESS	0x00
#define	CONFREG_ADDRESS	0x01
#define	TLOWREG_ADDRESS	0x02
#define	THIGHREG_ADDRESS	0x03

/*Default configuration on register*/
#define	CONFIG_DEFAULT	0xA060

/*Different modes in config_register
 * Or these with CONFIG_DEFAULT
 */
#define SHUTDOWN_MODE	0x0001
#define	THERMOSTAT_MODE	0x0002
#define POLARITY		0x0004
#define ONESHOT_MODE	0x0080
#define	EXTND_MODE		0x1000

/*Different modes for temperature conversions
 */
#define CELSIUS			0x00
#define FAHRENHEIT		0x01
#define	KELVIN			0x02

/*Enum of returns*/
enum{
	SUCCESS = 0,
	FAIL = -1
} returns;

int tmp102_init(int bus);
int write_pointerreg(int fd, uint8_t reg);
int write_configreg(int fd, uint16_t config_val);
int read_configreg(int fd, uint16_t * res);
int read_tempreg(int fd, uint16_t *res);
int convert_temp(int temp, int mode);
int shutdown_mode(int fd, int mode);
int change_resolution(int fd, int mode);
int print_temperature(int fd, int mode);
int close_tmp102(int fd);
int rw_allregs_tmp102(int fd);
float callibrate_temp(int temp, uint8_t mode);



#endif /* TMP102_H_ */
