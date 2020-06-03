#include "cconfigspace_internal.h"
#include "configuration_space_internal.h"
#include "configuration_internal.h"

static inline _ccs_configuration_space_ops_t *
ccs_configuration_space_get_ops(ccs_configuration_space_t configuration_space) {
	return (_ccs_configuration_space_ops_t *)configuration_space->obj.ops;
}

static ccs_error_t
_ccs_configuration_space_del(ccs_object_t object) {
	ccs_configuration_space_t configuration_space = (ccs_configuration_space_t)object;
	UT_array *array = configuration_space->data->hyperparameters;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		ccs_release_object(wrapper->hyperparameter);
		if (wrapper->condition)
			ccs_release_object(wrapper->condition);
	}
	array = configuration_space->data->forbidden_clauses;
	ccs_expression_t *expr = NULL;
	while ( (expr = (ccs_expression_t *)utarray_next(array, expr)) ) {
		ccs_release_object(*expr);
	}
	HASH_CLEAR(hh_name, configuration_space->data->name_hash);
	HASH_CLEAR(hh_handle, configuration_space->data->handle_hash);
	utarray_free(configuration_space->data->hyperparameters);
	utarray_free(configuration_space->data->forbidden_clauses);
	_ccs_distribution_wrapper_t *dw;
	_ccs_distribution_wrapper_t *tmp;
	DL_FOREACH_SAFE(configuration_space->data->distribution_list, dw, tmp) {
		DL_DELETE(configuration_space->data->distribution_list, dw);
		ccs_release_object(dw->distribution);
		free(dw);
	}
	ccs_release_object(configuration_space->data->rng);
	return CCS_SUCCESS;
}

static _ccs_configuration_space_ops_t _configuration_space_ops =
    { {&_ccs_configuration_space_del} };

static const UT_icd _hyperparameter_wrapper_icd = {
	sizeof(_ccs_hyperparameter_wrapper_t),
	NULL,
	NULL,
	NULL,
};

static const UT_icd _forbidden_clauses_icd = {
	sizeof(ccs_expression_t),
	NULL,
	NULL,
	NULL,
};

#undef  utarray_oom
#define utarray_oom() { \
	ccs_release_object(config_space->data->rng); \
	err = -CCS_ENOMEM; \
	goto arrays; \
}
ccs_error_t
ccs_create_configuration_space(const char                *name,
                               void                      *user_data,
                               ccs_configuration_space_t *configuration_space_ret) {
	if (!name || !configuration_space_ret)
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_configuration_space_s) + sizeof(struct _ccs_configuration_space_data_s) + strlen(name) + 1);
	if (!mem)
		return -CCS_ENOMEM;
	ccs_rng_t rng;
	ccs_error_t err = ccs_rng_create(&rng);
	if (err) {
		free((void *)mem);
		return err;
	}

	ccs_configuration_space_t config_space = (ccs_configuration_space_t)mem;
	_ccs_object_init(&(config_space->obj), CCS_CONFIGURATION_SPACE, (_ccs_object_ops_t *)&_configuration_space_ops);
	config_space->data = (struct _ccs_configuration_space_data_s*)(mem + sizeof(struct _ccs_configuration_space_s));
	config_space->data->name = (const char *)(mem + sizeof(struct _ccs_configuration_space_s) + sizeof(struct _ccs_configuration_space_data_s));
	config_space->data->user_data = user_data;
	config_space->data->rng = rng;
	config_space->data->hyperparameters = NULL;
	config_space->data->forbidden_clauses = NULL;
	utarray_new(config_space->data->hyperparameters, &_hyperparameter_wrapper_icd);
	utarray_new(config_space->data->forbidden_clauses, &_forbidden_clauses_icd);
	config_space->data->name_hash = NULL;
	config_space->data->distribution_list = NULL;
	strcpy((char *)(config_space->data->name), name);
	*configuration_space_ret = config_space;
	return CCS_SUCCESS;
arrays:
	if(config_space->data->hyperparameters)
		utarray_free(config_space->data->hyperparameters);
	if(config_space->data->forbidden_clauses)
		utarray_free(config_space->data->forbidden_clauses);
	free((void *)mem);
	return err;
}

