#pragma once

// macOS-only: creates an NSOpenGLContext (legacy profile, software rendering
// allowed — no NSOpenGLPFAAccelerated) and attaches it to the given GLFW
// window so ImGui can render via CGL even on GitHub Actions CI runners where
// NSGL hardware-accelerated context creation fails.

#ifdef __APPLE__

#ifdef __cplusplus
extern "C" {
#endif

// Create an NSOpenGLContext with software-renderer-compatible pixel format
// and attach it to the content view of the NSWindow returned by
// glfwGetCocoaWindow().  Makes the context current on success.
// Returns true on success.
bool grey_macos_create_software_gl_context(void* ns_window_ptr);

// Flush the software GL context (call instead of glfwSwapBuffers).
void grey_macos_gl_flush(void);

// Destroy the software GL context created by the call above.
void grey_macos_destroy_gl_context(void);

#ifdef __cplusplus
}
#endif

#endif // __APPLE__
