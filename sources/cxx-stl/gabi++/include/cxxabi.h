// Copyright (C) 2011 The Android Open Source Project
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//

#ifndef __GABIXX_CXXABI_H__
#define __GABIXX_CXXABI_H__

// The specifications for the declarations found in this header are
// the following:
//
// - Itanium C++ ABI [1]
//   Used on about every CPU architecture, _except_ ARM, this
//   is also commonly referred as the "generic C++ ABI".
//
//   NOTE: This document seems to only covers C++98
//
// - Itanium C++ ABI: Exception Handling. [2]
//   Supplement to the above document describing how exception
//   handle works with the generic C++ ABI. Again, this only
//   seems to support C++98.
//
// - C++ ABI for the ARM architecture [3]
//   Describes the ARM C++ ABI, mainly as a set of differences from
//   the generic one.
//
// - Exception Handling for the ARM Architecture [4]
//   Describes exception handling for ARM in detail. There are rather
//   important differences in the stack unwinding process and
//   exception cleanup.
//
// There are also no freely availabel documentation about certain
// features introduced in C++0x or later. In this case, the best
// source for information are the GNU and LLVM C++ runtime libraries
// (libcxxabi, libsupc++ and even libc++ sources), as well as a few
// proposals, for example:
//
// - For exception propagation:
//   http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2007/n2179.html
//   But the paper only describs the high-level language feature, not
//   the low-level runtime support required to implement it.
//
// - For nested exceptions:
//   http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2008/n2559.html
//   Yet another high-level description without low-level details.
// 
#include <exception>
#include <typeinfo>
#include <unwind.h>

namespace __cxxabiv1
{
  // Derived types of type_info below are based on 2.9.5 of C++ ABI.

  class __shim_type_info : public std::type_info
  {
   public:
    virtual ~__shim_type_info();
    virtual bool can_catch(const __shim_type_info* thrown_type,
                           void*& adjustedPtr) const = 0;
  };

  // Typeinfo for fundamental types.
  class __fundamental_type_info : public __shim_type_info
  {
  public:
    virtual ~__fundamental_type_info();
    virtual bool can_catch(const __shim_type_info* thrown_type,
                           void*& adjustedPtr) const;
  };

  // Typeinfo for array types.
  class __array_type_info : public __shim_type_info
  {
  public:
    virtual ~__array_type_info();
    virtual bool can_catch(const __shim_type_info* thrown_type,
                           void*& adjustedPtr) const;
  };

  // Typeinfo for function types.
  class __function_type_info : public __shim_type_info
  {
  public:
    virtual ~__function_type_info();
    virtual bool can_catch(const __shim_type_info* thrown_type,
                           void*& adjustedPtr) const;
  };

  // Typeinfo for enum types.
  class __enum_type_info : public __shim_type_info
  {
  public:
    virtual ~__enum_type_info();
    virtual bool can_catch(const __shim_type_info* thrown_type,
                           void*& adjustedPtr) const;
  };


  class __class_type_info;

  // Used in __vmi_class_type_info
  struct __base_class_type_info
  {
  public:
    const __class_type_info *__base_type;

    long __offset_flags;

    enum __offset_flags_masks {
      __virtual_mask = 0x1,
      __public_mask = 0x2,
      __offset_shift = 8   // lower 8 bits are flags
    };

    bool is_virtual() const {
      return (__offset_flags & __virtual_mask) != 0;
    }

    bool is_public() const {
      return (__offset_flags & __public_mask) != 0;
    }

    // FIXME: Right-shift of signed integer is implementation dependent.
    // GCC Implements it as signed (as we expect)
    long offset() const {
      return __offset_flags >> __offset_shift;
    }

    long flags() const {
      return __offset_flags & ((1 << __offset_shift) - 1);
    }
  };

  // Helper struct to support catch-clause match
  struct __UpcastInfo {
    enum ContainedStatus {
      unknown = 0,
      has_public_contained,
      has_ambig_or_not_public
    };

    ContainedStatus status;
    const __class_type_info* base_type;
    void* adjustedPtr;
    unsigned int premier_flags;
    bool nullobj_may_conflict;

    __UpcastInfo(const __class_type_info* type);
  };

  // Typeinfo for classes with no bases.
  class __class_type_info : public __shim_type_info
  {
  public:
    virtual ~__class_type_info();
    virtual bool can_catch(const __shim_type_info* thrown_type,
                           void*& adjustedPtr) const;

