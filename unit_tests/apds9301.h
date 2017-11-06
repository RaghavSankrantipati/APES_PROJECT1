/**********************************************************************
*@Filename:tmp102.h
*
*@Description:This is a header for library for APDS 9301 sensor
*@Author:Sai Raghavendra Sankrantipati
*@Date:11/5/2017
*@compiler:arm-linux-gnueabihf-gcc
*@Usage : Connect APDS 9301 to I2C 2 and use any of the library function to read and write registers
 **********************************************************************/


#ifndef APDS9301_H_
#define APDS9301_H_

#include<stdint.h>

/* Slave address of APDS 9301*/
#define slave_address 0x39

/*Default Command value*/
#define command_value	0x80

/*Address of different registers*/
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

/*Different Power options*/
#define power_up	0x03
#define shut_down	0x00

/*Different timings in registers*/
#define time_13ms 0x00
#define time_101ms	0x01
#define time_402ms	0x02
#define max_gain 0x10

/*Enable or disable interrupts*/
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
int print_id(int fd);
uint16_t read_data0reg(int fd);
uint16_t read_data1reg(int fd);
float get_luminosity(int fd);
int sensor_init(int bus);
int close_apds9301(int fd);
int rw_allregs_apds(int fd);


#endif /* APDS9301_H_ */
