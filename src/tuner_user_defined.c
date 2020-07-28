#include "cconfigspace_internal.h"
#include "tuner_internal.h"
#include "string.h"

static ccs_result_t
_ccs_tuner_user_defined_del(ccs_object_t o) {
	ccs_user_defined_tuner_data_t *d =
		(ccs_user_defined_tuner_data_t *)((ccs_tuner_t)o)->data;
	ccs_result_t err;
	err = d->vector.del(d);
	ccs_release_object(d->common_data.configuration_space);
	ccs_release_object(d->common_data.objective_space);
	return err;
}

static ccs_result_t
_ccs_tuner_user_defined_ask(_ccs_tuner_data_t   *data,
                            size_t               num_configurations,
                            ccs_configuration_t *configurations,
                            size_t              *num_configurations_ret) {
	ccs_user_defined_tuner_data_t *d =
		(ccs_user_defined_tuner_data_t *)data;
	return d->vector.ask(d, num_configurations, configurations, num_configurations_ret);
}

static ccs_result_t
_ccs_tuner_user_defined_tell(_ccs_tuner_data_t *data,
                             size_t             num_evaluations,
                             ccs_evaluation_t  *evaluations) {
	ccs_user_defined_tuner_data_t *d =
		(ccs_user_defined_tuner_data_t *)data;
	return d->vector.tell(d, num_evaluations, evaluations);
}

static ccs_result_t
_ccs_tuner_user_defined_get_optimums(_ccs_tuner_data_t *data,
                                     size_t             num_evaluations,
                                     ccs_evaluation_t  *evaluations,
                                     size_t            *num_evaluations_ret) {
	ccs_user_defined_tuner_data_t *d =
		(ccs_user_defined_tuner_data_t *)data;
	return d->vector.get_optimums(d, num_evaluations, evaluations, num_evaluations_ret);
}

static ccs_result_t
_ccs_tuner_user_defined_get_history(_ccs_tuner_data_t *data,
                                    size_t             num_evaluations,
                                    ccs_evaluation_t  *evaluations,
                                    size_t            *num_evaluations_ret) {
	ccs_user_defined_tuner_data_t *d =
		(ccs_user_defined_tuner_data_t *)data;
	return d->vector.get_history(d, num_evaluations, evaluations, num_evaluations_ret);
}

static _ccs_tuner_ops_t _ccs_tuner_user_defined_ops = {
	{ &_ccs_tuner_user_defined_del },
	&_ccs_tuner_user_defined_ask,
	&_ccs_tuner_user_defined_tell,
	&_ccs_tuner_user_defined_get_optimums,
	&_ccs_tuner_user_defined_get_history
};

extern ccs_result_t
ccs_create_user_defined_tuner(const char                      *name,
                              ccs_configuration_space_t        configuration_space,
                              ccs_objective_space_t            objective_space,
                              void                            *user_data,
                              ccs_user_defined_tuner_vector_t *vector,
                              void                            *tuner_data,
                              ccs_tuner_t                     *tuner_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_PTR(tuner_ret);
	CCS_CHECK_PTR(vector);
	CCS_CHECK_PTR(vector->del);
	CCS_CHECK_PTR(vector->ask);
	CCS_CHECK_PTR(vector->tell);
	CCS_CHECK_PTR(vector->get_optimums);
	CCS_CHECK_PTR(vector->get_history);

	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_tuner_s) +
		sizeof(struct ccs_user_defined_tuner_data_s) +
		strlen(name) + 1);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	ccs_tuner_t tun;
	ccs_user_defined_tuner_data_t * data;
	ccs_result_t err;
	err = ccs_retain_object(configuration_space);
	if (err)
		goto errmem;
	err = ccs_retain_object(objective_space);
	if (err)
		goto errcs;

	tun = (ccs_tuner_t)mem;
	_ccs_object_init(&(tun->obj), CCS_TUNER, (_ccs_object_ops_t *)&_ccs_tuner_user_defined_ops);
	tun->data = (struct _ccs_tuner_data_s *)(mem + sizeof(struct _ccs_tuner_s));
	data = (ccs_user_defined_tuner_data_t *)tun->data;
	data->common_data.type = CCS_TUNER_USER_DEFINED;
	data->common_data.name = (const char *)(mem +
		sizeof(struct _ccs_tuner_s) +
		sizeof(struct ccs_user_defined_tuner_data_s));
	data->common_data.user_data = user_data;
	data->common_data.configuration_space = configuration_space;
	data->common_data.objective_space = objective_space;
	data->vector = *vector;
	data->tuner_data = tuner_data;
	strcpy((char*)data->common_data.name, name);
	*tuner_ret = tun;
	return CCS_SUCCESS;
errcs:
	ccs_release_object(configuration_space);
errmem:
	free((void *)mem);
	return err;
}


