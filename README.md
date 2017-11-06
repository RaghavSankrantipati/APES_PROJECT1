# APES_PROJECT1
Project 1 repository for APES ECEN 5013
A project based on linux i2c drivers and posix messsage queues. A concurrent software application on Linux that will interact with both Userspace and kernel space in addition to multiple connected I2C devices. Two different tasks were created with these two interfaces to collect data, another task to log system status information and the main task to create these tasks and error checkings.

* There are 7 queues used in this project.
* For every 1 second temperature and light task takes data and sends a message to log task and main task. Message to main task contains heartbeat. Message to log task contains data.
* At random intervals temperature asks for various light api and light task asks for temperature. For this process a message is sent to log task. It analyses the request and sends a message according to the request.
* After a request is received a response is sent to log task. All these were logged into a file given at run time by log task
* Temperature task uses tmp102.c, a library for TMP102 sensor.
* Light task uses apsd9301.c, a C library for APDS 9301 sensor
* Main task keeps receiving heart beat. If it did not receive any task's heartbeat for more than 10 seconds it sends a message to log task.
* Log task then terminates the thread and writes into the file.
* In case of sensor failure a message will be sent by temperature and
* light tasks to main task. Like above, a message is sent to log task to terminate thread and write into file.
* Light looks for a transformation of day and light. If there is a change it logs a message and writes into file.

### Apparatus
* Beaglebone Green
* TMP102, Temperature sensor
* APDS9301, Light sensor
...Connect 2 sensors on I2C bus 2

### Execution
You can copy the binary 'project' and execute it. It expects a file name, in which everything is logged later.
For example:
```
# sudo ./project log.txt
```
Press CTRL + C to exit or remove I2C connections (Error handling).
### Compling
I used eclipse for cross-compilation and deployment.
Follow this tutorial for environment setup. After setup, copy src and include folders paste it in your project. Build.

[Derek Molly's environment setup tutorial](https://www.youtube.com/watch?v=vFv_-ykLppo&t=1438s)

### Unit tests
Cmocka is used for unit testing.

[Prof. Fosdick's Cmocka tutorial](https://github.com/afosdick/ecen5013/tree/develop/tutorials/unit_tests)
