#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "message.h"

/**********************************************************************
*@Filename:test_logtask.c
*
*@Description:This is a test few log task functions
*@Author:Sai Raghavendra Sankrantipati
*@Date:11/5/2017
*@Tool : Cmocka
 **********************************************************************/

/*Test log_init function which */
static void test_loginit(void** state)
{
	/*This function returns SUCCESS on
	 * successful writing to a file
	 */
	int status = log_init();
	assert_int_equal(status, SUCCESS);
}


/*Test log_lightqueue function which */
static void test_loglightQ(void** state)
{
	/*This function returns SUCCESS on
	 * successful writing to a file
	 */
	int status = log_lightqueue();
	/*As there are no messages it should return FAIL*/
	assert_int_equal(status, FAIL);
}

/*Test log_tempqueue function which */
static void test_logtempQ(void** state)
{
	/*This function returns SUCCESS on
	 * successful writing to a file
	 */
	int status = log_tempqueue();
	/*As there are no messages it should return FAIL*/
	assert_int_equal(status, FAIL);
}

/*Test log_mainqueue function which */
static void test_logmainQ(void** state)
{
	/*This function returns SUCCESS on
	 * successful writing to a file
	 */
	int status = log_mainqueue();
	/*As there are no messages it should return FAIL*/
	assert_int_equal(status, FAIL);
}


int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_loginit),
        cmocka_unit_test(test_loglightQ),
        cmocka_unit_test(test_logtempQ),
        cmocka_unit_test(test_logmainQ),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
