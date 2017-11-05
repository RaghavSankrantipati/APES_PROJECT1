
#include<stdio.h>
#include<unistd.h>

/**********************************************************************
*@Filename:usrled.c
*
*@Description:This is a library for User LEDs
*@Author:Sai Raghavendra Sankrantipati
*@Date:11/5/2017
*@compiler:arm-linux-gnueabihf-gcc
*@Usage : use any of the library function to on and off leds
 **********************************************************************/

int led2_on(){

	printf("LED2 on\n");
	FILE *LED1 = NULL;
	char *LED2 = "/sys/class/leds/beaglebone:green:usr2/brightness";

	LED1 = fopen(LED2, "r+");
	fwrite("1", sizeof(char), 1, LED1);
	fclose(LED1);
	return 1;
}

int led2_off(){

	printf("LED2 off\n");
	FILE *LED1 = NULL;
	char *LED2 = "/sys/class/leds/beaglebone:green:usr2/brightness";

	LED1 = fopen(LED2, "r+");
	fwrite("0", sizeof(char), 1, LED1);
	fclose(LED1);
	return 1;
}

int led1_on(){

	printf("LED1 on\n");
	FILE *LED1 = NULL;
	char *LED2 = "/sys/class/leds/beaglebone:green:usr1/brightness";

	LED1 = fopen(LED2, "r+");
	fwrite("1", sizeof(char), 1, LED1);
	fclose(LED1);
	return 1;
}

int led1_off(){

	printf("LED1 off\n");
	FILE *LED1 = NULL;
	char *LED2 = "/sys/class/leds/beaglebone:green:usr1/brightness";

	LED1 = fopen(LED2, "r+");
	fwrite("0", sizeof(char), 1, LED1);
	fclose(LED1);
	return 1;
}

