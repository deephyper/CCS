#ifndef _CCS_CONFIGURATION_H
#define _CCS_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file configuration.h
 * A configuration is a binding (see binding.h) on a configuration space (see
 * configuration_space.h).
 */

/**
 * Create a new instance of a configuration on a given configuration space. If
 * no values are provided the values will be initialized to #CCS_NONE.
 * @param[in] configuration_space
 * @param[in] num_values the number of provided values to initialize the
 *            configuration
 * @param[in] values an optional array of values to initialize the configuration
 * @param[in] user_data a pointer to the user data to attach to this
 *                      configuration instance
 * @param[out] configuration_ret a pointer to the variable that will hold the
 *             newly created configuration
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p configuration_ret is NULL; or if \p
 *                             values is NULL and \p num_values is greater than
 *                             0; or if the number of values provided is not
 *                             equal to the number of hyperparameters in the
 *                             configuration space
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             configuration
 */
extern ccs_result_t
ccs_create_configuration(ccs_configuration_space_t configuration_space,
                         size_t                    num_values,
                         ccs_datum_t              *values,
                         void                     *user_data,
                         ccs_configuration_t      *configuration_ret);

/**
 * Get the associated configuration space.
 * @param[in] configuration
 * @param[out] configuration_space_ret a pointer to the variable that will
 *                                     contain the configuration space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration is not a valid CCS
 *                              configuration
 * @return -#CCS_INVALID_VALUE if \p configuration_space_ret is NULL
 */
extern ccs_result_t
ccs_configuration_get_configuration_space(
	ccs_configuration_t        configuration,
	ccs_configuration_space_t *configuration_space_ret);

/**
 * Get the associated `user_data` pointer.
 * @param[in] configuration
 * @param[out] user_data_ret a pointer to `void *` variable that will contain
 *                           the value of the `user_data`
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration is not a valid CCS
 *                              configuration
 * @return -#CCS_INVALID_VALUE if \p user_data_ret is NULL
 */
extern ccs_result_t
ccs_configuration_get_user_data(ccs_configuration_t   configuration,
                                void                **user_data_ret);

/**
 * Get the value of the hyperparameter at the given index.
 * @param[in] configuration
 * @param[in] index index of the hyperparameter in the associated configuration
 *                  space
 * @param[out] value_ret a pointer to the variable that will hold the value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration is not a valid CCS
 *                              configuration
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the configuration space
 */
extern ccs_result_t
ccs_configuration_get_value(ccs_configuration_t  configuration,
                            size_t               index,
                            ccs_datum_t         *value_ret);

/**
 * Set the value of the hyperparameter at the given index. Transient values will
 * be validated and memoized if needed.
 * @param[in,out] configuration
 * @param[in] index index of the hyperparameter in the associated configuration
 *                  space
 * @param[in] value the value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration is not a valid CCS
 *                              configuration
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the configuration space
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory while memoizing a
 *                             string
 */
extern ccs_result_t
ccs_configuration_set_value(ccs_configuration_t configuration,
                            size_t              index,
                            ccs_datum_t         value);

/**
 * Get all the values in the configuration.
 * @param[in] configuration
 * @param[in] num_values the size of the \p values array
 * @param[out] values an array of size \p num_values to hold the returned
 *                    values, or NULL. If the array is too big, extra values
 *                    are set to #CCS_NONE
 * @param[out] num_values_ret a pointer to a variable that will contain the
 *                            number of values that are or would be returned.
 *                            Can be NULL
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration is not a valid CCS
 *                              configuration
 * @return -#CCS_INVALID_VALUE if \p values is NULL and \p num_values is greater
 *                             than 0; or if \p values is NULL and
 *                             num_values_ret is NULL; or if \p num_values is
 *                             less than the number of values that would be
 *                             returned
 */
extern ccs_result_t
ccs_configuration_get_values(ccs_configuration_t  configuration,
                             size_t               num_values,
                             ccs_datum_t         *values,
                             size_t              *num_values_ret);

/**
 * Get the value of the hyperparameter with the given name.
 * @param[in] configuration
 * @param[in] name the name of the hyperparameter whose value to retrieve
 * @param[out] value_ret a pointer to the variable that will hold the value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration is not a valid CCS
 *                              configuration
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL
 * @return -#CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the \p configuration space
 */
extern ccs_result_t
ccs_configuration_get_value_by_name(ccs_configuration_t  configuration,
                                    const char          *name,
                                    ccs_datum_t         *value_ret);

/**
 * Check that the configuration is a valid configuration for the configuration
 * space.
 * @param[in] configuration
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration is not a valid CCS
 *                              configuration
 * @return -#CCS_INVALID_CONFIGURATION if \p configuration is found to be
 *                                     invalid
 */
extern ccs_result_t
ccs_configuration_check(ccs_configuration_t configuration);

/**
 * Compute a hash value for the configuration by hashing together the
 * configuration space reference, the number of values, and the values
 * themselves.
 * @param[in] configuration
 * @param[out] hash_ret the address of the variable that will contain the hash
 *                      value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration is not a valid CCS
 *                              configuration
 * @return -#CCS_INVALID_VALUE if \p hash_ret is NULL
 */
extern ccs_result_t
ccs_configuration_hash(ccs_configuration_t  configuration,
                       ccs_hash_t          *hash_ret);

/**
 * Define a strict ordering of configuration instances. Configuration space,
 * number of values and values are compared.
 * @param[in] configuration the first configuration
 * @param[in] other_configuration the second configuration
 * @param[out] cmp_ret the pointer to the variable that will contain the result
 *                     of the comparison. Will contain -1, 0, or 1 depending on
 *                     if the first configuration is found to be respectively
 *                     lesser than, equal, or greater then the second
 *                     configuration
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration or \p other_configuration
 *                              are not a valid CCS object
 */
extern ccs_result_t
ccs_configuration_cmp(ccs_configuration_t  configuration,
                      ccs_configuration_t  other_configuration,
                      int                 *cmp_ret);
#ifdef __cplusplus
}
#endif

#endif //_CCS_CONFIGURATION_H
