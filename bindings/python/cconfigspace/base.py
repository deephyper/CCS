import ctypes as ct
from . import libcconfigspace

ccs_init = libcconfigspace.ccs_init
ccs_init.restype = ct.c_int

class Version(ct.Structure):
  _fields_ = [("revision", ct.c_ushort),
              ("patch",    ct.c_ushort),
              ("minor",    ct.c_ushort),
              ("major",    ct.c_ushort)]

# Base types
ccs_float  = ct.c_double
ccs_int    = ct.c_longlong
ccs_bool   = ct.c_int
ccs_result = ct.c_int
ccs_hash   = ct.c_uint
ccs_object = ct.c_void_p

# Objects
ccs_rng                 = ccs_object
ccs_distribution        = ccs_object
ccs_hyperparameter      = ccs_object
ccs_expression          = ccs_object
ccs_context             = ccs_object
ccs_configuration_space = ccs_object
ccs_configuration       = ccs_object
ccs_objective_space     = ccs_object
ccs_evaluation          = ccs_object
ccs_tuner               = ccs_object

ccs_false = 0
ccs_true = 1

# https://www.python-course.eu/python3_metaclasses.php
class Singleton(type):
    _instances = {}
    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]

class Inactive(metaclass=Singleton):
    pass

ccs_inactive = Inactive()

# derived and adapted from http://code.activestate.com/recipes/576415/
class CEnumerationType(type(ct.c_int)):
  def __new__(metacls, name, bases, dict):
    if not "_members_" in dict:
      raise ValueError("CEnumeration must define a _members_ attribute")
    last = -1
    if isinstance(dict["_members_"], list):
      _members_ = {}
      for item in dict["_members_"]:
        if isinstance(item, tuple):
          (i, v) = item
          _members_[i] = v
          last = v
        else:
          last += 1
          _members_[item] = last
      dict["_members_"] = _members_
    _reverse_members_ = {}
    for key,value in dict["_members_"].items():
      dict[key] = value
      _reverse_members_[value] = key
    dict["_reverse_members_"] = _reverse_members_
    cls = type(ct.c_int).__new__(metacls, name, bases, dict)
    for key,value in cls._members_.items():
      globals()[key] = value
    return cls

  def __contains__(self, value):
    return value in self._members_.values()

  def __repr__(self):
    return "<Enumeration %s>" % self.__name__

class CEnumeration(ct.c_int, metaclass=CEnumerationType):
  _members_ = {}
  def __init__(self, value):
    if value in self._reverse_members_:
        self.name = self._reverse_members_[value]
    else:
      raise ValueError("No enumeration member with value %r" % value)
    ct.c_int.__init__(self, value)

  @classmethod
  def from_param(cls, param):
    if isinstance(param, CEnumeration):
      if param.__class__ != cls:
        raise ValueError("Cannot mix enumeration members")
      else:
        return param
    else:
      return cls(param)

  def __repr__(self):
    return "<member %s=%d of %r>" % (self.name, self.value, self.__class__)

  def __str__(self):
    return "%s.%s" % (self.__class__.__name__, self.name)

class CEnumerationType64(type(ct.c_longlong)):
  def __new__(metacls, name, bases, dict):
    if not "_members_" in dict:
      raise ValueError("CEnumeration must define a _members_ attribute")
    last = -1
    if isinstance(dict["_members_"], list):
      _members_ = {}
      for item in dict["_members_"]:
        if isinstance(item, tuple):
          (i, v) = item
          _members_[i] = v
          last = v
        else:
          last += 1
          _members_[item] = last
      dict["_members_"] = _members_
    _reverse_members_ = {}
    for key,value in dict["_members_"].items():
      dict[key] = value
      _reverse_members_[value] = key
    dict["_reverse_members_"] = _reverse_members_
    cls = type(ct.c_longlong).__new__(metacls, name, bases, dict)
    for key,value in cls._members_.items():
      globals()[key] = value
    return cls

  def __contains__(self, value):
    return value in self._members_.values()

  def __repr__(self):
    return "<Enumeration %s>" % self.__name__

class CEnumeration64(ct.c_longlong, metaclass=CEnumerationType64):
  _members_ = {}
  def __init__(self, value):
    if value in self._reverse_members_:
        self._name = self._reverse_members_[value]
    else:
      raise ValueError("No enumeration member with value %r" % value)
    ct.c_longlong.__init__(self, value)

  def __repr__(self):
    return "<member %s=%d of %r>" % (self.name, self.value, self.__class__)

  def __str__(self):
    return "%s.%s" % (self.__class__.__name__, self.name)

  @property
  def name(self):
    if self.value in self._reverse_members_:
      return self._reverse_members_[self.value]
    else:
      raise ValueError("No enumeration member with value %r" % value)

  @classmethod
  def from_param(cls, param):
    if isinstance(param, CEnumeration):
      if param.__class__ != cls:
        raise ValueError("Cannot mix enumeration members")
      else:
        return param
    else:
      return cls(param)

class ccs_object_type(CEnumeration):
  _members_ = [
    ('RNG', 0),
    'DISTRIBUTION',
    'HYPERPARAMETER',
    'EXPRESSION',
    'CONFIGURATION_SPACE',
    'CONFIGURATION',
    'OBJECTIVE_SPACE',
    'EVALUATION',
    'TUNER' ]

