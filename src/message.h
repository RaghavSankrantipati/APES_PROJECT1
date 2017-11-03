/*
 * message.h
 *
 *  Created on: Nov 3, 2017
 *      Author: raghav
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

FILE *file;
char file_name[50];

typedef enum{
	STARTUP,
	INFO,
	ALERT,
	CRITICAL
}log_level_t;

typedef enum{
	LOG_TASK,
	LIGHT_TASK,
	TEMP_TASK,
	MAIN_TASK
}source_t;

typedef enum{
	INIT,
	FAILURE,
	DATA,
	REQUEST,
	RESPONSE,
	HEART_BEAT
}log_type_t;

typedef struct {
	struct timeval time_stamp;
	log_level_t log_level;
	source_t src_id;
	source_t dest_id;
	log_type_t log_type;
	float data;
}message_t;

#define	TEMP_TO_LOG		"/temptolog"
#define	LIGHT_TO_LOG	"/lighttolog"
#define LOG_TO_LIGHT	"/logtolight"
#define	LOG_TO_TEMP		"/logtotemp"
#define	LOG_TO_MAIN		"/logtomain"


pthread_t log_thread, temp_thread, light_thread;

mqd_t temp_to_log, light_to_log, log_to_light, log_to_temp, log_to_main;

int status = 0;

char data[500];

int exec_period_usecs = 1000000; /*in micro-seconds*/
int caught_signal = 0;
int counter_temp = 0;
int counter_light = 0;
int alive = 0;


struct	mq_attr	attr;

#endif /* MESSAGE_H_ */
