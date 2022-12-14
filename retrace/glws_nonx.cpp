// This file is to support: FB/DFB/Wayland retrace

#include <assert.h>
#include <stdlib.h>

#include <iostream>

#include <dlfcn.h>

#include "glproc.hpp"
#include "glws.hpp"

#include <EGL/eglext.h>
#include "gc_vdk.h"

namespace glws {

static vdkEGL vdk_egl;
static char const *eglExtensions = NULL;
static bool has_EGL_KHR_create_context = false;
static bool check_dfb();

static EGLenum
translateAPI(Profile profile)
{
    switch (profile.api) {
    case glfeatures::API_GL:
        return EGL_OPENGL_API;
    case glfeatures::API_GLES:
        return EGL_OPENGL_ES_API;
    default:
        assert(0);
        return EGL_NONE;
    }
}


/* Must be called before
 *
 * - eglCreateContext
 * - eglGetCurrentContext
 * - eglGetCurrentDisplay
 * - eglGetCurrentSurface
 * - eglMakeCurrent (when its ctx parameter is EGL_NO_CONTEXT ),
 * - eglWaitClient
 * - eglWaitNative
 */
static void
bindAPI(EGLenum api)
{
    if (eglBindAPI(api) != EGL_TRUE) {
        std::cerr << "error: eglBindAPI failed\n";
        exit(1);
    }

}

class NonxVisual : public Visual
{
public:
    EGLConfig config;

    NonxVisual(Profile prof, EGLConfig cfg) :
        Visual(prof),
        config(cfg)
    {}

    virtual ~NonxVisual() {
    }
};

class NonxDrawable : public Drawable
{
public:
    EGLNativePixmapType native_pixmap;
    EGLenum api;

    NonxDrawable(const Visual *vis, int w, int h, const pbuffer_info *info) :
        Drawable (vis, w, h, info),
        api(EGL_OPENGL_ES_API)
    {
        native_pixmap = (EGLNativePixmapType)0;
        createVDKDrawable(w, h);
    }

    virtual ~NonxDrawable()
    {
        destroyVDKDrawable();
    }

    void
    createVDKDrawable(int w, int h, bool pixmap = false) {
        if(check_dfb()) {
            int width, height, stride, bpp;
            unsigned long phy;
            vdkGetDisplayInfo(vdk_egl.display, &width, &height, &phy, &stride, &bpp);
            Drawable::resize(width, height);
            w = width;
            h = height;
        }
        vdk_egl.window = vdkCreateWindow(vdk_egl.display, 0, 0, w, h);

        EGLConfig config = static_cast<const NonxVisual *>(visual)->config;

        if(pixmap)
        {
            native_pixmap = vdkCreatePixmap(vdk_egl.display, w, h, 32);
            vdk_egl.eglSurface = eglCreatePixmapSurface(vdk_egl.eglDisplay, config, native_pixmap, NULL);
            if(vdk_egl.eglSurface == EGL_NO_SURFACE) {
               std::cerr << "error: failed to create pixmap surface:" << eglGetError() << "\n";
               exit(1);
            }
            std::cerr<<"@@@@create eglPixmapSurface:" << vdk_egl.eglSurface << "\n";
        }

        else {
            eglWaitNative(EGL_CORE_NATIVE_ENGINE);
            vdk_egl.eglSurface = eglCreateWindowSurface(vdk_egl.eglDisplay, config, (EGLNativeWindowType)vdk_egl.window, NULL);
            if(vdk_egl.eglSurface == EGL_NO_SURFACE) {
                std::cerr << "error: failed to create window surface\n";
                exit(1);
            }
        }

        vdkSetWindowTitle(vdk_egl.window, "retrace");
        vdkShowWindow(vdk_egl.window);

        std::cerr << "window size " << w << " x " << h << "\n";
    }

