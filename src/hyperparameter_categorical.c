#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"
#include <string.h>

struct _ccs_hyperparameter_categorical_data_s {
	_ccs_hyperparameter_common_data_t  common_data;
	ccs_int_t                          num_possible_values;
	ccs_datum_t                       *possible_values;
};
typedef struct _ccs_hyperparameter_categorical_data_s _ccs_hyperparameter_categorical_data_t;

static ccs_error_t
_ccs_hyperparameter_categorical_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_hyperparameter_categorical_samples(_ccs_hyperparameter_data_t *data,
                                        ccs_distribution_t          distribution,
                                        ccs_rng_t                   rng,
                                        size_t                      num_values,
                                        ccs_datum_t                *values) {
	_ccs_hyperparameter_categorical_data_t *d =
	    (_ccs_hyperparameter_categorical_data_t *)data;
	ccs_error_t err;
	ccs_numeric_t *vs = (ccs_numeric_t *)values + num_values;
        ccs_bool_t oversampling;
	err = ccs_distribution_check_oversampling(distribution,
	                                          &(d->common_data.interval),
	                                          &oversampling);
	if (err)
		return err;
	err = ccs_distribution_samples(distribution, rng, num_values, vs);
	if (err)
		return err;
	if (!oversampling) {
		for(size_t i = 0; i < num_values; i++)
			values[i] = d->possible_values[vs[i].i];
	} else {
		size_t found = 0;
		for(size_t i = 0; i < num_values; i++)
			if (vs[i].i >= 0 && vs[i].i < d->num_possible_values)
				values[found++] = d->possible_values[vs[i].i];
		vs = NULL;
		size_t coeff = 2;
		while (found < num_values) {
			size_t buff_sz = (num_values - found)*coeff;
			vs = (ccs_numeric_t *)malloc(sizeof(ccs_numeric_t)*buff_sz);
			if (!vs)
				return -CCS_ENOMEM;
			err = ccs_distribution_samples(distribution, rng,
			                               buff_sz, vs);
			for(size_t i = 0; i < buff_sz && found < num_values; i++)
				if (vs[i].i >= 0 && vs[i].i < d->num_possible_values)
					values[found++] = d->possible_values[vs[i].i];
			coeff <<= 1;
			free(vs);
			if (coeff > 32)
				return -CCS_SAMPLING_UNSUCCESSFUL;
		}
	}
	return CCS_SUCCESS;
}

ccs_error_t
_ccs_hyperparameter_categorical_get_default_distribution(
		_ccs_hyperparameter_data_t *data,
		ccs_distribution_t         *distribution) {
	_ccs_hyperparameter_categorical_data_t *d = (_ccs_hyperparameter_categorical_data_t *)data;
	ccs_interval_t *interval = &(d->common_data.interval);
	return ccs_create_uniform_distribution(interval->type,
	                                       interval->lower, interval->upper,
	                                       CCS_LINEAR, CCSI(0),
	                                       distribution);
}

static _ccs_hyperparameter_ops_t _ccs_hyperparameter_categorical_ops = {
	{ &_ccs_hyperparameter_categorical_del },
	&_ccs_hyperparameter_categorical_samples,
	&_ccs_hyperparameter_categorical_get_default_distribution
};

ccs_error_t
ccs_create_categorical_hyperparameter(const char           *name,
                                      size_t                num_possible_values,
                                      ccs_datum_t          *possible_values,
                                      size_t                default_value_index,
                                      void                 *user_data,
                                      ccs_hyperparameter_t *hyperparameter_ret) {
	if (!hyperparameter_ret || !name)
		return -CCS_INVALID_VALUE;
	if (!num_possible_values ||
	     num_possible_values > INT64_MAX ||
	     num_possible_values <= default_value_index)
		return -CCS_INVALID_VALUE;
	if (!possible_values)
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1,
	    sizeof(struct _ccs_hyperparameter_s) +
	    sizeof(_ccs_hyperparameter_categorical_data_t) +
	    sizeof(ccs_datum_t) * num_possible_values +
	    strlen(name) + 1);
	if (!mem)
		return -CCS_ENOMEM;

	ccs_interval_t interval;
	interval.type = CCS_NUM_INTEGER;
	interval.lower.i = 0;
	interval.upper.i = (ccs_int_t)num_possible_values;
	interval.lower_included = CCS_TRUE;
	interval.upper_included = CCS_FALSE;

	ccs_hyperparameter_t hyperparam = (ccs_hyperparameter_t)mem;
	_ccs_object_init(&(hyperparam->obj), CCS_HYPERPARAMETER, (_ccs_object_ops_t *)&_ccs_hyperparameter_categorical_ops);
	_ccs_hyperparameter_categorical_data_t *hyperparam_data =
	    (_ccs_hyperparameter_categorical_data_t *)(mem +
	         sizeof(struct _ccs_hyperparameter_s));
	hyperparam_data->common_data.type = CCS_CATEGORICAL;
	hyperparam_data->common_data.name = (char *)(mem +
	    sizeof(struct _ccs_hyperparameter_s) +
	    sizeof(_ccs_hyperparameter_categorical_data_t) +
	    sizeof(ccs_datum_t) * num_possible_values);
	strcpy((char *)hyperparam_data->common_data.name, name);
	hyperparam_data->common_data.user_data = user_data;
	hyperparam_data->common_data.default_value = possible_values[default_value_index];
	hyperparam_data->common_data.interval = interval;
	hyperparam_data->num_possible_values = num_possible_values;
	hyperparam_data->possible_values = (ccs_datum_t *)(mem +
	    sizeof(struct _ccs_hyperparameter_s) +
	    sizeof(_ccs_hyperparameter_categorical_data_t));
	memcpy(hyperparam_data->possible_values, possible_values,
	    sizeof(ccs_datum_t) * num_possible_values);
	hyperparam->data = (_ccs_hyperparameter_data_t *)hyperparam_data;
	*hyperparameter_ret = hyperparam;
	return CCS_SUCCESS;
}