class ccs_error(CEnumeration):
  _members_ = [ 
    ('SUCCESS', 0),
    'INVALID_OBJECT',
    'INVALID_VALUE',
    'INVALID_TYPE',
    'INVALID_SCALE',
    'INVALID_DISTRIBUTION',
    'INVALID_EXPRESSION',
    'INVALID_HYPERPARAMETER',
    'INVALID_CONFIGURATION',
    'INVALID_NAME',
    'INVALID_CONDITION',
    'INVALID_TUNER',
    'INVALID_GRAPH',
    'TYPE_NOT_COMPARABLE',
    'INVALID_BOUNDS',
    'OUT_OF_BOUNDS',
    'SAMPLING_UNSUCCESSFUL',
    'INACTIVE_HYPERPARAMETER',
    'OUT_OF_MEMORY',
    'UNSUPPORTED_OPERATION' ]

class ccs_data_type(CEnumeration64):
  _members_ = [
    ('NONE', 0),
    'INTEGER',
    'FLOAT',
    'BOOLEAN',
    'STRING',
    'INACTIVE',
    'OBJECT' ]
 
class ccs_numeric_type(CEnumeration64):
  _members_ = [
    ('NUM_INTEGER', ccs_data_type.INTEGER),
    ('NUM_FLOAT', ccs_data_type.FLOAT) ]

class Numeric(ct.Union):
  _fields_ = [('f', ccs_float),
              ('i', ccs_int)]

  def get_value(self, t):
    if t == ccs_numeric_type.NUM_INTEGER:
      return self.f
    elif t == ccs_numeric_type.NUM_FLOAT:
      return self.v
    else:
      raise Error(ccs_error.INVALID_VALUE)

  def set_value(self, v):
    if isinstance(v, int):
      self.i = v
    elif isinstance(v, float):
      self.f = v
    else:
      raise Error(ccs_error.INVALID_VALUE)

class Value(ct.Union):
  _fields_ = [('f', ccs_float),
              ('i', ccs_int),
              ('s', ct.c_char_p),
              ('o', ccs_object)]

class Datum(ct.Structure):
  _fields_ = [('_value', Value),
              ('type', ccs_data_type)]

  def __init__(self, v = None):
    super().__init__()
    self._string = None
    self._object = None
    self.value = v

  @property
  def value(self):
    if self.type.value == ccs_data_type.NONE:
      return None
    elif self.type.value == ccs_data_type.INTEGER:
      return self._value.i
    elif self.type.value == ccs_data_type.FLOAT:
      return self._value.f
    elif self.type.value == ccs_data_type.BOOLEAN:
      return False if self._value.i == ccs_false else True
    elif self.type.value == ccs_data_type.STRING:
      return self._value.s.decode()
    elif self.type.value == ccs_data_type.INACTIVE:
      return ccs_inactive
    elif self.type.value == ccs_data_type.OBJECT:
      return Object.from_handle(ct.c_void_p(self._value.o))
    else:
      raise Error(ccs_error.INVALID_VALUE)

  @value.setter
  def value(self, v):
    self._string = None
    self._object = None
    if v is None:
      self.type.value = ccs_data_type.NONE
      self._value.i = 0
    elif isinstance(v, bool):
      self.type.value = ccs_data_type.BOOLEAN
      self._value.i = 1 if v else 0
    elif isinstance(v, int):
      self.type.value = ccs_data_type.INTEGER
      self._value.i = v
    elif isinstance(v, float):
      self.type.value = ccs_data_type.FLOAT
      self._value.f = v
    elif isinstance(v, str):
      self.type.value = ccs_data_type.STRING
      self._string = str.encode(v)
      self._value.s = ct.c_char_p(self._string)
    elif v is ccs_inactive:
      self.type.value = ccs_data_type.INACTIVE
      self._value.i = 0
    elif isinstance(v, Object):
      self.type.value = ccs_data_type.OBJECT
      self_object = v
      self._value.o = v.handle
    else:
      raise Error(ccs_error.INVALID_VALUE)

class Error(Exception):
  def __init__(self, message):
    self.message = message

  @classmethod
  def check(cls, err):
    if err < 0:
      raise cls(ccs_error(-err))

ccs_get_version = libcconfigspace.ccs_get_version
ccs_get_version.restype = Version

ccs_retain_object = libcconfigspace.ccs_retain_object
ccs_retain_object.restype = ccs_result
ccs_retain_object.argtypes = [ccs_object]

ccs_release_object = libcconfigspace.ccs_release_object
ccs_release_object.restype = ccs_result
ccs_release_object.argtypes = [ccs_object]

ccs_object_get_type = libcconfigspace.ccs_object_get_type
ccs_object_get_type.restype = ccs_result
ccs_object_get_type.argtypes = [ccs_object, ct.POINTER(ccs_object_type)]

ccs_object_get_refcount = libcconfigspace.ccs_object_get_refcount
ccs_object_get_refcount.restype = ccs_result
ccs_object_get_refcount.argtypes = [ccs_object, ct.POINTER(ct.c_int)]

class Object:
  def __init__(self, handle, retain = False, auto_release = True):
    if handle is None:
      raise Error(ccs_error.INVALID_OBJECT)
    self.handle = handle
    self.auto_release = auto_release
    if retain:
      res = ccs_retain_object(handle)
      Error.check(res)

  def __del__(self):
    res = ccs_release_object(self.handle)
    Error.check(res)

  def object_type(self):
    t = ccs_object_type(0)
    res = ccs_object_get_type(self.handle, ct.byref(t))
    Error.check(res)
    return t

  def refcount(self):
    c = ct.c_int(0)
    res = ccs_object_get_refcount(self.handle, ct.byref(c))
    Error.check(res)
    return c.value

  @classmethod
  def from_handle(cls, h):
    t = ccs_object_type(0)
    res = ccs_object_get_type(h, ct.byref(t))
    Error.check(res)
    if t.value == ccs_object_type.RNG:
      from .rng import Rng
      return Rng.from_handle(h)
    else:
      raise Error(ccs_error.INVALID_OBJECT)