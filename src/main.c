#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<signal.h>
#include<time.h>
#include<mqueue.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/time.h>

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

struct	mq_attr	attr;

static void sig_handler(int signal){
	switch(signal){
	case SIGINT:
		printf("\nCaught SIGINT\nCleaning up...\n");
		pthread_cancel(log_thread);
		pthread_cancel(temp_thread);
		pthread_cancel(light_thread);
		mq_close(temp_to_log);
		mq_unlink(TEMP_TO_LOG);
		mq_close(light_to_log);
		mq_unlink(LIGHT_TO_LOG);
		mq_close(log_to_light);
		mq_unlink(LOG_TO_LIGHT);
		mq_close(log_to_temp);
		mq_unlink(LOG_TO_TEMP);
		mq_close(log_to_main);
		mq_unlink(LOG_TO_MAIN);
		caught_signal = 1;
		fclose(file);
		break;
	}
}

void *log_task(){

	printf("In Log Task\n");
	/* access checks for file existence*/
	if( access (file_name, F_OK ) != -1){
		int ret = remove(file_name);
		   if(ret == 0) {
		      printf("File deleted successfully\n");
		   } else {
		      printf("Error: unable to delete the file\n");
		   }
	}

	file = fopen(file_name, "a+");

    int status;
    message_t rmsg_light, rmsg_temp;

	static message_t smsg_light, smsg_temp, smsg_main;
	gettimeofday(&smsg_light.time_stamp, NULL);
	smsg_light.log_level = STARTUP;
	smsg_light.src_id = TEMP_TASK;
	smsg_light.dest_id = LOG_TASK;
	smsg_light.log_type = INIT;
	smsg_light.data = 1.1111;

	gettimeofday(&smsg_temp.time_stamp, NULL);
	smsg_temp.log_level = STARTUP;
	smsg_temp.src_id = TEMP_TASK;
	smsg_temp.dest_id = LOG_TASK;
	smsg_temp.log_type = INIT;
	smsg_temp.data = 1.111;

	gettimeofday(&smsg_main.time_stamp, NULL);
	smsg_main.log_level = STARTUP;
	smsg_main.src_id = TEMP_TASK;
	smsg_main.dest_id = LOG_TASK;
	smsg_main.log_type = INIT;
	smsg_main.data = 1.111;

    while(1) {

        if( mq_send(log_to_light, (const char*)&smsg_light, sizeof(smsg_light), 1) == -1)
        	printf("\nUnable to send");

        if( mq_send(log_to_temp, (const char*)&smsg_temp, sizeof(smsg_temp), 1) == -1)
        	printf("\nUnable to send");

        if( mq_send(log_to_main, (const char*)&smsg_main, sizeof(smsg_main), 1) == -1)
        	printf("\nUnable to send");

        status = mq_receive(temp_to_log, (char*)&rmsg_temp, \
                            sizeof(rmsg_temp), NULL);

        if (status > 0) {
            printf("RECVd MSG in 1 LOG_THREAD: %f\n", rmsg_temp.data);
        }

        status = mq_receive(light_to_log, (char*)&rmsg_light, \
                            sizeof(rmsg_light), NULL);

        if (status > 0) {
            printf("RECVd MSG in 2 LOG_THREAD: %f\n", rmsg_light.data);
        }



        usleep(exec_period_usecs);
    }


}

void *temp_task(){
	printf("In Temp task \n");
	static message_t smsg, rmsg;
	gettimeofday(&smsg.time_stamp, NULL);
	smsg.log_level = STARTUP;
	smsg.src_id = TEMP_TASK;
	smsg.dest_id = LOG_TASK;
	smsg.log_type = INIT;
	smsg.data = 3.33;

    while(1) {
        if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
        	printf("\nUnable to send");

        status = mq_receive(log_to_temp, (char*)&rmsg, \
                            sizeof(rmsg), NULL);

        if (status > 0) {
            printf("RECVd MSG in temp_thread: %f\n", rmsg.data);
        }

        usleep(exec_period_usecs);
    }

}

void *light_task(){

	printf("In LIGHT task \n");
	static message_t smsg, rmsg;
	gettimeofday(&smsg.time_stamp, NULL);
	smsg.log_level = STARTUP;
	smsg.src_id = LIGHT_TASK;
	smsg.dest_id = LOG_TASK;
	smsg.log_type = INIT;
	smsg.data = 2.2222;

    while(1) {
        if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
        	printf("\nUnable to send");
       // ASSERT(status != -1);

        status = mq_receive(log_to_light, (char*)&rmsg, \
                            sizeof(rmsg), NULL);

        if (status > 0) {
            printf("RECVd MSG in light_thread: %f\n", rmsg.data);
        }
        usleep(exec_period_usecs);
    }

}

int main(int argc, char *argv[])
{
	strcpy(file_name, argv[1]);

	message_t rmsg;

	mq_close(temp_to_log);
	mq_unlink(TEMP_TO_LOG);
	mq_close(light_to_log);
	mq_unlink(LIGHT_TO_LOG);
	mq_close(log_to_light);
	mq_unlink(LOG_TO_LIGHT);
	mq_close(log_to_temp);
	mq_unlink(LOG_TO_TEMP);
	mq_close(log_to_main);
	mq_unlink(LOG_TO_MAIN);

	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(rmsg);
	attr.mq_flags = 0;

	temp_to_log = mq_open (TEMP_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	light_to_log = mq_open (LIGHT_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	log_to_light = mq_open (LOG_TO_LIGHT, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	log_to_temp = mq_open (LOG_TO_TEMP, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	log_to_main = mq_open (LOG_TO_MAIN, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);

	struct sigaction sig;
	sig.sa_flags = SA_SIGINFO;
	sigemptyset(&sig.sa_mask);
	sig.sa_handler = sig_handler;

	if(sigaction(SIGINT, &sig, NULL) == -1)
	{
		perror("sigaction");
		printf("unable to sigaction 1");
	}


	pthread_create(&log_thread, NULL, log_task, NULL);
	pthread_create(&temp_thread, NULL, temp_task, NULL);
	pthread_create(&light_thread, NULL, light_task, NULL);


    while(caught_signal == 0) {
        status = mq_receive(log_to_main, (char*)&rmsg, \
                            sizeof(rmsg), NULL);

        if (status > 0) {
            printf("RECVd MSG in main_thread: %f\n", rmsg.data);
        }
        usleep(exec_period_usecs);
    }

	pthread_join(log_thread, NULL);
	pthread_join(temp_thread, NULL);
	pthread_join(light_thread, NULL);

}
