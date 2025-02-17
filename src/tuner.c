#include "cconfigspace_internal.h"
#include "tuner_internal.h"

static inline _ccs_tuner_ops_t *
ccs_tuner_get_ops(ccs_tuner_t tuner) {
	return (_ccs_tuner_ops_t *)tuner->obj.ops;
}

ccs_result_t
ccs_tuner_get_type(ccs_tuner_t       tuner,
                   ccs_tuner_type_t *type_ret) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	CCS_CHECK_PTR(type_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*type_ret = d->type;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_tuner_get_name(ccs_tuner_t   tuner,
                   const char  **name_ret) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	CCS_CHECK_PTR(name_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*name_ret = d->name;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_tuner_get_user_data(ccs_tuner_t   tuner,
                        void        **user_data_ret) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	CCS_CHECK_PTR(user_data_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*user_data_ret = d->user_data;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_tuner_get_configuration_space(ccs_tuner_t                tuner,
                                  ccs_configuration_space_t *configuration_space_ret) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	CCS_CHECK_PTR(configuration_space_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*configuration_space_ret = d->configuration_space;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_tuner_get_objective_space(ccs_tuner_t            tuner,
                              ccs_objective_space_t *objective_space_ret) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	CCS_CHECK_PTR(objective_space_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*objective_space_ret = d->objective_space;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_tuner_ask(ccs_tuner_t          tuner,
              size_t               num_configurations,
              ccs_configuration_t *configurations,
              size_t              *num_configurations_ret) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	CCS_CHECK_ARY(num_configurations, configurations);
	if (!configurations && !num_configurations_ret)
		return -CCS_INVALID_VALUE;
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	return ops->ask(tuner->data, num_configurations, configurations, num_configurations_ret);
}

ccs_result_t
ccs_tuner_tell(ccs_tuner_t       tuner,
               size_t            num_evaluations,
               ccs_evaluation_t *evaluations) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	CCS_CHECK_ARY(num_evaluations, evaluations);
        /* TODO: check that evaluations have the same objective and
         * configuration sapce than the tuner */
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	return ops->tell(tuner->data, num_evaluations, evaluations);
}

ccs_result_t
ccs_tuner_get_optimums(ccs_tuner_t       tuner,
                       size_t            num_evaluations,
                       ccs_evaluation_t *evaluations,
                       size_t           *num_evaluations_ret) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	if (!evaluations && !num_evaluations_ret)
		return -CCS_INVALID_VALUE;
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	return ops->get_optimums(tuner->data, num_evaluations, evaluations, num_evaluations_ret);
}

ccs_result_t
ccs_tuner_get_history(ccs_tuner_t       tuner,
                      size_t            num_evaluations,
                      ccs_evaluation_t *evaluations,
                      size_t           *num_evaluations_ret) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	if (!evaluations && !num_evaluations_ret)
		return -CCS_INVALID_VALUE;
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	return ops->get_history(tuner->data, num_evaluations, evaluations, num_evaluations_ret);
}

ccs_result_t
ccs_tuner_suggest(ccs_tuner_t          tuner,
                  ccs_configuration_t *configuration) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	if (!ops->suggest)
		return -CCS_UNSUPPORTED_OPERATION;
	CCS_CHECK_PTR(configuration);
	return ops->suggest(tuner->data, configuration);
}
