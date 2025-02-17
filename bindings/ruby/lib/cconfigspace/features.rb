module CCS

  attach_function :ccs_create_features, [:ccs_features_space_t, :size_t, :pointer, :pointer, :pointer], :ccs_result_t
  attach_function :ccs_features_check, [:ccs_features_t], :ccs_result_t

  class Features < Binding
    alias features_space context
    include Comparable

    def initialize(handle = nil, retain: false, auto_release: true,
                   features_space: nil,  values: nil, user_data: nil)
      if (handle)
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
        ptr = MemoryPointer::new(:ccs_features_t)
        res = CCS.ccs_create_features(features_space, count, values, user_data, ptr)
        CCS.error_check(res)
        super(ptr.read_ccs_features_t, retain: false)
      end
    end

    def self.from_handle(handle, retain: true, auto_release: true)
      self::new(handle, retain: retain, auto_release: auto_release)
    end

    def check
      res = CCS.ccs_features_check(@handle)
      CCS.error_check(res)
      self
    end

  end

end
