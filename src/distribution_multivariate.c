#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>
#include <string.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"

struct _ccs_distribution_multivariate_data_s {
	_ccs_distribution_common_data_t  common_data;
	size_t                           num_distributions;
	ccs_distribution_t              *distributions;
	size_t                          *dimensions;
	ccs_interval_t                  *bounds;
};
typedef struct _ccs_distribution_multivariate_data_s _ccs_distribution_multivariate_data_t;

static ccs_result_t
_ccs_distribution_multivariate_del(ccs_object_t o) {
	struct _ccs_distribution_multivariate_data_s *data =
		(struct _ccs_distribution_multivariate_data_s *)(((ccs_distribution_t)o)->data);
	for (size_t i = 0; i < data->num_distributions; i++) {
		ccs_release_object(data->distributions[i]);
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_multivariate_get_bounds(_ccs_distribution_data_t *data,
                                          ccs_interval_t           *interval_ret);

static ccs_result_t
_ccs_distribution_multivariate_samples(_ccs_distribution_data_t *data,
                                       ccs_rng_t                 rng,
                                       size_t                    num_values,
                                       ccs_numeric_t            *values);

static ccs_result_t
_ccs_distribution_multivariate_strided_samples(_ccs_distribution_data_t *data,
                                               ccs_rng_t                 rng,
                                               size_t                    num_values,
                                               size_t                    stride,
                                               ccs_numeric_t            *values);

static ccs_result_t
_ccs_distribution_multivariate_soa_samples(_ccs_distribution_data_t  *data,
                                           ccs_rng_t                  rng,
                                           size_t                     num_values,
                                           ccs_numeric_t            **values);

static _ccs_distribution_ops_t _ccs_distribution_multivariate_ops = {
	{ &_ccs_distribution_multivariate_del },
	&_ccs_distribution_multivariate_samples,
	&_ccs_distribution_multivariate_get_bounds,
	&_ccs_distribution_multivariate_strided_samples,
	&_ccs_distribution_multivariate_soa_samples
};

ccs_result_t
ccs_create_multivariate_distribution(size_t              num_distributions,
                                     ccs_distribution_t *distributions,
                                     ccs_distribution_t *distribution_ret) {
	CCS_CHECK_ARY(num_distributions, distributions);
	CCS_CHECK_PTR(distribution_ret);
	if (!num_distributions || num_distributions > INT64_MAX)
		return -CCS_INVALID_VALUE;

	ccs_result_t err;
	size_t       i = 0;
	size_t       dimension = 0;
	ccs_distribution_t distrib;
	_ccs_distribution_multivariate_data_t *distrib_data;

        for (i = 0; i < num_distributions; i++) {
		size_t dim;
		CCS_VALIDATE(ccs_distribution_get_dimension(distributions[i], &dim));
		dimension += dim;
	}

	uintptr_t mem, cur_mem;
	mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_distribution_s) +
		sizeof(_ccs_distribution_multivariate_data_t) +
		sizeof(ccs_distribution_t)*num_distributions +
		sizeof(size_t)*num_distributions +
		sizeof(ccs_interval_t)*dimension +
		sizeof(ccs_numeric_type_t)*dimension);

	if (!mem)
		return -CCS_OUT_OF_MEMORY;
        cur_mem = mem;

	distrib = (ccs_distribution_t)cur_mem;
	cur_mem += sizeof(struct _ccs_distribution_s);
	_ccs_object_init(&(distrib->obj), CCS_DISTRIBUTION, (_ccs_object_ops_t *)&_ccs_distribution_multivariate_ops);
	distrib_data = (_ccs_distribution_multivariate_data_t *)(cur_mem);
	cur_mem += sizeof(_ccs_distribution_multivariate_data_t);
	distrib_data->common_data.type        = CCS_MULTIVARIATE;
	distrib_data->common_data.dimension   = dimension;
	distrib_data->num_distributions       = num_distributions;
	distrib_data->distributions           = (ccs_distribution_t *)(cur_mem);
	cur_mem += sizeof(ccs_distribution_t)*num_distributions;
	distrib_data->dimensions              = (size_t *)(cur_mem);
	cur_mem += sizeof(size_t)*num_distributions;
	distrib_data->bounds                  = (ccs_interval_t *)(cur_mem);
	cur_mem += sizeof(ccs_interval_t)*dimension;
	distrib_data->common_data.data_types  = (ccs_numeric_type_t *)(cur_mem);
	cur_mem += sizeof(ccs_numeric_type_t)*dimension;

	dimension = 0;
        for (i = 0; i < num_distributions; i++) {
		size_t dim;
		CCS_VALIDATE_ERR_GOTO(err,
		  ccs_distribution_get_data_types(distributions[i], distrib_data->common_data.data_types + dimension),
		  errmemory);
		CCS_VALIDATE_ERR_GOTO(err,
		  ccs_distribution_get_bounds(distributions[i], distrib_data->bounds + dimension),
		  errmemory);
		CCS_VALIDATE_ERR_GOTO(err, ccs_distribution_get_dimension(distributions[i], &dim), errmemory);
		distrib_data->dimensions[i] = dim;
		dimension += dim;
	}

	for (i = 0; i < num_distributions; i++) {
		CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(distributions[i]), distrib);
		distrib_data->distributions[i] = distributions[i];
	}
	distrib->data = (_ccs_distribution_data_t *)distrib_data;
	*distribution_ret = distrib;

	return CCS_SUCCESS;
