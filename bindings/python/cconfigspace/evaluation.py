import ctypes as ct
from .base import Object, Error, CEnumeration, ccs_error, ccs_result, _ccs_get_function, ccs_context, ccs_hyperparameter, ccs_configuration_space, ccs_configuration, ccs_datum, ccs_objective_space, ccs_evaluation
from .context import Context
from .hyperparameter import Hyperparameter
from .configuration_space import ConfigurationSpace
from .configuration import Configuration
from .objective_space import ObjectiveSpace

class ccs_comparison(CEnumeration):
  _members_ = [
    ('BETTER', -1),
    ('EQUIVALENT', 0),
    ('WORSE', 1),
    ('NOT_COMPARABLE', 2) ]

ccs_create_evaluation = _ccs_get_function("ccs_create_evaluation", [ccs_objective_space, ccs_configuration, ccs_result, ct.c_size_t, ct.POINTER(ccs_datum), ct.c_void_p, ct.POINTER(ccs_evaluation)])
ccs_evaluation_get_objective_space = _ccs_get_function("ccs_evaluation_get_objective_space", [ccs_evaluation, ct.POINTER(ccs_objective_space)])
ccs_evaluation_get_configuration = _ccs_get_function("ccs_evaluation_get_configuration", [ccs_evaluation, ct.POINTER(ccs_configuration)])
ccs_evaluation_get_user_data = _ccs_get_function("ccs_evaluation_get_user_data", [ccs_evaluation, ct.POINTER(ct.c_void_p)])
ccs_evaluation_get_error = _ccs_get_function("ccs_evaluation_get_error", [ccs_evaluation, ct.POINTER(ccs_result)])
ccs_evaluation_set_error = _ccs_get_function("ccs_evaluation_set_error", [ccs_evaluation, ccs_result])
ccs_evaluation_get_value = _ccs_get_function("ccs_evaluation_get_value", [ccs_evaluation, ct.c_size_t, ct.POINTER(ccs_datum)])
ccs_evaluation_set_value = _ccs_get_function("ccs_evaluation_set_value", [ccs_evaluation, ct.c_size_t, ccs_datum])
ccs_evaluation_get_values = _ccs_get_function("ccs_evaluation_get_values", [ccs_evaluation, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ct.c_size_t)])
ccs_evaluation_get_value_by_name = _ccs_get_function("ccs_evaluation_get_value_by_name", [ccs_evaluation, ct.c_char_p, ccs_datum])
ccs_evaluation_get_objective_value = _ccs_get_function("ccs_evaluation_get_objective_value", [ccs_evaluation, ct.c_size_t, ct.POINTER(ccs_datum)])
ccs_evaluation_get_objective_values = _ccs_get_function("ccs_evaluation_get_objective_values", [ccs_evaluation, ct.c_size_t, ct.POINTER(ccs_datum), ct.POINTER(ct.c_size_t)])
ccs_evaluation_cmp = _ccs_get_function("ccs_evaluation_cmp", [ccs_evaluation, ccs_evaluation, ct.POINTER(ccs_comparison)])

class Evaluation(Object):
  def __init__(self, handle = None, retain = False, objective_space = None, configuration = None, error = ccs_error.SUCCESS, values = None, user_data = None):
    if handle is None:
      count = 0
      if values:
        count = len(values)
        vals = (ccs_datum * count)()
        for i in range(count):
          vals[i].value = values[i]
      else:
        vals = None
      handle = ccs_evaluation()
      res = ccs_create_evaluation(objective_space.handle, configuration.handle, error, count, vals, user_data, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)

  @classmethod
  def from_handle(cls, handle):
    return cls(handle = handle, retain = True)

  @property
  def user_data(self):
    if hasattr(self, "_user_data"):
      return self._user_data
    v = ct.c_void_p()
    res = ccs_evaluation_get_user_data(self.handle, ct.byref(v))
    Error.check(res)
    self._user_data = v
    return v

  @property
  def objective_space(self):
    if hasattr(self, "_objective_space"):
      return self._objective_space
    v = ccs_objective_space()
    res = ccs_evaluation_get_objective_space(self.handle, ct.byref(v))
    Error.check(res)
    self._objective_space = ObjectiveSpace.from_handle(v)
    return self._objective_space

  @property
  def configuration(self):
    if hasattr(self, "_configuration"):
      return self._configuration
    v = ccs_configuration()
    res = ccs_evaluation_get_configuration(self.handle, ct.byref(v))
    Error.check(res)
    self._configuration = Configuration.from_handle(v)
    return self._configuration

  @property
  def error(self):
    v = ccs_result()
    res = ccs_evaluation_get_error(self.handle, ct.byref(v)) 
    Error.check(res)
    return v.value

  @error.setter
  def error(self, v):
    res = ccs_evaluation_set_error(self.handle, v)
    Error.check(res)

  def set_value(self, hyperparameter, value):
    if isinstance(hyperparameter, Hyperparameter):
      hyperparameter = self.objective_space.hyperparameter_index(hyperparameter)
    elif isinstance(hyperparameter, str):
      hyperparameter = self.objective_space.hyperparameter_index_by_name(hyperparameter)
    pv = ccs_datum(value)
    v = ccs_datum_fix()
    v.value = pv._value.i
    v.type = pv.type
    res = ccs_evaluation_set_value(self.handle, hyperparameter, v)
    Error.check(res)

  def value(self, hyperparameter):
    v = ccs_datum()
    if isinstance(hyperparameter, Hyperparameter):
      res = ccs_evaluation_get_value(self.handle, self.objective_space.hyperparameter_index(hyperparameter), ct.byref(v))
    elif isinstance(hyperparameter, str):
      res = ccs_evaluation_get_value_by_name(self.handle, str.encode(hyperparameter), ct.byref(v))
    else:
      res = ccs_evaluation_get_value(self.handle, hyperparameter, ct.byref(v))
    Error.check(res)
    return v.value

  @property
  def num_values(self):
    if hasattr(self, "_num_values"):
      return self._num_values
    v = ct.c_size_t()
    res = ccs_evaluation_get_values(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_values = v.value
    return self._num_values

  @property
  def values(self):
    sz = self.num_values
    if sz == 0:
      return []
    v = (ccs_datum * sz)()
    res = ccs_evaluation_get_values(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]

  @property
  def num_objective_values(self):
    if hasattr(self, "_num_objective_values"):
      return self._num_objective_values
    v = ct.c_size_t()
    res = ccs_evaluation_get_objective_values(self.handle, 0, None, ct.byref(v))
    Error.check(res)
    self._num_objective_values = v.value
    return self._num_objective_values

  @property
  def objective_values(self):
    sz = self.num_objective_values
    if sz == 0:
      return []
    v = (ccs_datum * sz)()
    res = ccs_evaluation_get_objective_values(self.handle, sz, v, None)
    Error.check(res)
    return [x.value for x in v]

  def cmp(self, other):
    v = ccs_comparison()
    res = ccs_evaluation_cmp(self.handle, other.handle, ct.byref(v))
    Error.check(res)
    return v