    enum class_type_info_code {
      CLASS_TYPE_INFO_CODE,
      SI_CLASS_TYPE_INFO_CODE,
      VMI_CLASS_TYPE_INFO_CODE
    };

    virtual class_type_info_code
      code() const { return CLASS_TYPE_INFO_CODE; }

    virtual bool walk_to(const __class_type_info* base_type,
                         void*& adjustedPtr,
                         __UpcastInfo& info) const;

  protected:
    bool self_class_type_match(const __class_type_info* base_type,
                               void*& adjustedPtr,
                               __UpcastInfo& info) const;
  };

  // Typeinfo for classes containing only a single, public, non-virtual base at
  // offset zero.
  class __si_class_type_info : public __class_type_info
  {
  public:
    virtual ~__si_class_type_info();
    const __class_type_info *__base_type;

    virtual __class_type_info::class_type_info_code
      code() const { return SI_CLASS_TYPE_INFO_CODE; }

    virtual bool walk_to(const __class_type_info* base_type,
                         void*& adjustedPtr,
                         __UpcastInfo& info) const;
  };


  // Typeinfo for classes with bases that do not satisfy the
  // __si_class_type_info constraints.
  class __vmi_class_type_info : public __class_type_info
  {
  public:
    virtual ~__vmi_class_type_info();
    unsigned int __flags;
    unsigned int __base_count;
    __base_class_type_info __base_info[1];

    enum __flags_masks {
      __non_diamond_repeat_mask = 0x1,
      __diamond_shaped_mask = 0x2,
    };

    virtual __class_type_info::class_type_info_code
      code() const { return VMI_CLASS_TYPE_INFO_CODE; }

    virtual bool walk_to(const __class_type_info* base_type,
                         void*& adjustedPtr,
                         __UpcastInfo& info) const;
  };

  class __pbase_type_info : public __shim_type_info
  {
  public:
    virtual ~__pbase_type_info();
    virtual bool can_catch(const __shim_type_info* thrown_type,
                           void*& adjustedPtr) const;
    unsigned int __flags;
    const __shim_type_info* __pointee;

    enum __masks {
      __const_mask = 0x1,
      __volatile_mask = 0x2,
      __restrict_mask = 0x4,
      __incomplete_mask = 0x8,
      __incomplete_class_mask = 0x10
    };


    virtual bool can_catch_typeinfo_wrapper(const __shim_type_info* thrown_type,
                                            void*& adjustedPtr,
                                            unsigned tracker) const;

  protected:
    enum __constness_tracker_status {
      first_time_init = 0x1,
      keep_constness = 0x2,
      after_gap = 0x4         // after one non-const qualified,
                              // we cannot face const again in future
    };

  private:
    bool can_catch_ptr(const __pbase_type_info *thrown_type,
                       void *&adjustedPtr,
                       unsigned tracker) const;

    // Return true if making decision done.
    virtual bool do_can_catch_ptr(const __pbase_type_info* thrown_type,
                                  void*& adjustedPtr,
                                  unsigned tracker,
                                  bool& result) const = 0;
  };

  class __pointer_type_info : public __pbase_type_info
  {
  public:
    virtual ~__pointer_type_info();

  private:
    virtual bool do_can_catch_ptr(const __pbase_type_info* thrown_type,
                                  void*& adjustedPtr,
                                  unsigned tracker,
                                  bool& result) const;
  };

  class __pointer_to_member_type_info : public __pbase_type_info
  {
  public:
    __class_type_info* __context;

    virtual ~__pointer_to_member_type_info();

  private:
    virtual bool do_can_catch_ptr(const __pbase_type_info* thrown_type,
                                  void*& adjustedPtr,
                                  unsigned tracker,
                                  bool& result) const;
  };



