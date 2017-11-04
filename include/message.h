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
#define	MAIN_TO_LOG		"/maintolog"
#define TEMP_TO_MAIN	"/temptomain"
#define LIGHT_TO_MAIN	"/lighttomain"


pthread_t log_thread, temp_thread, light_thread;

mqd_t temp_to_log, light_to_log, log_to_light, log_to_temp, main_to_log,\
		temp_to_main, light_to_main;

int status = 0;

char data[500];

#define LIGHT_TASK	0x01
#define TEMP_TASK	0x02

int exec_period_usecs = 1000000; /*in micro-seconds*/
int caught_signal = 0;
int counter_temp = 1;
int counter_light = 1;
int temp_failure = 0;
int temp_degree = 0;
int light_failure = 0;
float prev_lumen = 0;
int day = 0, night = 0;
int fd;
int c = 0, k = 0, f = 0;
int temp_alive = 0, light_alive = 0;

struct	mq_attr	attr;
pthread_mutex_t mutex;

void close_queues(void);
void open_queues(void);

#endif /* MESSAGE_H_ */
