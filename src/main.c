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

static unsigned int counter;

mqd_t temp_to_log, light_to_log, log_to_light, log_to_temp, log_to_main;

char data[500];

struct	mq_attr	attr;

static void sig_handler(int signal){
	switch(signal){
	case SIGINT:
		printf("Caught SIGINT\n");
		pthread_cancel(log_thread);
		pthread_cancel(temp_thread);
		pthread_cancel(light_thread);
		mq_close(TEMP_TO_LOG);
		mq_unlink(TEMP_TO_LOG);
		mq_close(LIGHT_TO_LOG);
		mq_unlink(LIGHT_TO_LOG);
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

    unsigned int exec_period_usecs;
    int status;
    message_t msg1, msg2;

    exec_period_usecs = 10000; /*in micro-seconds*/

    printf("Thread 2 started. Execution period = %d uSecs\n",\
                                           exec_period_usecs);

    while(1) {
        status = mq_receive(temp_to_log, (char*)&msg1, \
                            sizeof(msg1), NULL);

        if (status > 0) {
            printf("RECVd MSG in THRD_2: %f\n", msg1.data);
            counter += 1;
        }

        status = mq_receive(light_to_log, (char*)&msg2, \
                            sizeof(msg2), NULL);

        if (status > 0) {
            printf("RECVd MSG in THRD_2: %f\n", msg2.data);
            counter += 1;
        }

        usleep(exec_period_usecs);
    }


}

void *temp_task(){
	printf("In Temp task \n");

    unsigned int exec_period_usecs;
    int status;

    exec_period_usecs = 1000000; /*in micro-seconds*/

    printf("Thread 1 started. Execution period = %d uSecs\n",\
                                           exec_period_usecs);
	static message_t init_msg;
	gettimeofday(&init_msg.time_stamp, NULL);
	init_msg.log_level = STARTUP;
	init_msg.src_id = TEMP_TASK;
	init_msg.dest_id = LOG_TASK;
	init_msg.log_type = INIT;
	init_msg.data = 11.111;

    while(1) {
        if( mq_send(temp_to_log, (const char*)&init_msg, sizeof(init_msg), 1) == -1)
        	printf("\nUnable to send");
       // ASSERT(status != -1);
        usleep(exec_period_usecs);
    }

}

void *light_task(){

	printf("In LIGHT task \n");

    unsigned int exec_period_usecs;
    int status;

    exec_period_usecs = 1000000; /*in micro-seconds*/

    printf("Thread 1 started. Execution period = %d uSecs\n",\
                                           exec_period_usecs);
	static message_t init_msg;
	gettimeofday(&init_msg.time_stamp, NULL);
	init_msg.log_level = STARTUP;
	init_msg.src_id = LIGHT_TASK;
	init_msg.dest_id = LOG_TASK;
	init_msg.log_type = INIT;
	init_msg.data = 2.2222;

    while(1) {
        if( mq_send(light_to_log, (const char*)&init_msg, sizeof(init_msg), 1) == -1)
        	printf("\nUnable to send");
       // ASSERT(status != -1);
        usleep(exec_period_usecs);
    }

}

int main(int argc, char *argv[])
{
	strcpy(file_name, argv[1]);

	message_t msg;

	mq_close(TEMP_TO_LOG);
	mq_unlink(TEMP_TO_LOG);
	mq_close(LIGHT_TO_LOG);
	mq_unlink(LIGHT_TO_LOG);
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(msg);
	attr.mq_flags = 0;
	temp_to_log = mq_open (TEMP_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	light_to_log = mq_open (LIGHT_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);

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

	pthread_join(log_thread, NULL);
	pthread_join(temp_thread, NULL);
	pthread_join(light_thread, NULL);

}
