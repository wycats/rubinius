#ifndef RBX_SUBTEND_RUBY_H
#define RBX_SUBTEND_RUBY_H

/**
 *  @file
 *
 *  Notes:
 *
 *    - The function prefix capi_* is used for functions that implement
 *      the Ruby C-API but should NEVER be used in a C extension's code.
 *
 *      Just in case, that means NEVER, like NOT EVER. If you do, we'll
 *      call your mother.
 *
 *  @todo Blocks/iteration. rb_iterate normally uses fptrs, could
 *        maybe do that or then support 'function objects' --rue
 *
 *  @todo Const correctness. --rue
 *
 *  @todo Add some type of checking for too-long C strings etc? --rue
 *
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/select.h>

// A number of extensions expect these to be already included
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "intern.h"

#define RUBY
#define RUBINIUS

#define RUBY_COMPAT_LEVEL 2

#ifdef __cplusplus
# ifndef  HAVE_PROTOTYPES
#  define HAVE_PROTOTYPES 1
# endif
# ifndef  HAVE_STDARG_PROTOTYPES
#  define HAVE_STDARG_PROTOTYPES 1
# endif
#endif

#undef _
#ifdef HAVE_PROTOTYPES
# define _(args) args
#else
# define _(args) ()
#endif

#undef __
#ifdef HAVE_STDARG_PROTOTYPES
# define __(args) args
#else
# define __(args) ()
#endif

#ifdef __cplusplus
#define ANYARGS ...
#else
#define ANYARGS
#endif

#ifndef NORETURN
#define NORETURN(x) x
#endif

#ifdef __STDC__
# include <limits.h>
#else
# ifndef LONG_MAX
#  ifdef HAVE_LIMITS_H
#   include <limits.h>
#  else
    /* assuming 32bit(2's compliment) long */
#   define LONG_MAX 2147483647
#  endif
# endif
# ifndef LONG_MIN
#  define LONG_MIN (-LONG_MAX-1)
# endif
# ifndef CHAR_BIT
#  define CHAR_BIT 8
# endif
#endif

#ifndef RUBY_EXTERN
#define RUBY_EXTERN extern
#endif

void* XMALLOC(size_t bytes);
void  XFREE(void* ptr);
void* XREALLOC(void* ptr, size_t bytes);
void* XCALLOC(size_t items, size_t bytes);

#define xmalloc   XMALLOC
#define xcalloc   XCALLOC
#define xrealloc  XREALLOC
#define xfree     XFREE

#define ruby_xmalloc   xmalloc
#define ruby_xcalloc   xcalloc
#define ruby_xrealloc  xrealloc
#define ruby_xfree     xfree

/* need to include <ctype.h> to use these macros */
#ifndef ISPRINT
#define ISASCII(c) isascii((int)(unsigned char)(c))
#undef ISPRINT
#define ISPRINT(c) (ISASCII(c) && isprint((int)(unsigned char)(c)))
#define ISSPACE(c) (ISASCII(c) && isspace((int)(unsigned char)(c)))
#define ISUPPER(c) (ISASCII(c) && isupper((int)(unsigned char)(c)))
#define ISLOWER(c) (ISASCII(c) && islower((int)(unsigned char)(c)))
#define ISALNUM(c) (ISASCII(c) && isalnum((int)(unsigned char)(c)))
#define ISALPHA(c) (ISASCII(c) && isalpha((int)(unsigned char)(c)))
#define ISDIGIT(c) (ISASCII(c) && isdigit((int)(unsigned char)(c)))
#define ISXDIGIT(c) (ISASCII(c) && isxdigit((int)(unsigned char)(c)))
#endif

/**
 *  In MRI, VALUE represents an object.
 *
 *  In rbx, this is a Handle.
 */
#ifdef VALUE
#undef VALUE
#endif

#define VALUE intptr_t

/**
 *  In MRI, ID represents an interned string, i.e. a Symbol.
 *
 *  In rbx, this is a Symbol pointer.
 */
#define ID    intptr_t

/**
 * In MRI, RUBY_DATA_FUNC is used for the mark and free functions in
 * Data_Wrap_Struct and Data_Make_Struct.
 */
typedef void (*RUBY_DATA_FUNC)(void*);

/* "Stash" the real versions. */
#define RBX_Qfalse      (reinterpret_cast<Object*>(0x0aUL))
#define RBX_Qnil        (reinterpret_cast<Object*>(0x1aUL))
#define RBX_Qtrue       (reinterpret_cast<Object*>(0x12UL))
#define RBX_Qundef      (reinterpret_cast<Object*>(0x22UL))

#define RBX_FALSE_P(o)  (reinterpret_cast<Object*>((o)) == RBX_Qfalse)
#define RBX_TRUE_P(o)   (reinterpret_cast<Object*>((o)) == RBX_Qtrue)
#define RBX_NIL_P(o)    (reinterpret_cast<Object*>((o)) == RBX_Qnil)
#define RBX_UNDEF_P(o)  (reinterpret_cast<Object*>((o)) == RBX_Qundef)

#define RBX_RTEST(o)    ((reinterpret_cast<uintptr_t>(o) & 0xf) != 0xa)


/* Reset relative to our VALUEs */
#undef Qfalse
#undef Qtrue
#undef Qnil
#undef Qundef

#undef ALLOC
#undef ALLOC_N
#undef ALLOCA_N
#undef REALLOC_N
#undef FIXNUM_P
#undef REFERENCE_P
#undef NIL_P
#undef RTEST

#define RUBY_METHOD_FUNC(func) ((VALUE (*)(ANYARGS))func)

