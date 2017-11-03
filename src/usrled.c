/*
 * usrled.c
 *
 *  Created on: Nov 3, 2017
 *      Author: raghav
 */


#include<stdio.h>
#include<unistd.h>

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