ccs_error_t
ccs_configuration_space_get_name(ccs_configuration_space_t   configuration_space,
                                 const char                **name_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!name_ret)
		return -CCS_INVALID_VALUE;
	*name_ret = configuration_space->data->name;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_user_data(ccs_configuration_space_t   configuration_space,
                                      void                      **user_data_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!user_data_ret)
		return -CCS_INVALID_VALUE;
	*user_data_ret = configuration_space->data->user_data;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_set_rng(ccs_configuration_space_t configuration_space,
                                ccs_rng_t                 rng) {
	if (!configuration_space || !configuration_space->data || !rng)
		return -CCS_INVALID_OBJECT;
	ccs_error_t err;
	err = ccs_release_object(configuration_space->data->rng);
	if(err)
		return err;
	configuration_space->data->rng = rng;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_rng(ccs_configuration_space_t  configuration_space,
                                ccs_rng_t                 *rng_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!rng_ret)
		return -CCS_INVALID_VALUE;
	*rng_ret = configuration_space->data->rng;
	return CCS_SUCCESS;
}


#undef  utarray_oom
#define utarray_oom() { \
	err = -CCS_ENOMEM; \
	goto errordistrib_wrapper; \
}
#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	err = -CCS_ENOMEM; \
	goto errorutarray; \
}
ccs_error_t
ccs_configuration_space_add_hyperparameter(ccs_configuration_space_t configuration_space,
                                           ccs_hyperparameter_t      hyperparameter,
                                           ccs_distribution_t        distribution) {
	if (!configuration_space || !configuration_space->data || !hyperparameter)
		return -CCS_INVALID_OBJECT;
	ccs_error_t err;
	const char *name;
	size_t sz_name;
	_ccs_hyperparameter_wrapper_t *p_hyper_wrapper;
	err = ccs_hyperparameter_get_name(hyperparameter, &name);
	if (err)
		goto error;
	sz_name = strlen(name);
	HASH_FIND(hh_name, configuration_space->data->name_hash,
	          name, sz_name, p_hyper_wrapper);
	if (p_hyper_wrapper) {
		err = -CCS_INVALID_HYPERPARAMETER;
		goto error;
	}
	UT_array *hyperparameters;
	unsigned int index;
	size_t dimension;
	_ccs_hyperparameter_wrapper_t hyper_wrapper;
	_ccs_distribution_wrapper_t *distrib_wrapper;
	uintptr_t pmem;
	hyper_wrapper.hyperparameter = hyperparameter;
	err = ccs_retain_object(hyperparameter);
	if (err)
		goto error;

	if (distribution) {
		err = ccs_distribution_get_dimension(distribution, &dimension);
		if (err)
			goto errorhyper;
		if (dimension != 1) {
			err = -CCS_INVALID_DISTRIBUTION;
			goto errorhyper;
		}
		err = ccs_retain_object(distribution);
		if (err)
			goto errorhyper;
	} else {
		err = ccs_hyperparameter_get_default_distribution(hyperparameter, &distribution);
		if (err)
			goto errorhyper;
		dimension = 1;
	}
	pmem = (uintptr_t)malloc(sizeof(_ccs_distribution_wrapper_t) + sizeof(size_t)*dimension);
	if (!pmem) {
		err = -CCS_ENOMEM;
		goto errordistrib;
	}
        distrib_wrapper = (_ccs_distribution_wrapper_t *)pmem;
	distrib_wrapper->distribution = distribution;
	distrib_wrapper->dimension = dimension;
	distrib_wrapper->hyperparameter_indexes = (size_t *)(pmem + sizeof(_ccs_distribution_wrapper_t));
	hyperparameters = configuration_space->data->hyperparameters;
	index = utarray_len(hyperparameters);
	distrib_wrapper->hyperparameter_indexes[0] = index;
	hyper_wrapper.index = index;
	hyper_wrapper.distribution_index = 0;
	hyper_wrapper.distribution = distrib_wrapper;
	hyper_wrapper.name = name;
	hyper_wrapper.condition = NULL;
	utarray_push_back(hyperparameters, &hyper_wrapper);

	p_hyper_wrapper =
	   (_ccs_hyperparameter_wrapper_t*)utarray_eltptr(hyperparameters, index);
	HASH_ADD_KEYPTR( hh_name, configuration_space->data->name_hash,
	                 name, sz_name, p_hyper_wrapper );
	HASH_ADD( hh_handle, configuration_space->data->handle_hash,
	          hyperparameter, sizeof(ccs_hyperparameter_t), p_hyper_wrapper );
	DL_APPEND( configuration_space->data->distribution_list, distrib_wrapper );

	return CCS_SUCCESS;
errorutarray:
	utarray_pop_back(hyperparameters);
errordistrib_wrapper:
	free(distrib_wrapper);
errordistrib:
	ccs_release_object(distribution);
errorhyper:
	ccs_release_object(hyperparameter);
error:
	return err;
}
#undef  utarray_oom
#define utarray_oom() exit(-1)

