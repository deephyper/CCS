#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"

struct _ccs_distribution_uniform_data_s {
	_ccs_distribution_common_data_t common_data;
	ccs_value_t                     lower;
	ccs_value_t                     upper;
	ccs_data_type_t                 internal_type;
	ccs_value_t                     internal_lower;
        ccs_value_t                     internal_upper;
	int                             quantize;
};
typedef struct _ccs_distribution_uniform_data_s _ccs_distribution_uniform_data_t;

static ccs_error_t
_ccs_distribution_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_uniform_get_num_parameters(_ccs_distribution_data_t *data,
                                             size_t                   *num_parameters_ret);

static ccs_error_t
_ccs_distribution_uniform_get_parameters(_ccs_distribution_data_t *data,
                                         size_t                    num_parameters,
                                         ccs_datum_t              *parameters,
                                         size_t                   *num_parameters_ret);

static ccs_error_t
_ccs_distribution_uniform_samples(_ccs_distribution_data_t *data,
                                  ccs_rng_t                 rng,
                                  size_t                    num_values,
                                  ccs_value_t              *values);

_ccs_distribution_ops_t _ccs_distribution_uniform_ops = {
	{ &_ccs_distribution_del },
	&_ccs_distribution_uniform_get_num_parameters,
	&_ccs_distribution_uniform_get_parameters,
	&_ccs_distribution_uniform_samples
 };

static ccs_error_t
_ccs_distribution_uniform_get_num_parameters(_ccs_distribution_data_t *data,
                                             size_t                   *num_parameters_ret) {
	*num_parameters_ret = 2;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_uniform_get_parameters(_ccs_distribution_data_t *data,
                                         size_t                    num_parameters,
                                         ccs_datum_t              *parameters,
                                         size_t                   *num_parameters_ret) {
	if (num_parameters > 0 && num_parameters < 2)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_uniform_data_t *d = (_ccs_distribution_uniform_data_t *)data;

	if (num_parameters_ret)
		*num_parameters_ret = 2;

	if (parameters) {
		parameters[0].type = d->common_data.data_type;
		parameters[0].value = d->lower;
		parameters[1].type = d->common_data.data_type;
		parameters[1].value = d->upper;
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_uniform_samples(_ccs_distribution_data_t *data,
                                  ccs_rng_t                 rng,
                                  size_t                    num_values,
                                  ccs_value_t              *values) {
	_ccs_distribution_uniform_data_t *d = (_ccs_distribution_uniform_data_t *)data;
	size_t i;
	const ccs_data_type_t  data_type      = d->common_data.data_type;
	const ccs_scale_type_t scale_type     = d->common_data.scale_type;
	const ccs_value_t      quantization   = d->common_data.quantization;
	const ccs_value_t      lower          = d->lower;
	const ccs_value_t      internal_lower = d->internal_lower;
	const ccs_value_t      internal_upper = d->internal_upper;
	const int              quantize       = d->quantize;
	gsl_rng *grng;
	ccs_error_t err = ccs_rng_get_gsl_rng(rng, &grng);
	if (err)
		return err;

	if (data_type == CCS_FLOAT) {
		for (i = 0; i < num_values; i++) {
			values[i].f = gsl_ran_flat(grng, internal_lower.f, internal_upper.f);
		}
		if (scale_type == CCS_LOGARITHMIC) {
			for (i = 0; i < num_values; i++)
				values[i].f = exp(values[i].f);
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i].f = floor((values[i].f - lower.f)/quantization.f) * quantization.f + lower.f;
		} else
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i].f = floor(values[i].f) * quantization.f + lower.f;
			else
				for (i = 0; i < num_values; i++)
					values[i].f += lower.f;
	} else {
		if (scale_type == CCS_LOGARITHMIC) {
			for (i = 0; i < num_values; i++) {
				values[i].i = floor(exp(gsl_ran_flat(grng, internal_lower.f, internal_upper.f)));
			}
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i].i = ((values[i].i - lower.i)/quantization.i) * quantization.i + lower.i;
		} else {
			for (i = 0; i < num_values; i++) {
				values[i].i = gsl_rng_uniform_int(grng, internal_upper.i);
			}
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i].i = values[i].i * quantization.i + lower.i;
			else
				for (i = 0; i < num_values; i++)
					values[i].i += lower.i;
		}
	}
	return CCS_SUCCESS;
}

