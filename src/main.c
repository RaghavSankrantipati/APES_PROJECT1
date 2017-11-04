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
		mq_close(main_to_log);
		mq_unlink(MAIN_TO_LOG);
		mq_close(temp_to_main);
		mq_unlink(TEMP_TO_MAIN);
		mq_close(light_to_main);
		mq_unlink(LIGHT_TO_MAIN);

    	led2_off();
    	led1_off();
		caught_signal = 1;
		light_alive = 1;
		temp_alive = 1;
		fclose(file);
		break;
	}
}

void clean_everything(void)
{
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
	mq_close(main_to_log);
	mq_unlink(MAIN_TO_LOG);
	mq_close(temp_to_main);
	mq_unlink(TEMP_TO_MAIN);
	mq_close(light_to_main);
	mq_unlink(LIGHT_TO_MAIN);
	led2_off();
	led1_off();
	caught_signal = 1;
	fclose(file);
}

void log_tempqueue(){

	message_t rmsg_temp;
	if(mq_receive(temp_to_log, (char*)&rmsg_temp, \
                        sizeof(rmsg_temp), NULL)>0){
    	if(rmsg_temp.log_type == DATA){

    		if(f == 1){
    			sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | TEMPERATURE : %fF \n",\
    					rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec, rmsg_temp.data);
    			fwrite(data, sizeof(char), strlen(data), file);
    			f = 0;
    		}

    		if(k == 1){
    			sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | TEMPERATURE : %fK \n",\
    					rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec, rmsg_temp.data);
    			fwrite(data, sizeof(char), strlen(data), file);
    			k = 0;
    		}
    		if(c == 1){
    			sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | TEMPERATURE : %fC \n",\
    					rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec, rmsg_temp.data);
    			fwrite(data, sizeof(char), strlen(data), file);
    			c = 0;
    		}

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
            if( mq_send(log_to_light, (const char*)&rmsg_temp, sizeof(rmsg_temp), 1) == -1)
            	printf("\nUnable to send");

    	}
    	if(rmsg_temp.log_type == RESPONSE){
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | Temperature : %f | Response to Light task \n",\
    				rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec, rmsg_temp.data);
    		fwrite(data, sizeof(char), strlen(data), file);
    	}

    }
}

