/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "bsp/charInputOutput/Bspio.h"

#include "charInputOutput/charIo.h"

#include <printf/printf.h>

#include <cstring>

#ifdef __cplusplus
extern "C"
{
#endif

// NOLINTBEGIN(bugprone-reserved-identifier): Explanation for this suppression is below
/**
 * \par Linking this module
 * The diab compiler is providing these methods for char in-/output. They will
 * be used per default. To use the compiler's printf/scanf functions for debug
 * in-/output, the defaults have to be overwritten. This can be achieved by
 * linking this module's object, e.g: ld -l:src/bios/bspIO/chario.o
 *
 */
int __inchar(void) { return (charIO__inchar()); }

int __inedit(void) { return __inchar(); }

int __outchar(int const c, int const last) { return charIO__outchar(c, last); }

int __outedit(int const c, int const last)
{
    if (c == 0xA)
    {
        (void)__outchar(0xD, last);
    }
    return __outchar(c, last);
}

// NOLINTEND(bugprone-reserved-identifier)

int vsnprintf(char* buf, size_t const maxsize, char const* fmt, va_list args)
{
    return vsnprintf_(buf, maxsize, fmt, args);
}

int vsprintf(char* buf, char const* fmt, va_list args) { return vsprintf_(buf, fmt, args); }

// C standard library compatibility wrappers — variadic signatures required by API contract.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
int snprintf(char* buf, size_t maxsize, char const* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    auto const ret = vsnprintf_(buf, maxsize, fmt, args);
    va_end(args);
    return ret;
}

int sprintf(char* buf, char const* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    auto const ret = vsprintf_(buf, fmt, args);
    va_end(args);
    return ret;
}

void putchar_(char const character)
{
    // __outedit takes an int - cast to unsigned before passing along
    unsigned char const u_char = static_cast<unsigned char>(character);
    (void)__outedit(static_cast<int>(u_char), 0);
}

void fctprintf_helper(char c, void* /* satisfy fctprintf outFunc reqs */)
{
    // __outedit takes an int - cast to unsigned before passing along
    unsigned char const u_char = static_cast<unsigned char>(c);
    (void)__outedit(static_cast<int>(u_char), 0);
}

/*
 * Retarget the C library's printf() to the BSP character output.
 *
 * NOTE: do NOT use `#pragma weak printf = <impl>` for this. In C++ clang applies C++ name
 * mangling to the pragma operand even inside an `extern "C"` block, so it emits a weak
 * `_Z6printfPKcz` symbol instead of `printf`. The retargeting is then silently ineffective and
 * printf() calls end up in the (unretargeted) C library implementation, producing no output.
 * A weak definition with C linkage works for both gcc and clang.
 */
__attribute__((weak)) int printf(char const* format, ...)
{
    va_list args;
    va_start(args, format);
    auto const ret = vfctprintf(&fctprintf_helper, nullptr, format, args);
    va_end(args);

    return ret;
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)

#ifdef __cplusplus
} // extern "C"
#endif
