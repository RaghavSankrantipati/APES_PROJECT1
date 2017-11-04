
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdint.h>
#include<linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include<complex.h>
#include"../include/tmp102.h"

int tmp102_init(int bus){
	int file;
	char filename[20];

	snprintf(filename, 19, "/dev/i2c-%d", bus);
	file = open(filename, O_RDWR);
	if (file < 0) {
		perror("Unable to open file\n");
		exit(-1);
	}
	int addr = TMP102_ADDRESS; /* The I2C address */

	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		perror("Unable to ioctl\n");
		exit(-1);
	}
	return file;
}

int rw_allregs_tmp102(int fd){
	int status;
	uint16_t *res;
	res = malloc(sizeof(uint16_t));
	status = write_configreg(fd, CONFIG_DEFAULT);
	if( status ==  FAIL )
		return FAIL;
	status = read_configreg(fd, res);
	if( status ==  FAIL )
		return FAIL;
	status = read_tempreg(fd, res);
	if( status ==  FAIL )
		return FAIL	;
	return SUCCESS;
}

int write_pointerreg(int fd, uint8_t reg){
	uint8_t buf = POINTER_ADDRESS | reg;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	return SUCCESS;

}

int write_configreg(int fd, uint16_t config_val){
	write_pointerreg(fd, CONFREG_ADDRESS);

	uint8_t temp1 = (CONFIG_DEFAULT|config_val)>>8;
	uint8_t temp2 = (CONFIG_DEFAULT|config_val);
	uint8_t buf[3] = { POINTER_ADDRESS | CONFREG_ADDRESS, temp2, temp1};

	if( write(fd, buf, 3) != 3){
		perror("Unable to write\n");
		return FAIL;
	}

	return SUCCESS;
}

int read_configreg(int fd, uint16_t * res){

	write_pointerreg(fd, CONFREG_ADDRESS);
	uint16_t buf;
	if( read(fd, &buf, 2) != 2){
		perror("Unable to read\n");
		return FAIL;
	}
	printf("Config Register (TMP 102) : %d\n", buf);
	return SUCCESS;
}

int read_tempreg(int fd, uint16_t *res){
	write_pointerreg(fd, TEMPREG_ADDRESS);
	uint8_t buf[2];
	if( read(fd, buf, 2) != 2){
		perror("Unable to read\n");
		return FAIL;
	}

	int temp = (uint16_t)buf[0]<<4 | buf[0]>>4;
	*res = temp;
	return SUCCESS;
}


int convert_temp(int temp, int mode){

	if(mode == EXTND_MODE){

		if((temp & 0xA00) == 0){
			float celsius = temp * 0.0625;
			float fahrenheit = (1.8 * celsius) +32;
			float kelvin = celsius + 273.15;

			printf("Temperature in \nCelsius: %f\nFahrenheit:  %f \nKelvin: %f\n", celsius, fahrenheit, kelvin);
			return SUCCESS;
		} else {
			temp = temp ^ 0xFFFF;
			float celsius = temp * (-0.0625);
			float fahrenheit = (1.8 * celsius) +32;
			float kelvin = celsius + 273.15;

			printf("Temperature in \nCelsius: %f\nFahrenheit:  %f \nKelvin: %f\n", celsius, fahrenheit, kelvin);
			return SUCCESS;
		}
	} else if(mode == CONFIG_DEFAULT){
		if((temp & 0x800) == 0){
			float celsius = temp * 0.0625;
			float fahrenheit = (1.8 * celsius) +32;
			float kelvin = celsius + 273.15;

			printf("Temperature in \nCelsius: %f\nFahrenheit:  %f \nKelvin: %f\n", celsius, fahrenheit, kelvin);
			return SUCCESS;
		} else {
			temp = temp ^ 0xFFFF;
			float celsius = temp * (-0.0625);
			float fahrenheit = (1.8 * celsius) +32;
			float kelvin = celsius + 273.15;

			printf("Temperature in \nCelsius: %f\nFahrenheit:  %f \nKelvin: %f\n", celsius, fahrenheit, kelvin);
			return SUCCESS;
		}
	}
	return FAIL;
}

int shutdown_mode(int fd, int mode){
	return write_configreg(fd, mode);
}

int change_resolution(int fd, int mode){
	return write_configreg(fd, mode);
}

int print_temperature(int fd, int mode){
	write_configreg(fd, mode);
	uint16_t *temp = malloc(sizeof(uint16_t));

	read_tempreg(fd, temp);
	convert_temp(*temp, mode);
	return SUCCESS;
}

int close_tmp102(int fd){
	close(fd);
	return 1;
}