    void
    destroyVDKDrawable(void) {
        if(vdk_egl.eglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(vdk_egl.eglDisplay, vdk_egl.eglSurface);
            eglWaitClient();
            vdk_egl.eglSurface = EGL_NO_SURFACE;
        }
        if(native_pixmap != NULL) {
            vdkDestroyPixmap(native_pixmap);
            native_pixmap = NULL;
        }
        if(vdk_egl.window != NULL) {
            /* Hide the window. */
            vdkHideWindow(vdk_egl.window);
            /* Destroy the window. */
            vdkDestroyWindow(vdk_egl.window);
            vdk_egl.window = NULL;
            eglWaitNative(EGL_CORE_NATIVE_ENGINE);
        }
    }

    void
    resize(int w, int h) {
        if (w == width && h == height) {
            return;
        }

        eglWaitClient();
        /* Hide the window. */
        vdkHideWindow(vdk_egl.window);
        /* Destroy the window. */
        vdkDestroyWindow(vdk_egl.window);
        vdk_egl.window = NULL;
        eglWaitNative(EGL_CORE_NATIVE_ENGINE);

        vdk_egl.window = vdkCreateWindow(vdk_egl.display, 0, 0, w, h);
        EGLContext currentContext = eglGetCurrentContext();
        EGLConfig config = static_cast<const NonxVisual *>(visual)->config;
        EGLSurface oldSurface = vdk_egl.eglSurface;
        vdk_egl.eglSurface = eglCreateWindowSurface(vdk_egl.eglDisplay, config, (EGLNativeWindowType)vdk_egl.window, NULL);
        eglMakeCurrent(vdk_egl.eglDisplay, vdk_egl.eglSurface, vdk_egl.eglSurface, currentContext);
        eglDestroySurface(vdk_egl.eglDisplay, oldSurface);

        vdkSetWindowTitle(vdk_egl.window, "retrace");
        vdkShowWindow(vdk_egl.window);

        std::cerr << "window resize " << w << " x " << h << "\n";
        Drawable::resize(w, h);
    }

    void
    show(void) {
        if (visible) {
            return;
        }

        eglWaitClient();

        vdkShowWindow(vdk_egl.window);

        eglWaitNative(EGL_CORE_NATIVE_ENGINE);

        Drawable::show();
    }

    void
    swapBuffers(void) {
        bindAPI(api);
        vdkSwapEGL(&vdk_egl);
    }
};

class NonxContext : public Context
{
public:
    EGLContext context;

    NonxContext(const Visual *vis, EGLContext ctx) :
        Context(vis),
        context(ctx)
    {}

