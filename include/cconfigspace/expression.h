#ifndef _CCS_CONDITION_H
#define _CCS_CONDITION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file expression.h
 * An expression in CCS is a combination of constants, variables
 * (hyperparameters), and operators. Expressions are usually evaluated in the
 * context of a binding where hyperparameters are associated values. For
 * convinience CCS suggests a grammar that can be used to create an expression
 * parser.
 */

/**
 * Supported expression types.
 */
enum ccs_expression_type_e {
	/** Or boolean operator */
	CCS_OR = 0,
	/** And boolean operator */
	CCS_AND,
	/** Equality test operator */
	CCS_EQUAL,
	/** Inequality test operator */
	CCS_NOT_EQUAL,
	/** Lesser than comparison operator */
	CCS_LESS,
	/** Greater than comparison operator */
	CCS_GREATER,
	/** Lesser than or equal comparison operator */
	CCS_LESS_OR_EQUAL,
	/** Greater than or equal comparison operator */
	CCS_GREATER_OR_EQUAL,
	/** Addition operator */
	CCS_ADD,
	/** Substraction operator */
	CCS_SUBSTRACT,
	/** Multiplication operator */
	CCS_MULTIPLY,
	/** Division operator */
	CCS_DIVIDE,
	/** Modulo operator */
	CCS_MODULO,
	/** Unary plus operator */
	CCS_POSITIVE,
	/** Unary minus operator */
	CCS_NEGATIVE,
	/** Not boolean operator */
	CCS_NOT,
	/** List inclusion test operator */
	CCS_IN,
	/** List */
	CCS_LIST,
	/** Literal constant */
	CCS_LITERAL,
	/** Variable */
	CCS_VARIABLE,
	/** Guard */
	CCS_EXPRESSION_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_EXPRESSION_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent an expression type.
 */
typedef enum ccs_expression_type_e ccs_expression_type_t;

/**
 * An array of precedence of operators as defined by CCS grammar:
 *  - 0 : OR
 *  - 1 : AND
 *  - 2 : EQUAL, NOT_EQUAL
 *  - 3 : LESS, GREATER, LESS_OR_EQUAL, GREATER_OR_EQUAL
 *  - 4 : ADD, SUBSTRACT
 *  - 5 : MULTIPLY, DIVIDE, MODULO
 *  - 6 : POSITIVE, NEGATIVE, NOT
 *  - 7 : IN
 *  - max - 1: LIST
 *  - max : LITERAL, VARIABLE
 *
 * Those are similar to C's precedence
 */
extern const int ccs_expression_precedence[];

/**
 * Associativity of CCS operators:
 */
enum ccs_associativity_type_e {
	/** No associativity */
	CCS_ASSOCIATIVITY_TYPE_NONE = 0,
	/** left to right associativity */
	CCS_LEFT_TO_RIGHT,
	/** right to left associativity */
	CCS_RIGHT_TO_LEFT,
	/** Guard */
	CCS_ASSOCIATIVITY_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_ASSOCIATIVITY_TYPE_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent an associativity.
 */
typedef enum ccs_associativity_type_e ccs_associativity_type_t;

/**
 * An array of associativity of operators as defined by CCS grammar:
 *  - left: OR
 *  - left: AND
 *  - left: EQUAL, NOT_EQUAL
 *  - left: LESS, GREATER, LESS_OR_EQUAL, GREATER_OR_EQUAL
 *  - left: ADD, SUBSTRACT
 *  - left: MULTIPLY, DIVIDE, MODULO
 *  - right: POSITIVE, NEGATIVE, NOT
 *  - left: IN
 *  - left: LIST
 *  - none: LITERAL, VARIABLE
 */
extern const ccs_associativity_type_t ccs_expression_associativity[];

/**
 * An array of suggested symbols (NULL terminated strings) for CCS operators.
 *  - OR: ||
 *  - AND: &&
 *  - EQUAL: ==
 *  - NOT_EQUAL: !=
 *  - LESS: <
 *  - GREATER: >
 *  - LESS_OR_EQUAL: <=
 *  - GREATER_OR_EQUAL: >=
 *  - ADD: +
 *  - SUBSTRACT: -
 *  - MULTIPLY: *
 *  - DIVIDE: /
 *  - MODULO: %
 *  - POSITIVE: +
 *  - NEGATIVE: -
 *  - NOT: !
 *  - IN: #
 *  - LIST: NULL
 *  - LITERAL: NULL
 *  - VARIABLE: NULL
 */
extern const char *ccs_expression_symbols[];

/**
 * An array of arity of CCS operators
 *  - 2: OR
 *  - 2: AND
 *  - 2: EQUAL, NOT_EQUAL
 *  - 2: LESS, GREATER, LESS_OR_EQUAL, GREATER_OR_EQUAL
 *  - 2: ADD, SUBSTRACT
 *  - 2: MULTIPLY, DIVIDE, MODULO
 *  - 1: POSITIVE, NEGATIVE, NOT
 *  - 2: IN
 *  - -1: LIST
 *  - 0: LITERAL, VARIABLE
 */
extern const int ccs_expression_arity[];

/**
 * The different terminal types of ccs expressions.
 */
enum ccs_terminal_type_e {
	/** The #CCS_NONE_VAL value */
	CCS_TERM_NONE = 0,
	/** The #CCS_TRUE_VAL value */
	CCS_TERM_TRUE,
	/** The #CCS_FALSE_VAL value */
	CCS_TERM_FALSE,
	/** A #CCS_STRING value */
	CCS_TERM_STRING,
	/** An identifer (name of a hyperparameter) */
	CCS_TERM_IDENTIFIER,
	/** A #CCS_INTEGER value */
	CCS_TERM_INTEGER,
	/** A #CCS_FLOAT value */
	CCS_TERM_FLOAT,
	/** Guard */
	CCS_TERMINAL_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_TERMINAL_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represend CCS terminal types
 */
typedef enum ccs_terminal_type_e ccs_terminal_type_t;

/**
 * An array of integers defining terminal precedence in order to disambiguate
 * NONE, TRUE and FALSE from identifiers:
 *  - 0: STRING, IDENTIFIER, INTEGER, FLOAT
 *  - 1: NONE, TRUE, FALSE
 */
extern const int ccs_terminal_precedence[];

/**
 * An array of regexp that define terminals:
 *  - NONE: /none/
 *  - TRUE: /true/
 *  - FALSE: /false/
 *  - STRING:
 *  /"([^\0\t\n\r\f"\\\\]|\\\\[0tnrf"\\])+"|'([^\0\\t\\n\\r\\f'\\\\]|\\\\[0tnrf'\\\\])+'/
 *  - IDENTIFIER: /[a-zA-Z_][a-zA-Z_0-9]/
 *  - INTEGER: /-?[0-9]+/
 *  - FLOAT: /-?[0-9]+([eE][+-]?[0-9]+|\\.[0-9]+([eE][+-]?[0-9]+)?)/
 */
extern const char *ccs_terminal_regexp[];

/**
 * An array of symbols (NULL terminated strings) for terminals that define them:
 *  - NONE: "none"
 *  - TRUE: "true"
 *  - FALSE: "false"
 *  - STRING: NULL
 *  - IDENTIFIER: NULL
 *  - INTEGER: NULL
 *  - FLOAT: NULL
 */
extern const char *ccs_terminal_symbols[];

/**
 * Create a new expression.
 * @param[in] type the type of the expression
 * @param[in] num_nodes the number of the expression children nodes. Must be
 *                      compatible with the arity of the expression
 * @param[in] nodes an array of \p num_nodes expressions
 * @param[out] expression_ret a pointer to the variable that will hold the newly
 *             created expression
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p type is not a valid CCS expression type; or
 *                             if \p num_nodes is not compatible with the arity
 *                             of \p type; or if one the nodes given is of type
 *                             #CCS_OBJECT but is neither a #CCS_HYPERPARAMETER
 *                             nor a #CCS_EXPRESSION; or if one the nodes given
 *                             node is not a type #CCS_OBJECT, #CCS_NONE,
 *                             #CCS_INTEGER, #CCS_FLOAT, #CCS_BOOLEAN, or
 *                             #CCS_STRING; or if \p expression_ret is NULL
 * @return -#CCS_INVALID_OBJECT if one the nodes given is of type #CCS_OBJECT
 *                              but the object is not a valid CCS object
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             expression
 */
extern ccs_result_t
ccs_create_expression(ccs_expression_type_t  type,
	              size_t                 num_nodes,
                      ccs_datum_t           *nodes,
                      ccs_expression_t      *expression_ret);

/**
 * Create a new binary expression. Convenience wrapper around
 * #ccs_create_expression for binary operators.
 * @param[in] type the type of the expression
 * @param[in] node_left left child node
 * @param[in] node_right right child node
 * @param[out] expression_ret a pointer to the variable that will hold the newly
 *             created expression
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p type is not a valid CCS expression type; or
 *                             if \p type arity is not 2; or if \p node_left or
 *                             \p node_right are of type #CCS_OBJECT but are
 *                             neither a #CCS_HYPERPARAMETER nor a
 *                             #CCS_EXPRESSION; or if \p node_left or \p
 *                             node_right are not of type #CCS_OBJECT,
 *                             #CCS_NONE, #CCS_INTEGER, #CCS_FLOAT,
 *                             #CCS_BOOLEAN, or #CCS_STRING; or if \p
 *                             expression_ret is NULL
 * @return -#CCS_INVALID_OBJECT if \p node_left or \p node_right are of type
 *                              #CCS_OBJECT but the object is not a valid CCS
 *                              object
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             expression
 */
extern ccs_result_t
ccs_create_binary_expression(ccs_expression_type_t  type,
                             ccs_datum_t            node_left,
                             ccs_datum_t            node_right,
                             ccs_expression_t      *expression_ret);


/**
 * Create a new unary expression. Convenience wrapper around
 * #ccs_create_expression for unary expressions.
 * @param[in] type the type of the expression
 * @param[in] node child node
 * @param[out] expression_ret a pointer to the variable that will hold the newly
 *             created expression
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p type is not a valid CCS expression type; or
 *                             if \p type arity is not 1; or if \p node is of
 *                             type #CCS_OBJECT but is neither a
 *                             #CCS_HYPERPARAMETER nor a #CCS_EXPRESSION; or if
 *                             \p node is not of type #CCS_OBJECT, #CCS_NONE,
 *                             #CCS_INTEGER, #CCS_FLOAT, #CCS_BOOLEAN, or
 *                             #CCS_STRING; or if \p expression_ret is NULL
 * @return -#CCS_INVALID_OBJECT if \p node is of type #CCS_OBJECT but the object
 *                              is not a valid CCS object
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             expression
 */
extern ccs_result_t
ccs_create_unary_expression(ccs_expression_type_t  type,
                            ccs_datum_t            node,
                            ccs_expression_t      *expression_ret);

/**
 * Create a new literal expression.
 * @param[in] value the value of the literal
 * @param[out] expression_ret a pointer to the variable that will hold the newly
 *             created expression. If value is of type #CCS_STRING, the string
 *             value is memoized
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p value is not of type #CCS_NONE,
 *                             #CCS_INTEGER, #CCS_FLOAT, #CCS_BOOLEAN, or
 *                             #CCS_STRING; or if \p expression_ret is NULL
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             expression
 */
extern ccs_result_t
ccs_create_literal(ccs_datum_t       value,
                   ccs_expression_t *expression_ret);

/**
 * Create a new variable expression.
 * @param[in] hyperparameter hyperparameter to use as a variable
 * @param[out] expression_ret a pointer to the variable that will hold the newly
 *             created expression
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p expression_ret is NULL
 * @return -#CCS_INVALID_OBJECT if \p hyperparameter is not a valid CCS
 *                              hyperparameter
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             expression
 */
extern ccs_result_t
ccs_create_variable(ccs_hyperparameter_t  hyperparameter,
                    ccs_expression_t     *expression_ret);


/**
 * Get the type of an expression.
 * @param[in] expression
 * @param[out] type_ret a pointer to the variable that will contain the type of
 *                      the expression
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p type_ret is NULL
 * @return -#CCS_INVALID_OBJECT if \p expression is not a valid CCS expression
 */
extern ccs_result_t
ccs_expression_get_type(ccs_expression_t       expression,
                        ccs_expression_type_t *type_ret);

/**
 * Get the number of child node of an expression.
 * @param[in] expression
 * @param[out] num_nodes_ret a pointer to the variable that will contain the
 *                           number of child node of the expression
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p num_nodes_ret is NULL
 * @return -#CCS_INVALID_OBJECT if \p expression is not a valid CCS expression
 */
extern ccs_result_t
ccs_expression_get_num_nodes(ccs_expression_t  expression,
                             size_t           *num_nodes_ret);

/**
 * Get the child nodes of an expression.
 * @param[in] expression
 * @param[in] num_nodes the size of the \p nodes array
 * @param[out] nodes an array of size \p num_nodes to hold the returned values
 *                   or NULL. If the array is too big, extra values are set NULL
 * @param[out] num_nodes_ret a pointer to a variable that will contain the
 *                           number of nodes that are or would be returned. Can
 *                           be NULL
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p expression is not a valid CCS expression
 * @return -#CCS_INVALID_VALUE if \p nodes is NULL and \p num_nodes is greater
 *                             than 0; or if \p nodes is NULL and num_nodes_ret
 *                             is NULL; or if num_values is less than the number
 *                             of values that would be returned
 */
extern ccs_result_t
ccs_expression_get_nodes(ccs_expression_t  expression,
                         size_t            num_nodes,
                         ccs_expression_t *nodes,
                         size_t           *num_nodes_ret);

/**
 * Get the value of a literal expression.
 * @param[in] expression
 * @param[out] value_ret a pointer to a variable that will contain the value of
 *                       the literal
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p expression is not a valid CCS expression
 * @return -#CCS_INVALID_EXPRESSION if \p expression is not a #CCS_LITERAL
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL
 */
extern ccs_result_t
ccs_literal_get_value(ccs_expression_t  expression,
                      ccs_datum_t      *value_ret);

/**
 * Get the hyperparameter of a variable expression.
 * @param[in] expression
 * @param[out] hyperparameter_ret a pointer to a variable that will contain the
 *                                hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p expression is not a valid CCS expression
 * @return -#CCS_INVALID_EXPRESSION if \p expression is not a #CCS_VARIABLE
 * @return -#CCS_INVALID_VALUE if \p hyperparameter_ret is NULL
 */
extern ccs_result_t
ccs_variable_get_hyperparameter(ccs_expression_t      expression,
                                ccs_hyperparameter_t *hyperparameter_ret);

/**
 * Get the value of an expression in a given context, provided a list of values
 * for the context hyperparameters.
 * @param[in] expression
 * @param[in] context the context to evaluate the expression into. Can be NULL
 * @param[in] values an array of values, one for each hyperparameter in \p
 *                   context. Can be NULL
 * @param[out] result_ret a pointer to a variable that will contain the result
 *                        of the evaluation of the expression
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p expression is not a valid CCS variable
 *                              expression; or if \p context is NULL and
 *                              expression must evaluate a variable
 * @return -#CCS_INVALID_VALUE if \p result_ret is NULL; or if \p values is NULL
 *                             and expression must evaluate a variable; or if an
 *                             illegal arithmetic or comparison operation would
 *                             have occurred; or if a non boolean value is used
 *                             in a boolean operation
 * @return -#CCS_INACTIVE_HYPERPARAMETER if one of the variable was evaluated
 *                                       and found to be inactive during the
 *                                       evaluation
 */
extern ccs_result_t
ccs_expression_eval(ccs_expression_t  expression,
                    ccs_context_t     context,
                    ccs_datum_t      *values,
                    ccs_datum_t      *result_ret);

/**
 * Evaluate the entry of a list at a given index, in a given context, provided a
 * list of values for the context hyperparameters.
 * @param[in] expression
 * @param[in] context the context to evaluate the expression into. Can be NULL
 * @param[in] values an array of values, one for each hyperparameter in \p
 *                   context. Can be NULL
 * @param[in] index index of the child node to evaluate
 * @param[out] result_ret a pointer to a variable that will contain the result
 *                        of the evaluation of the expression
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p expression is not a valid CCS expression;
 *                              or if \p context is NULL and \p expression must
 *                              evaluate a variable
 * @return -#CCS_INVALID_EXPRESSION if \p expression is not a #CCS_LIST
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the number of child
 *                             nodes in the list
 * @return -#CCS_INVALID_VALUE if \p result_ret is NULL; or if \p values is NULL
 *                             and expression must evaluate a variable; or if an
 *                             illegal arithmetic or comparison operation would
 *                             have occurred; or if a non boolean value is used
 *                             in a boolean operation
 * @return -#CCS_INACTIVE_HYPERPARAMETER if one of the variable was evaluated
 *                                       and found to be inactive during the
 *                                       evaluation
 */
extern ccs_result_t
ccs_expression_list_eval_node(ccs_expression_t  expression,
                              ccs_context_t     context,
                              ccs_datum_t      *values,
                              size_t            index,
                              ccs_datum_t      *result_ret);

/**
 * Get the hyperparameters used in an expression.
 * @param[in] expression
 * @param[in] num_hyperparameters the size of the \p hyperparameters array
 * @param[in] hyperparameters an array of size \p num_hyperparameters to hold
 *                            the returned values, or NULL. If the array is too
 *                            big, extra values are set to NULL
 * @param[out] num_hyperparameters_ret a pointer to a variable that will contain
 *                                     the number of hyperparameters that are or
 *                                     would be returned. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p expression is not a valid CCS expression
 * @return -#CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                              num_hyperparameters is greater than 0; or if \p
 *                              hyperparameters is NULL and \p
 *                              num_hyperparameters_ret is NULL; or if \p
 *                              num_hyperparameters is less than the number of
 *                              hyperparameters that would be returned
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             temporary storage
 */
extern ccs_result_t
ccs_expression_get_hyperparameters(ccs_expression_t      expression,
                                   size_t                num_hyperparameters,
                                   ccs_hyperparameter_t *hyperparameters,
                                   size_t               *num_hyperparameters_ret);

/**
 * Validate that an expression can be evaluated in the given context.
 * @param[in] expression
 * @param[in] context the context to verify the expression can be evaluated in.
 *            Can be NULL
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p expression is not a valid CCS expression;
 *                              or if expression depends on a hyperparameter and
 *                              \p context is not a valid CCS context
 * @return -#CCS_INVALID_VALUE if the expression depends on a hyperparameter and
 *                             \p context is NULL
 * @return -#CCS_INVALID_HYPERPARAMETER if \p context does not contain one of
 *                                      the hyperparameters referenced by the
 *                                      expression
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             temporary storage
 */
extern ccs_result_t
ccs_expression_check_context(ccs_expression_t expression,
                             ccs_context_t    context);
#ifdef __cplusplus
}
#endif

#endif //_CCS_CONDITION_H