distrib:
	for (i = 0; i < num_distributions; i++) {
		if (distrib_data->distributions[i])
			ccs_release_object(distributions[i]);
	}
errmemory:
	free((void *)mem);
	return err;
}

static ccs_result_t
_ccs_distribution_multivariate_get_bounds(_ccs_distribution_data_t *data,
                                     ccs_interval_t           *interval_ret) {
	_ccs_distribution_multivariate_data_t *d =
		(_ccs_distribution_multivariate_data_t *)data;
	memcpy(interval_ret, d->bounds, d->common_data.dimension*sizeof(ccs_interval_t));
	return CCS_SUCCESS;
}

static inline _ccs_distribution_ops_t *
ccs_distribution_get_ops(ccs_distribution_t distribution) {
	return (_ccs_distribution_ops_t *)distribution->obj.ops;
}

static ccs_result_t
_ccs_distribution_multivariate_samples(_ccs_distribution_data_t *data,
                                       ccs_rng_t                 rng,
                                       size_t                    num_values,
                                       ccs_numeric_t            *values) {
	_ccs_distribution_multivariate_data_t *d =
		(_ccs_distribution_multivariate_data_t *)data;

	for (size_t i = 0; i < d->num_distributions; i++) {
		CCS_VALIDATE(ccs_distribution_get_ops(d->distributions[i])->strided_samples(
			d->distributions[i]->data, rng, num_values,
			d->common_data.dimension, values));
		values += d->dimensions[i];
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_multivariate_strided_samples(_ccs_distribution_data_t *data,
                                               ccs_rng_t                 rng,
                                               size_t                    num_values,
                                               size_t                    stride,
                                               ccs_numeric_t            *values) {
	_ccs_distribution_multivariate_data_t *d =
		(_ccs_distribution_multivariate_data_t *)data;
	
	for (size_t i = 0; i < d->num_distributions; i++) {
		CCS_VALIDATE(ccs_distribution_get_ops(d->distributions[i])->strided_samples(
			d->distributions[i]->data, rng, num_values,
			stride, values));
		values += d->dimensions[i];
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_multivariate_soa_samples(_ccs_distribution_data_t  *data,
                                           ccs_rng_t                  rng,
                                           size_t                     num_values,
                                           ccs_numeric_t            **values) {
	_ccs_distribution_multivariate_data_t *d =
		(_ccs_distribution_multivariate_data_t *)data;
	
	for (size_t i = 0; i < d->num_distributions; i++) {
		CCS_VALIDATE(ccs_distribution_get_ops(d->distributions[i])->soa_samples(
			d->distributions[i]->data, rng, num_values, values));
		values += d->dimensions[i];
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_multivariate_distribution_get_num_distributions(ccs_distribution_t  distribution,
                                               size_t             *num_distributions_ret) {
	CCS_CHECK_DISTRIBUTION(distribution, CCS_MULTIVARIATE);
	CCS_CHECK_PTR(num_distributions_ret);
	_ccs_distribution_multivariate_data_t * data = (_ccs_distribution_multivariate_data_t *)distribution->data;
	*num_distributions_ret = data->num_distributions;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_multivariate_distribution_get_distributions(ccs_distribution_t  distribution,
                                           size_t              num_distributions,
                                           ccs_distribution_t *distributions,
                                           size_t             *num_distributions_ret) {
	CCS_CHECK_DISTRIBUTION(distribution, CCS_MULTIVARIATE);
	CCS_CHECK_ARY(num_distributions, distributions);
	if (!distributions && !num_distributions_ret)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_multivariate_data_t * data = (_ccs_distribution_multivariate_data_t *)distribution->data;
	if (distributions) {
		if (num_distributions < data->num_distributions)
			return -CCS_INVALID_VALUE;
		for (size_t i = 0; i < data->num_distributions; i++)
			distributions[i] = data->distributions[i];
		for (size_t i = data->num_distributions; i < num_distributions; i++)
			distributions[i] = NULL;
	}
	if (num_distributions_ret)
		*num_distributions_ret = data->num_distributions;
	return CCS_SUCCESS;
}

