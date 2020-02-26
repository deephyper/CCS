#include <stdint.h>
#include "ccs/configuration_space.h"
#include "ccs/configuration.h"
#include "ccs/hyperparameter.h"

typedef struct _ccs_configuration_space_s *ccs_configuration_space_t;
typedef struct _ccs_configuration_s       *ccs_configuration_t;
typedef struct _ccs_hyperparameter_s      *ccs_hyperparameter_t;
typedef struct _ccs_condition_s           *ccs_condition_t;
typedef struct _ccs_forbidden_clause_s    *ccs_forbidden_clause_t;

enum ccs_error_e {
	CCS_SUCCESS,
	CCS_ERROR_MAX,
	CCS_ERROR_FORCE_32BIT = MAX_INT
};

typedef enum ccs_error_e ccs_error_t;

enum ccs_data_type_e {
	CCS_INTEGER,
	CCS_FLOAT,
	CCS_STRING,
	CCS_DATA_TYPE_MAX,
	CCS_DATA_TYPE_FORCE_32BIT = MAX_INT
};

typedef enum ccs_data_type_e ccs_data_type_t;

struct ccs_data_u {
	union {
		double      d;
		int64_t     i;
		const char *s;
	} value;
	ccs_data_type_t type;
};

typedef struct ccs_data_u ccs_data_t;

