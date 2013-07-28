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
#include <gabixx_config.h>


#include <exception>
#include <typeinfo>
#include <unwind.h>

namespace __cxxabiv1
{
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

  } // extern "C"

} // namespace __cxxabiv1

namespace abi = __cxxabiv1;

#endif /* defined(__GABIXX_CXXABI_H__) */

