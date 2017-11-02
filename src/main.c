

#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#define TEMP_MSG_QUEUE_RESPONSE "/temp_log_queue_response"

void main () {

	struct mq_attr mq_attr_log;
	mq_attr_log.mq_maxmsg = 10;
	mq_attr_log.mq_msgsize = 2;
	mq_attr_log.mq_flags = 0;
	mqd_t mqd_temp = mq_open(TEMP_MSG_QUEUE_RESPONSE, \
						O_CREAT|O_RDWR|O_NONBLOCK, \
						0666, \
&mq_attr_log);
	printf("%d", mqd_temp);
}