/** Global class/object etc. types. */
#ifdef __cplusplus
extern "C" {
#endif

  /**
   *  Global object abstraction.
   *
   *  @internal.
   */
  typedef enum {
    cCApiArray = 0,
    cCApiBignum,
    cCApiClass,
    cCApiComparable,
    cCApiData,
    cCApiEnumerable,
    cCApiFalse,
    cCApiFile,
    cCApiFixnum,
    cCApiFloat,
    cCApiHash,
    cCApiInteger,
    cCApiIO,
    cCApiKernel,
    cCApiMatch,
    cCApiModule,
    cCApiNil,
    cCApiNumeric,
    cCApiObject,
    cCApiRange,
    cCApiRegexp,
    cCApiRubinius,
    cCApiString,
    cCApiStruct,
    cCApiSymbol,
    cCApiThread,
    cCApiTime,
    cCApiTrue,
    cCApiProc,

    cCApiArgumentError,
    cCApiEOFError,
    cCApiErrno,
    cCApiException,
    cCApiFatal,
    cCApiFloatDomainError,
    cCApiIndexError,
    cCApiInterrupt,
    cCApiIOError,
    cCApiLoadError,
    cCApiLocalJumpError,
    cCApiNameError,
    cCApiNoMemoryError,
    cCApiNoMethodError,
    cCApiNotImplementedError,
    cCApiRangeError,
    cCApiRegexpError,
    cCApiRuntimeError,
    cCApiScriptError,
    cCApiSecurityError,
    cCApiSignalException,
    cCApiStandardError,
    cCApiSyntaxError,
    cCApiSystemCallError,
    cCApiSystemExit,
    cCApiSystemStackError,
    cCApiTypeError,
    cCApiThreadError,
    cCApiZeroDivisionError,

    // MUST be last
    cCApiMaxConstant
  } CApiConstant;


  /**
   *  Integral type map for MRI's types.
   *
   *  Rubinius does not implement all of these,
   *  so T_OBJECT is returned instead in those
   *  cases.
   */
  typedef enum {
    T_ARRAY,
    T_NONE,
    T_NIL,
    T_OBJECT,
    T_CLASS,
    T_ICLASS,
    T_MODULE,
    T_FLOAT,
    T_STRING,
    T_REGEXP,
    T_FIXNUM,
    T_HASH,
    T_STRUCT,
    T_BIGNUM,
    T_FILE,
    T_TRUE,
    T_FALSE,
    T_DATA,
    T_MATCH,
    T_SYMBOL,
    T_BLKTAG,
    T_UNDEF,
    T_VARMAP,
    T_SCOPE,
    T_NODE

  } CApiType;

  /**
   *  Method variants that can be defined.
   */
  typedef enum {
    cCApiPublicMethod,
    cCApiProtectedMethod,
    cCApiPrivateMethod,
    cCApiSingletonMethod

  } CApiMethodKind;

struct RString {
  size_t len;
  char *ptr;
  char *dmwmb;
  union {
    size_t capa;
    VALUE shared;
  } aux;
};

#define RSTRING(str)    capi_rstring_struct(str)

struct RArray {
  size_t len;
  union {
    size_t capa;
    VALUE shared;
  } aux;
  VALUE *ptr;
  VALUE *dmwmb;
};

#define RARRAY(ary)     capi_rarray_struct(ary)

struct RData {
  void (*dmark)(void*);
  void (*dfree)(void*);
  void *data;
};

#define RDATA(d)        capi_rdata_struct(d)

struct RFloat {
  double value;
};

#define RFLOAT(d)       capi_rfloat_struct(d)

// To provide nicer error reporting
#define RHASH(obj) assert(???? && "RHASH() is not supported")

/*
 * The immediates.
 */

#define cCApiQfalse     (0x00)
#define cCApiQtrue      (0x22)
#define cCApiQnil       (0x42)
#define cCApiQundef     (0x62)

/** The false object.
 *
 * NOTE: This is defined to be 0 because it is 0 in MRI and
 * extensions written for MRI have (absolutely wrongly,
 * infuriatingly, but-what-can-you-do-now) relied on that
 * assumption in boolean contexts. Rather than fighting a
 * myriad subtle bugs, we just go along with it.
 */
#define Qfalse ((VALUE)cCApiQfalse)
/** The true object. */
#define Qtrue  ((VALUE)cCApiQtrue)
/** The nil object. */
#define Qnil   ((VALUE)cCApiQnil)
/** The undef object. NEVER EXPOSE THIS TO USER CODE. EVER. */
#define Qundef ((VALUE)cCApiQundef)

#define ruby_verbose (rb_gv_get("$VERBOSE"))
#define ruby_debug   (rb_gv_get("$DEBUG"))

/* Global Class objects */

#define rb_cArray             (capi_get_constant(cCApiArray))
#define rb_cBignum            (capi_get_constant(cCApiBignum))
#define rb_cClass             (capi_get_constant(cCApiClass))
#define rb_cData              (capi_get_constant(cCApiData))
#define rb_cFalseClass        (capi_get_constant(cCApiFalse))
#define rb_cFile              (capi_get_constant(cCApiFile))
#define rb_cFixnum            (capi_get_constant(cCApiFixnum))
#define rb_cFloat             (capi_get_constant(cCApiFloat))
#define rb_cHash              (capi_get_constant(cCApiHash))
#define rb_cInteger           (capi_get_constant(cCApiInteger))
#define rb_cIO                (capi_get_constant(cCApiIO))
#define rb_cMatch             (capi_get_constant(cCApiMatch))
#define rb_cModule            (capi_get_constant(cCApiModule))
#define rb_cNilClass          (capi_get_constant(cCApiNil))
#define rb_cNumeric           (capi_get_constant(cCApiNumeric))
#define rb_cObject            (capi_get_constant(cCApiObject))
#define rb_cRange             (capi_get_constant(cCApiRange))
#define rb_cRegexp            (capi_get_constant(cCApiRegexp))
#define rb_mRubinius          (capi_get_constant(cCApiRubinius))
#define rb_cString            (capi_get_constant(cCApiString))
#define rb_cStruct            (capi_get_constant(cCApiStruct))
#define rb_cSymbol            (capi_get_constant(cCApiSymbol))
#define rb_cThread            (capi_get_constant(cCApiThread))
#define rb_cTime              (capi_get_constant(cCApiTime))
#define rb_cTrueClass         (capi_get_constant(cCApiTrue))
#define rb_cProc              (capi_get_constant(cCApiProc))

/* Global Module objects. */

#define rb_mComparable        (capi_get_constant(cCApiComparable))
#define rb_mEnumerable        (capi_get_constant(cCApiEnumerable))
#define rb_mKernel            (capi_get_constant(cCApiKernel))


/* Exception classes. */

#define rb_eArgError          (capi_get_constant(cCApiArgumentError))
#define rb_eEOFError          (capi_get_constant(cCApiEOFError))
#define rb_mErrno             (capi_get_constant(cCApiErrno))
#define rb_eException         (capi_get_constant(cCApiException))
#define rb_eFatal             (capi_get_constant(cCApiFatal))
#define rb_eFloatDomainError  (capi_get_constant(cCApiFloatDomainError))
#define rb_eIndexError        (capi_get_constant(cCApiIndexError))
#define rb_eInterrupt         (capi_get_constant(cCApiInterrupt))
#define rb_eIOError           (capi_get_constant(cCApiIOError))
#define rb_eLoadError         (capi_get_constant(cCApiLoadError))
#define rb_eLocalJumpError    (capi_get_constant(cCApiLocalJumpError))
#define rb_eNameError         (capi_get_constant(cCApiNameError))
#define rb_eNoMemError        (capi_get_constant(cCApiNoMemoryError))
#define rb_eNoMethodError     (capi_get_constant(cCApiNoMethodError))
#define rb_eNotImpError       (capi_get_constant(cCApiNotImplementedError))
#define rb_eRangeError        (capi_get_constant(cCApiRangeError))
#define rb_eRegexpError       (capi_get_constant(cCApiRegexpError))
#define rb_eRuntimeError      (capi_get_constant(cCApiRuntimeError))
#define rb_eScriptError       (capi_get_constant(cCApiScriptError))
#define rb_eSecurityError     (capi_get_constant(cCApiSecurityError))
#define rb_eSignal            (capi_get_constant(cCApiSignalException))
#define rb_eStandardError     (capi_get_constant(cCApiStandardError))
#define rb_eSyntaxError       (capi_get_constant(cCApiSyntaxError))
#define rb_eSystemCallError   (capi_get_constant(cCApiSystemCallError))
#define rb_eSystemExit        (capi_get_constant(cCApiSystemExit))
#define rb_eSysStackError     (capi_get_constant(cCApiSystemStackError))
#define rb_eTypeError         (capi_get_constant(cCApiTypeError))
#define rb_eThreadError       (capi_get_constant(cCApiThreadError))
#define rb_eZeroDivError      (capi_get_constant(cCApiZeroDivisionError))


/* Interface macros */

/** Allocate memory for type. Must NOT be used to allocate Ruby objects. */
#define ALLOC(type)       (type*)malloc(sizeof(type))

/** Allocate memory for N of type. Must NOT be used to allocate Ruby objects. */
#define ALLOC_N(type, n)  (type*)malloc(sizeof(type) * (n))

/** Allocate memory for N of type in the stack frame of the caller. */
#define ALLOCA_N(type,n)  (type*)alloca(sizeof(type)*(n))

/** Reallocate memory allocated with ALLOC or ALLOC_N. */
#define REALLOC_N(ptr, type, n) (ptr)=(type*)realloc(ptr, sizeof(type) * (n));

/** Interrupt checking (no-op). */
#define CHECK_INTS        /* No-op */

#define FIXNUM_FLAG       0x1

/** True if the value is a Fixnum. */
#define FIXNUM_P(f)       (((long)(f))&FIXNUM_FLAG)

/** Convert a Fixnum to a long int. */
#define FIX2LONG(x)       (((long)(x)) >> 1)

/** Convert a Fixnum to an unsigned long int. */
#define FIX2ULONG(x)      (((unsigned long)(x))>>1)

/** Convert a VALUE into a long int. */
#define NUM2LONG(x)       (FIXNUM_P(x)?FIX2LONG(x):rb_num2long((VALUE)x))

/** Convert a VALUE into a long int. */
#define NUM2ULONG(x)      rb_num2ulong((VALUE)x)

/** Convert a VALUE into an int. */
#define NUM2INT(x)        ((int)NUM2LONG(x))

/** Convert a VALUE into a long int. */
#define NUM2UINT(x)       ((unsigned int)NUM2ULONG(x))

/** Convert a Fixnum into an int. */
#define FIX2INT(x)        ((int)FIX2LONG(x))

/** Convert a Fixnum into an unsigned int. */
#define FIX2UINT(x)       ((unsigned int)FIX2ULONG(x))

#ifndef SYMBOL_P
#define SYMBOL_P(obj)     (((unsigned int)obj & 7) == 6)
#endif

/** Get a handle for the Symbol object represented by ID. */
#define ID2SYM(id)        (id)

/** Infect o2 if o1 is tainted */
#define OBJ_INFECT(o1, o2) capi_infect((o1), (o2))

/** Taints the object */
#define OBJ_TAINT(obj)    capi_taint((obj))

/** Returns 1 if the object is tainted, 0 otherwise. */
#define OBJ_TAINTED(obj)  capi_tainted_p((obj))

/** Convert int to a Ruby Integer. */
#define INT2FIX(i)        ((VALUE)(((long)(i))<<1 | FIXNUM_FLAG))

/** Convert long to a Ruby Integer. @todo Should we warn if overflowing? --rue */
#define LONG2FIX(i)       INT2FIX(i)

long long rb_num2ll(VALUE);
unsigned long long rb_num2ull(VALUE);
#define NUM2LL(x) (FIXNUM_P(x)?FIX2LONG(x):rb_num2ll((VALUE)x))
#define NUM2ULL(x) rb_num2ull((VALUE)x)

/** Convert from a Float to a double */
double rb_num2dbl(VALUE);
#define NUM2DBL(x) rb_num2dbl((VALUE)(x))

/** Zero out N elements of type starting at given pointer. */
#define MEMZERO(p,type,n) memset((p), 0, (sizeof(type) * (n)))

/** Copies n objects of type from p2 to p1. Behavior is undefined if objects
 * overlap.
 */
#define MEMCPY(p1,p2,type,n) memcpy((p1), (p2), sizeof(type)*(n))

/** Copies n objects of type from p2 to p1. Objects may overlap. */
#define MEMMOVE(p1,p2,type,n) memmove((p1), (p2), sizeof(type)*(n))

/** Compares n objects of type. */
#define MEMCMP(p1,p2,type,n) memcmp((p1), (p2), sizeof(type)*(n))

/** Whether object is nil. */
#define NIL_P(v)          capi_nil_p((v))

/** The length of the array. */
#define RARRAY_LEN(ary)   rb_ary_size(ary)

/** The pointer to the array's data. */
#define RARRAY_PTR(ary)   (RARRAY(ary)->ptr)

/** The length of string str. */
#define RSTRING_LEN(str)  rb_str_len(str)

/** The pointer to the string str's data. */
#ifdef RUBY_READONLY_STRING
#define RSTRING_PTR(str)  rb_str_ptr_readonly(str)
#else
#define RSTRING_PTR(str)  (RSTRING(str)->ptr)
#endif

/** The pointer to the data. */
#define DATA_PTR(d)       (RDATA(d)->data)

/** Return true if expression is not Qfalse or Qnil. */
#define RTEST(v)          (((VALUE)(v) & ~Qnil) != 0)

/** Return the super class of the object */
#define RCLASS_SUPER(klass)   capi_class_superclass((klass))

/** Rubinius' SafeStringValue is the same as StringValue. */
#define SafeStringValue   StringValue

#define REFERENCE_TAG         0x0
#define REFERENCE_MASK        0x3
#define REFERENCE_P(x)        ({ VALUE __v = (VALUE)x; __v && (__v & REFERENCE_MASK) == REFERENCE_TAG; })
#define IMMEDIATE_P(x)        (!REFERENCE_P(x))

/** Return true if expression is an immediate, Qfalse or Qnil. */
#define SPECIAL_CONST_P(x)    (IMMEDIATE_P(x) || !RTEST(x))

/** Modifies the VALUE object in place by calling rb_obj_as_string(). */
#define StringValue(v)        rb_string_value(&(v))
#define StringValuePtr(v)     rb_string_value_ptr(&(v))
#define StringValueCStr(str)  rb_string_value_cstr(&(str))
#define STR2CSTR(str)         rb_str2cstr((VALUE)(str), 0)

#define Check_SafeStr(x)

/** Retrieve the ID given a Symbol handle. */
#define SYM2ID(sym)       (sym)

/** Return an integer type id for the object. @see rb_type() */
#define TYPE(handle)      rb_type(handle)

/** Convert unsigned int to a Ruby Integer. @todo Should we warn if overflowing? --rue */
#define UINT2FIX(i)       UINT2NUM((i))

#define LL2NUM(val) rb_ll2inum(val)

  VALUE rb_ll2inum(long long val);
  VALUE rb_ull2inum(unsigned long long val);


/* Secret extra stuff */

  typedef VALUE (*CApiAllocFunction)(ANYARGS);
  typedef VALUE (*CApiGenericFunction)(ANYARGS);


  /**
   *  \internal
   *
   *  Backend for defining methods after normalization.
   *
   *  @see  rb_define_*_method.
   */
  void    capi_define_method(const char* file,
                                           VALUE target,
                                           const char* name,
                                           CApiGenericFunction fptr,
                                           int arity,
                                           CApiMethodKind kind);

  /** Call method on receiver, args as varargs. */
  VALUE   capi_rb_funcall(const char* file, int line,
                                        VALUE receiver, ID method_name,
                                        int arg_count, ...);

  /** Call the method with args provided in a C array. */
  VALUE   capi_rb_funcall2(const char* file, int line,
                                         VALUE receiver, ID method_name,
                                         int arg_count, VALUE* args);

  /** Retrieve a Handle to a globally available object. @internal. */
  VALUE   capi_get_constant(CApiConstant type);

  /** Returns the string associated with a symbol. */
  const char *rb_id2name(ID sym);

  /** Infect obj2 if obj1 is tainted. @internal.*/
  void    capi_infect(VALUE obj1, VALUE obj2);

  /** False if expression evaluates to nil, true otherwise. @internal. */
  int     capi_nil_p(VALUE expression_result);

  /** Taints obj. @internal. */
  void    capi_taint(VALUE obj);

  /** Returns 1 if obj is tainted, 0 otherwise. @internal. */
  int     capi_tainted_p(VALUE obj);

  /** Returns the superclass of klass or NULL. This is not the same as
   * rb_class_superclass. See MRI's rb_class_s_alloc which returns a
   * class created with rb_class_boot(0), i.e. having a NULL superclass.
   * RCLASS_SUPER(klass) is used in a boolean context to exit a loop in
   * the Digest extension. It's likely other extensions do the same thing.
   */
  VALUE   capi_class_superclass(VALUE class_handle);

  struct RArray* capi_rarray_struct(VALUE ary_handle);
  struct RData* capi_rdata_struct(VALUE data_handle);
  struct RString* capi_rstring_struct(VALUE str_handle);
  struct RFloat* capi_rfloat_struct(VALUE data_handle);

/* Real API */

  /** Convert a VALUE into a long int. */
  long    rb_num2long(VALUE obj);

  /** Convert a VALUE to an unsigned long int. */
  unsigned long rb_num2ulong(VALUE obj);

  /** Convert an int into an Integer. */
  VALUE   INT2NUM(int number);

  /** Convert a long int into an Integer. */
  VALUE   LONG2NUM(long int number);

  /** Convert an unsigned long to an Integer. */
  VALUE   ULONG2NUM(unsigned long);

  /** Convert unsigned int into a Numeric. */
  VALUE   UINT2NUM(unsigned int number);

#define   Data_Make_Struct(klass, type, mark, free, sval) (\
            sval = ALLOC(type), \
            memset(sval, NULL, sizeof(type)), \
            Data_Wrap_Struct(klass, mark, free, sval)\
          )

