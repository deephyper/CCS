#ifndef _CCS_HYPERPARAMETER_H
#define _CCS_HYPERPARAMETER_H

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_hyperparameter_type_e {
	CCS_HYPERPARAMETER_TYPE_NUMERICAL,
	CCS_HYPERPARAMETER_TYPE_CATEGORICAL,
	CCS_HYPERPARAMETER_TYPE_ORDINAL,
	CCS_HYPERPARAMETER_TYPE_DISCRETE,
	CCS_HYPERPARAMETER_TYPE_STRING,
	CCS_HYPERPARAMETER_TYPE_MAX,
	CCS_HYPERPARAMETER_TYPE_FORCE_32BIT = INT_MAX
};

typedef enum ccs_hyperparameter_type_e ccs_hyperparameter_type_t;

// Hyperparameter Interface
extern ccs_result_t
ccs_create_numerical_hyperparameter(const char           *name,
                                    ccs_numeric_type_t    data_type,
                                    ccs_numeric_t         lower,
                                    ccs_numeric_t         upper,
                                    ccs_numeric_t         quantization,
                                    ccs_numeric_t         default_value,
                                    void                 *user_data,
                                    ccs_hyperparameter_t *hyperparameter_ret);

extern ccs_result_t
ccs_numerical_hyperparameter_get_parameters(ccs_hyperparameter_t  hyperparameter,
                                            ccs_numeric_type_t   *data_type_ret,
                                            ccs_numeric_t        *lower_ret,
                                            ccs_numeric_t        *upper_ret,
                                            ccs_numeric_t        *quantization_ret);

extern ccs_result_t
ccs_create_categorical_hyperparameter(const char           *name,
                                      size_t                num_possible_values,
                                      ccs_datum_t          *possible_values,
                                      size_t                default_value_index,
                                      void                 *user_data,
                                      ccs_hyperparameter_t *hyperparameter_ret);

extern ccs_result_t
ccs_categorical_hyperparameter_get_values(ccs_hyperparameter_t  hyperparameter,
                                          size_t                num_possible_values,
                                          ccs_datum_t          *possible_values,
                                          size_t               *num_possible_values_ret);

extern ccs_result_t
ccs_create_ordinal_hyperparameter(const char           *name,
                                  size_t                num_possible_values,
                                  ccs_datum_t          *possible_values,
                                  size_t                default_value_index,
                                  void                 *user_data,
                                  ccs_hyperparameter_t *hyperparameter_ret);

extern ccs_result_t
ccs_ordinal_hyperparameter_get_values(ccs_hyperparameter_t  hyperparameter,
                                      size_t                num_possible_values,
                                      ccs_datum_t          *possible_values,
                                      size_t               *num_possible_values_ret);

extern ccs_result_t
ccs_ordinal_hyperparameter_compare_values(ccs_hyperparameter_t  hyperparameter,
                                          ccs_datum_t           value1,
                                          ccs_datum_t           value2,
                                          ccs_int_t            *comp_ret);

/* Ordered collection of numeric values */
extern ccs_result_t
ccs_create_discrete_hyperparameter(const char           *name,
                                   size_t                num_possible_values,
                                   ccs_datum_t          *possible_values,
                                   size_t                default_value_index,
                                   void                 *user_data,
                                   ccs_hyperparameter_t *hyperparameter_ret);

extern ccs_result_t
ccs_discrete_hyperparameter_get_values(ccs_hyperparameter_t  hyperparameter,
                                       size_t                num_possible_values,
                                       ccs_datum_t          *possible_values,
                                       size_t               *num_possible_values_ret);

/* An hyperparameter to represent an undetermined string, to be used within
   feature space. Cannot be sampled and thus doesn't have a default value.
   Checks will always return valid unless the value is not a string. */

extern ccs_result_t
ccs_create_string_hyperparameter(const char           *name,
                                 void                 *user_data,
                                 ccs_hyperparameter_t *hyperparameter_ret);

//   Accessors

extern ccs_result_t
ccs_hyperparameter_get_type(ccs_hyperparameter_t       hyperparameter,
                            ccs_hyperparameter_type_t *type_ret);

extern ccs_result_t
ccs_hyperparameter_get_default_value(ccs_hyperparameter_t  hyperparameter,
                                     ccs_datum_t          *value_ret);

extern ccs_result_t
ccs_hyperparameter_get_name(ccs_hyperparameter_t   hyperparameter,
                            const char           **name_ret);

extern ccs_result_t
ccs_hyperparameter_get_user_data(ccs_hyperparameter_t   hyperparameter,
                                 void                 **user_data_ret);

extern ccs_result_t
ccs_hyperparameter_get_default_distribution(ccs_hyperparameter_t  hyperparameter,
                                            ccs_distribution_t   *distribution);

extern ccs_result_t
ccs_hyperparameter_check_value(ccs_hyperparameter_t  hyperparameter,
                               ccs_datum_t           value,
                               ccs_bool_t           *result_ret);

extern ccs_result_t
ccs_hyperparameter_check_values(ccs_hyperparameter_t  hyperparameter,
                                size_t                num_values,
                                const ccs_datum_t    *values,
                                ccs_bool_t           *results);

extern ccs_result_t
ccs_hyperparameter_validate_value(ccs_hyperparameter_t  hyperparameter,
                                  ccs_datum_t           value,
                                  ccs_datum_t          *value_ret,
                                  ccs_bool_t           *result_ret);

extern ccs_result_t
ccs_hyperparameter_validate_values(ccs_hyperparameter_t  hyperparameter,
                                   size_t                num_values,
                                   const ccs_datum_t    *values,
                                   ccs_datum_t          *values_ret,
                                   ccs_bool_t           *results);

extern ccs_result_t
ccs_hyperparameter_convert_samples(ccs_hyperparameter_t  hyperparameter,
                                   ccs_bool_t            oversampling,
                                   size_t                num_values,
                                   const ccs_numeric_t  *values,
                                   ccs_datum_t          *results);

//   Sampling Interface
extern ccs_result_t
ccs_hyperparameter_sample(ccs_hyperparameter_t  hyperparameter,
                          ccs_distribution_t    distribution,
                          ccs_rng_t             rng,
                          ccs_datum_t          *value_ret);

extern ccs_result_t
ccs_hyperparameter_samples(ccs_hyperparameter_t  hyperparameter,
                           ccs_distribution_t    distribution,
                           ccs_rng_t             rng,
                           size_t                num_values,
                           ccs_datum_t          *values);

extern ccs_result_t
ccs_hyperparameter_sampling_interval(ccs_hyperparameter_t  hyperparameter,
                                     ccs_interval_t       *interval_ret);
#ifdef __cplusplus
}
#endif

#endif //_CCS_HYPERPARAMETER_H
