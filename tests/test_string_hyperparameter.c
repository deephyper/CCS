#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

void test_create() {
	ccs_hyperparameter_t       hyperparameter;
	ccs_hyperparameter_type_t  type;
	ccs_datum_t                default_value;
	ccs_result_t               err;
	ccs_bool_t                 check;
	ccs_interval_t             interval;
	const char                *name;
	void *                     user_data;
	ccs_distribution_t         distribution;

	err = ccs_create_string_hyperparameter("my_param", (void *)0xdeadbeef,
	                                       &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_hyperparameter_get_type(hyperparameter, &type);
	assert( err == CCS_SUCCESS );
	assert( type == CCS_HYPERPARAMETER_TYPE_STRING );

	err = ccs_hyperparameter_get_default_value(hyperparameter, &default_value);
	assert( err == CCS_SUCCESS );
	assert( default_value.type == CCS_NONE );

	err = ccs_hyperparameter_get_name(hyperparameter, &name);
	assert( err == CCS_SUCCESS );
	assert( strcmp(name, "my_param") == 0 );

	err = ccs_hyperparameter_get_user_data(hyperparameter, &user_data);
	assert( err == CCS_SUCCESS );
	assert( (void*)0xdeadbeef == user_data );

	err = ccs_hyperparameter_get_default_distribution(hyperparameter, &distribution);
	assert( err == -CCS_UNSUPPORTED_OPERATION );

	err = ccs_hyperparameter_sampling_interval(hyperparameter, &interval);
	assert( err == CCS_SUCCESS );
	err = ccs_interval_empty(&interval, &check );
	assert( err == CCS_SUCCESS );
	assert( CCS_TRUE == check );

	ccs_release_object(hyperparameter);
}

void test_string_memoization() {
	ccs_hyperparameter_t  hyperparameter;
	ccs_result_t          err;
	ccs_datum_t           din, dout, dout2;
	ccs_bool_t            check;
	char                 *str;

	err = ccs_create_string_hyperparameter("my_param", NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );

	str = strdup("my string");
	din = ccs_string(str);
	din.flags = CCS_FLAG_TRANSIENT;

	err = ccs_hyperparameter_validate_value(hyperparameter, din, &dout, &check);
	assert( err == CCS_SUCCESS );
	assert( check == CCS_TRUE );
	assert( dout.type == CCS_STRING );
	assert( dout.flags == 0 );
	assert( strcmp(dout.value.s, "my string") == 0 );
	err = ccs_hyperparameter_validate_value(hyperparameter, din, &dout2, NULL);
	assert( err == CCS_SUCCESS );
	assert( dout2.value.s == dout.value.s );

	strcpy(str, "nope");
	assert( strcmp(dout.value.s, "my string") == 0 );

	err = ccs_hyperparameter_validate_value(hyperparameter, din, &dout, &check);
	assert( err == CCS_SUCCESS );
	assert( check == CCS_TRUE );
	assert( dout.type == CCS_STRING );
	assert( dout.flags == 0 );
	assert( strcmp(dout.value.s, "nope") == 0 );

	din = ccs_int(3);
	err = ccs_hyperparameter_validate_value(hyperparameter, din, &dout, &check);
	assert( err == CCS_SUCCESS );
	assert( check == CCS_FALSE );
	assert( dout.type == CCS_INACTIVE );

	ccs_release_object(hyperparameter);
}

int main() {
	ccs_init();
	test_create();
	ccs_fini();
	return 0;
}