ccs_error_t
ccs_configuration_space_add_hyperparameters(ccs_configuration_space_t  configuration_space,
                                            size_t                     num_hyperparameters,
                                            ccs_hyperparameter_t      *hyperparameters,
                                            ccs_distribution_t        *distributions) {
	if (!configuration_space)
		return -CCS_INVALID_OBJECT;
	if (num_hyperparameters > 0 && !hyperparameters)
		return -CCS_INVALID_VALUE;
	for (size_t i = 0; i < num_hyperparameters; i++) {
		ccs_distribution_t distribution = NULL;
		if (distributions)
			distribution = distributions[i];
		ccs_error_t err =
		    ccs_configuration_space_add_hyperparameter( configuration_space,
		                                                hyperparameters[i],
		                                                distribution );
		if (err)
			return err;
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_num_hyperparameters(ccs_configuration_space_t  configuration_space,
                                                size_t                     *num_hyperparameters_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!num_hyperparameters_ret)
		return -CCS_INVALID_VALUE;
	*num_hyperparameters_ret = utarray_len(configuration_space->data->hyperparameters);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_hyperparameter(ccs_configuration_space_t  configuration_space,
                                           size_t                     index,
                                           ccs_hyperparameter_t      *hyperparameter_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!hyperparameter_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper = (_ccs_hyperparameter_wrapper_t*)
	    utarray_eltptr(configuration_space->data->hyperparameters, (unsigned int)index);
	if (!wrapper)
		return -CCS_OUT_OF_BOUNDS;
	*hyperparameter_ret = wrapper->hyperparameter;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_hyperparameter_by_name(
		ccs_configuration_space_t  configuration_space,
		const char *               name,
		ccs_hyperparameter_t      *hyperparameter_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!hyperparameter_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper;
	size_t sz_name;
	sz_name = strlen(name);
	HASH_FIND(hh_name, configuration_space->data->name_hash,
	          name, sz_name, wrapper);
	if (!wrapper)
		return -CCS_INVALID_NAME;
	*hyperparameter_ret = wrapper->hyperparameter;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_hyperparameter_index_by_name(
		ccs_configuration_space_t  configuration_space,
		const char                *name,
		size_t                    *index_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!index_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper;
	size_t sz_name;
	sz_name = strlen(name);
	HASH_FIND(hh_name, configuration_space->data->name_hash,
	          name, sz_name, wrapper);
	if (!wrapper)
		return -CCS_INVALID_NAME;
	*index_ret = wrapper->index;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_hyperparameter_index(
		ccs_configuration_space_t  configuration_space,
		ccs_hyperparameter_t       hyperparameter,
		size_t                    *index_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!hyperparameter)
		return -CCS_INVALID_HYPERPARAMETER;
	if (!index_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper;
	HASH_FIND(hh_handle, configuration_space->data->handle_hash,
	          &hyperparameter, sizeof(ccs_hyperparameter_t), wrapper);
	if (!wrapper)
		return -CCS_INVALID_HYPERPARAMETER;
	*index_ret = wrapper->index;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_hyperparameters(ccs_configuration_space_t  configuration_space,
                                            size_t                     num_hyperparameters
,
                                            ccs_hyperparameter_t      *hyperparameters,
                                            size_t                    *num_hyperparameters_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_hyperparameters && !hyperparameters)
		return -CCS_INVALID_VALUE;
	if (hyperparameters && !num_hyperparameters)
		return -CCS_INVALID_VALUE;
	if (!num_hyperparameters_ret && !hyperparameters)
		return -CCS_INVALID_VALUE;
	UT_array *array = configuration_space->data->hyperparameters;
	size_t size = utarray_len(array);
	if (hyperparameters) {
		if (num_hyperparameters < size)
			return -CCS_INVALID_VALUE;
		_ccs_hyperparameter_wrapper_t *wrapper = NULL;
		size_t index = 0;
		while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) )
			hyperparameters[index++] = wrapper->hyperparameter;
		for (size_t i = size; i < num_hyperparameters; i++)
			hyperparameters[i] = NULL;
	}
	if (num_hyperparameters_ret)
		*num_hyperparameters_ret = size;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_default_configuration(ccs_configuration_space_t  configuration_space,
                                                  ccs_configuration_t       *configuration_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!configuration_ret)
		return -CCS_INVALID_VALUE;
	ccs_error_t err;
	ccs_configuration_t config;
	err = ccs_create_configuration(configuration_space, 0, NULL, NULL, &config);
	if (err)
		return err;
	UT_array *array = configuration_space->data->hyperparameters;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	ccs_datum_t *values = config->data->values;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		err = ccs_hyperparameter_get_default_value(wrapper->hyperparameter,
		                                           values++);
		if (unlikely(err)) {
			ccs_release_object(config);
			return err;
		}
	}
	*configuration_ret = config;
	return CCS_SUCCESS;
}

static inline ccs_error_t
 _check_configuration(UT_array    *array,
                      size_t       num_values,
                      ccs_datum_t *values) {
	if (num_values != utarray_len(array))
		return -CCS_INVALID_CONFIGURATION;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	size_t index = 0;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		ccs_bool_t res;
		ccs_error_t err;
		err = ccs_hyperparameter_check_value(wrapper->hyperparameter,
		                                     values[index++], &res);
		if (unlikely(err))
			return err;
		if (res == CCS_FALSE)
			return -CCS_INVALID_CONFIGURATION;
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_check_configuration(ccs_configuration_space_t configuration_space,
                                            ccs_configuration_t       configuration) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!configuration || !configuration->data)
		return -CCS_INVALID_OBJECT;
	if (configuration->data->configuration_space != configuration_space)
		return -CCS_INVALID_CONFIGURATION;
	return _check_configuration(configuration_space->data->hyperparameters,
	                            configuration->data->num_values,
	                            configuration->data->values);
}

ccs_error_t
ccs_configuration_space_check_configuration_values(ccs_configuration_space_t  configuration_space,
                                                   size_t                     num_values,
                                                   ccs_datum_t               *values) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!values)
		return -CCS_INVALID_VALUE;
	return _check_configuration(configuration_space->data->hyperparameters,
	                            num_values, values);
}


