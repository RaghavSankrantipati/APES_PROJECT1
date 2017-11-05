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

/**********************************************************************
*@Filename:main.c
*@Description:This is 4 threaded file. In which Light task uses apsd9301 library for
*				accessing light sensor and temp task uses tmp102 library
*s
*				Main Task - It creates tasks, checks for hearbeat, In case of an error it logs
*							and LED1 (for temp task) or LED2( for light task) are turned on
*
*				Temperature task - Periodic reads from tmp 102, logs data, sends heart beat to main task
*									return in multiple systems(F, C and K), sends data to decision/log task
*
*				Light task - Supports API sent from other tasks, conversion of readings from APDS 9301,
*							logs at unexpected changes, sends data to decision/log task
*
*				Log task and decision task- Takes log from light, temp and main tasks, writes to file, graceful shutdown when required,
*							implements log packets, takes decision
*
*				Look for software architecture for queues and structure.
*@Author:Sai Raghavendra Sankrantipati
*@Date:11/5/2017
*@compiler:arm-linux-gnueabihf-gcc
*@Usage : Look for Readme for detailed informations
 **********************************************************************/


/* Signal handler for SIGINT (CTRL + C)*/
static void sig_handler(int signal){
	switch(signal){
	case SIGINT:
		printf("\nCaught SIGINT\nCleaning up...\n");
		/*closes threads*/
		pthread_cancel(log_thread);
		pthread_cancel(temp_thread);
		pthread_cancel(light_thread);
		/*close and unlink all messages*/
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
		/*controls LED*/
    	led2_off();
    	led1_off();
    	/*activate global signalling variables*/
		caught_signal = 1;
		light_alive = 1;
		temp_alive = 1;

		/*close the file given on command line*/
		fclose(file);
		break;
	}
}



