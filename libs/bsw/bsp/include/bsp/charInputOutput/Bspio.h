/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Embedded version of printf(), retargeted to the BSP character output.
 *
 * This is a weak definition overriding the one of the C library, so an application may still
 * provide its own strong implementation.
 *
 * \return On success, the total number of characters written is returned.
 *          On failure, a negative value is returned.
 */
int printf(char const* format, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif
