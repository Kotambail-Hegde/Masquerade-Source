#pragma once

// ============================================================================
// macros.h
//
// The small set of project wide macros and types that BOTH helpers.h and masquerade_io.h need.
// They used to live in helpers.h; they were moved here so masquerade_io.h can use the project style
// (RETURN, YES, NO, ZERO, FLAG, MASQ_INLINE) without including helpers.h, which includes masquerade_io.h
// (that would be a circular include).
//
// This header is dependency free (no includes) and safe on every platform, including the Pico.
// Do not add anything here that needs other project headers.
// ============================================================================

// --- Numeric constants --------------------------------
#define ZERO                                        0

// --- Boolean style constants --------------------------
#define YES                                         true
#define NO                                          false

// --- Misc flow / utility macros ----------------------
#define RETURN                                      return

// --- Inline / hint attributes ------------------------
#if defined(_MSC_VER)
#define MASQ_INLINE         __forceinline
#define MASQ_SUGGEST_INLINE __inline
#elif defined(__GNUC__) || defined(__clang__)
#define MASQ_INLINE         __attribute__((always_inline)) inline
#define MASQ_SUGGEST_INLINE inline
#else
#define MASQ_INLINE         inline
#define MASQ_SUGGEST_INLINE inline
#endif

// --- Types --------------------------------------------
typedef bool     FLAG;