/*prints messages in temp_to_log queue to the file*/
void log_tempqueue(){

	message_t rmsg_temp;

	/*checks for message in /temptolog queue*/
	if(mq_receive(temp_to_log, (char*)&rmsg_temp, \
                        sizeof(rmsg_temp), NULL)>0){

		/*If log type is dataa*/
    	if(rmsg_temp.log_type == DATA){

    		/*check for global signalling variable f.
    		 * f is 1 when given temperature is in Fahrenheit*/
    		if(f == 1){
    			sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | TEMPERATURE : %fF \n",\
    					rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec, rmsg_temp.data);
    			fwrite(data, sizeof(char), strlen(data), file);
    			f = 0;
    		}

    		/*check for global signalling variable k.
    		 * k is 1 when given temperature is in Kelvin*/
    		if(k == 1){
    			sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | TEMPERATURE : %fK \n",\
    					rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec, rmsg_temp.data);
    			fwrite(data, sizeof(char), strlen(data), file);
    			k = 0;
    		}

    		/*check for global signalling variable C.
    		 * C is 1 when given temperature is in Centigrade*/
    		if(c == 1){
    			sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | TEMPERATURE : %fC \n",\
    					rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec, rmsg_temp.data);
    			fwrite(data, sizeof(char), strlen(data), file);
    			c = 0;
    		}

    	}

    	/* checks for log_type INIT, indicates task just initialised*/
    	if(rmsg_temp.log_type == INIT){
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | Task Initialized\n",\
    				rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    	}

    	/*Checks if the message is of request type*/
    	if(rmsg_temp.log_type == REQUEST){
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | Request Light\n",\
    				rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
            if( mq_send(log_to_light, (const char*)&rmsg_temp, sizeof(rmsg_temp), 1) == -1)
            	printf("\nUnable to send");

    	}

    	/*checks if the recieved message is of response type*/
    	if(rmsg_temp.log_type == RESPONSE){
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : Temperature task | Temperature : %f | Response to Light task \n",\
    				rmsg_temp.time_stamp.tv_sec, rmsg_temp.time_stamp.tv_usec, rmsg_temp.data);
    		fwrite(data, sizeof(char), strlen(data), file);
    	}

    }
}


/*dumps messages in light_to_log queue into file*/
void log_lightqueue(void){

	message_t rmsg_light;

	/*checks for any message in the queue*/
    if( mq_receive(light_to_log, (char*)&rmsg_light, \
                        sizeof(rmsg_light), NULL) > 0){
    	/*checks if the log_type is DATA and log_level is INFO*/
    	if(rmsg_light.log_type == DATA && rmsg_light.log_level == INFO){
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task 	 | Lumen : %f\n",\
    				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec, rmsg_light.data);
    		fwrite(data, sizeof(char), strlen(data), file);
    	}

    	/*checks if the log_type is init*/
    	if(rmsg_light.log_type == INIT){
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | Task Initialized\n",\
    				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    	}
    	/*checks if log_type is REQUEST*/
    	if(rmsg_light.log_type == REQUEST){
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | Request Temperature\n",\
    				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
    		/*forwards messgae to log_to_temp queue*/
    		fwrite(data, sizeof(char), strlen(data), file);
            if( mq_send(log_to_temp, (const char*)&rmsg_light, sizeof(rmsg_light), 1) == -1)
            	printf("\nUnable to send");
    	}

    	/*checks if the message is a response*/
    	if(rmsg_light.log_type == RESPONSE ){
    		if( rmsg_light.data == RW_ALL_REGS){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | Successful RWs all regs | Response to temp task \n",\
        				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);
    		}

    		else if( rmsg_light.data == PRINT_ID){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | Part no: 5 revision: 0 | Response to temp task \n",\
        				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);
    		}

    		else if( rmsg_light.data == ENABLE_INT){
        		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | Interrupt Enabled | Response to temp task \n",\
        				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
        		fwrite(data, sizeof(char), strlen(data), file);
    		}
    		else {
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | Lumen : %f | Response to temp task \n",\
    				rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec, rmsg_light.data);
    		fwrite(data, sizeof(char), strlen(data), file);
    		}
    	}

    	/*checks for alert in log_level*/
    	if(rmsg_light.log_level == ALERT){
    		/*if its a alert and receieved data is 0, Day tansformed to night*/
    		if(rmsg_light.data == 0){
    				sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | It's Night\n",\
    						rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
    				fwrite(data, sizeof(char), strlen(data), file);
    		}

    		/*If data is 1, night transformed to day*/
    		if(rmsg_light.data == 1){
    				sprintf(data, "Time : %ld secs, %ld usecs | Source : Light task       | It's Day\n",\
    						rmsg_light.time_stamp.tv_sec, rmsg_light.time_stamp.tv_usec);
    				fwrite(data, sizeof(char), strlen(data), file);
    		}
    	}
    }
}


/*dumps messgaes recieved from main task*/
void log_mainqueue(void){
	message_t rmsg_main;

	/*checks if there is any message in main_to_log queue*/
	if(mq_receive(main_to_log, (char*)&rmsg_main, \
	                            sizeof(rmsg_main), NULL)>0){

		/* checks of the log_level is alert and its from LIGHT_TASK*/
    	if(rmsg_main.data == LIGHT_TASK && rmsg_main.log_level == ALERT){
    		printf("Recievd Light failure\n");
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : MAIN task | APDS 9301 I2C Failed\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : LIGHT task | Terminating Light task\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		/*Activates global signalling variable
    		 * If it becomes 1 light_thread closes
    		 */
    		light_alive = 1;
    		sleep(1);
    		pthread_cancel(light_thread);
    		printf("Terminating Light thread\n");

    	}

		/* checks of the log_level is alert and its from TEMP_TASK*/
    	else if(rmsg_main.data == TEMP_TASK && rmsg_main.log_level == ALERT ){
    		printf("Recievd temp failure\n");
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : MAIN task | TMP 102 I2C Failed\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		sprintf(data, "Time : %ld secs, %ld usecs | Source : TEMPERATURE task | Terminating Temperature task\n",\
    				rmsg_main.time_stamp.tv_sec, rmsg_main.time_stamp.tv_usec);
    		fwrite(data, sizeof(char), strlen(data), file);
    		temp_alive = 1;
    		/*Activates global signaling variable
    		 * If it becomes 1 temp_thread closes
    		 */
    		sleep(1);
    		pthread_cancel(temp_thread);
    		printf("Terminating TEMP thread\n");
    	}

		/* checks of the log_level is critical and its from LIGHT_TASK
		 * It means light_thead closed
		 * */
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
		/* checks of the log_level is critical and its from TEMP_TASK
		 * It means temp_thead closed
		 * */
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



/* Cleans everything */
void clean_everything(void){
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

/*This is log thread as mentioned in description
 * It's also decision task
 */
void *log_task(){

	printf("In Log Task\n");
	/* access checks for file existence
	 * If the file existed it's deleted and created a new*/
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

    	/* checks for messages from main and dumps in file*/
    	log_mainqueue();

    	/* Lot of global signalling variables are used, hence mutex*/
    	pthread_mutex_lock(&mutex);
    	/* checks for messages from temperature task and dumps in file*/
    	log_tempqueue();
    	log_tempqueue();
    	/* checks for messages from light task and dumps in file*/
    	log_lightqueue();
		pthread_mutex_unlock(&mutex);

      usleep(exec_period_usecs);
	}

}

/*This is log thread as mentioned in description
 */
void *temp_task(){
	printf("In Temp task \n");
	static message_t smsg, rmsg;

	/* send message to log task that this thread is initilised*/
	gettimeofday(&smsg.time_stamp, NULL);
	smsg.log_level = STARTUP;
	smsg.src_id = TEMP_TASK;
	smsg.dest_id = LOG_TASK;
	smsg.log_type = INIT;
	smsg.data = 0;

    if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    	printf("\nUnable to send");

    /* A sample request message to light. It is sent to log task and it takes decision*/
	gettimeofday(&smsg.time_stamp, NULL);
	smsg.log_level = INFO;
	smsg.src_id = TEMP_TASK;
	smsg.dest_id = LOG_TASK;
	smsg.log_type = REQUEST;
	smsg.data = 0;

	if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
		printf("\nUnable to send");


	/* temp_alive is set when sigint is caught or if sensor fails responding*/
    while(temp_alive == 0) {
    	pthread_mutex_lock(&mutex);

    	/*inialiise sensor on i2c 2 bus*/
    	fd = tmp102_init(2);
    	if( fd == FAIL)
    		temp_failure = FAIL;

    	/* get temperature raw data*/
    	float temp = print_temperature(fd, CONFIG_DEFAULT);
    	if( temp == FAIL)
    		temp_failure = FAIL;

    	/*convert it into Human readable format*/
    	float temperature;

    	/* It changes from Celcisu to Fahrenhiet to kelvin in every while loop*/
    	if(temp_degree == 0){
    		temperature = callibrate_temp(temp, CELSIUS);
    		/*change global variables*/
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

    	/*If check library is changed to 1 all tmp102 library functions are checked*/
    	if(CHECK_LIBRARY){
    		status = rw_allregs_tmp102(fd);
    		if(status != FAIL)
    			printf("Successful read and writes in TMP102\n");
    		print_temperature(fd, CONFIG_DEFAULT);
    		uint16_t *res;
    		res = malloc(sizeof(uint16_t));
    		read_configreg(fd, res);
    		shutdown_mode(fd, SHUTDOWN_MODE);
    		printf("Changing to shutdown mode TMP102\n");
    		read_configreg(fd, res);
    	}

    	/* close sensor*/
    	close_tmp102(fd);
    	pthread_mutex_unlock(&mutex);

    	//temp_failure = FAIL;

    	/* if sensor fails to respond send a message to main*/
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

    		/* for every 7 secs a request is sent to fetch light to log_task*/
        	if(counter_temp % 7 ==  0)
        	{
        		gettimeofday(&smsg.time_stamp, NULL);
        		smsg.log_level = INFO;
        		smsg.src_id = TEMP_TASK;
        		smsg.dest_id = LOG_TASK;
        		smsg.log_type = REQUEST;
        		smsg.data = LUMEN;

        		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
        			printf("\nUnable to send");
        		goto end;
        	}

        	/* For every 11 second make apds9301 library to read and write all regs*/
        	if(counter_temp % 11 ==  0)
        	{
        		gettimeofday(&smsg.time_stamp, NULL);
        		smsg.log_level = INFO;
        		smsg.src_id = TEMP_TASK;
        		smsg.dest_id = LOG_TASK;
        		smsg.log_type = REQUEST;
        		smsg.data = RW_ALL_REGS;

        		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
        			printf("\nUnable to send");
        		goto end;
        	}

        	/* for every 13 secs a request is sent to print apds9301 sensnor id to log_task*/
        	if(counter_temp % 13 ==  0)
        	{
        		gettimeofday(&smsg.time_stamp, NULL);
        		smsg.log_level = INFO;
        		smsg.src_id = TEMP_TASK;
        		smsg.dest_id = LOG_TASK;
        		smsg.log_type = REQUEST;
        		smsg.data = PRINT_ID;

        		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
        			printf("\nUnable to send");
        		goto end;
        	}

        	/*For every 17 seconds make a request to enable interrupt in light sensor*/
        	if(counter_temp % 17 ==  0)
        	{
        		gettimeofday(&smsg.time_stamp, NULL);
        		smsg.log_level = INFO;
        		smsg.src_id = TEMP_TASK;
        		smsg.dest_id = LOG_TASK;
        		smsg.log_type = REQUEST;
        		smsg.data = ENABLE_INT;

        		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
        			printf("\nUnable to send");
        		goto end;
        	}




        	/*If it recieves a message from log_task
        	 * It means light task asked for temperature
        	 * sends a message to log task including temperature
        	 */
            if( mq_receive(log_to_temp, (char*)&rmsg, sizeof(rmsg), NULL) != -1){
            	gettimeofday(&smsg.time_stamp, NULL);
            	smsg.log_level = INFO;
            	smsg.src_id = TEMP_TASK;
            	smsg.dest_id = LOG_TASK;
            	smsg.log_type = RESPONSE;
            	smsg.data = temperature;

            		if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
            			printf("\nUnable to send");
            } else {

            	/*If no request is recied it sends a message with temperature to log ask*/
            	gettimeofday(&smsg.time_stamp, NULL);
            	smsg.log_level = INFO;
            	smsg.src_id = TEMP_TASK;
            	smsg.dest_id = LOG_TASK;
            	smsg.log_type = DATA;
            	smsg.data = temperature;

            	if( mq_send(temp_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
            		printf("\nUnable to send");
            }

            /*If there is no failure send a message to main task as heart beat*/
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


	/* send message to log task that this thread is initilised*/
	gettimeofday(&smsg.time_stamp, NULL);
	smsg.log_level = STARTUP;
	smsg.src_id = LIGHT_TASK;
	smsg.dest_id = LOG_TASK;
	smsg.log_type = INIT;
	smsg.data = 0;

    if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
    	printf("\nUnable to send");


	/* light_alive is set when sigint is caught or if sensor fails responding*/
    while(light_alive == 0) {


    	pthread_mutex_lock(&mutex);

    	/*inialiise sensor on i2c 2 bus*/
    	fd = sensor_init(2);
    	if( fd == FAIL)
    		light_failure = FAIL;

    	/*If check library is changed to 1 all APDS9301 library functions are checked*/
    	if(CHECK_LIBRARY){
    		int status = rw_allregs_apds(fd);
    		if(status != FAIL)
    			printf("Successful reads and writes to all registers in APDS9301\n");
    		write_interrupt_controlreg(fd, INT_ENABLE);
    		status = read_interrupt_controlreg(fd);
    		if(status != FAIL)
    			printf("Interrupt enabled in APDS9301\n");
    		print_id(fd);

    	}

    	/* gets calibrated lumen value from APDS9301*/
    	float lumen = get_luminosity(fd);
    	if( lumen == FAIL)
    		light_failure = FAIL;

    	/*compares previos state with present
    	 * If there is change it means there is transformation
    	 * if lumen is less than 1  and previous value is greater than 1
    	 * its a night
    	 *
    	 * If lumen is greater than 1 and previous value is less than 1
    	 * its a day
    	 */
    	if(lumen < 1 && prev_lumen > 1)
    		night = 1;
    	if( lumen > 1 && prev_lumen < 1)
    		day = 1;
    	printf("lumen %f, prev_lumen %f, day %d, night %d\n", lumen, prev_lumen, day, night);

    	prev_lumen = lumen;



    	/*checks for sensor failue and if there is, a message is sent to main task*/
    	//light_failure = FAIL;
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


        	/*If it recieves a message from log_task*/

            if( mq_receive(log_to_light, (char*)&rmsg, sizeof(rmsg), NULL) != -1){
            	/*If the received message asks for LUMEN*/
            	if(rmsg.log_type == REQUEST && rmsg.data == LUMEN){

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
                	printf("Responding as Lumen \n");
            	}

            	/*If the message asks for coammand read write all regs*/
            	else if(rmsg.log_type == REQUEST && rmsg.data == RW_ALL_REGS){
            		printf("Response to temp\n");
            		gettimeofday(&smsg.time_stamp, NULL);
            		smsg.log_level = INFO;
            		smsg.src_id = LIGHT_TASK;
            		smsg.dest_id = LOG_TASK;
            		smsg.log_type = RESPONSE;
            		smsg.data = RW_ALL_REGS;

            		day = 0;
            		night = 0;
            		printf("Responding as RW all regs \n");
            		/*RW all regs*/
                	int status = rw_allregs_apds(fd);
                	/*On a successful read and write send a message*/
                	if(status == SUCCESS){
                		if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
                			printf("\nUnable to send");
                	}
            	}

            	/*If messgae asks for Enable interrupt*/
            	else if(rmsg.log_type == REQUEST && rmsg.data == ENABLE_INT){
            		printf("Response to temp\n");
            		gettimeofday(&smsg.time_stamp, NULL);
            		smsg.log_level = INFO;
            		smsg.src_id = LIGHT_TASK;
            		smsg.dest_id = LOG_TASK;
            		smsg.log_type = RESPONSE;
            		smsg.data = ENABLE_INT;

            		day = 0;
            		night = 0;
            		printf("Responding as write interrupt  \n");
                	int status = write_interrupt_controlreg(fd, INT_ENABLE);
                	/*If interrupt is enabled successfully send a message*/
                	if(status == SUCCESS){
                		if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
                			printf("\nUnable to send");
                	}
            	}

            	/*If the message asks for Sensor ID*/
            	else if(rmsg.log_type == REQUEST && rmsg.data == PRINT_ID){
            		printf("Response to temp\n");
            		gettimeofday(&smsg.time_stamp, NULL);
            		smsg.log_level = INFO;
            		smsg.src_id = LIGHT_TASK;
            		smsg.dest_id = LOG_TASK;
            		smsg.log_type = RESPONSE;
            		smsg.data = PRINT_ID;

            		day = 0;
            		night = 0;
            		printf("Responding as print ID  \n");
                	int status = print_id(fd);
                	if(status == SUCCESS){
                		if( mq_send(light_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
                			printf("\nUnable to send");
                	}
            	}



            } else {

            	/* for every 10 secs a request is sent to fetch light to log_task*/
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

            	/* If its a transformation to a day. send a message to log_task as alert*/
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

            	/* If its a transformation to a night. send a message to log_task as alert*/
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
                	/* If its not a  transformation. send a message to log_task as info and lumen*/
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



            /*If there is no failure send a message to main task as heart beat*/
    		gettimeofday(&smsg.time_stamp, NULL);
    		smsg.log_level = INFO;
    		smsg.src_id = LIGHT_TASK;
    		smsg.dest_id = MAIN_TASK;
    		smsg.log_type = HEART_BEAT;
    		smsg.data = 0;

    		if( mq_send(light_to_main, (const char*)&smsg, sizeof(smsg), 1) == -1)
    				printf("\nUnable to send heart_beat : LIGHT TASK");

    	}
    	/*close file*/
    	close_apds9301(fd);

    	pthread_mutex_unlock(&mutex);
    	counter_light++;
        usleep(exec_period_usecs);
    }

}

/* It closes all queues*/

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

/*It opens all queues*/
void open_queues(void){


	temp_to_log = mq_open (TEMP_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	light_to_log = mq_open (LIGHT_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	log_to_light = mq_open (LOG_TO_LIGHT, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	log_to_temp = mq_open (LOG_TO_TEMP, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	main_to_log = mq_open (MAIN_TO_LOG, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	temp_to_main = mq_open (TEMP_TO_MAIN, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);
	light_to_main = mq_open (LIGHT_TO_MAIN, O_CREAT|O_RDWR|O_NONBLOCK, 0666, &attr);

}



int main(int argc, char *argv[]){
	strcpy(file_name, argv[1]);

	message_t rmsg1, rmsg2, smsg;


	/*close all queues*/
	close_queues();

	/*set attributes*/
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(rmsg1);
	attr.mq_flags = 0;

	/*open all queues*/
	open_queues();

	/*Initilise signal handler*/
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

	/* create three threads*/
	pthread_create(&log_thread, NULL, log_task, NULL);
	pthread_create(&temp_thread, NULL, temp_task, NULL);
	pthread_create(&light_thread, NULL, light_task, NULL);

	usleep(exec_period_usecs);
	uint8_t temp_life = 0, light_life = 0;

	/*checks for caught_signal temp_alive and light_live
	 * If signal is caught while loop exits
	 * checks if atleat 1 thread is alive.
	 *
	 * If no thread alive it closes
	 */
	while( caught_signal == 0 || (light_alive | temp_alive) == 0){


		/*checks if light thread is alive*/
		if(light_alive == 0){

			/*checks for message from light_task*/
			if(mq_receive(light_to_main, (char*)&rmsg1, sizeof(rmsg1), NULL) == -1){
				printf("\n No heartbeat from light task\n");
				light_life++;
			} else {
				/*If there is a sensor failure sends info to log_task*/
				if(rmsg1.log_type == FAILURE){

					gettimeofday(&smsg.time_stamp, NULL);
					smsg.log_level = ALERT;
					smsg.src_id = MAIN_TASK;
					smsg.dest_id = LOG_TASK;
					smsg.log_type = FAILURE;
					smsg.data = 1;

					if( mq_send(main_to_log, (const char*)&smsg, sizeof(smsg), 1) == -1)
	    				printf("\nUnable to send errorlog : MAIN TASK");
					/*On failure led1 is turned on*/
					led1_on();
					printf("In light check\n");
				}
			}


			/* If no message is recievd for 10 consecutive seconds it sends a message to log_taks to kill
			 * light thread
			 */
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

		/*checks if temp thread is alive*/
		if(temp_alive == 0){
			/*checks for message from temp task*/
			if(mq_receive(temp_to_main, (char*)&rmsg2, sizeof(rmsg2), NULL) == -1){
				printf("\nNo heartbeat from temp task\n");
				temp_life++;
			} else {
				/*If there is a sensor failure sends info to log_task*/
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

			/* If no message is received for 10 consecutive seconds it sends a message to log_taks to kill
			 * temp thread
			 */
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
	sleep(2);

	/*IF there is no sigint but want to close everything*/
	if(caught_signal !=1)
		clean_everything();
	pthread_join(log_thread, NULL);
	pthread_join(temp_thread, NULL);
	pthread_join(light_thread, NULL);


}
