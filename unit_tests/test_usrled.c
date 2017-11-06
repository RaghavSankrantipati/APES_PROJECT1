#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "usrled.h"

/**********************************************************************
*@Filename:test_usrled.c
*
*@Description:This is a test for library of LEDs on Beaglebone
*@Author:Sai Raghavendra Sankrantipati
*@Date:11/5/2017
*@Tool : Cmocka
 **********************************************************************/
/*test LED2 on function*/
static void test_led2on(void** state)
{
	int status = led2_on();
	assert_int_equal(status, 1);
}

/*test LED2 off function*/
static void test_led2off(void** state)
{
	int status = led2_off();
	assert_int_equal(status, 1);
}

/*test LED1 on function*/
static void test_led1on(void** state)
{
	int status = led1_on();
	assert_int_equal(status, 1);
}

/*test LED1 off function*/
static void test_led1off(void** state)
{
	int status = led1_off();
	assert_int_equal(status, 1);
}


int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_led1on),
        cmocka_unit_test(test_led2on),
        cmocka_unit_test(test_led1off),
        cmocka_unit_test(test_led2off),

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
