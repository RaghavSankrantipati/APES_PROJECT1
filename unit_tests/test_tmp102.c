#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tmp102.h"

/**********************************************************************
*@Filename:test_tmp102.c
*
*@Description:This is a test for library of TMP102 sensor
*@Author:Sai Raghavendra Sankrantipati
*@Date:11/5/2017
*@Tool : Cmocka
*@Usage : Connect TMP102 to I2C 2 and check library functions to read and write registers
 **********************************************************************/

/*testing read write all registers command*/
static void test_rwallregs(void** state)
{
	int fd = tmp102_init(2);
	int status = rw_allregs_tmp102(fd);
    assert_int_equal(status, SUCCESS);

}

/*testing writes to interrupt control register*/
static void test_enableint(void **state)
{
	int fd = tmp102_init(2);

	int status;
	/*writing different values to different registers*/
	status = write_pointerreg(fd, TEMPREG_ADDRESS);
	assert_int_equal(status, SUCCESS);
	status = write_pointerreg(fd, CONFREG_ADDRESS);
	assert_int_equal(status, SUCCESS);
	status = write_pointerreg(fd, POINTER_ADDRESS);
	assert_int_equal(status, SUCCESS);
}

/*testing enabling shutdown mode*/
static void test_shutdownmode(void **state)
{

	int fd = tmp102_init(2);
	int status;
	uint16_t *res;
	res = malloc(sizeof(uint16_t));
	/*reading config register before enabling shutdown mode*/
	status = read_configreg(fd, res);
	assert_int_equal(status, SUCCESS);
	/*enabling shutdonw mode*/
	shutdown_mode(fd, SHUTDOWN_MODE);
	printf("Changing to shutdown mode TMP102\n");
	/*reading config register after enabling shutdown mode*/
	status = read_configreg(fd, res);
	assert_int_equal(status, SUCCESS);
}

/*test printing temperatire*/
static void test_pritnttemperature(void **state)
{
	int fd = tmp102_init(2);
	int status;
	status = print_temperature(fd, CONFIG_DEFAULT);
	assert_int_not_equal(status, FAIL);
}

int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_rwallregs),
        cmocka_unit_test(test_writepointer),
        cmocka_unit_test(test_shutdownmode),
        cmocka_unit_test(test_pritnttemperature),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
