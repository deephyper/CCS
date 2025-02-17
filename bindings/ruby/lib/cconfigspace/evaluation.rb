module CCS
  Comparison = enum FFI::Type::INT32, :ccs_comparison_t, [
    :CCS_BETTER, -1,
    :CCS_EQUIVALENT, 0,
    :CCS_WORSE, 1,
    :CCS_NOT_COMPARABLE, 2
  ]
  class MemoryPointer
    def read_ccs_comparison_t
      Comparison.from_native(read_int32, nil)
    end
  end

  attach_function :ccs_create_evaluation, [:ccs_objective_space_t, :ccs_configuration_t, :ccs_result_t, :size_t, :pointer, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_evaluation_get_configuration, [:ccs_evaluation_t, :pointer], :ccs_result_t
  attach_function :ccs_evaluation_get_error, [:ccs_evaluation_t, :pointer], :ccs_result_t
  attach_function :ccs_evaluation_set_error, [:ccs_evaluation_t, :ccs_result_t], :ccs_result_t
  attach_function :ccs_evaluation_get_objective_value, [:ccs_evaluation_t, :size_t, :pointer], :ccs_result_t
  attach_function :ccs_evaluation_get_objective_values, [:ccs_evaluation_t, :size_t, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_evaluation_compare, [:ccs_evaluation_t, :ccs_evaluation_t, :pointer], :ccs_result_t
  attach_function :ccs_evaluation_check, [:ccs_evaluation_t], :ccs_result_t

  class Evaluation < Binding
    alias objective_space context
    add_handle_property :configuration, :ccs_configuration_t, :ccs_evaluation_get_configuration, memoize: true
    add_property :error, :ccs_result_t, :ccs_evaluation_get_error, memoize: false

    def initialize(handle = nil, retain: false, auto_release: true,
                   objective_space: nil, configuration: nil, error: :CCS_SUCCESS, values: nil, user_data: nil)
      if handle
        super(handle, retain: retain, auto_release: auto_release)
      else
        if values
          count = values.size
          raise CCSError, :CCS_INVALID_VALUE if count == 0
          p_values = MemoryPointer::new(:ccs_datum_t, count)
          values.each_with_index {  |v, i| Datum::new(p_values[i]).value = v }
          values = p_values
        else
          count = 0
        end
        ptr = MemoryPointer::new(:ccs_evaluation_t)
        res = CCS.ccs_create_evaluation(objective_space, configuration, error, count, values, user_data, ptr)
        CCS.error_check(res)
        super(ptr.read_ccs_evaluation_t, retain: false)
        @objective_space = objective_space
        @configuration = configuration
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def error=(err)
      res = CCS.ccs_evaluation_set_error(@handle, err)
      CCS.error_check(res)
      err
    end

    def num_objective_values
      @num_values ||= begin
        ptr = MemoryPointer::new(:size_t)
        res = CCS.ccs_evaluation_get_objective_values(@handle, 0, nil, ptr)
        CCS.error_check(res)
        ptr.read_size_t
      end
    end

    def objective_values
      count = num_values
      return [] if count == 0
      values = MemoryPointer::new(:ccs_datum_t, count)
      res = CCS.ccs_evaluation_get_objective_values(@handle, count, values, nil)
      CCS.error_check(res)
      count.times.collect { |i| Datum::new(values[i]).value }
    end

    def check
      res = CCS.ccs_evaluation_check(@handle)
      CCS.error_check(res)
      self
    end

    def compare(other)
      ptr = MemoryPointer::new(:ccs_comparison_t)
      res = CCS.ccs_evaluation_compare(@handle, other, ptr)
      CCS.error_check(res)
      ptr.read_ccs_comparison_t
    end

    def <=>(other)
      ptr = MemoryPointer::new(:ccs_comparison_t)
      res = CCS.ccs_evaluation_compare(@handle, other, ptr)
      CCS.error_check(res)
      r = ptr.read_int32
      r == 2 ? nil : r 
    end

  end
end
