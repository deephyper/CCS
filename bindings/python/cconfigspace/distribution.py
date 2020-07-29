import ctypes as ct
from . import libcconfigspace
from .base import Object, Error, ccs_error, ccs_int, ccs_float, ccs_bool, ccs_result, ccs_rng, ccs_distribution, ccs_numeric_type, ccs_numeric, CEnumeration, NUM_FLOAT, NUM_INTEGER, _ccs_get_function
from .interval import ccs_interval

class ccs_distribution_type(CEnumeration):
  _members_ = [
    ('UNIFORM', 0),
    'NORMAL',
    'ROULETTE' ]

class ccs_scale_type(CEnumeration):
  _members_ = [
    ('LINEAR', 0),
    'LOGARITHMIC' ]

ccs_distribution_get_type = _ccs_get_function("ccs_distribution_get_type", [ccs_distribution, ct.POINTER(ccs_distribution_type)])
ccs_distribution_get_data_type = _ccs_get_function("ccs_distribution_get_data_type", [ccs_distribution, ct.POINTER(ccs_numeric_type)])
ccs_distribution_get_dimension = _ccs_get_function("ccs_distribution_get_dimension", [ccs_distribution, ct.POINTER(ct.c_size_t)])
ccs_distribution_get_scale_type = _ccs_get_function("ccs_distribution_get_scale_type", [ccs_distribution, ct.POINTER(ccs_scale_type)])
ccs_distribution_get_quantization = _ccs_get_function("ccs_distribution_get_quantization", [ccs_distribution, ct.POINTER(ccs_numeric)])
ccs_distribution_get_bounds = _ccs_get_function("ccs_distribution_get_bounds", [ccs_distribution, ct.POINTER(ccs_interval)])
ccs_distribution_check_oversampling = _ccs_get_function("ccs_distribution_check_oversampling", [ccs_distribution, ct.POINTER(ccs_interval), ct.POINTER(ccs_bool)])
ccs_distribution_sample = _ccs_get_function("ccs_distribution_sample", [ccs_distribution, ccs_rng, ct.POINTER(ccs_numeric)])
ccs_distribution_samples = _ccs_get_function("ccs_distribution_samples", [ccs_distribution, ccs_rng, ct.c_size_t, ct.POINTER(ccs_numeric)])

class Distribution(Object):

  @classmethod
  def from_handle(cls, handle):
    v = ccs_distribution_type(0)
    res = ccs_distribution_get_type(handle, ct.byref(v))
    Error.check(res)
    v = v.value
    if v == ccs_distribution_type.UNIFORM:
      return UniformDistribution(handle = handle, retain = True)
    elif v == ccs_distribution_type.NORMAL:
      return NormalDistribution(handle = handle, retain = True)
    elif v == ccs_distribution_type.ROULETTE:
      return RouletteDistribution(handle = handle, retain = True)
    else:
      raise Error(ccs_error.INVALID_DISTRIBUTION)

  @property
  def type(self):
    if hasattr(self, "_type"):
      return self._type
    v = ccs_distribution_type(0)
    res = ccs_distribution_get_type(self.handle, ct.byref(v))
    Error.check(res)
    self._type = v
    return v

  @property
  def data_type(self):
    if hasattr(self, "_data_type"):
      return self._data_type
    v = ccs_numeric_type(0)
    res = ccs_distribution_get_data_type(self.handle, ct.byref(v))
    Error.check(res)
    self._data_type = v
    return v

  @property
  def dimension(self):
    if hasattr(self, "_dimension"):
      return self._dimension
    v = ct.c_size_t()
    res = ccs_distribution_get_dimension(self.handle, ct.byref(v))
    Error.check(res)
    self._dimension = v.value
    return v.value

  @property
  def scale_type(self):
    if hasattr(self, "_scale_type"):
      return self._scale_type
    v = ccs_scale_type(0)
    res = ccs_distribution_get_scale_type(self.handle, ct.byref(v))
    Error.check(res)
    self._scale_type = v
    return v

  @property
  def quantization(self):
    if hasattr(self, "_quantization"):
      return self._quantization
    v = ccs_numeric(0)
    res = ccs_distribution_get_quantization(self.handle, ct.byref(v))
    Error.check(res)
    t = self.data_type.value
    if t == ccs_numeric_type.NUM_INTEGER:
      self._quantization = v.i
    elif t == ccs_numeric_type.NUM_FLOAT:
      self._quantization = v.f
    else:
      raise Error(ccs_error.INVALID_VALUE)
    return self._quantization

  @property
  def bounds(self):
    if hasattr(self, "_bounds"):
      return self._bounds
    v = ccs_interval()
    res = ccs_distribution_get_bounds(self.handle, ct.byref(v))
    Error.check(res)
    self._bounds = v
    return v

  def oversampling(self, interval):
    v = ccs_bool()
    res = ccs_distribution_check_oversampling(self.handle, ct.byref(interval), ct.byref(v))
    Error.check(res)
    return False if v.value == ccs_false else True

  def sample(self, rng):
    v = ccs_numeric()
    res = ccs_distribution_sample(self.handle, rng.handle, ct.byref(v))
    Error.check(res)
    t = self.data_type.value
    if t == ccs_numeric_type.NUM_INTEGER:
      return v.i
    elif t == ccs_numeric_type.NUM_FLOAT:
      return v.f
    else:
      raise Error(ccs_error.INVALID_VALUE)
    
  def samples(self, rng, count):
    t = self.data_type.value
    if t == ccs_numeric_type.NUM_INTEGER:
      v = (ccs_int * count)()
    elif t == ccs_numeric_type.NUM_FLOAT:
      v = (ccs_float * count)()
    else:
      raise Error(ccs_error.INVALID_VALUE)
    res = ccs_distribution_samples(self.handle, rng.handle, count, ct.cast(v, ct.POINTER(ccs_numeric)))
    Error.check(res)
    return list(v)