// This is temporary until I figure out how correlated sampling should work
ccs_error_t
ccs_configuration_space_sample(ccs_configuration_space_t  configuration_space,
                               ccs_configuration_t       *configuration_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!configuration_ret)
		return -CCS_INVALID_VALUE;
	ccs_error_t err;
	ccs_configuration_t config;
	err = ccs_create_configuration(configuration_space, 0, NULL, NULL, &config);
	if (err)
		return err;
	ccs_rng_t rng = configuration_space->data->rng;
	UT_array *array = configuration_space->data->hyperparameters;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	ccs_datum_t *values = config->data->values;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		err = ccs_hyperparameter_sample(wrapper->hyperparameter,
		                                wrapper->distribution->distribution,
		                                rng, values++);
		if (unlikely(err)) {
			ccs_release_object(config);
			return err;
		}
	}
	*configuration_ret = config;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_samples(ccs_configuration_space_t  configuration_space,
                                size_t                     num_configurations,
                                ccs_configuration_t       *configurations) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_configurations && !configurations)
		return -CCS_INVALID_VALUE;
	if (!num_configurations)
		return CCS_SUCCESS;
	ccs_error_t err;
	UT_array *array = configuration_space->data->hyperparameters;
	size_t num_hyper = utarray_len(array);
	ccs_datum_t *values = (ccs_datum_t *)calloc(1, sizeof(ccs_datum_t)*num_configurations*num_hyper);
	ccs_datum_t *p_values = values;
	ccs_rng_t rng = configuration_space->data->rng;
	_ccs_hyperparameter_wrapper_t *wrapper = NULL;
	while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) ) {
		err = ccs_hyperparameter_samples(wrapper->hyperparameter,
		                                 wrapper->distribution->distribution,
						 rng, num_configurations, p_values);
		if (unlikely(err)) {
			free(values);
			return err;
		}
		p_values += num_configurations;
	}
	size_t i;
	for(i = 0; i < num_configurations; i++) {
		err = ccs_create_configuration(configuration_space, 0, NULL, NULL, configurations + i);
		if (unlikely(err)) {
			free(values);
			for(size_t j = 0; j < i; j++) {
				ccs_release_object(configurations + j);
			}
			return err;
		}
	}
	for(i = 0; i < num_configurations; i++)
		for(size_t j = 0; j < num_hyper; j++)
			configurations[i]->data->values[j] =
				values[j*num_configurations + i];
	free(values);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_set_condition(ccs_configuration_space_t configuration_space,
                                      size_t                    hyperparameter_index,
                                      ccs_expression_t          expression) {
	if (!configuration_space || !configuration_space->data || !expression)
		return -CCS_INVALID_OBJECT;
	_ccs_hyperparameter_wrapper_t *wrapper = (_ccs_hyperparameter_wrapper_t*)
	    utarray_eltptr(configuration_space->data->hyperparameters,
	                   (unsigned int)hyperparameter_index);
	if (!wrapper)
		return -CCS_OUT_OF_BOUNDS;
	if (wrapper->condition)
		return -CCS_INVALID_HYPERPARAMETER;
	ccs_error_t err = ccs_retain_object(expression);
	if (err)
		return err;
	wrapper->condition = expression;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_condition(ccs_configuration_space_t  configuration_space,
                                      size_t                     hyperparameter_index,
                                      ccs_expression_t          *expression_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!expression_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_wrapper_t *wrapper = (_ccs_hyperparameter_wrapper_t*)
	    utarray_eltptr(configuration_space->data->hyperparameters,
	                   (unsigned int)hyperparameter_index);
	if (!wrapper)
		return -CCS_OUT_OF_BOUNDS;
	*expression_ret = wrapper->condition;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_conditions(ccs_configuration_space_t  configuration_space,
                                       size_t                     num_expressions,
                                       ccs_expression_t          *expressions,
                                       size_t                    *num_expressions_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_expressions && !expressions)
		return -CCS_INVALID_VALUE;
	if (!expressions && !num_expressions_ret)
		return -CCS_INVALID_VALUE;
	UT_array *array = configuration_space->data->hyperparameters;
	size_t size = utarray_len(array);
	if (expressions) {
		if (num_expressions < size)
			return -CCS_INVALID_VALUE;
		_ccs_hyperparameter_wrapper_t *wrapper = NULL;
		size_t index = 0;
		while ( (wrapper = (_ccs_hyperparameter_wrapper_t *)utarray_next(array, wrapper)) )
			expressions[index++] = wrapper->condition;
		for (size_t i = size; i < num_expressions; i++)
			expressions[i] = NULL;
	}
	if (num_expressions_ret)
		*num_expressions_ret = size;
	return CCS_SUCCESS;
}

#undef  utarray_oom
#define utarray_oom() { \
	return -CCS_ENOMEM; \
}
ccs_error_t
ccs_configuration_space_add_forbidden_clause(ccs_configuration_space_t configuration_space,
                                             ccs_expression_t          expression) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	ccs_error_t err = ccs_retain_object(expression);
	if (err)
		return err;
	utarray_push_back(configuration_space->data->forbidden_clauses, &expression);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_add_forbidden_clauses(ccs_configuration_space_t  configuration_space,
                                              size_t                     num_expressions,
                                              ccs_expression_t          *expressions) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_expressions && !expressions)
		return -CCS_INVALID_VALUE;
	for (size_t i = 0; i < num_expressions; i++) {
		ccs_error_t err = ccs_retain_object(expressions[i]);
		if (err)
			return err;
		utarray_push_back(configuration_space->data->forbidden_clauses, expressions + i);
	}
	return CCS_SUCCESS;
}
#undef  utarray_oom
#define utarray_oom() exit(-1)

