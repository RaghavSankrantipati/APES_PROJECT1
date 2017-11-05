
/**********************************************************************
*@Filename:tmp102.c
*
*@Description:This is a library for APDS 9301 sensor
*@Author:Sai Raghavendra Sankrantipati
*@Date:11/5/2017
*@compiler:arm-linux-gnueabihf-gcc
*@Usage : Connect APDS 9301 to I2C 2 and use any of the library function to read and write registers
 **********************************************************************/


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


/* Initialise sensor on I2C bus 2
 * Pass 2 for I2C bus 2 and  1 for I2C bus 1
 * */

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

/* Read write all registers
 * Use it for debug
 */
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
	/*Interrupt threshhold register reads 4 bytes*/

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


/*Write to control register*/
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

/* read from control register and returns read value
 * In case of error returns Failure
 */
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

/*Write to timing register
 * params file descriptor and value to be written
 */
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

/* Reads from timing register
 * return read value and return Fail
 */

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

/* Write 4 bytes to interrupt threshhold register*/
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

/*Read value from interrupt_threshhold register
 * Returns either read value and fail on failure
 */
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

/*Wirte to interrupt control register*/
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

/*read from interrupt control register */
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

/* Read id register*/
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

/*Print sensor id and version*/
int print_id(int fd){
	uint8_t id2, id1 = read_idreg(fd);
	id2 = id1;
	printf("Printing id register\n");
	printf("Part no: %d\n", (id1>>4 & 0xFF));
	printf("Rev no: %d\n", (id2 & 0x0F));
	return SUCCESS;
}

/* Read adc0 register of light*/
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

/*Read adc1 register of light*/
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

/*get calibrated luminosity */
float get_luminosity(int fd){
	float ch0, ch1, adc, luminosity;

	write_controlreg(fd, power_up);
	write_timingreg(fd, time_402ms|max_gain);
	usleep(5000);

	ch0 = (float)read_data0reg(fd);
	ch1 = (float)read_data1reg(fd);

	adc = ch1/ch0;

	/* According to data sheet*/
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

/*close file descriptor*/
int close_apds9301(int fd){
	close(fd);
	return SUCCESS;
}