void log_lightqueue(void){

	message_t rmsg_light;
    if( mq_receive(light_to_log, (char*)&rmsg_light, \
                        sizeof(rmsg_light), NULL) > 0){
    	if(rmsg_light.log_type == DATA && rmsg_light.log_level == INFO){
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task 	 | Lumen : %f\n",\
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
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | Lumen : %f | Response to temp task \n",\
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

    }
}

void log_mainqueue(void){
	message_t rmsg_main;

	if(mq_receive(main_to_log, (char*)&rmsg_main, \
	                            sizeof(rmsg_main), NULL)>0){

    	if(rmsg_main.data == LIGHT_TASK && rmsg_main.log_level == ALERT){
    		printf("Recievd Light failure\n");
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : MAIN task | APDS 9301 I2C Failed\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : LIGHT task | Terminating Light task\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		light_alive = 1;
    		sleep(1);
    		pthread_cancel(light_thread);
    		printf("Terminating Light thread\n");

    	}

    	else if(rmsg_main.data == TEMP_TASK && rmsg_main.log_level == ALERT ){
    		printf("Recievd temp failure\n");
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : MAIN task | TMP 102 I2C Failed\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : TEMPERATURE task | Terminating Temperature task\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		temp_alive = 1;
    		sleep(1);
    		pthread_cancel(temp_thread);
    		printf("Terminating TEMP thread\n");
    	}
    	else if(rmsg_main.data == LIGHT_TASK && rmsg_main.log_level == CRITICAL ){
    		printf("Recievd Light no hb\n");
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : MAIN task | No LIGHT task Heart beat\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : LIGHT task | Terminating Light task\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		pthread_cancel(light_thread);
    		printf("Terminating Light thread\n");
    	}
    	else if(rmsg_main.data == TEMP_TASK && rmsg_main.log_level == CRITICAL ){
    		printf("Recievd temp no hb\n");
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : TEMPERATURE task | No TEMP Heart beat\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : TEMPERATURE task | Terminating Temperature task\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		pthread_cancel(temp_thread);
    		printf("Terminating TEMP thread\n");
    	}
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

    while(1) {
    	log_mainqueue();

    	pthread_mutex_lock(&mutex);

    	log_tempqueue();
    	log_tempqueue();
    	log_lightqueue();
		pthread_mutex_unlock(&mutex);

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
	smsg.data = 0;

    if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    	printf("\nUnable to send");

	gettimeofday(&smsg.time_stamp, NULL);
	smsg.log_level = INFO;
	smsg.src_id = TEMP_TASK;
	smsg.dest_id = LOG_TASK;
	smsg.log_type = REQUEST;
	smsg.data = 0;

	if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
		printf("\nUnable to send");


    while(temp_alive == 0) {
    	pthread_mutex_lock(&mutex);

    	fd = tmp102_init(2);
    	if( fd == FAIL)
    		temp_failure = FAIL;

    	float temp = print_temperature(fd, CONFIG_DEFAULT);
    	if( temp == FAIL)
    		temp_failure = FAIL;

    	float temperature;
    	if(temp_degree == 0){
    		temperature = callibrate_temp(temp, CELSIUS);
    		c = 1;
    		f = 0;
    		k = 0;
    		temp_degree++;
    	} else if(temp_degree == 1){
    		temperature = callibrate_temp(temp, FAHRENHEIT);
    		f = 1;
    		c = 0;
    		k = 0;
    		temp_degree++;
    	} else if(temp_degree == 2){
    		temperature = callibrate_temp(temp, KELVIN);
    		c = 0;
    		f = 0;
    		k = 1;
    		temp_degree = 0;
    	}
    	if( temperature == FAIL)
    		temp_failure = FAIL;

    	printf("Temperature %f\n", temperature);

    	if(CHECK_LIBRARY){
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
    	}

    	close_tmp102(fd);
    	pthread_mutex_unlock(&mutex);

    	temp_failure = FAIL;

    	if(temp_failure == FAIL){
        	gettimeofday(&smsg.time_stamp, NULL);
        	smsg.log_level = ALERT;
        	smsg.src_id = TEMP_TASK;
        	smsg.dest_id = MAIN_TASK;
        	smsg.log_type = FAILURE;
        	smsg.data = 0;

        	if( mq_send(temp_to_main, (const char*)&smsg, sizeof(smsg), 1) == -1)
        			printf("\nUnable to send heart_beat : TEMP TASK");

    	} else {

        	if(counter_temp % 7 ==  0)
        	{
        		gettimeofday(&smsg.time_stamp, NULL);
        		smsg.log_level = INFO;
        		smsg.src_id = TEMP_TASK;
        		smsg.dest_id = LOG_TASK;
        		smsg.log_type = REQUEST;
        		smsg.data = counter_temp;

        		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
        			printf("\nUnable to send");
        		counter_temp++;

        		goto end;
        	}


            if( mq_receive(log_to_temp, (char*)&rmsg, sizeof(rmsg), NULL) != -1){
            	gettimeofday(&smsg.time_stamp, NULL);
            	smsg.log_level = INFO;
            	smsg.src_id = LIGHT_TASK;
            	smsg.dest_id = LOG_TASK;
            	smsg.log_type = RESPONSE;
            	smsg.data = temperature;

            		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
            			printf("\nUnable to send");
            } else {

            	gettimeofday(&smsg.time_stamp, NULL);
            	smsg.log_level = INFO;
            	smsg.src_id = TEMP_TASK;
            	smsg.dest_id = LOG_TASK;
            	smsg.log_type = DATA;
            	smsg.data = temperature;

            	if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
            		printf("\nUnable to send");
            }

            end:
        	gettimeofday(&smsg.time_stamp, NULL);
        	smsg.log_level = INFO;
        	smsg.src_id = TEMP_TASK;
        	smsg.dest_id = MAIN_TASK;
        	smsg.log_type = HEART_BEAT;
        	smsg.data = 0;

        	if( mq_send(temp_to_main, (const char*)&smsg, sizeof(smsg), 1) == -1)
        		printf("\nUnable to send heart_beat : TEMP TASK");

    	}
    	counter_temp++;

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


    while(light_alive == 0) {


    	pthread_mutex_lock(&mutex);

    	fd = sensor_init(2);
    	if( fd == FAIL)
    		light_failure = FAIL;

    	if(CHECK_LIBRARY){
    		int status = rw_allregs_apds(fd);
    		write_interrupt_controlreg(fd, INT_ENABLE);
    		status = read_interrupt_controlreg(fd);
    		print_id(fd);
    		printf("fd light %d status: %d\n", fd, status);
    	}
    	float lumen = get_luminosity(fd);
    	if( lumen == FAIL)
    		light_failure = FAIL;

    	if(lumen < 1 && prev_lumen > 1)
    		night = 1;
    	if( lumen > 1 && prev_lumen < 1)
    		day = 1;
    	printf("lumen %f, prev_lumen %f, day %d, night %d\n", lumen, prev_lumen, day, night);

    	prev_lumen = lumen;
    	close_apds9301(fd);

    	pthread_mutex_unlock(&mutex);

    	light_failure = FAIL;
    	if(light_failure == FAIL){
        	gettimeofday(&smsg.time_stamp, NULL);
        	smsg.log_level = ALERT;
        	smsg.src_id = LIGHT_TASK;
        	smsg.dest_id = MAIN_TASK;
        	smsg.log_type = FAILURE;
        	smsg.data = 0;

        	if( mq_send(light_to_main, (const char*)&smsg, sizeof(smsg), 1) == -1)
        			printf("\nUnable to send heart_beat : LIGHT TASK");

    	} else {

            if( mq_receive(log_to_light, (char*)&rmsg, sizeof(rmsg), NULL) != -1){
            	printf("Response to temp\n");
            	gettimeofday(&smsg.time_stamp, NULL);
            	smsg.log_level = INFO;
            	smsg.src_id = LIGHT_TASK;
            	smsg.dest_id = LOG_TASK;
            	smsg.log_type = RESPONSE;
            	smsg.data = lumen;

            	day = 0;
            	night = 0;
                	if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
                		printf("\nUnable to send");
            } else {

            	if( counter_light % 10 == 0){
            		gettimeofday(&smsg.time_stamp, NULL);
            		smsg.log_level = INFO;
            		smsg.src_id = LIGHT_TASK;
            		smsg.dest_id = LOG_TASK;
            		smsg.log_type = REQUEST;
            		smsg.data = counter_light;

            		if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
            			printf("\nUnable to send");
            	}

            	if( day == 1){
            		gettimeofday(&smsg.time_stamp, NULL);
            		smsg.log_level = ALERT;
            		smsg.src_id = LIGHT_TASK;
            		smsg.dest_id = LOG_TASK;
            		smsg.log_type = DATA;
            		smsg.data = 1;

            		if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
            			printf("\nUnable to send");
            		day = 0;
            	}
            	else if( night == 1){

            		gettimeofday(&smsg.time_stamp, NULL);
            		smsg.log_level = ALERT;
            		smsg.src_id = LIGHT_TASK;
            		smsg.dest_id = LOG_TASK;
            		smsg.log_type = DATA;
            		smsg.data = 0;

            		if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
            			printf("\nUnable to send");
            		night = 0;

            	} else if (day == 0 && night == 0){

            		gettimeofday(&smsg.time_stamp, NULL);
        			smsg.log_level = INFO;
        			smsg.src_id = LIGHT_TASK;
        			smsg.dest_id = LOG_TASK;
        			smsg.log_type = DATA;
        			smsg.data = lumen;

        			if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
        			printf("\nUnable to send");
            	}
            }

    		gettimeofday(&smsg.time_stamp, NULL);
    		smsg.log_level = INFO;
    		smsg.src_id = LIGHT_TASK;
    		smsg.dest_id = MAIN_TASK;
    		smsg.log_type = HEART_BEAT;
    		smsg.data = 0;

    		if( mq_send(light_to_main, (const char*)&smsg, sizeof(smsg), 1) == -1)
    				printf("\nUnable to send heart_beat : LIGHT TASK");

    	}

    	counter_light++;
        usleep(exec_period_usecs);
    }

}

void close_queues(void){

	mq_close(temp_to_log);
	mq_unlink(TEMP_TO_LOG);
	mq_close(light_to_log);
	mq_unlink(LIGHT_TO_LOG);
	mq_close(log_to_light);
	mq_unlink(LOG_TO_LIGHT);
	mq_close(log_to_temp);
	mq_unlink(LOG_TO_TEMP);
	mq_close(main_to_log);
	mq_unlink(MAIN_TO_LOG);
	mq_close(temp_to_main);
	mq_unlink(TEMP_TO_MAIN);
	mq_close(light_to_main);
	mq_unlink(LIGHT_TO_MAIN);
}

void open_queues(void){


	temp_to_log = mq_open (TEMP_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	light_to_log = mq_open (LIGHT_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	log_to_light = mq_open (LOG_TO_LIGHT, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	log_to_temp = mq_open (LOG_TO_TEMP, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	main_to_log = mq_open (MAIN_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	temp_to_main = mq_open (TEMP_TO_MAIN, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	light_to_main = mq_open (LIGHT_TO_MAIN, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);

}



int main(int argc, char *argv[])
{
	strcpy(file_name, argv[1]);

	message_t rmsg1, rmsg2, smsg;

	close_queues();

	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(rmsg1);
	attr.mq_flags = 0;

	open_queues();

	struct sigaction sig;
	sig.sa_flags = SA_SIGINFO;
	sigemptyset(&sig.sa_mask);
	sig.sa_handler = sig_handler;

	if(sigaction(SIGINT, &sig, NULL) == -1)
	{
		perror("sigaction");
		printf("unable to sigaction 1");
	}

	pthread_mutex_init(&mutex, NULL);

	pthread_create(&log_thread, NULL, log_task, NULL);
	pthread_create(&temp_thread, NULL, temp_task, NULL);
	pthread_create(&light_thread, NULL, light_task, NULL);

	usleep(exec_period_usecs);
	uint8_t temp_life = 0, light_life = 0;
	while( caught_signal == 0 && (light_alive | temp_alive) == 0){

		if(light_alive == 0){
			if(mq_receive(light_to_main, (char*)&rmsg1, sizeof(rmsg1), NULL) == -1){
				printf("\n No heartbeat from light task\n");
				light_life++;
			} else {
				if(rmsg1.log_type == FAILURE){

					gettimeofday(&smsg.time_stamp, NULL);
					smsg.log_level = ALERT;
					smsg.src_id = MAIN_TASK;
					smsg.dest_id = LOG_TASK;
					smsg.log_type = FAILURE;
					smsg.data = 1;

					if( mq_send(main_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
	    				printf("\nUnable to send errorlog : MAIN TASK");
					led1_on();
					printf("In light check\n");
				}
			}


			if(light_life == 10){
				led1_on();

				gettimeofday(&smsg.time_stamp, NULL);
				smsg.log_level = CRITICAL;
				smsg.src_id = MAIN_TASK;
				smsg.dest_id = LOG_TASK;
				smsg.log_type = FAILURE;
				smsg.data = 1;

				if( mq_send(main_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    				printf("\nUnable to send errorlog : MAIN TASK");
				led1_on();

			}

		}
		if(temp_alive == 0){
			if(mq_receive(temp_to_main, (char*)&rmsg2, sizeof(rmsg2), NULL) == -1){
				printf("\nNo heartbeat from temp task\n");
				temp_life++;
			} else {
				if(rmsg2.log_type == FAILURE){

					gettimeofday(&smsg.time_stamp, NULL);
					smsg.log_level = ALERT;
					smsg.src_id = MAIN_TASK;
					smsg.dest_id = LOG_TASK;
					smsg.log_type = FAILURE;
					smsg.data = 2;

					if( mq_send(main_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
						printf("\nUnable to send errorlog : MAIN TASK");
					led2_on();

					printf("In temp check%d\n", temp_alive);
				}
			}

			if(temp_life == 10){
				led2_on();
				printf("In send\n");
				gettimeofday(&smsg.time_stamp, NULL);
				smsg.log_level = CRITICAL;
				smsg.src_id = MAIN_TASK;
				smsg.dest_id = LOG_TASK;
				smsg.log_type = FAILURE;
				smsg.data = 2;

				if( mq_send(main_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    				printf("\nUnable to send errorlog : MAIN TASK");

			}

		}

		usleep(exec_period_usecs);
	}
	sleep(5);
	if(caught_signal !=1)
		clean_everything();
	pthread_join(log_thread, NULL);
	pthread_join(temp_thread, NULL);
	pthread_join(light_thread, NULL);


}