    virtual ~NonxContext() {
        eglDestroyContext(vdk_egl.eglDisplay, context);
    }
};

/**
 * Load the symbols from the specified shared object into global namespace, so
 * that they can be later found by dlsym(RTLD_NEXT, ...);
 */
static void
load(const char *filename)
{
    if (!dlopen(filename, RTLD_GLOBAL | RTLD_LAZY)) {
        std::cerr << "error: unable to open " << filename << "\n";
        exit(1);
    }
}

static bool
check_dfb() {
    static int status = -1;
    if(status != -1)
        return (status == 1);

    void *h = dlopen("libdirectfb_gal.so", RTLD_GLOBAL | RTLD_LAZY);
    if (h == NULL) {
        status = 0;
        return false;
    }

    std::cerr << "It is DirectFB\n";

    dlclose(h);
    status = 1;
    return true;
}

void
init(void) {
    load("libEGL.so.1");

    vdk_egl.vdk = vdkInitialize();
    if(vdk_egl.vdk == 0) {
        std::cerr << "error: failed to init vdk\n";
        exit(1);
    }
    // vdk display as native display
    vdk_egl.display = vdkGetDisplay(vdk_egl.vdk);
    std::cerr << "native display: " << vdk_egl.display << "\n";

    vdk_egl.eglDisplay = eglGetDisplay(vdk_egl.display);
    if (vdk_egl.eglDisplay == EGL_NO_DISPLAY) {
        std::cerr << "error: unable to get EGL display\n";
        exit(1);
    }

    EGLint major, minor;
    if (!eglInitialize(vdk_egl.eglDisplay, &major, &minor)) {
        std::cerr << "error: unable to initialize EGL display\n";
        exit(1);
    }

    eglExtensions = eglQueryString(vdk_egl.eglDisplay, EGL_EXTENSIONS);
    has_EGL_KHR_create_context = checkExtension("EGL_KHR_create_context", eglExtensions);

}

void
cleanup(void) {
    if (vdk_egl.eglDisplay != EGL_NO_DISPLAY) {
        eglTerminate(vdk_egl.eglDisplay);
    }
    vdkDestroyDisplay(vdk_egl.display);
    vdkExit(vdk_egl.vdk);
}

Visual *
createVisual(bool doubleBuffer, unsigned samples, Profile profile) {
    EGLint api_bits;
    if (profile.api == glfeatures::API_GL) {
        api_bits = EGL_OPENGL_BIT;
        if (profile.core && !has_EGL_KHR_create_context) {
            return NULL;
        }
    } else if (profile.api == glfeatures::API_GLES) {
        switch (profile.major) {
        case 1:
            api_bits = EGL_OPENGL_ES_BIT;
            break;
        case 3:
            if (has_EGL_KHR_create_context) {
                api_bits = EGL_OPENGL_ES3_BIT;
                break;
            }
            /* fall-through */
        case 2:
            api_bits = EGL_OPENGL_ES2_BIT;
            break;
        default:
            return NULL;
        }
    } else {
        assert(0);
        return NULL;
    }

    Attributes<EGLint> attribs;
    attribs.add(EGL_SURFACE_TYPE, EGL_WINDOW_BIT);
    attribs.add(EGL_RED_SIZE, 1);
    attribs.add(EGL_GREEN_SIZE, 1);
    attribs.add(EGL_BLUE_SIZE, 1);
    attribs.add(EGL_ALPHA_SIZE, 1);
    attribs.add(EGL_DEPTH_SIZE, 1);
    attribs.add(EGL_STENCIL_SIZE, 1);
    attribs.add(EGL_RENDERABLE_TYPE, api_bits);
    attribs.end(EGL_NONE);

    EGLint num_configs = 0;
    if (!eglGetConfigs(vdk_egl.eglDisplay, NULL, 0, &num_configs) ||
        num_configs <= 0) {
        return NULL;
    }

    std::vector<EGLConfig> configs(num_configs);
    if (!eglChooseConfig(vdk_egl.eglDisplay, attribs, &configs[0], num_configs,  &num_configs) ||
        num_configs <= 0) {
        return NULL;
    }

    // We can't tell what other APIs the trace will use afterwards, therefore
    // try to pick a config which supports the widest set of APIs.
    int bestScore = -1;
    EGLConfig config = configs[0];
    for (EGLint i = 0; i < num_configs; ++i) {
        EGLint renderable_type = EGL_NONE;
        eglGetConfigAttrib(vdk_egl.eglDisplay, configs[i], EGL_RENDERABLE_TYPE, &renderable_type);
        int score = 0;
        assert(renderable_type & api_bits);
        renderable_type &= ~api_bits;
        if (renderable_type & EGL_OPENGL_ES2_BIT) {
            score += 1 << 4;
        }
        if (renderable_type & EGL_OPENGL_ES3_BIT) {
            score += 1 << 3;
        }
        if (renderable_type & EGL_OPENGL_ES_BIT) {
            score += 1 << 2;
        }
        if (renderable_type & EGL_OPENGL_BIT) {
            score += 1 << 1;
        }
        if (score > bestScore) {
            config = configs[i];
            bestScore = score;
        }
    }
    assert(bestScore >= 0);

    EGLint visual_id;
    if (!eglGetConfigAttrib(vdk_egl.eglDisplay, config, EGL_NATIVE_VISUAL_ID, &visual_id)) {
        assert(0);
        return NULL;
    }

    Visual *visual = NULL;

    if(config != 0) {
        std::cerr << "eglChooseConfig ok\n";
        visual = new NonxVisual(profile, config);
    }
    else {
        std::cerr << "error: failed to get egl config\n";
    }

    return visual;
}

Drawable *
createDrawable(const Visual *visual, int width, int height,
        const pbuffer_info *info)
{
    return new NonxDrawable(visual, width, height, info);
}

Context *
createContext(const Visual *_visual, Context *shareContext, bool debug)
{
    Profile profile = _visual->profile;
    const NonxVisual *visual = static_cast<const NonxVisual *>(_visual);
    EGLContext share_context = EGL_NO_CONTEXT;
    EGLContext context;
    Attributes<EGLint> attribs;

    if (shareContext) {
        share_context = static_cast<NonxContext*>(shareContext)->context;
    }

    int contextFlags = 0;
    if (profile.api == glfeatures::API_GL) {
        load("libGL.so.1");

        if (has_EGL_KHR_create_context) {
            attribs.add(EGL_CONTEXT_MAJOR_VERSION_KHR, profile.major);
            attribs.add(EGL_CONTEXT_MINOR_VERSION_KHR, profile.minor);
            int profileMask = profile.core ? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR : EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR;
            attribs.add(EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, profileMask);
            if (profile.forwardCompatible) {
                contextFlags |= EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR;
            }
        } else if (profile.versionGreaterOrEqual(3, 2)) {
            std::cerr << "error: EGL_KHR_create_context not supported\n";
            return NULL;
        }
    } else if (profile.api == glfeatures::API_GLES)
 {
        if (profile.major >= 2) {
            load("libGLESv2.so.2");
        } else {
            load("libGLESv1_CM.so.1");
        }

        if (has_EGL_KHR_create_context) {
            attribs.add(EGL_CONTEXT_MAJOR_VERSION_KHR, profile.major);
            attribs.add(EGL_CONTEXT_MINOR_VERSION_KHR, profile.minor);
        } else {
            attribs.add(EGL_CONTEXT_CLIENT_VERSION, profile.major);
        }
    } else {
        assert(0);
        return NULL;
    }

    if (debug) {
        contextFlags |= EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
    }
    if (contextFlags && has_EGL_KHR_create_context) {
        attribs.add(EGL_CONTEXT_FLAGS_KHR, contextFlags);
    }
    attribs.end(EGL_NONE);

    EGLenum api = translateAPI(profile);
    bindAPI(api);

    context = eglCreateContext(vdk_egl.eglDisplay, visual->config, share_context, attribs);
    if (!context) {
        if (debug) {
            // XXX: Mesa has problems with EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR
            // with OpenGL ES contexts, so retry without it
            return createContext(_visual, shareContext, false);
        }
        return NULL;
    }
    return new NonxContext(visual, context);
}

bool
makeCurrentInternal(Drawable *drawable,  Drawable *readable, Context *context)
{
    if (!drawable || !context) {
        return eglMakeCurrent(vdk_egl.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    } else {
        NonxDrawable *eglDrawable = static_cast<NonxDrawable *>(drawable);
        NonxContext *eglContext = static_cast<NonxContext *>(context);
        EGLBoolean ok;

        EGLenum api = translateAPI(eglContext->profile);
        bindAPI(api);

        ok = eglMakeCurrent(vdk_egl.eglDisplay, vdk_egl.eglSurface,
                            vdk_egl.eglSurface, eglContext->context);

        if (ok) {
            eglDrawable->api = api;
        }

        return ok;
    }
}

bool
processEvents(void)
{
    return false;
}

bool
bindTexImage(Drawable *pBuffer, int iBuffer) {
    std::cerr << "error: NonX::wglBindTexImageARB not implemented.\n";
    assert(pBuffer->pbuffer);
    return true;
}

bool
releaseTexImage(Drawable *pBuffer, int iBuffer) {
    std::cerr << "error: NonX::wglReleaseTexImageARB not implemented.\n";
    assert(pBuffer->pbuffer);
    return true;
}

bool
setPbufferAttrib(Drawable *pBuffer, const int *attribList) {
    assert(pBuffer->pbuffer);
    return true;
}

}
