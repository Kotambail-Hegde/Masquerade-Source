// --- Emscripten / WebGL / GLES3 ---
#ifdef __EMSCRIPTEN__
    // Tell ImGui OpenGL backend to use GLES3 path
#ifndef IMGUI_IMPL_OPENGL_ES3
#define IMGUI_IMPL_OPENGL_ES3
#endif
// ImGui *should* include this automatically, but its detection fails under Emscripten.
// Manually forcing it is correct.
#include <GLES3/gl3.h>
#endif

// --- ARM Linux / Raspberry Pi / GLES3 ---
#if !defined(__EMSCRIPTEN__) && (defined(__arm__) || defined(__aarch64__))
    // Raspberry Pi uses OpenGL ES 3
#ifndef IMGUI_IMPL_OPENGL_ES3
#define IMGUI_IMPL_OPENGL_ES3
#endif
#include <GLES3/gl3.h>
#endif

// --- PRIu64 / PRId64 support (inttypes.h) ---
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
// Only include <inttypes.h> if not already included
#ifndef PRId64
#include <inttypes.h>
#endif