#define   Data_Wrap_Struct(klass, mark, free, sval) \
            rb_data_object_alloc(klass, (void*)sval, (RUBY_DATA_FUNC)mark, \
                                 (RUBY_DATA_FUNC)free)

#define   Data_Get_Struct(obj,type,sval) do {\
            Check_Type(obj, T_DATA); \
            sval = (type*)DATA_PTR(obj);\
} while (0)

  /** Return Qtrue if obj is an immediate, Qfalse or Qnil. */
  int     rb_special_const_p(VALUE obj);

  /** Return obj if it is an Array, or return wrapped (i.e. [obj]) */
  VALUE   rb_Array(VALUE obj_handle);

  /** Remove all elements from the Array. Returns self. */
  VALUE   rb_ary_clear(VALUE self_handle);

  /** Return shallow copy of the Array. The elements are not dupped. */
  VALUE   rb_ary_dup(VALUE self_handle);

  /** Return object at index. Out-of-bounds access returns Qnil. */
  VALUE   rb_ary_entry(VALUE self_handle, int index);

  /** Return Qtrue if the array includes the item. */
  VALUE   rb_ary_includes(VALUE self, VALUE obj);

  /** Array#join. Returns String with all elements to_s, with optional separator String. */
  VALUE   rb_ary_join(VALUE self_handle, VALUE separator_handle);

  /** New, empty Array. */
  VALUE   rb_ary_new();

  /** New Array of nil elements at given length. */
  VALUE   rb_ary_new2(unsigned long length);

  /** New Array of given length, filled with varargs elements. */
  VALUE   rb_ary_new3(unsigned long length, ...);

  /** New Array of given length, filled with copies of given object. */
  VALUE   rb_ary_new4(unsigned long length, const VALUE* object_handle);

  /** Remove and return last element of Array or nil. */
  VALUE   rb_ary_pop(VALUE self_handle);

  /** Appends value to end of Array and returns self. */
  VALUE   rb_ary_push(VALUE self_handle, VALUE object_handle);

  /** Returns a new Array with elements in reverse order. Elements not dupped. */
  VALUE   rb_ary_reverse(VALUE self_handle);

  /** Remove and return first element of Array or nil. Changes other elements' indexes. */
  VALUE   rb_ary_shift(VALUE self_handle);

  /** Number of elements in given Array. @todo MRI specifies int return, problem? */
  size_t  rb_ary_size(VALUE self_handle);

  /** Store object at given index. Supports negative indexes. Returns object. */
  void    rb_ary_store(VALUE self_handle, long int index, VALUE object_handle);

  /** Add object to the front of Array. Changes old indexes +1. Returns object. */
  VALUE   rb_ary_unshift(VALUE self_handle, VALUE object_handle);

  /** Returns the element at index, or returns a subarray or returns a subarray specified by a range. */
  VALUE   rb_ary_aref(int argc, VALUE *argv, VALUE object_handle);

  VALUE   rb_ary_each(VALUE ary);

  /** Return new Array with elements first and second. */
  VALUE   rb_assoc_new(VALUE first, VALUE second);

  /** @see rb_ivar_get */
  VALUE   rb_attr_get(VALUE obj_handle, ID attr_name);

  void    rb_attr(VALUE klass, ID id, int read, int write, int ex);

  /** Return 1 if this send has a block, 0 otherwise. */
  int     rb_block_given_p();

  /* Converts implicit block into a new Proc. */
  VALUE   rb_block_proc();

  VALUE   rb_big2str(VALUE self, int base);

  long    rb_big2long(VALUE obj);

  unsigned long rb_big2ulong(VALUE obj);

  double  rb_big2dbl(VALUE obj);

  /** Calls this method in a superclass. */
  VALUE rb_call_super(int argc, const VALUE *argv);

  /** If object responds to #to_ary, returns the result of that call, otherwise nil. */
  VALUE   rb_check_array_type(VALUE object_handle);

  /** If object responds to #to_str, returns the result of that call, otherwise nil. */
  VALUE   rb_check_string_type(VALUE object_handle);

  /** Raises an exception if obj_handle is frozen. */
  void    rb_check_frozen(VALUE obj_handle);

  /** Raises an exception if obj_handle is not the same type as 'type'. */
  void    rb_check_type(VALUE obj_handle, CApiType type);

