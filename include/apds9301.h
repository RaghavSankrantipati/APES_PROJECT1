/*
 * apds9301.h
 *
 *  Created on: Nov 3, 2017
 *      Author: raghav
 */

#ifndef APDS9301_H_
#define APDS9301_H_

#include<stdint.h>

#define slave_address 0x39

#define command_value	0x80
#define control_reg	0x00
#define timing_reg	0x01
#define threshlowlow_reg	0x02
#define threshlowhigh_reg	0x03
#define threshhighlow_reg	0x04
#define	threshhighhigh_reg	0x05
#define int_control_reg		0x06
#define id_reg				0x0A
#define data0low_reg  		0x0C
#define data0high_reg		0x0D
#define data1low_reg		0x0E
#define data1high_reg		0x0F

#define power_up	0x03
#define shut_down	0x00

#define time_13ms 0x00
#define time_101ms	0x01
#define time_402ms	0x02
#define max_gain 0x10

#define INT_ENABLE	0x10
#define INT_DISABLE	0x00

int write_controlreg(int fd, uint8_t val);
uint8_t read_controlreg(int fd);
int write_timingreg(int fd, uint8_t val);
uint8_t read_timingreg(int fd);
int write_interrupt_thresholdreg(int fd, uint8_t *write_array);
int read_interrupt_threshholdreg(int fd, uint8_t * read_array);
int write_interrupt_controlreg(int fd, uint8_t val);
uint8_t read_interrupt_controlreg(int fd);
uint8_t read_idreg(int fd);
void print_id(int fd);
uint16_t read_data0reg(int fd);
uint16_t read_data1reg(int fd);
float get_luminosity(int fd);
int sensor_init(int bus);
int close_apds9301(int fd);
int rw_allregs(int fd);


#endif /* APDS9301_H_ */