ccs_create_uniform_distribution = _ccs_get_function("ccs_create_uniform_distribution", [ccs_numeric_type, ccs_int, ccs_int, ccs_scale_type, ccs_int, ct.POINTER(ccs_distribution)])
ccs_create_uniform_int_distribution = _ccs_get_function("ccs_create_uniform_int_distribution", [ccs_int, ccs_int, ccs_scale_type, ccs_int, ct.POINTER(ccs_distribution)])
ccs_create_uniform_float_distribution = _ccs_get_function("ccs_create_uniform_float_distribution", [ccs_float, ccs_float, ccs_scale_type, ccs_float, ct.POINTER(ccs_distribution)])
ccs_uniform_distribution_get_parameters = _ccs_get_function("ccs_uniform_distribution_get_parameters", [ccs_distribution, ct.POINTER(ccs_numeric), ct.POINTER(ccs_numeric)])

class UniformDistribution(Distribution):
  def __init__(self, handle = None, retain = False, data_type = NUM_FLOAT, lower = 0.0, upper = 1.0, scale = ccs_scale_type.LINEAR, quantization = 0.0):
    if handle is None:
      handle = ccs_distribution(0)
      if data_type == NUM_FLOAT:
        res = ccs_create_uniform_float_distribution(lower, upper, scale, quantization, ct.byref(handle))
      elif data_type == NUM_INTEGER:
        res = ccs_create_uniform_int_distribution(lower, upper, scale, quantization, ct.byref(handle))
      else:
        raise Error(ccs_error.INVALID_VALUE)
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)

  @classmethod
  def int(cls, lower, upper, scale = ccs_scale_type.LINEAR, quantization = 0):
    return cls(data_type = NUM_INTEGER, lower = lower, upper = upper, scale = scale, quantization = quantization)

  @classmethod
  def float(cls, lower, upper, scale = ccs_scale_type.LINEAR, quantization = 0.0):
    return cls(data_type = NUM_FLOAT, lower = lower, upper = upper, scale = scale, quantization = quantization)

  @property
  def lower(self):
    if hasattr(self, "_lower"):
      return self._lower
    v = ccs_numeric()
    res = ccs_uniform_distribution_get_parameters(self.handle, ct.byref(v), None)
    Error.check(res)
    t = self.data_type.value
    if t == ccs_numeric_type.NUM_INTEGER:
      self._lower = v.i
    elif t == ccs_numeric_type.NUM_FLOAT:
      self._lower = v.f
    else:
      raise Error(ccs_error.INVALID_VALUE)
    return self._lower

  @property
  def upper(self):
    if hasattr(self, "_upper"):
      return self._upper
    v = ccs_numeric()
    res = ccs_uniform_distribution_get_parameters(self.handle, None, ct.byref(v))
    Error.check(res)
    t = self.data_type.value
    if t == ccs_numeric_type.NUM_INTEGER:
      self._upper = v.i
    elif t == ccs_numeric_type.NUM_FLOAT:
      self._upper = v.f
    else:
      raise Error(ccs_error.INVALID_VALUE)
    return self._upper

