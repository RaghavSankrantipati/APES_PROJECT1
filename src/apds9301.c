/*
 ============================================================================
 Name        : project.c
 Author      : Sai Raghavendra Sankrantipati
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdint.h>
#include<linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include<complex.h>

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

void write_controlreg(int fd, uint8_t val);
uint8_t read_controlreg(int fd);
void write_timingreg(int fd, uint8_t val);
uint8_t read_timingreg(int fd);
void write_interrupt_thresholdreg(int fd, uint8_t *write_array);
void read_interrupt_threshholdreg(int fd, uint8_t * read_array);
void write_interrupt_controlreg(int fd, uint8_t val);
uint8_t read_interrupt_controlreg(int fd);
uint8_t read_idreg(int fd);
void print_id(int fd);
uint16_t read_data0reg(int fd);
uint16_t read_data1reg(int fd);
float get_luminosity(int fd);
int sensor_init(int bus);


int sensor_init(int bus){
	int file;
	char filename[20];

	snprintf(filename, 19, "/dev/i2c-%d", bus);
	file = open(filename, O_RDWR);
	if (file < 0) {
		perror("Unable to open file");
		exit(-1);
	}
	int addr = slave_address; /* The I2C address */

	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		perror("Unable to ioctl");
		exit(-1);
	}
	return file;
}

void write_controlreg(int fd, uint8_t val){

	int buf = command_value | control_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	buf = val;
	if( write(fd, &buf, 1) != 1)
		  perror("Unable to write\n");

}


uint8_t read_controlreg(int fd){
	uint8_t buf =  command_value | control_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	if( read(fd, &buf, 1) != 1)
		perror("Unable to read\n");
	printf("%d\n", buf);
	return buf;
}


void write_timingreg(int fd, uint8_t val){

	int buf = command_value | timing_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	buf = val;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");

}


uint8_t read_timingreg(int fd){
	uint8_t buf =  command_value | timing_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	if( read(fd, &buf, 1) != 1)
		perror("Unable to read\n");
	printf("%d\n", buf);
	return buf;
}

void write_interrupt_thresholdreg(int fd, uint8_t *write_array){

	int buf = command_value | threshlowlow_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	buf = write_array[0];
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");

	buf = command_value | threshlowhigh_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	buf = write_array[1];
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");

	buf = command_value | threshhighlow_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	buf = write_array[2];
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");

	buf = command_value | threshhighhigh_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	buf = write_array[3];
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");

}

void read_interrupt_threshholdreg(int fd, uint8_t * read_array){

	uint8_t buf =  command_value | threshlowlow_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	if( read(fd, &buf, 1) != 1)
		perror("Unable to read\n");
	read_array[0] = buf;

	buf =  command_value | threshlowhigh_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	if( read(fd, &buf, 1) != 1)
		perror("Unable to read\n");
	read_array[1] = buf;

	buf =  command_value | threshhighlow_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	if( read(fd, &buf, 1) != 1)
		perror("Unable to read\n");
	read_array[2] = buf;

	buf =  command_value | threshhighhigh_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	if( read(fd, &buf, 1) != 1)
		perror("Unable to read\n");
	read_array[3] = buf;

}

void write_interrupt_controlreg(int fd, uint8_t val){
	uint8_t buf = command_value | int_control_reg ;
    if( write(fd, &buf, 1) != 1)
    	perror("Unable to write\n");
	buf = val;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
}


uint8_t read_interrupt_controlreg(int fd){
	uint8_t buf =  command_value | int_control_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	if( read(fd, &buf, 1) != 1)
		perror("Unable to read\n");
	printf("%d\n", buf);
	return buf;
}

uint8_t read_idreg(int fd){
	uint8_t buf =  command_value | id_reg ;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	if( read(fd, &buf, 1) != 1)
		perror("Unable to read\n");
	printf("%d\n", buf);
	return buf;
}

void print_id(int fd){
	uint8_t id2, id1 = read_idreg(fd);
	id2 = id1;
	printf("Part no: %d\n", (id1>>4 & 0xFF));
	printf("Rev no: %d\n", (id2 & 0x0F));
}

uint16_t read_data0reg(int fd){
	uint8_t buf =  command_value |  data0low_reg;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	uint8_t dl0;
	if( read(fd, &dl0, 1) != 1)
		perror("Unable to read\n");
	printf("%d\n", dl0);

	buf =  command_value |  data0high_reg;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	uint16_t dh0;
	if( read(fd, &dh0, 1) != 1)
		perror("Unable to read\n");
	printf("%d\n", dh0);

	uint16_t data0 = dh0<<8 | dl0;
	printf("adc0: %d\n", data0);
	return data0;
}


uint16_t read_data1reg(int fd){
	uint8_t buf =  command_value |  data1low_reg;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	uint8_t dl;
	if( read(fd, &dl, 1) != 1)
		perror("Unable to read\n");
	printf("%d\n", dl);

	buf =  command_value |  data0high_reg;
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
	uint16_t dh;
	if( read(fd, &dh, 1) != 1)
		perror("Unable to read\n");
	printf("%d\n", dh);

	uint16_t data1 = dh<<8 | dl;
	printf("adc1: %d\n", data1);
	return data1;
}


float get_luminosity(int fd){
	float ch0, ch1, adc, luminosity;

	ch0 = (float)read_data0reg(fd);
	ch1 = (float)read_data1reg(fd);

	adc = ch1/ch0;

	printf("ratio : %f", adc);

	if(adc>0 && adc <= 0.5)
		return luminosity = (0.0304 * ch0) - (0.062 * ch0 * powf(adc, 1.4));
	else if(adc>0.5 && adc<=0.61)
		return luminosity = (0.0224 * ch0) - (0.031 * ch1);
    else if((adc>0.61)&&(adc<=0.80))
        return luminosity= (0.0128 * ch0) - (0.0153 * ch1);
    else if((adc>0.80) && (adc<=1.30))
        return luminosity= (0.00146 * ch0) - (0.00112 * ch1);
    else if(adc > 1.30)
        return luminosity=0;

	return -1;
}
