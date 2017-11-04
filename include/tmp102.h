/*
 * tmp102.h
 *
 *  Created on: Nov 3, 2017
 *      Author: raghav
 */

#ifndef TMP102_H_
#define TMP102_H_

#include<stdint.h>


#define TMP102_ADDRESS	0x48

#define POINTER_ADDRESS	0x00
#define TEMPREG_ADDRESS	0x00
#define	CONFREG_ADDRESS	0x01
#define	TLOWREG_ADDRESS	0x02
#define	THIGHREG_ADDRESS	0x03

#define	CONFIG_DEFAULT	0xA060

#define SHUTDOWN_MODE	0x0001
#define	THERMOSTAT_MODE	0x0002
#define POLARITY		0x0004
#define ONESHOT_MODE	0x0080
#define	EXTND_MODE		0x1000

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



#endif /* TMP102_H_ */