  extern "C" {

    // TODO: Support dependent exception
    // TODO: Support C++0x exception propagation
    // http://sourcery.mentor.com/archives/cxx-abi-dev/msg01924.html
    struct __cxa_exception;
    struct __cxa_eh_globals;

    __cxa_eh_globals* __cxa_get_globals();
    __cxa_eh_globals* __cxa_get_globals_fast();

    void* __cxa_allocate_exception(size_t thrown_size);
    void __cxa_free_exception(void* thrown_exception);

    void __cxa_throw(void* thrown_exception, std::type_info* tinfo, void (*dest)(void*));
    void __cxa_rethrow();

    void* __cxa_begin_catch(void* exceptionObject);
    void __cxa_end_catch();

    bool __cxa_begin_cleanup(_Unwind_Exception*);
    void __cxa_end_cleanup();

    void __cxa_bad_cast();
    void __cxa_bad_typeid();

    void* __cxa_get_exception_ptr(void* exceptionObject);

    void __cxa_pure_virtual();

    // Missing libcxxabi functions.
    bool __cxa_uncaught_exception() _GABIXX_NOEXCEPT;
    void __cxa_decrement_exception_refcount(void* exceptionObject) _GABIXX_NOEXCEPT;
    void __cxa_increment_exception_refcount(void* exceptionObject) _GABIXX_NOEXCEPT;
    void __cxa_rethrow_primary_exception(void* exceptionObject);
    void* __cxa_current_primary_exception() _GABIXX_NOEXCEPT;

    // The ARM ABI mandates that constructors and destructors
    // must return 'this', i.e. their first parameter. This is
    // also true for __cxa_vec_ctor and __cxa_vec_cctor.
#ifdef __arm__
    typedef void* __cxa_vec_ctor_return_type;
#else
    typedef void __cxa_vec_ctor_return_type;
#endif

    typedef __cxa_vec_ctor_return_type
        (*__cxa_vec_constructor)(void *);

    typedef __cxa_vec_constructor __cxa_vec_destructor;

    typedef __cxa_vec_ctor_return_type
        (*__cxa_vec_copy_constructor)(void*, void*);

    void* __cxa_vec_new(size_t element_count,
                        size_t element_size,
                        size_t padding_size,
                        __cxa_vec_constructor constructor,
                        __cxa_vec_destructor destructor);

    void* __cxa_vec_new2(size_t element_count,
                         size_t element_size,
                         size_t padding_size,
                         __cxa_vec_constructor constructor,
                         __cxa_vec_destructor destructor,
                         void* (*alloc)(size_t),
                         void  (*dealloc)(void*));

    void* __cxa_vec_new3(size_t element_count,
                         size_t element_size,
                         size_t padding_size,
                         __cxa_vec_constructor constructor,
                         __cxa_vec_destructor destructor,
                         void* (*alloc)(size_t),
                         void  (*dealloc)(void*, size_t));

    __cxa_vec_ctor_return_type
    __cxa_vec_ctor(void*  array_address,
                   size_t element_count,
                   size_t element_size,
                   __cxa_vec_constructor constructor,
                   __cxa_vec_destructor destructor);

    void __cxa_vec_dtor(void*  array_address,
                        size_t element_count,
                        size_t element_size,
                        __cxa_vec_destructor destructor);

    void __cxa_vec_cleanup(void* array_address,
                           size_t element_count,
                           size_t element_size,
                           __cxa_vec_destructor destructor);

    void __cxa_vec_delete(void*  array_address,
                          size_t element_size,
                          size_t padding_size,
                          __cxa_vec_destructor destructor);

    void __cxa_vec_delete2(void* array_address,
                           size_t element_size,
                           size_t padding_size,
                           __cxa_vec_destructor destructor,
                           void  (*dealloc)(void*));

    void __cxa_vec_delete3(void* array_address,
                           size_t element_size,
                           size_t padding_size,
                           __cxa_vec_destructor destructor,
                           void  (*dealloc) (void*, size_t));

    __cxa_vec_ctor_return_type
    __cxa_vec_cctor(void*  dest_array,
                    void*  src_array,
                    size_t element_count,
                    size_t element_size,
                    __cxa_vec_copy_constructor constructor,
                    __cxa_vec_destructor destructor );

    // The ARM ABI mandates that constructors and destructors
    // must return 'this', i.e. their first parameter. This is
    // also true for __cxa_vec_ctor and __cxa_vec_cctor.
#ifdef __arm__
    typedef void* __cxa_vec_ctor_return_type;
#else
    typedef void __cxa_vec_ctor_return_type;
#endif

    typedef __cxa_vec_ctor_return_type
        (*__cxa_vec_constructor)(void *);

    typedef __cxa_vec_constructor __cxa_vec_destructor;

    typedef __cxa_vec_ctor_return_type
        (*__cxa_vec_copy_constructor)(void*, void*);

    void* __cxa_vec_new(size_t element_count,
                        size_t element_size,
                        size_t padding_size,
                        __cxa_vec_constructor constructor,
                        __cxa_vec_destructor destructor);

    void* __cxa_vec_new2(size_t element_count,
                         size_t element_size,
                         size_t padding_size,
                         __cxa_vec_constructor constructor,
                         __cxa_vec_destructor destructor,
                         void* (*alloc)(size_t),
                         void  (*dealloc)(void*));

    void* __cxa_vec_new3(size_t element_count,
                         size_t element_size,
                         size_t padding_size,
                         __cxa_vec_constructor constructor,
                         __cxa_vec_destructor destructor,
                         void* (*alloc)(size_t),
                         void  (*dealloc)(void*, size_t));

    __cxa_vec_ctor_return_type
    __cxa_vec_ctor(void*  array_address,
                   size_t element_count,
                   size_t element_size,
                   __cxa_vec_constructor constructor,
                   __cxa_vec_destructor destructor);

    void __cxa_vec_dtor(void*  array_address,
                        size_t element_count,
                        size_t element_size,
                        __cxa_vec_destructor destructor);

    void __cxa_vec_cleanup(void* array_address,
                           size_t element_count,
                           size_t element_size,
                           __cxa_vec_destructor destructor);

    void __cxa_vec_delete(void*  array_address,
                          size_t element_size,
                          size_t padding_size,
                          __cxa_vec_destructor destructor);

    void __cxa_vec_delete2(void* array_address,
                           size_t element_size,
                           size_t padding_size,
                           __cxa_vec_destructor destructor,
                           void  (*dealloc)(void*));

    void __cxa_vec_delete3(void* array_address,
                           size_t element_size,
                           size_t padding_size,
                           __cxa_vec_destructor destructor,
                           void  (*dealloc) (void*, size_t));

    __cxa_vec_ctor_return_type
    __cxa_vec_cctor(void*  dest_array,
                    void*  src_array,
                    size_t element_count,
                    size_t element_size,
                    __cxa_vec_copy_constructor constructor,
                    __cxa_vec_destructor destructor );

  } // extern "C"

} // namespace __cxxabiv1

