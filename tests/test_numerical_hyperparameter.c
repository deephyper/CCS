#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

#define NUM_SAMPLES 10000

void test_create() {
	ccs_hyperparameter_t       hyperparameter;
	ccs_hyperparameter_type_t  type;
	ccs_datum_t                default_value;
	ccs_result_t               err;
	ccs_bool_t                 check;
	const char                *name;
	void *                     user_data;
	ccs_distribution_t         distribution;
	ccs_distribution_type_t    dist_type;
	ccs_interval_t             interval;

	err = ccs_create_numerical_hyperparameter("my_param", CCS_NUM_FLOAT,
	                                          CCSF(-5.0), CCSF(5.0),
	                                          CCSF(0.0), CCSF(1.0),
	                                          (void *)0xdeadbeef,
	                                          &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_hyperparameter_get_type(hyperparameter, &type);
	assert( err == CCS_SUCCESS );
	assert( type == CCS_HYPERPARAMETER_TYPE_NUMERICAL );

	err = ccs_hyperparameter_get_default_value(hyperparameter, &default_value);
	assert( err == CCS_SUCCESS );
	assert( default_value.type == CCS_FLOAT );
	assert( default_value.value.f == 1.0 );

	err = ccs_hyperparameter_get_name(hyperparameter, &name);
	assert( err == CCS_SUCCESS );
	assert( strcmp(name, "my_param") == 0 );

	err = ccs_hyperparameter_get_user_data(hyperparameter, &user_data);
	assert( err == CCS_SUCCESS );
	assert( user_data == (void *)0xdeadbeef );

	err = ccs_hyperparameter_get_default_distribution(hyperparameter, &distribution);
	assert( err == CCS_SUCCESS );
	assert( distribution );

	err = ccs_distribution_get_type(distribution, &dist_type);
	assert( err == CCS_SUCCESS );
	assert( dist_type == CCS_UNIFORM );

	err = ccs_distribution_get_bounds(distribution, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_FLOAT );
	assert( interval.lower.f  == -5.0 );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.f  == 5.0 );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_hyperparameter_check_value(hyperparameter, default_value, &check);
	assert( err == CCS_SUCCESS );
	assert( check == CCS_TRUE );

	default_value.value.f = 6.0;
	err = ccs_hyperparameter_check_value(hyperparameter, default_value, &check);
	assert( err == CCS_SUCCESS );
	assert( check == CCS_FALSE );

	err = ccs_release_object(distribution);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
}

void test_samples() {
	ccs_rng_t                  rng;
	ccs_hyperparameter_t       hyperparameter;
	ccs_distribution_t         distribution;
	const size_t               num_samples = NUM_SAMPLES;
	ccs_datum_t                samples[NUM_SAMPLES];
	ccs_result_t               err;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_numerical_hyperparameter("my_param", CCS_NUM_FLOAT,
	                                          CCSF(-5.0), CCSF(5.0),
	                                          CCSF(0.0), CCSF(1.0),
	                                          NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_hyperparameter_get_default_distribution(hyperparameter, &distribution);
	assert( err == CCS_SUCCESS );
	assert( distribution );

	err = ccs_hyperparameter_samples(hyperparameter, distribution, rng,
	                                 num_samples, samples);
	assert( err == CCS_SUCCESS );

	for( size_t i = 0; i < num_samples; i++) {
		assert( samples[i].type == CCS_FLOAT );
		assert( samples[i].value.f >= -5.0 && samples[i].value.f < 5.0 );
	}

	err = ccs_release_object(distribution);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_oversampling() {
	ccs_rng_t                  rng;
	ccs_hyperparameter_t       hyperparameter;
	ccs_distribution_t         distribution;
	const size_t               num_samples = NUM_SAMPLES;
	ccs_datum_t                samples[NUM_SAMPLES];
	ccs_result_t               err;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );

	err = ccs_create_normal_float_distribution(0.0, 1.0, CCS_LINEAR, 0.0,
	                                           &distribution);
	assert( err == CCS_SUCCESS );

	err = ccs_create_numerical_hyperparameter("my_param", CCS_NUM_FLOAT,
	                                          CCSF(-1.0), CCSF(1.0),
	                                          CCSF(0.0), CCSF(0.0),
	                                          NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_hyperparameter_samples(hyperparameter, distribution, rng,
	                                 num_samples, samples);
	assert( err == CCS_SUCCESS );

	for( size_t i = 0; i < num_samples; i++) {
		assert( samples[i].type == CCS_FLOAT );
		assert( samples[i].value.f >= -1.0 && samples[i].value.f < 1.0 );
	}

	err = ccs_release_object(distribution);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

int main() {
	ccs_init();
	test_create();
	test_samples();
	test_oversampling();
	ccs_fini();
	return 0;
}