ccs_create_normal_distribution = _ccs_get_function("ccs_create_normal_distribution", [ccs_numeric_type, ccs_float, ccs_float, ccs_scale_type, ccs_int, ct.POINTER(ccs_distribution)])
ccs_create_normal_int_distribution = _ccs_get_function("ccs_create_normal_int_distribution", [ccs_float, ccs_float, ccs_scale_type, ccs_int, ct.POINTER(ccs_distribution)])
ccs_create_normal_float_distribution = _ccs_get_function("ccs_create_normal_float_distribution", [ccs_float, ccs_float, ccs_scale_type, ccs_float, ct.POINTER(ccs_distribution)])
ccs_normal_distribution_get_parameters = _ccs_get_function("ccs_normal_distribution_get_parameters", [ccs_distribution, ct.POINTER(ccs_float), ct.POINTER(ccs_float)])

class NormalDistribution(Distribution):
  def __init__(self, handle = None, retain = False, data_type = NUM_FLOAT, mu = 0.0, sigma = 1.0, scale = ccs_scale_type.LINEAR, quantization = 0.0):
    if handle is None:
      handle = ccs_distribution(0)
      if data_type == NUM_FLOAT:
        res = ccs_create_normal_float_distribution(mu, sigma, scale, quantization, ct.byref(handle))
      elif data_type == NUM_INTEGER:
        res = ccs_create_normal_int_distribution(mu, sigma, scale, quantization, ct.byref(handle))
      else:
        raise Error(ccs_error.INVALID_VALUE)
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)

  @property
  def mu(self):
    if hasattr(self, "_mu"):
      return self._mu
    v = ccs_float()
    res = ccs_normal_distribution_get_parameters(self.handle, ct.byref(v), None)
    Error.check(res)
    t = self.data_type.value
    self._mu = v.value
    return self._mu

  @property
  def sigma(self):
    if hasattr(self, "_sigma"):
      return self._sigma
    v = ccs_float()
    res = ccs_normal_distribution_get_parameters(self.handle, None, ct.byref(v))
    Error.check(res)
    t = self.data_type.value
    self._sigma = v.value
    return self._sigma

ccs_create_roulette_distribution = _ccs_get_function("ccs_create_roulette_distribution", [ct.c_size_t, ct.POINTER(ccs_float), ct.POINTER(ccs_distribution)])
ccs_roulette_distribution_get_num_areas = _ccs_get_function("ccs_roulette_distribution_get_num_areas", [ccs_distribution, ct.POINTER(ct.c_size_t)])
ccs_roulette_distribution_get_areas = _ccs_get_function("ccs_roulette_distribution_get_areas", [ccs_distribution, ct.c_size_t, ct.POINTER(ccs_float), ct.POINTER(ct.c_size_t)])

class RouletteDistribution(Distribution):
  def __init__(self, handle = None, retain = False, areas = []):
    if handle is None:
      handle = ccs_distribution(0)
      v = (ccs_float * len(areas))(*areas)
      res = ccs_create_roulette_distribution(len(areas), v, ct.byref(handle))
      Error.check(res)
      super().__init__(handle = handle, retain = False)
    else:
      super().__init__(handle = handle, retain = retain)

  @property
  def num_areas(self):
    if hasattr(self, "_num_areas"):
      return self._num_areas
    v = ct.c_size_t()
    res = ccs_roulette_distribution_get_num_areas(self.handle, ct.byref(v))
    Error.check(res)
    self._num_areas = v.value
    return self._num_areas

  @property
  def areas(self):
    if hasattr(self, "_areas"):
      return self._areas
    v = (ccs_float * self.num_areas)()
    res = ccs_roulette_distribution_get_areas(self.handle, self.num_areas, v, None)
    Error.check(res)
    self._areas = list(v)
    return self._areas

