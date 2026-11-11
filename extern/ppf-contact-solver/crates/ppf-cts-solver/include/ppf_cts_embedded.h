/* SPDX-FileCopyrightText: ZOZO, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0 */
#ifndef PPF_CTS_EMBEDDED_H
#define PPF_CTS_EMBEDDED_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initial CPU-only ABI boundary. Scene pointers must refer to Rust DataSet and
 * ParamSet values created by code built against this solver; their layouts are
 * not a stable C representation, and this header does not provide constructors.
 */
uint32_t ppf_cts_embedded_api_version(void);
bool ppf_cts_embedded_begin(void);
bool ppf_cts_embedded_initialize(const void *rust_dataset, const void *rust_params);
bool ppf_cts_embedded_step(void *rust_step_result);
bool ppf_cts_embedded_fetch(void);

/* Returns required bytes including NUL; a null buffer performs a size query. */
size_t ppf_cts_embedded_last_error(char *buffer, size_t capacity);

bool ppf_cts_embedded_end(void);

#ifdef __cplusplus
}
#endif

#endif