#define Check_Type(v,t) rb_check_type((VALUE)(v),t)

  /**
   *  Safe type conversion.
   *
   *  If the object responds to the given method name, the method is
   *  called and the result returned. Otherwise returns nil.
   *
   *  @see rb_check_array_type() and rb_check_string_type().
   */
  VALUE   rb_check_convert_type(VALUE object_handle, int type,
      const char* type_name, const char* method_name);

  /** Returns String representation of the class' name. */
  VALUE   rb_class_name(VALUE class_handle);

  /** As Ruby's .new, with the given arguments. Returns the new object. */
  VALUE   rb_class_new_instance(int arg_count, VALUE* args, VALUE class_handle);

  /** Returns the Class object this object is an instance of. */
  VALUE   rb_class_of(VALUE object_handle);

  /** Returns the Class object contained in the klass field of object
   * (ie, a metaclass if it's there) */
  VALUE   CLASS_OF(VALUE object_handle);

  /** C string representation of the class' name. You must free this string. */
  char*   rb_class2name(VALUE class_handle);

  /** Return the module referred to by qualified path (e.g. A::B::C) */
  VALUE   rb_path2class(const char*);

  /** Print the value to $stdout */
  void    rb_p(VALUE);

  /** Returns object returned by invoking method on object if right type, or raises error. */
  VALUE   rb_convert_type(VALUE object_handle, int type,
      const char* type_name, const char* method_name);

  /** Nonzero if constant corresponding to Symbol exists in the Module. */
  int     rb_const_defined(VALUE module_handle, ID const_id);

  /** Retrieve constant from given module. */
  VALUE   rb_const_get(VALUE module_handle, ID name);

  /** Set constant on the given module */
  void rb_const_set(VALUE module_handle, ID name, VALUE const_handle);

  /** Parses a string into a double value. If badcheck is true, raises an
   * exception if the string contains non-digit or '.' characters.
   */
  double rb_cstr_to_dbl(const char *p, int badcheck);

  /** Return Integer obtained from String#to_i using given base. */
  VALUE   rb_cstr2inum(const char* string, int base);
  VALUE   rb_cstr_to_inum(const char* str, int base, int badcheck);

  /** Returns module's named class variable. @@ is optional. */
  VALUE   rb_cv_get(VALUE module_handle, const char* name);

  /** Set module's named class variable to given value. Returns the value. @@ is optional. */
  VALUE   rb_cv_set(VALUE module_handle, const char* name, VALUE value);

  /** Returns a value evaluating true if module has named class var. @@ is optional. */
  VALUE   rb_cvar_defined(VALUE module_handle, ID name);

  /** Returns class variable by (Symbol) name from module. @@ is optional. */
  VALUE   rb_cvar_get(VALUE module_handle, ID name);

  /** Set module's named class variable to given value. Returns the value. @@ is optional. */
  VALUE   rb_cvar_set(VALUE module_handle, ID name, VALUE value, int unused);

  VALUE   rb_data_object_alloc(VALUE klass, void* sval,
      RUBY_DATA_FUNC mark, RUBY_DATA_FUNC free);

  /** Alias method by old name as new name. Methods are independent of eachother. */
  void    rb_define_alias(VALUE module_handle, const char *new_name, const char *old_name);

  /** Define an .allocate for the given class. Should take no args and return a VALUE. */
  void    rb_define_alloc_func(VALUE class_handle, CApiAllocFunction allocator);

  /** Ruby's attr_* for given name. Nonzeros to toggle read/write. */
  void    rb_define_attr(VALUE module_handle, const char* attr_name,
      int readable, int writable);

  /** Reopen or create new top-level class with given superclass and name. Returns the Class object. */
  VALUE   rb_define_class(const char* name, VALUE superclass_handle);

  /** Reopen or create new class with superclass and name under parent module. Returns the Class object. */
  VALUE   rb_define_class_under(VALUE parent_handle, const char* name, VALUE superclass_handle);

  /** Define a constant in given Module's namespace. */
  void    rb_define_const(VALUE module_handle, const char* name, VALUE obj_handle);

  /** Generate a NativeMethod to represent a method defined as a C function. Records file. */
  #define rb_define_method(mod, name, fptr, arity) \
          capi_define_method(__FILE__, mod, name, \
                                           (CApiGenericFunction)fptr, arity, \
                                           cCApiPublicMethod)

  /** Defines the method on Kernel. */
  void    rb_define_global_function(const char* name, CApiGenericFunction func, int argc);

  /** Reopen or create new top-level Module. */
  VALUE   rb_define_module(const char* name);

  /** Defines the method as a private instance method and a singleton method of module. */
  void    rb_define_module_function(VALUE module_handle,
      const char* name, CApiGenericFunction func, int args);

  /** Reopen or create a new Module inside given parent Module. */
  VALUE   rb_define_module_under(VALUE parent_handle, const char* name);

  /** Generate a NativeMethod to represent a private method defined in the C function. */
  #define rb_define_private_method(mod, name, fptr, arity) \
          capi_define_method(__FILE__, mod, name, \
                                           (CApiGenericFunction)fptr, arity, \
                                           cCApiPrivateMethod)

  /** Generate a NativeMethod to represent a protected method defined in the C function. */
  #define rb_define_protected_method(mod, name, fptr, arity) \
          capi_define_method(__FILE__, mod, name, \
                                           (CApiGenericFunction)fptr, arity, \
                                           cCApiProtectedMethod)

  /** Generate a NativeMethod to represent a singleton method. @see capi_define_method. */
  #define rb_define_singleton_method(mod, name, fptr, arity) \
          capi_define_method(__FILE__, mod, name, \
                                           (CApiGenericFunction)fptr, arity, \
                                           cCApiSingletonMethod)

  /** Create an Exception from a class, C string and length. */
  VALUE   rb_exc_new(VALUE etype, const char *ptr, long len);

  /** Create an Exception from a class and C string. */
  VALUE   rb_exc_new2(VALUE etype, const char *s);

  /** Create an Exception from a class and Ruby string. */
  VALUE   rb_exc_new3(VALUE etype, VALUE str);

  /** Raises passed exception handle */
  void    rb_exc_raise(VALUE exc_handle);

  /** Remove a previously declared global variable. */
  void    rb_free_global(VALUE global_handle);

  /**
   *  Freeze object and return it.
   *
   *  NOT supported in Rubinius.
   */
  #define rb_obj_freeze(object_handle)

  /**
   *  Call method on receiver, args as varargs. Calls private methods.
   *
   *  @todo Requires C99, change later for production code if needed.
   *        Pretty much all C++ compilers support this too.  It can be
   *        done by introducing an intermediary function to grab the
   *        debug info, but it is far uglier. --rue
   *
   *  See http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
   *  regarding use of ##__VA_ARGS__.
   */
  #define rb_funcall(receiver, method_name, arg_count, ...) \
          capi_rb_funcall(__FILE__, __LINE__, \
                                        (receiver), (method_name), \
                                        (arg_count) , ##__VA_ARGS__)

  /** Call the method with args provided in a C array. Calls private methods. */
  #define rb_funcall2(receiver, method_name, arg_count, args) \
          capi_rb_funcall2(__FILE__, __LINE__, \
                                         (receiver), (method_name), \
                                         (arg_count), (args) )

  /** @todo define rb_funcall3, which is the same as rb_funcall2 but
   * will not call private methods.
   */
  #define rb_funcall3 rb_funcall2

  /** Create a new Hash object */
  VALUE   rb_hash_new();

  /** Return the value associated with the key. */
  VALUE   rb_hash_aref(VALUE self, VALUE key);

  /** Set the value associated with the key. */
  VALUE   rb_hash_aset(VALUE self, VALUE key, VALUE value);

  /** Remove the key and return the associated value. */
  VALUE   rb_hash_delete(VALUE self, VALUE key);

  /** Returns the number of entries as a Fixnum. */
  VALUE   rb_hash_size(VALUE self);

  /** Iterate over the hash, calling the function. */
  void rb_hash_foreach(VALUE self,
                       int (*func)(VALUE key, VALUE val, VALUE data),
                       VALUE farg);

  // A macro to access the size "directly"
#define RHASH_SIZE(obj) FIX2INT(rb_hash_size(obj))

  void    rb_eof_error();

  /** Send #write to io passing str. */
  VALUE   rb_io_write(VALUE io, VALUE str);

  int     rb_io_fd(VALUE io);
  void    rb_io_wait_readable(int fd);
  void    rb_io_wait_writable(int fd);
  void    rb_thread_wait_fd(int fd);

  /** Mark ruby object ptr. */
  void    rb_gc_mark(VALUE ptr);

  /**
   * Marks an object if it is in the heap. Equivalent to rb_gc_mark in
   * Rubinius since that function checks if a handle is a GC object.
   */
  void    rb_gc_mark_maybe(VALUE ptr);

  /** Manually runs the garbage collector. */
  VALUE   rb_gc_start();

  /** Mark variable global. Will not be GC'd. */
  void    rb_global_variable(VALUE* handle_address);

  /** Retrieve global by name. Because of MRI, the leading $ is optional but recommended. */
  VALUE   rb_gv_get(const char* name);

  /** Set named global to given value. Returns value. $ optional. */
  VALUE   rb_gv_set(const char* name, VALUE value);

  /** Set a global name to be used to address the VALUE at addr */
  void rb_define_readonly_variable(const char* name, VALUE* addr);

  /** Include Module in another Module, just as Ruby's Module#include. */
  void    rb_include_module(VALUE includer_handle, VALUE includee_handle);

  /** Convert string to an ID */
  ID      rb_intern(const char* string);

  /** Coerce x and y and perform 'x func y' */
  VALUE rb_num_coerce_bin(VALUE x, VALUE y, ID func);

  /** Coerce x and y; perform 'x func y' if coerce succeeds, else return Qnil. */
  VALUE rb_num_coerce_cmp(VALUE x, VALUE y, ID func);

  /** Call #initialize on the object with given arguments. */
  void    rb_obj_call_init(VALUE object_handle, int arg_count, VALUE* args);

  /** Returns the Class object this object is an instance of. */
  #define rb_obj_class(object_handle) \
          rb_class_of((object_handle))

  /** String representation of the object's class' name. You must free this string. */
  char*   rb_obj_classname(VALUE object_handle);

  /** Returns true-ish if object is an instance of specific class. */
  VALUE   rb_obj_is_instance_of(VALUE object_handle, VALUE class_handle);

  /** Returns true-ish if module is object's class or other ancestor. */
  VALUE   rb_obj_is_kind_of(VALUE object_handle, VALUE module_handle);

  /** Returns the object_id of the object. */
  VALUE   rb_obj_id(VALUE self);

  /** Return object's instance variable by name. @ optional. */
  VALUE   rb_iv_get(VALUE self_handle, const char* name);

  /** Set instance variable by name to given value. Returns the value. @ optional. */
  VALUE   rb_iv_set(VALUE self_handle, const char* name, VALUE value);

  /** Get object's instance variable. */
  VALUE   rb_ivar_get(VALUE obj_handle, ID ivar_name);

  /** Set object's instance variable to given value. */
  VALUE   rb_ivar_set(VALUE obj_handle, ID ivar_name, VALUE value);

  /** Checks if obj_handle has an ivar named ivar_name. */
  VALUE   rb_ivar_defined(VALUE obj_handle, ID ivar_name);

  /** Allocate uninitialised instance of given class. */
  VALUE   rb_obj_alloc(VALUE klass);

  /** Call #to_s on object. */
  VALUE   rb_obj_as_string(VALUE obj_handle);

  /** Return a clone of the object by calling the method bound
   * to Kernel#clone (i.e. does NOT call specialized #clone method
   * on obj_handle if one exists).
   */
  VALUE rb_obj_clone(VALUE obj_handle);

  /** Adds the module's instance methods to the object. */
  void rb_extend_object(VALUE obj, VALUE mod);

  /** Call #inspect on an object. */
  VALUE rb_inspect(VALUE obj_handle);

  /**
   *  Raise error of given class using formatted message.
   *
   *  @todo Implement for real. --rue
   */
  void    rb_raise(VALUE error_handle, const char* format_string, ...);

  /**
   * Calls the function 'func', with arg1 as the argument.  If an exception
   * occurs during 'func', it calls 'raise_func' with arg2 as the argument.  The
   * return value of rb_rescue() is the return value from 'func' if no
   * exception occurs, from 'raise_func' otherwise.
   */
  VALUE rb_rescue(VALUE (*func)(ANYARGS), VALUE arg1, VALUE (*raise_func)(ANYARGS), VALUE arg2);

  /**
   * Same as rb_rescue() except that it also receives a list of exceptions classes.
   * It will only call 'raise_func' if the exception occurred during 'func' is a
   * kind_of? one of the passed exception classes.
   * The last argument MUST always be 0!
   */
  VALUE rb_rescue2(VALUE (*func)(ANYARGS), VALUE arg1, VALUE (*raise_func)(ANYARGS), VALUE arg2, ...);

  /**
   * Calls the function func(), with arg1 as the argument, then call ensure_func()
   * with arg2, even if func() raised an exception. The return value from rb_ensure()
   * is the return of func().
   */
  VALUE rb_ensure(VALUE (*func)(ANYARGS), VALUE arg1, VALUE (*ensure_func)(ANYARGS), VALUE arg2);

  /**
   * Call func(), and if there is an exception, returns nil and sets
   * *status to 1, otherwise the return of func is returned and *status
   * is 0.
   */
  VALUE rb_protect(VALUE (*func)(ANYARGS), VALUE data, int* status);

  /**
   * Continue raising a pending exception if status is not 0
   */
  void rb_jump_tag(int status);

  /**
   *  Require a Ruby file.
   *
   *  Returns true on first load, false if already loaded or raises.
   */
  VALUE   rb_require(const char* name);

  /** 1 if obj.respond_to? method_name evaluates true, 0 otherwise. */
  int     rb_respond_to(VALUE obj_handle, ID method_name);

  /** Returns the current $SAFE level. */
  int     rb_safe_level();

  /**
   *  Process arguments using a template rather than manually.
   *
   *  The first two arguments are simple: the number of arguments given
   *  and an array of the args. Usually you get these as parameters to
   *  your function.
   *
   *  The spec works like this: it must have one (or more) of the following
   *  specifiers, and the specifiers that are given must always appear
   *  in the order given here. If the first character is a digit (0-9),
   *  it is the number of required parameters. If there is a second digit
   *  (0-9), it is the number of optional parameters. The next character
   *  may be "*", indicating a "splat" i.e. it consumes all remaining
   *  parameters. Finally, the last character may be "&", signifying
   *  that the block given (or Qnil) should be stored.
   *
   *  The remaining arguments are pointers to the variables in which
   *  the aforementioned format assigns the scanned parameters. For
   *  example in some imaginary function:
   *
   *    VALUE required1, required2, optional, splat, block
   *    rb_scan_args(argc, argv, "21*&", &required1, &required2,
   *                                     &optional,
   *                                     &splat,
   *                                     &block);
   *
   *  The required parameters must naturally always be exact. The
   *  optional parameters are set to nil when parameters run out.
   *  The splat is always an Array, but may be empty if there were
   *  no parameters that were not consumed by required or optional.
   *  Lastly, the block may be nil.
   */
  int     rb_scan_args(int argc, const VALUE* argv, const char* spec, ...);

  /** Raise error if $SAFE is not higher than the given level. */
  void    rb_secure(int level);

  /** Set $SAFE to given _higher_ level. Lowering $SAFE is not allowed. */
  void    rb_set_safe_level(int new_level);

  /** Returns the MetaClass object of the object. */
  VALUE   rb_singleton_class(VALUE object_handle);

  /** Tries to return a String using #to_str. Error raised if no or invalid conversion. */
  VALUE   rb_String(VALUE object_handle);

  /** Returns a pointer to a persistent char [] that contains the same data as
   * that contained in the Ruby string. The buffer is flushed to the string
   * when control returns to Ruby code. The buffer is updated with the string
   * contents when control crosses to C code.
   *
   * @note This is NOT an MRI C-API function.
   */
  char *rb_str_ptr(VALUE self);

  /** Write the contents of the cached data at the pointer returned by
   * rb_str_ptr to the Ruby object.
   *
   * @note This is NOT an MRI C-API function.
   */
  void rb_str_flush(VALUE self);

  /** Update the cached data at the pointer returned by rb_str_ptr with the
   * contents of the Ruby object.
   *
   * @note This is NOT an MRI C-API function.
   */
  void rb_str_update(VALUE self);

  /** Returns a pointer to a persistent char [] that contains the same data as
   * that contained in the Ruby string. The buffer is intended to be
   * read-only. No changes to the buffer will be propagated to the Ruby
   * string, and no changes to the Ruby object will be propagated to the
   * buffer.  The buffer will persist as long as the Ruby string persists.
   *
   * @note This is NOT an MRI C-API function.
   */
  char* rb_str_ptr_readonly(VALUE self);

  /** Appends other String to self and returns the modified self. */
  VALUE   rb_str_append(VALUE self_handle, VALUE other_handle);

  /** Appends other String to self and returns self. @see rb_str_append */
  VALUE   rb_str_buf_append(VALUE self_handle, VALUE other_handle);

  /** Append given number of bytes from C string to and return the String. */
  VALUE   rb_str_buf_cat(VALUE string_handle, const char* other, size_t size);

  /** Append C string to and return the String. Uses strlen(). @see rb_str_buf_cat */
  VALUE   rb_str_buf_cat2(VALUE string_handle, const char* other);

  /**
   *  Return new empty String with preallocated storage.
   *
   *  @note   Not supported by Rubinius for a few more days.
   *  @todo   Update as soon as
   */
  VALUE   rb_str_buf_new(long capacity);

  /** Return new String concatenated of the two. */
  VALUE   rb_str_cat(VALUE string_handle, const char* other, size_t length);

  /** Return new String concatenated of the two. Uses strlen(). @see rb_str_cat */
  VALUE   rb_str_cat2(VALUE string_handle, const char* other);

  /** Compare Strings as Ruby String#<=>. Returns -1, 0 or 1. */
  int     rb_str_cmp(VALUE first_handle, VALUE second_handle);

  /** Append other String or character to self, and return the modified self. */
  VALUE   rb_str_concat(VALUE self_handle, VALUE other_handle);

  /** As Ruby's String#dup, returns copy of self as a new String. */
  VALUE   rb_str_dup(VALUE self_handle);

  /** Returns a symbol created from this string. */
  VALUE   rb_str_intern(VALUE self);

  /** Returns the size of the string. It accesses the size directly and does
   * not cause the string to be cached.
   *
   * @note This is NOT an MRI C-API function.
   */
  size_t  rb_str_len(VALUE self);

  void    rb_str_clamp(VALUE self, size_t len);
#define rb_str_set_len(s,l) rb_str_clamp(s,l)

  /** Create a String using the designated length of given C string. */
  /* length is a long because MRI has it as a long, and it also has
   * to check that length is greater than 0 properly */
  VALUE   rb_str_new(const char* string, long length);

  /** Create a String from a C string. */
  VALUE   rb_str_new2(const char* string);

  void    rb_str_modify(VALUE str);

  /** Returns a new String created from concatenating self with other. */
  VALUE   rb_str_plus(VALUE self_handle, VALUE other_handle);

  /** Makes str at least len characters. */
  VALUE   rb_str_resize(VALUE self_handle, size_t len);

  /** Splits self using the separator string. Returns Array of substrings. */
  VALUE   rb_str_split(VALUE self_handle, const char* separator);

  /**
   *  As Ruby's String#slice.
   *
   *  Returns new String with copy of length characters
   *  starting from the given index of self. The index
   *  may be negative. Normal String#slice border conditions
   *  apply.
   */
  VALUE   rb_str_substr(VALUE self_handle, size_t starting_index, size_t length);

  /** Return a new String containing given number of copies of self. */
  VALUE   rb_str_times(VALUE self_handle, VALUE times);

  /** Return an Integer obtained from String#to_i, using the given base. */
  VALUE   rb_str2inum(VALUE self_handle, int base);

  /** Try to return a String using #to_str. Error raised if no or invalid conversion. */
  VALUE   rb_str_to_str(VALUE object_handle);

  /** Call #to_s on object pointed to and _replace_ it with the String. */
  VALUE   rb_string_value(VALUE* object_variable);

  char*   rb_string_value_ptr(VALUE* object_variable);
  /**
   *  As rb_string_value but also returns a C string of the new String.
   *
   *  You must free the string.
   */
  char*   rb_string_value_cstr(VALUE* object_variable);

  /**
   * Returns an editable pointer to the String, the length is returned
   * in len parameter, which can be NULL.
   */
  char*   rb_str2cstr(VALUE str_handle, long *len);

  /** Raises an exception from the value of errno. */
  void rb_sys_fail(const char* mesg);

  /** Evaluate the given string. */
  VALUE   rb_eval_string(const char* string);

  /** Create a String from the C string. */
  VALUE   rb_tainted_str_new2(const char* string);

  /** Create a String from the C string. */
  VALUE   rb_tainted_str_new(const char* string, long size);

  /** Issue a thread.pass. */
  void    rb_thread_schedule();

  /** Stubbed to always return 0. */
  int    rb_thread_alone();

  /** Request status of file descriptors */
  int     rb_thread_select(int max, fd_set* read, fd_set* write, fd_set* except,
                           struct timeval *timeval);

  /** Returns an integer value representing the object's type. */
  int     rb_type(VALUE object_handle);

  /** Call #to_sym on object. */
  ID      rb_to_id(VALUE object_handle);

  /** Module#undefine_method. Objects of class will not respond to name. @see rb_remove_method */
  void    rb_undef_method(VALUE module_handle, const char* name);

  /** Call block with given argument or raise error if no block given. */
  VALUE   rb_yield(VALUE argument_handle);

  VALUE   rb_marshal_load(VALUE string);

  VALUE   rb_float_new(double val);
  
  VALUE   rb_Float(VALUE object_handle);
  
  VALUE   rb_Integer(VALUE object_handle);

  void    rb_bug(const char *fmt, ...);

  void    rb_fatal(const char *fmt, ...);

  /** Raises an ArgumentError exception. */
  void rb_invalid_str(const char *str, const char *type);

  /** Print a warning if $VERBOSE is not nil. */
  void    rb_warn(const char *fmt, ...);

  /** Print a warning if $VERBOSE is true. */
  void    rb_warning(const char *fmt, ...);

  /** Creates a Range object from begin to end */
  VALUE   rb_range_new(VALUE begin, VALUE end, int exclude_end);

  VALUE   rb_range_beg_len(VALUE range, long* begp, long* lenp, long len, int err);
#ifdef __cplusplus
}
#endif

#endif
