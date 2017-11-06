#APES_PROJECT1
Project 1 repository for APES ECEN 5013
A project based on linux i2c drivers and posix messsage queues. A concurrent software application on Linux that will interact with both Userspace and kernel space in addition to multiple connected I2C devices. Two different tasks were created with these two interfaces to collect data, another task to log system status information and the main task to create these tasks and error checkings.

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
### Compling
I used eclipse for cross-compilation and deployment.
Follow this tutorial for environment setup.

[Derek Molly's environment setup tutorial](https://www.youtube.com/watch?v=vFv_-ykLppo&t=1438s)

### Unit tests
Cmocka is used for unit testing.

[Prof. Fosdick's Cmocka tutorial](https://github.com/afosdick/ecen5013/tree/develop/tutorials/unit_tests)