namespace abi = __cxxabiv1;

#if _GABIXX_ARM_ABI
// ARM-specific ABI additions. They  must be provided by the
// C++ runtime to simplify calling code generated by the compiler.
// Note that neither GCC nor Clang seem to use these, but this can
// happen when using machine code generated with other ocmpilers
// like RCVT.

namespace __aeabiv1 {
extern "C" {

using __cxxabiv1::__cxa_vec_constructor;
using __cxxabiv1::__cxa_vec_copy_constructor;
using __cxxabiv1::__cxa_vec_destructor;

void* __aeabi_vec_ctor_nocookie_nodtor(void* array_address,
                                       __cxa_vec_constructor constructor,
                                       size_t element_size,
                                       size_t element_count);

void* __aeabi_vec_ctor_cookie_nodtor(void* array_address,
                                     __cxa_vec_constructor constructor,
                                     size_t element_size,
                                     size_t element_count);

void* __aeabi_vec_cctor_nocookie_nodtor(
    void* dst_array,
    void* src_array,
    size_t element_size,
    size_t element_count,
    __cxa_vec_copy_constructor constructor);

void* __aeabi_vec_new_nocookie_noctor(size_t element_size,
                                      size_t element_count);

void* __aeabi_vec_new_nocookie(size_t element_size,
                               size_t element_count,
                               __cxa_vec_constructor constructor);

void* __aeabi_vec_new_cookie_nodtor(size_t element_size,
                                    size_t element_count,
                                    __cxa_vec_constructor constructor);

void* __aeabi_vec_new_cookie(size_t element_size,
                             size_t element_count,
                             __cxa_vec_constructor constructor,
                             __cxa_vec_destructor destructor);

void* __aeabi_vec_dtor(void* array_address,
                       __cxa_vec_destructor destructor,
                       size_t element_size,
                       size_t element_count);
  
void* __aeabi_vec_dtor_cookie(void* array_address,
                              __cxa_vec_destructor destructor);

void __aeabi_vec_delete(void* array_address,
                        __cxa_vec_destructor destructor);

void __aeabi_vec_delete3(void* array_address,
                         __cxa_vec_destructor destructor,
                         void (*dealloc)(void*, size_t));

void __aeabi_vec_delete3_nodtor(void* array_address,
                                void (*dealloc)(void*, size_t));

}  // extern "C"
}  // namespace __

#endif  // _GABIXX_ARM_ABI == 1

#endif /* defined(__GABIXX_CXXABI_H__) */