ccs_error_t
ccs_configuration_space_get_forbidden_clause(ccs_configuration_space_t  configuration_space,
                                             size_t                     index,
                                             ccs_expression_t          *expression_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (!expression_ret)
		return -CCS_INVALID_VALUE;
	ccs_expression_t *p_expr = (ccs_expression_t*)
	    utarray_eltptr(configuration_space->data->forbidden_clauses,
	                   (unsigned int)index);
	if (!p_expr)
		return -CCS_OUT_OF_BOUNDS;
	*expression_ret = *p_expr;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_space_get_forbidden_clauses(ccs_configuration_space_t  configuration_space,
                                              size_t                     num_expressions,
                                              ccs_expression_t          *expressions,
                                              size_t                    *num_expressions_ret) {
	if (!configuration_space || !configuration_space->data)
		return -CCS_INVALID_OBJECT;
	if (num_expressions && !expressions)
		return -CCS_INVALID_VALUE;
	if (!expressions && !num_expressions_ret)
		return -CCS_INVALID_VALUE;
	UT_array *array = configuration_space->data->forbidden_clauses;
	size_t size = utarray_len(array);
	if (expressions) {
		if (num_expressions < size)
			return -CCS_INVALID_VALUE;
		ccs_expression_t *p_expr = NULL;
		size_t index = 0;
		while ( (p_expr = (ccs_expression_t *)utarray_next(array, p_expr)) )
			expressions[index++] = *p_expr;
		for (size_t i = size; i < num_expressions; i++)
			expressions[i] = NULL;
	}
	if (num_expressions_ret)
		*num_expressions_ret = size;
	return CCS_SUCCESS;
}
