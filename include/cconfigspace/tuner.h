#ifndef _CCS_TUNER_H
#define _CCS_TUNER_H

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_error_t
ccs_tuner_get_name(ccs_tuner_t   tuner,
                   const char  **name_ret);

extern ccs_error_t
ccs_tuner_get_user_data(ccs_tuner_t   tuner,
                        void        **user_data_ret);

extern ccs_error_t
ccs_tuner_get_configuration_space(ccs_tuner_t                tuner,
                                  ccs_configuration_space_t *configuration_space_ret);

extern ccs_error_t
ccs_tuner_get_objective_spce(ccs_tuner_t            tuner,
                             ccs_objective_space_t *objective_space_ret);

extern ccs_error_t
ccs_tuner_ask(ccs_tuner_t          tuner,
              size_t               num_configurations,
              ccs_configuration_t *configurations,
              size_t              *num_configurations_ret);

extern ccs_error_t
ccs_tuner_tell(ccs_tuner_t       tuner,
               size_t            num_evaluations,
               ccs_evaluation_t *evaluations);

extern ccs_error_t
ccs_tuner_get_optimums(ccs_tuner_t       tuner,
                       size_t            num_evaluations,
                       ccs_evaluation_t *evaluations,
                       size_t           *num_evaluations_ret);

extern ccs_error_t
ccs_tuner_get_history(ccs_tuner_t       tuner,
                      size_t            num_evaluations,
                      ccs_evaluation_t *evaluations,
                      size_t           *num_evaluations_ret);

extern ccs_error_t
ccs_create_random_tuner(const char                *name,
                        ccs_configuration_space_t  configuration_space,
                        ccs_objective_space_t      objective_space,
                        void                      *user_data,
                        ccs_tuner_t               *tuner_ret);
#ifdef __cplusplus
}
#endif

#endif //_CCS_TUNER_H