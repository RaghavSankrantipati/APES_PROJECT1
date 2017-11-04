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

#include"../include/apds9301.h"
#include"../include/tmp102.h"

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

int rw_allregs_apds(int fd){

	int status;
	status = write_controlreg(fd, 0x03);
	if( status ==  FAIL )
		return FAIL;
	status = read_controlreg(fd);
	if( status ==  FAIL )
		return FAIL;
	status = write_timingreg(fd, 0x12);
	if( status ==  FAIL )
		return FAIL;
	status = read_timingreg(fd);
	if( status ==  FAIL )
		return FAIL;
	uint8_t arr[4] = {0, 0, 0, 0};

	status = read_interrupt_threshholdreg(fd, arr);
	if( status ==  FAIL )
		return FAIL;
	status = write_interrupt_controlreg(fd, 0x0F);
	if( status ==  FAIL )
		return FAIL;
	status = read_idreg(fd);
	if( status ==  FAIL )
		return FAIL;
	status = read_data0reg(fd);
	if( status ==  FAIL )
		return FAIL;
	status = read_data1reg(fd);
	if( status ==  FAIL )
		return FAIL;

	return SUCCESS;

}

int write_controlreg(int fd, uint8_t val){

	int buf = command_value | control_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	buf = val;
	if( write(fd, &buf, 1) != 1){
		  perror("Unable to write\n");
		  return FAIL;
	}
	return 0;
}


uint8_t read_controlreg(int fd){
	uint8_t buf =  command_value | control_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	if( read(fd, &buf, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}

	return buf;
}


int write_timingreg(int fd, uint8_t val){

	int buf = command_value | timing_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	buf = val;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	return SUCCESS;

}


uint8_t read_timingreg(int fd){
	uint8_t buf =  command_value | timing_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	if( read(fd, &buf, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}

	return buf;
}

int write_interrupt_thresholdreg(int fd, uint8_t *write_array){

	int buf = command_value | threshlowlow_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	buf = write_array[0];
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}

	buf = command_value | threshlowhigh_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	buf = write_array[1];
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}

	buf = command_value | threshhighlow_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	buf = write_array[2];{
	if( write(fd, &buf, 1) != 1)
		perror("Unable to write\n");
		return FAIL;
	}
	buf = command_value | threshhighhigh_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	buf = write_array[3];
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	return SUCCESS;

}

int read_interrupt_threshholdreg(int fd, uint8_t * read_array){

	uint8_t buf =  command_value | threshlowlow_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	if( read(fd, &buf, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}
	read_array[0] = buf;

	buf =  command_value | threshlowhigh_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	if( read(fd, &buf, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}
	read_array[1] = buf;

	buf =  command_value | threshhighlow_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	if( read(fd, &buf, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}
	read_array[2] = buf;

	buf =  command_value | threshhighhigh_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	if( read(fd, &buf, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}
	read_array[3] = buf;
	return SUCCESS;

}

int write_interrupt_controlreg(int fd, uint8_t val){
	uint8_t buf = command_value | int_control_reg ;
    if( write(fd, &buf, 1) != 1){
    	perror("Unable to write\n");
    	return FAIL;
    }
	buf = val;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	return SUCCESS;
}


uint8_t read_interrupt_controlreg(int fd){
	uint8_t buf =  command_value | int_control_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	if( read(fd, &buf, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}

	return buf;
}

uint8_t read_idreg(int fd){
	uint8_t buf =  command_value | id_reg ;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	if( read(fd, &buf, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}

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
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	uint8_t dl0;
	if( read(fd, &dl0, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}


	buf =  command_value |  data0high_reg;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	uint16_t dh0;
	if( read(fd, &dh0, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}


	uint16_t data0 = dh0<<8 | dl0;
	return data0;
}


uint16_t read_data1reg(int fd){
	uint8_t buf =  command_value |  data1low_reg;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	uint8_t dl;
	if( read(fd, &dl, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}

	buf =  command_value |  data0high_reg;
	if( write(fd, &buf, 1) != 1){
		perror("Unable to write\n");
		return FAIL;
	}
	uint16_t dh;
	if( read(fd, &dh, 1) != 1){
		perror("Unable to read\n");
		return FAIL;
	}

	uint16_t data1 = dh<<8 | dl;
	return data1;
}


float get_luminosity(int fd){
	float ch0, ch1, adc, luminosity;

	write_controlreg(fd, power_up);
	write_timingreg(fd, time_402ms|max_gain);
	usleep(5000);

	ch0 = (float)read_data0reg(fd);
	ch1 = (float)read_data1reg(fd);

	adc = ch1/ch0;

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

	return FAIL;
}

int close_apds9301(int fd){
	close(fd);
	return SUCCESS;
}
