#ifndef _CCS_EVALUATION_H
#define _CCS_EVALUATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file evaluation.h
 * An evaluation is a binding (see binding.h) over an objective space given a
 * specific configuration (see configuration.h). Successful evaluations over the
 * same objective space are weakly ordered by their objective values. Evaluation
 * that have failed must report an error code.
 */

/**
 * The different possible return codes when comparing two evaluations.
 */
enum ccs_comparison_e {
	/** The first configuration is better then the second */
	CCS_BETTER = -1,
	/** The two configurations are equivalent */
	CCS_EQUIVALENT = 0,
	/** The first configuration is worse than the second */
	CCS_WORSE = 1,
	/** The two configurations cannot be compared */
	CCS_NOT_COMPARABLE = 2,
	/** Guard */
	CCS_COMPARISON_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_COMPARISON_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS the result of comparing evaluations.
 */
typedef enum ccs_comparison_e ccs_comparison_t;

/**
 * Create a new instance of an evaluation on a given objective_space for a given
 * configuration.
 * @param[in] objective_space the objective space to associate with the
 *                            evaluation
 * @param[in] configuration the configuration to associate with the evaluation
 * @param[in] error the error code associated with the evaluation
 * @param[in] num_values the number of provided values to initialize the
 *                       evaluation
 * @param[in] values an optional array of values to initialize the evaluation
 * @param[in] user_data a pointer to the user data to attach to this evaluation
 *                      instance
 * @param[out] evaluation_ret a pointer to the variable that will hold the
 *             newly created evaluation
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space; or if \p configuration is not a
 *                              valid CCS configuration
 * @return -#CCS_INVALID_VALUE if \p evaluation_ret is NULL; or if \p values is
 *                             NULL and \p num_values is greater than 0; or if
 *                             the number of values provided is not equal to the
 *                             number of hyperparameters in the objective space
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             evaluation
 */
extern ccs_result_t
ccs_create_evaluation(ccs_objective_space_t  objective_space,
                      ccs_configuration_t    configuration,
                      ccs_result_t           error,
                      size_t                 num_values,
                      ccs_datum_t           *values,
                      void                  *user_data,
                      ccs_evaluation_t      *evaluation_ret);

/**
 * Get the objective space associated with an evaluation.
 * @param[in] evaluation
 * @param[out] objective_space_ret a pointer to the variable that will
 *                                 contain the objective space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p objective_space_ret is NULL
 */
extern ccs_result_t
ccs_evaluation_get_objective_space(ccs_evaluation_t       evaluation,
                                   ccs_objective_space_t *objective_space_ret);

/**
 * Get the configuration associated with an evaluation.
 * @param[in] evaluation
 * @param[out] configuration_ret a pointer to the variable that will contain
 *                               the configuration
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p configuration_ret is NULL
 */
extern ccs_result_t
ccs_evaluation_get_configuration(ccs_evaluation_t     evaluation,
                                 ccs_configuration_t *configuration_ret);

/**
 * Get the user data pointer associated with an evaluation.
 * @param[in] evaluation
 * @param[out] user_data_ret a pointer to `void *` variable that will contain
 *                           the value of the `user_data`
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p user_data_ret is NULL
 */
extern ccs_result_t
ccs_evaluation_get_user_data(ccs_evaluation_t   evaluation,
                             void             **user_data_ret);

/**
 * Get the error code associated with an evaluation.
 * @param[in] evaluation
 * @param[out] error_ret a pointer to the variable that will contain the
 *                       returned error code
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p error_ret is NULL
 */
extern ccs_result_t
ccs_evaluation_get_error(ccs_evaluation_t  evaluation,
                         ccs_result_t     *error_ret);

/**
 * Set the error code associated with an evaluation. A successful
 * evaluation should have it's error set to #CCS_SUCCESS.
 * @param[in,out] evaluation
 * @param[in] error the error code associated withe the evaluation
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 */
extern ccs_result_t
ccs_evaluation_set_error(ccs_evaluation_t evaluation,
                         ccs_result_t     error);

/**
 * Get the value of the hyperparameter at the given index.
 * @param[in] evaluation
 * @param[in] index index of the hyperparameter in the associated objective
                    space
 * @param[out] value_ret a pointer to the variable that will hold the value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p binding is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the objective space
 */
extern ccs_result_t
ccs_evaluation_get_value(ccs_evaluation_t  evaluation,
                         size_t            index,
                         ccs_datum_t      *value_ret);

/**
 * Set the value of the hyperparameter at the given index. Transient values will
 * be validated and memoized if needed.
 * @param[in,out] evaluation
 * @param[in] index index of the hyperparameter in the associated objective
 *                  space
 * @param[in] value the value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the objective space
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory while memoizing a
 *                             string
 */
extern ccs_result_t
ccs_evaluation_set_value(ccs_evaluation_t evaluation,
                         size_t           index,
                         ccs_datum_t      value);

/**
 * Get all the values in an evaluation.
 * @param[in] evaluation
 * @param[in] num_values the size of the \p values array
 * @param[out] values an array of size \p num_values to hold the returned values
 *                    or NULL. If the array is too big, extra values are set to
 *                    #CCS_NONE
 * @param[out] num_values_ret a pointer to a variable that will contain the
 *                            number of values that are or would be returned.
 *                            Can be NULL
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p values is NULL and \p num_values is greater
 *                             than 0; or if \p values is NULL and
 *                             num_values_ret is NULL; or if num_values is less
 *                             than the number of values that would be returned
 */
extern ccs_result_t
ccs_evaluation_get_values(ccs_evaluation_t  evaluation,
                          size_t            num_values,
                          ccs_datum_t      *values,
                          size_t           *num_values_ret);

/**
 * Get the value of the hyperparameter with the given name.
 * @param[in] evaluation
 * @param[in] name the name of the hyperparameter whose value to retrieve
 * @param[out] value_ret a pointer to the variable that will hold the value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL
 * @return -#CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the \p objective space
 */
extern ccs_result_t
ccs_evaluation_get_value_by_name(ccs_evaluation_t  evaluation,
                                 const char       *name,
                                 ccs_datum_t      *value_ret);

/**
 * Check that an evaluation values are valid in the objective space.
 * @param[in] evaluation
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_EVALUATION if \p evaluation was found to be invalid in
 *                                  the context of the objective space
 */
extern ccs_result_t
ccs_evaluation_check(ccs_evaluation_t  evaluation);

/**
 * Get the value of an objective for a valid evaluation in the context of its
 * objective space.
 * @param[in] evaluation
 * @param[in] index the index of the objective in the objective space
 * @param[out] value_ret a pointer to the variable that will contain the value
 *                       of the objective
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL; or if there was an issue
 *                             evaluating the objective
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the number of
 *                             objective in the objective space
 */
extern ccs_result_t
ccs_evaluation_get_objective_value(ccs_evaluation_t  evaluation,
                                   size_t            index,
                                   ccs_datum_t      *value_ret);

/**
 * Get the values of the objectives for a valid evaluation in the context of its
 * objective space.
 * @param[in] evaluation
 * @param[in] num_values the number of values that \p values can contain
 * @param[out] values an optional array of values that will contain the returned
 *                    objective values. If values is bigger than the number of
 *                    objectives, extra values will be set to #CCS_NONE
 * @param[out] num_values_ret an optional pointer to a variable that will
 *                            contain the number of values that are or would be
 *                            returned. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p values is NULL and \p num_values is greater
 *                             than 0; or if values is NULL and num_values_ret
 *                             is NULL; or if there was an issue evaluating any
 *                             of the objectives
 */
extern ccs_result_t
ccs_evaluation_get_objective_values(ccs_evaluation_t  evaluation,
                                    size_t            num_values,
                                    ccs_datum_t      *values,
                                    size_t           *num_values_ret);

/**
 * Compute a hash value for the evaluation by hashing together the objective
 * space reference, the configuration, the number of values, the values
 * themselves, and the associated error.
 * @param[in] evaluation
 * @param[out] hash_ret the address of the variable that will contain the hash
 *                      value.
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation is not a valid CCS evaluation
 * @return -#CCS_INVALID_VALUE if \p hash_ret is NULL
 */
extern ccs_result_t
ccs_evaluation_hash(ccs_evaluation_t  evaluation,
                    ccs_hash_t       *hash_ret);

/**
 * Define a strict ordering of evaluation instances. Objective space,
 * configuration, number of values, values, and error codes are compared.
 * @param[in] evaluation the first evaluation
 * @param[in] other_evaluation the second evaluation
 * @param[out] cmp_ret a pointer to the variable that will contain the result
 *                     of the comparison. Will contain -1, 0, or 1 depending on
 *                     if the first evaluation is found to be respectively
 *                     lesser than, equal, or greater then the second evaluation
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation or \p other_evaluation are not
 *                              valid CCS evaluations
 * @return -#CCS_INVALID_VALUE if \p cmp_ret is NULL
 */
extern ccs_result_t
ccs_evaluation_cmp(ccs_evaluation_t  evaluation,
                   ccs_evaluation_t  other_evaluation,
                   int              *cmp_ret);

/**
 * Compare two successful evaluations objectives.
 * @param[in] evaluation the first evaluation
 * @param[in] other_evaluation the second evaluation
 * @param[out] result_ret a pointer to the variable that will contain the result
 *                        of the comparison. Will contain #CCS_BETTER,
 *                        #CCS_EQUIVALENT, #CCS_WORSE, or #CCS_NOT_COMPARABLE if
 *                        the first evaluation is found to be respectively
 *                        better, equivalent, worse, or not comparable with the
 *                        second evaluation.
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p evaluation or \p other_evaluation are not
 *                              valid CCS evaluations; or if \p evaluation and
 *                              \p other_evaluation do not share the same
 *                              objective space; or if any of the configuration
 *                              is associated an error code different than
 *                              CCS_SUCESS
 * @return -#CCS_INVALID_VALUE if \p result_ret is NULL; or if there was an
 *                                issue evaluating any of the objectives
 */
extern ccs_result_t
ccs_evaluation_compare(ccs_evaluation_t  evaluation,
                       ccs_evaluation_t  other_evaluation,
                       ccs_comparison_t *result_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_EVALUATION_H
