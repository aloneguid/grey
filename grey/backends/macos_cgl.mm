// macOS CGL software-rendering helper for the GLFW/OpenGL3 backend.
//
// On GitHub Actions macOS runners GLFW's NSGL backend always requests
// NSOpenGLPFAAccelerated, which fails with
//   "NSGL: Failed to find a suitable pixel format"
// because the CI display session does not expose hardware-accelerated OpenGL.
//
// This file creates an NSOpenGLContext *without* NSOpenGLPFAAccelerated (so
// macOS is free to pick the generic software renderer) and attaches it to the
// content view of the GLFW NSWindow.  ImGui's OpenGL3 backend then works
// normally with glsl "#version 120" since the software renderer supports
// OpenGL 2.1.
//
// Note: This file is compiled WITHOUT ARC (-fno-objc-arc). Manual retain/
// release is used intentionally so the file works both in ARC and non-ARC
// build environments without requiring a per-file compiler flag override.

#import <AppKit/AppKit.h>
#include <OpenGL/OpenGL.h>
#include <stdio.h>

#include "macos_cgl.h"

static NSOpenGLContext* s_ctx = nil;

bool grey_macos_create_software_gl_context(void* ns_window_ptr) {
    NSWindow* window = (NSWindow*)ns_window_ptr;

    // Build a pixel-format attribute list that does NOT include
    // NSOpenGLPFAAccelerated.  This allows macOS to fall back to the
    // generic (software) renderer, which always succeeds.
    NSOpenGLPixelFormatAttribute attribs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        NSOpenGLPFAColorSize,     (NSOpenGLPixelFormatAttribute)24,
        NSOpenGLPFAAlphaSize,     (NSOpenGLPixelFormatAttribute)8,
        NSOpenGLPFADepthSize,     (NSOpenGLPixelFormatAttribute)24,
        NSOpenGLPFADoubleBuffer,
        // NOTE: NO NSOpenGLPFAAccelerated — this is the key difference from
        // GLFW's NSGL backend which always adds NSOpenGLPFAAccelerated.
        (NSOpenGLPixelFormatAttribute)0
    };

    NSOpenGLPixelFormat* pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
    if (!pf) {
        fprintf(stderr, "macOS GL fallback: NSOpenGLPixelFormat failed\n");
        return false;
    }

    s_ctx = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
    [pf release];

    if (!s_ctx) {
        fprintf(stderr, "macOS GL fallback: NSOpenGLContext init failed\n");
        return false;
    }

    // Make the content view layer-backed so NSOpenGLContext can render into it
    // (required on macOS 10.14 Mojave and later).
    NSView* view = [window contentView];
    [view setWantsLayer:YES];

    [s_ctx setView:view];
    [s_ctx makeCurrentContext];

    fprintf(stderr, "macOS GL fallback: software GL context created successfully\n");
    return true;
}

void grey_macos_gl_flush(void) {
    if (s_ctx)
        [s_ctx flushBuffer];
}

void grey_macos_destroy_gl_context(void) {
    if (s_ctx) {
        [NSOpenGLContext clearCurrentContext];
        [s_ctx clearDrawable];
        [s_ctx release];
        s_ctx = nil;
    }
}
