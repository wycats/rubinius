class Fixnum < Integer

  def self.===(obj)
    Ruby.primitive :fixnum_s_eqq
    super
  end

  # unary operators

  def ~
    Ruby.primitive :fixnum_invert
    raise PrimitiveFailure, "Fixnum#~ primitive failed"
  end

  def -@
    Ruby.primitive :fixnum_neg
    raise PrimitiveFailure, "Fixnum#-@ primitive failed"
  end

  # binary math operators

  def +(o)
    Ruby.primitive :fixnum_add
    super(o)
  end

  def -(o)
    Ruby.primitive :fixnum_sub
    super(o)
  end

  def *(o)
    Ruby.primitive :fixnum_mul
    super(o)
  end

  # this method is aliased to / in core
  # see README-DEVELOPERS regarding safe math compiler plugin
  def divide(o)
    Ruby.primitive :fixnum_div
    super(o)
  end

  # Must be it's own method, so that super calls the correct method
  # on Numeric
  def div(o)
    Ruby.primitive :fixnum_div
    super(o)
  end

  def %(o)
    Ruby.primitive :fixnum_mod
    super(o)
  end

  def divmod(other)
    Ruby.primitive :fixnum_divmod
    super(other)
  end

  # bitwise binary operators

  def &(o)
    Ruby.primitive :fixnum_and
    if o.__kind_of__ Float
      raise RangeError, ("float %-.10g is out of range of a Fixnum" % o)
    end
    super(o)
  end

  def |(o)
    Ruby.primitive :fixnum_or
    if o.__kind_of__ Float
      raise RangeError, ("float %-.10g is out of range of a Fixnum" % o)
    end
    super(o)
  end

  def ^(o)
    Ruby.primitive :fixnum_xor
    if o.__kind_of__ Float
      raise RangeError, ("float %-.10g is out of range of a Fixnum" % o)
    end
    super(o)
  end

  def <<(o)
    Ruby.primitive :fixnum_left_shift
    super(o)
  end

  def >>(o)
    Ruby.primitive :fixnum_right_shift
    super(o)
  end

  def **(o)
    Ruby.primitive :fixnum_pow
    super(o)
  end

  def __bignum_new__(value)
    Ruby.primitive :bignum_new
    raise PrimitiveFailure, "primitive failed"
  end

  # comparison operators

  def ==(o)
    Ruby.primitive :fixnum_equal
    super(o)
  end

  def <=>(other)
    Ruby.primitive :fixnum_compare
    super(other)
  end

  def <(o)
    Ruby.primitive :fixnum_lt
    super(o)
  end

  def <=(o)
    Ruby.primitive :fixnum_le
    super(o)
  end

  def >(o)
    Ruby.primitive :fixnum_gt
    super(o)
  end

  def >=(o)
    Ruby.primitive :fixnum_ge
    super(o)
  end

  # predicates

  def zero?
    self == 0
  end

  # conversions

  def coerce(other)
    Ruby.primitive :fixnum_coerce
    super other
  end

  def to_s(base=10)
    based_to_s(base)
  end

  def based_to_s(base)
    Ruby.primitive :fixnum_to_s
    raise PrimitiveFailure, "Fixnum#based_to_s primitive failed"
  end

  def to_f
    Ruby.primitive :fixnum_to_f
    raise PrimitiveFailure, "Fixnum#to_f primitive failed"
  end

  def size
    Ruby.primitive :fixnum_size
    raise PrimitiveFailure, "Fixnum#size primitive failed"
  end

  def inspect
    based_to_s(10)
  end
end