ccs_error_t
_ccs_create_uniform_distribution(ccs_data_type_t     data_type,
                                 ccs_value_t         lower,
                                 ccs_value_t         upper,
                                 ccs_scale_type_t    scale_type,
                                 ccs_value_t         quantization,
                                 ccs_distribution_t *distribution_ret) {
	if (!distribution_ret)
		return -CCS_INVALID_VALUE;
	if (data_type != CCS_FLOAT && data_type != CCS_INTEGER)
		return -CCS_INVALID_TYPE;
	if (scale_type != CCS_LINEAR && scale_type != CCS_LOGARITHMIC)
		return -CCS_INVALID_SCALE;
	if (data_type == CCS_INTEGER && (
		lower.i >= upper.i ||
		(scale_type == CCS_LOGARITHMIC && lower.i <= 0) ||
		quantization.i < 0 ||
		quantization.i > upper.i - lower.i ) )
		return -CCS_INVALID_VALUE;
	if (data_type == CCS_FLOAT && (
		lower.f >= upper.f ||
		(scale_type == CCS_LOGARITHMIC && lower.f <= 0.0) ||
		quantization.f < 0.0 ||
		quantization.f > upper.f - lower.f ) )
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_uniform_data_t));

	if (!mem)
		return -CCS_ENOMEM;
	ccs_distribution_t distrib = (ccs_distribution_t)mem;
	_ccs_object_init(&(distrib->obj), CCS_DISTRIBUTION, (_ccs_object_ops_t *)&_ccs_distribution_uniform_ops);
        _ccs_distribution_uniform_data_t * distrib_data = (_ccs_distribution_uniform_data_t *)(mem + sizeof(struct _ccs_distribution_s));
	distrib_data->common_data.type         = CCS_UNIFORM;
	distrib_data->common_data.data_type    = data_type;
	distrib_data->common_data.scale_type   = scale_type;
	distrib_data->common_data.quantization = quantization;
	distrib_data->lower                    = lower;
	distrib_data->upper                    = upper;

	if (data_type == CCS_FLOAT) {
		if (quantization.f != 0.0)
			distrib_data->quantize = 1;
		if (scale_type == CCS_LOGARITHMIC) {
			distrib_data->internal_lower.f = log(lower.f);
			distrib_data->internal_upper.f = log(upper.f);
		} else {
			distrib_data->internal_lower.f = 0.0;
			distrib_data->internal_upper.f = upper.f - lower.f;
			if (distrib_data->quantize)
				distrib_data->internal_upper.f /= quantization.f;
		}
	} else {
		if (quantization.i != 0)
			distrib_data->quantize = 1;
		if (scale_type == CCS_LOGARITHMIC) {
			distrib_data->internal_lower.f = log(lower.i);
			distrib_data->internal_upper.f = log(upper.i);
		} else {
			distrib_data->internal_lower.i = 0;
			distrib_data->internal_upper.i = upper.i - lower.i;
			if (quantization.i != 0)
				distrib_data->internal_upper.i /= quantization.i;
		}
	}
	distrib->data = (_ccs_distribution_data_t *)distrib_data;
	*distribution_ret = distrib;
	return CCS_SUCCESS;
}

extern ccs_error_t
ccs_uniform_distribution_get_parameters(ccs_distribution_t  distribution,
                                        ccs_datum_t        *lower,
                                        ccs_datum_t        *upper) {
	if (!distribution || distribution->obj.type != CCS_DISTRIBUTION)
		return -CCS_INVALID_OBJECT;
	if (!distribution->data || ((_ccs_distribution_common_data_t*)distribution->data)->type != CCS_UNIFORM)
		return -CCS_INVALID_OBJECT;
	if (!lower && !upper)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_uniform_data_t * data = (_ccs_distribution_uniform_data_t *)distribution->data;

	if (lower) {
		lower->value = data->lower;
		lower->type  = data->common_data.data_type;
	}
	if (upper) {
		upper->value = data->upper;
		upper->type  = data->common_data.data_type;
	}
	return CCS_SUCCESS;
}

