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
#include"../include/message.h"
#include"../include/usrled.h"
#include"../include/tmp102.h"
#include"../include/apds9301.h"


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
    	led2_off();
    	led1_off();
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

    message_t rmsg_light, rmsg_temp, smsg_main;


    while(1) {

        if(mq_receive(temp_to_log, (char*)&rmsg_temp, \
                            sizeof(rmsg_temp), NULL)>0){
        	if(rmsg_temp.log_type == DATA){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | Data : %f\n",\
        				rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec, rmsg_temp.data);
        		fwrite(data, sizeof(char), strlen(data), file);
        	}
        	if(rmsg_temp.log_type == INIT){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | Task Initialized\n",\
        				rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);
        	}
        	if(rmsg_temp.log_type == REQUEST){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | Request Light\n",\
        				rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);

        	}
        	if(rmsg_temp.log_type == RESPONSE){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | Data : %f | Response to Light task \n",\
        				rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec, rmsg_temp.data);
        		fwrite(data, sizeof(char), strlen(data), file);
        	}

        	if(rmsg_temp.log_type == FAILURE){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | FAILED \n",\
        				rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);
        	}
        	alive = 1;
        	if(rmsg_temp.log_type == FAILURE)
        		alive = 0;

        } else{
        	alive = 0;
        }


        if( mq_receive(light_to_log, (char*)&rmsg_light, \
                            sizeof(rmsg_light), NULL) > 0){
        	if(rmsg_light.log_type == DATA){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task 	 | Data : %f\n",\
        				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec, rmsg_light.data);
        		fwrite(data, sizeof(char), strlen(data), file);
        	}
        	if(rmsg_light.log_type == INIT){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | Task Initialized\n",\
        				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);
        	}
        	if(rmsg_light.log_type == REQUEST){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | Request Temperature\n",\
        				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);
                if( mq_send(log_to_temp, (const char*)&rmsg_light, sizeof(rmsg_light), 1) == -1)
                	printf("\nUnable to send");
        	}
        	if(rmsg_light.log_type == RESPONSE){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | Data : %f | Response to temp task \n",\
        				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec, rmsg_light.data);
        		fwrite(data, sizeof(char), strlen(data), file);
        	}

        	if(rmsg_light.log_level == ALERT){
        		if(rmsg_light.data == 0){
        				sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | It's Night\n",\
        						rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
        				fwrite(data, sizeof(char), strlen(data), file);
        		}

        		if(rmsg_light.data == 1){
        				sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | It's Day\n",\
        						rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
        				fwrite(data, sizeof(char), strlen(data), file);
        		}
        	}
        	alive = alive + 2;
        	if(rmsg_light.log_type == FAILURE){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | FAILED\n",\
        				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);

        	}

        	if(rmsg_light.log_type == FAILURE)
        		alive = alive -2;

        }


    	gettimeofday(&smsg_main.time_stamp, NULL);
    	smsg_main.log_level = INFO;
    	smsg_main.src_id = LOG_TASK;
    	smsg_main.dest_id = MAIN_TASK;
    	smsg_main.log_type = DATA;
    	smsg_main.data = alive;

        if( mq_send(log_to_main, (const char*)&smsg_main, sizeof(smsg_main), 1) == -1)
        	printf("\nUnable to send");

        usleep(exec_period_usecs);

        alive = 0;
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
	smsg.data = 0;

    if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    	printf("\nUnable to send");


    while(1) {

    	if(counter_temp == 0){
    		gettimeofday(&smsg.time_stamp, NULL);
    		smsg.log_level = INFO;
    		smsg.src_id = TEMP_TASK;
    		smsg.dest_id = LOG_TASK;
    		smsg.log_type = DATA;
    		smsg.data = counter_temp;

    		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    			printf("\nUnable to send");

    		counter_temp++;
    	}

    	else if(counter_temp == 1){
    		gettimeofday(&smsg.time_stamp, NULL);
    		smsg.log_level = INFO;
    		smsg.src_id = TEMP_TASK;
    		smsg.dest_id = LOG_TASK;
    		smsg.log_type = REQUEST;
    		smsg.data = counter_temp;

    		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    			printf("\nUnable to send");

    		counter_temp++;
    	}

    	else if(counter_temp == 2){
    		gettimeofday(&smsg.time_stamp, NULL);
    		smsg.log_level = CRITICAL;
    		smsg.src_id = TEMP_TASK;
    		smsg.dest_id = LOG_TASK;
    		smsg.log_type = DATA;
    		smsg.data = 0;

    		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    			printf("\nUnable to send");

    		counter_temp = 3;
    	}

    	else if(counter_temp == 3){
    		gettimeofday(&smsg.time_stamp, NULL);
    		smsg.log_level = CRITICAL;
    		smsg.src_id = TEMP_TASK;
    		smsg.dest_id = LOG_TASK;
    		smsg.log_type = FAILURE;
    		smsg.data = 0;

    		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    			printf("\nUnable to send");

    		counter_temp = 0;
    	}

        if( mq_receive(log_to_temp, (char*)&rmsg, sizeof(rmsg), NULL) != -1){
        	gettimeofday(&smsg.time_stamp, NULL);
        	smsg.log_level = INFO;
        	smsg.src_id = LIGHT_TASK;
        	smsg.dest_id = LOG_TASK;
        	smsg.log_type = RESPONSE;
        	smsg.data = 10000;

        		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
        			printf("\nUnable to send");
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
	smsg.data = 0;

    if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    	printf("\nUnable to send");


    while(1) {

    	if(counter_light == 0){
    		gettimeofday(&smsg.time_stamp, NULL);
    		smsg.log_level = INFO;
    		smsg.src_id = LIGHT_TASK;
    		smsg.dest_id = LOG_TASK;
    		smsg.log_type = DATA;
    		smsg.data = counter_light;

    		if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    			printf("\nUnable to send");

    		counter_light++;
    		}

    	else if(counter_light == 1){
    		gettimeofday(&smsg.time_stamp, NULL);
    		smsg.log_level = INFO;
    		smsg.src_id = LIGHT_TASK;
    		smsg.dest_id = LOG_TASK;
    		smsg.log_type = REQUEST;
    		smsg.data = counter_light;

    		if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    			printf("\nUnable to send");

    		counter_light++;
    	}

    	else if(counter_light == 2){
    		gettimeofday(&smsg.time_stamp, NULL);
    		smsg.log_level = CRITICAL;
    		smsg.src_id = LIGHT_TASK;
    		smsg.dest_id = LOG_TASK;
    		smsg.log_type = FAILURE;
    		smsg.data = counter_light;

    		if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    			printf("\nUnable to send");

    		counter_light = 3;
    	}

    	else if(counter_light == 3){
    		gettimeofday(&smsg.time_stamp, NULL);
    		smsg.log_level = ALERT;
    		smsg.src_id = LIGHT_TASK;
    		smsg.dest_id = LOG_TASK;
    		smsg.log_type = DATA;
    		smsg.data = 1;

    		if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    			printf("\nUnable to send");

    		counter_light = 0;
    	}

        if( mq_receive(log_to_light, (char*)&rmsg, sizeof(rmsg), NULL) != -1){
        	gettimeofday(&smsg.time_stamp, NULL);
        	smsg.log_level = STARTUP;
        	smsg.src_id = LIGHT_TASK;
        	smsg.dest_id = LOG_TASK;
        	smsg.log_type = RESPONSE;
        	smsg.data = 10000;

            	if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
            		printf("\nUnable to send");
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

	int fd;
	fd = sensor_init(2);
	int status = rw_allregs_apds(fd);
	write_interrupt_controlreg(fd, INT_ENABLE);
	status = read_interrupt_controlreg(fd);
	print_id(fd);
	printf("fd light %d status: %d\n", fd, status);
	float lumen = get_luminosity(fd);
	printf("lumen %f", lumen);

	close_apds9301(fd);

	 fd = tmp102_init(2);
	  status = rw_allregs_tmp102(fd);
	 printf("\nfd temp %d, ststus %d\n", fd, status);
	print_temperature(fd, CONFIG_DEFAULT);
	uint16_t *res;
	res = malloc(sizeof(uint16_t));
	read_configreg(fd, res);
	printf("config_reg %d\n", *res);
	shutdown_mode(fd, SHUTDOWN_MODE);
	read_configreg(fd, res);
	printf("config_reg %d\n", *res);
	close_tmp102(fd);





/*	pthread_create(&log_thread, NULL, log_task, NULL);
	pthread_create(&temp_thread, NULL, temp_task, NULL);
	pthread_create(&light_thread, NULL, light_task, NULL);


    while(caught_signal == 0) {
        status = mq_receive(log_to_main, (char*)&rmsg, \
                            sizeof(rmsg), NULL);

        if (status < 0) {
            printf("Unable to recieve\n");
        }

        if(rmsg.data == 3){
        	printf("Both tasks are alive \n");
        	led2_off();
        	led1_off();
        } else if(rmsg.data == 1){
        	printf("Light task is dead \n");
        	led1_off();
        	led2_on();
        } else if(rmsg.data == 2){
        	printf("Temp task is dead\n");
        	led2_off();
        	led1_on();
        } else {
        	printf("All tasks dead\n");
        	led2_off();
        	led1_off();
        	led2_on();
        	led1_on();

        }

        usleep(exec_period_usecs);
    }


	pthread_join(log_thread, NULL);
	pthread_join(temp_thread, NULL);
	pthread_join(light_thread, NULL);
*/

}
