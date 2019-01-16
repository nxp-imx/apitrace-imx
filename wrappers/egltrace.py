##########################################################################
#
# Copyright 2011 LunarG, Inc.
# All Rights Reserved.
#
# Based on glxtrace.py, which has
#
#   Copyright 2011 Jose Fonseca
#   Copyright 2008-2010 VMware, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
##########################################################################/


"""EGL tracing generator."""


from gltrace import GlTracer
from specs.stdapi import Module, API
from specs.glapi import glapi
from specs.eglapi import eglapi


class EglTracer(GlTracer):

    def isFunctionPublic(self, function):
        # The symbols visible in libEGL.so can vary, so expose them all
        return True

    getProcAddressFunctionNames = [
        "eglGetProcAddress",
    ]

    def traceFunctionImplBody(self, function):
        if function.name == 'glTexDirectMapVIV':
            print('    // prevent loop call')
            print('    glTexDirectVIVMap(target, width, height, format, Logical, Physical);')
            return

        if function.name == 'glTexDirectInvalidateVIV':
            print('    // get current texture')
            print('    GLint tex = 0;')
            print('    int32_t size = 0;')
            print('    int32_t ysize = 0;')
            print('    int32_t usize = 0;')
            print('    int32_t vsize = 0;')
            print('    _glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex);')
            print('    if(tex == 0)')
            print('    {')
            print('        return;')
            print('    }')
            print('    TEXDIRECTVIVDATA &data = _directTextureDataMap[tex];')
            print('    switch(data.format){')
            print('    case GL_VIV_YV12:')
            print('    case GL_VIV_I420:')
            print('        ysize=data.width * data.height;')
            print('        usize=ysize/4;')
            print('        vsize=usize;')
            print('        break;')
            print('    case GL_VIV_NV12:')
            print('    case GL_VIV_NV21:')
            print('        ysize=data.width * data.height;')
            print('        usize=ysize/2;')
            print('        vsize=0;')
            print('        break;')
            print('    case GL_RGBA:')
            print('    case GL_BGRA_EXT:')
            print('        ysize=data.width * data.height *4;')
            print('        usize=0;')
            print('        vsize=0;')
            print('        break;')
            print('    case GL_RGB:')
            print('        ysize=data.width * data.height *3;')
            print('        usize=0;')
            print('        vsize=0;')
            print('        break;')
            print('    case GL_VIV_YUY2:')
            print('    case GL_VIV_UYVY:')
            print('    case GL_RGB565_OES:')
            print('    case GL_LUMINANCE8_ALPHA8_EXT:')
            print('        ysize=data.width * data.height *2;')
            print('        usize=0;')
            print('        vsize=0;')
            print('        break;')
            print('    case GL_ALPHA:')
            print('        ysize=data.width * data.height;')
            print('        usize=0;')
            print('        vsize=0;')
            print('        break;')
            print('    default:')
            print('        return;')
            print('    }')
            print('    if (NULL==(GLvoid*)data.logical) {')
            print('        if (ysize > 0) {')
            self.emit_memcpy('(GLvoid*)data.planes[0]', 'ysize')
            print('         }')
            print('        if (usize > 0) {')
            self.emit_memcpy('(GLvoid*)data.planes[1]', 'usize')
            print('         }')
            print('        if (vsize > 0) {')
            self.emit_memcpy('(GLvoid*)data.planes[2]', 'vsize')
            print('         }')
            print('    } else {')
            print('        size = ysize + usize + vsize;')
            print('        if (size > 0) {')
            self.emit_memcpy('(GLvoid*)data.logical', 'size')
            print('         }')
            print('    }')

        if function.name == 'eglGetProcAddress':
            print('    procname = __get_alias_func_name(procname);')

        GlTracer.traceFunctionImplBody(self, function)

        if function.name == 'glTexDirectVIV':
            print('    // get current texture')
            print('    GLint tex = 0;')
            print('    _glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex);')
            print('    if(tex != 0)')
            print('    {')
            print('        TEXDIRECTVIVDATA &data = _directTextureDataMap[tex];')
            print('        data.width = width;')
            print('        data.height = height;')
            print('        data.format = format;')
            print('        data.planes[0] = pixels[0];')
            print('        data.planes[1] = pixels[1];')
            print('        data.planes[2] = pixels[2];')
            print('    }')

        if function.name == 'glTexDirectVIVMap' or function.name == 'glTexDirectTiledMapVIV':
            print('    // get current texture')
            print('    GLint tex = 0;')
            print('    _glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex);')
            print('    if(tex != 0)')
            print('    {')
            print('        TEXDIRECTVIVDATA &data = _directTextureDataMap[tex];')
            print('        data.width = width;')
            print('        data.height = height;')
            print('        data.format = format;')
            print('        data.logical = *Logical; // Logical != NULL')
            print('        data.physical = *Physical;')
            print('    }')

        if function.name == 'eglCreateContext':
            print('    if (_result != EGL_NO_CONTEXT)')
            print('        gltrace::createContext((uintptr_t)_result, (uintptr_t)share_context);')

        if function.name == 'eglMakeCurrent':
            print(r'    if (_result) {')
            print(r'        // update the profile')
            print(r'        if (ctx != EGL_NO_CONTEXT) {')
            print(r'            gltrace::setContext((uintptr_t)ctx);')
            print(r'            gltrace::Context *tr = gltrace::getContext();')
            print(r'            EGLint api = EGL_OPENGL_ES_API;')
            print(r'            _eglQueryContext(dpy, ctx, EGL_CONTEXT_CLIENT_TYPE, &api);')
            print(r'            if (api == EGL_OPENGL_API) {')
            print(r'                assert(tr->profile.api == glfeatures::API_GL);')
            print(r'            } else if (api == EGL_OPENGL_ES_API) {')
            print(r'                EGLint client_version = 1;')
            print(r'                _eglQueryContext(dpy, ctx, EGL_CONTEXT_CLIENT_VERSION, &client_version);')
            print(r'                if (tr->profile.api != glfeatures::API_GLES ||')
            print(r'                    tr->profile.major < client_version) {')
            print(r'                    std::string version = tr->profile.str();')
            print(r'                    os::log("apitrace: warning: eglMakeCurrent: expected OpenGL ES %i.x context, but got %s\n",')
            print(r'                            client_version, version.c_str());')
            print(r'                }')
            print(r'            } else {')
            print(r'                assert(0);')
            print(r'            }')
            print(r'        } else {')
            print(r'            gltrace::clearContext();')
            print(r'        }')
            print(r'    }')

        if function.name == 'eglDestroyContext':
            print('    if (_result) {')
            print('        gltrace::releaseContext((uintptr_t)ctx);')
            print('    }')

        if function.name == 'glEGLImageTargetTexture2DOES':
            print('    image_info *info = _EGLImageKHR_get_image_info(target, image);')
            print('    if (info) {')
            print('        GLint level = 0;')
            print('        GLint internalformat = info->internalformat;')
            print('        GLsizei width = info->width;')
            print('        GLsizei height = info->height;')
            print('        GLint border = 0;')
            print('        GLenum format = info->format;')
            print('        GLenum type = info->type;')
            print('        const GLvoid * pixels = info->pixels;')
            self.emitFakeTexture2D()
            print('        _EGLImageKHR_free_image_info(info);')
            print('    }')


if __name__ == '__main__':
    print('#include <stdlib.h>')
    print('#include <string.h>')
    print()
    print('#include "trace_writer_local.hpp"')
    print()
    print('// To validate our prototypes')
    print('#define GL_GLEXT_PROTOTYPES')
    print('#define EGL_EGLEXT_PROTOTYPES')
    print()
    print('#include "glproc.hpp"')
    print('#include "glsize.hpp"')
    print('#include "eglsize.hpp"')
    print()
    print('static const char *__get_alias_func_name(const char *origFunName)')
    print('{')
    print('    /* Vivante driver uses alias name for following OES/EXT functions, that means dlsym for thoese functions will fail */')
    print('    static const char * __glExtProcAlias[][2] =')
    print('    {')
    print('        /* Extension API alias for GL_OES_texture_3D */')
    print('        {"glTexImage3DOES",               "glTexImage3D"},')
    print('        {"glTexSubImage3DOES",            "glTexSubImage3D"},')
    print('        {"glCopyTexSubImage3DOES",        "glCopyTexSubImage3D"},')
    print('        {"glCompressedTexImage3DOES",     "glCompressedTexImage3D"},')
    print('        {"glCompressedTexSubImage3DOES",  "glCompressedTexSubImage3D"},')
    print()
    print('        /* Extension API alias for GL_OES_get_program_binary */')
    print('        {"glGetProgramBinaryOES",         "glGetProgramBinary"},')
    print('        {"glProgramBinaryOES",            "glProgramBinary"},')
    print()
    print('        /* Extension API alias for GL_OES_vertex_array_object */')
    print('        {"glBindVertexArrayOES",          "glBindVertexArray"},')
    print('        {"glDeleteVertexArraysOES",       "glDeleteVertexArrays"},')
    print('        {"glGenVertexArraysOES",          "glGenVertexArrays"},')
    print('        {"glIsVertexArrayOES",            "glIsVertexArray"},')
    print()
    print('        /* Extension API alias for GL_OES_blend_minmax */')
    print('        {"glBlendEquationEXT",            "glBlendEquation"}')
    print('    };')
    print()
    print('    int count = sizeof(__glExtProcAlias) / sizeof(__glExtProcAlias[0]);')
    print('    int i;')
    print()
    print('    for(i=0; i<count; i++)')
    print('    {')
    print('        if(strcmp(__glExtProcAlias[i][0], origFunName) == 0)')
    print('            return __glExtProcAlias[i][1];')
    print('    }')
    print()
    print('    return origFunName;')
    print('}')
    
    print()
    print('typedef struct TexDirectVivData')
    print('{')
    print('    int width;')
    print('    int height;')
    print('    GLenum format;')
    print('    GLvoid*planes[3]; // used for glTexDirectVIV')
    print('    GLvoid*logical;   // used for glTexDirectVIVMap/glTexDirectMapVIV/glTexDirectTiledMapVIV')
    print('    GLuint physical;')
    print('}TEXDIRECTVIVDATA;')
    print()
    print('static std::map<GLint, TEXDIRECTVIVDATA> _directTextureDataMap;')
    print()
    print('#define GL_VIV_YV12                        0x8FC0')
    print('#define GL_VIV_NV12                        0x8FC1')
    print('#define GL_VIV_YUY2                        0x8FC2')
    print('#define GL_VIV_UYVY                        0x8FC3')
    print('#define GL_VIV_NV21                        0x8FC4')
    print('#define GL_VIV_I420                        0x8FC5')
    print()

    module = Module()
    module.mergeModule(eglapi)
    module.mergeModule(glapi)
    api = API()
    api.addModule(module)
    tracer = EglTracer()
    tracer.traceApi(api)

    print(r'''
#if defined(ANDROID)

/*
 * Undocumented Android extensions used by the wrappers which have bound
 * information passed to it, but is currently ignored, so probably unreliable.
 *
 * See:
 * https://github.com/android/platform_frameworks_base/search?q=glVertexPointerBounds
 */

extern "C" PUBLIC
void APIENTRY glColorPointerBounds(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer, GLsizei count) {
    (void)count;
    glColorPointer(size, type, stride, pointer);
}

extern "C" PUBLIC
void APIENTRY glNormalPointerBounds(GLenum type, GLsizei stride, const GLvoid * pointer, GLsizei count) {
    (void)count;
    glNormalPointer(type, stride, pointer);
}

extern "C" PUBLIC
void APIENTRY glTexCoordPointerBounds(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer, GLsizei count) {
    (void)count;
    glTexCoordPointer(size, type, stride, pointer);
}

extern "C" PUBLIC
void APIENTRY glVertexPointerBounds(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer, GLsizei count) {
    (void)count;
    glVertexPointer(size, type, stride, pointer);
}

extern "C" PUBLIC
void GL_APIENTRY glPointSizePointerOESBounds(GLenum type, GLsizei stride, const GLvoid *pointer, GLsizei count) {
    (void)count;
    glPointSizePointerOES(type, stride, pointer);
}

extern "C" PUBLIC
void APIENTRY glMatrixIndexPointerOESBounds(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, GLsizei count) {
    (void)count;
    glMatrixIndexPointerOES(size, type, stride, pointer);
}

extern "C" PUBLIC
void APIENTRY glWeightPointerOESBounds(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, GLsizei count) {
    (void)count;
    glWeightPointerOES(size, type, stride, pointer);
}

/*
 * There is also a glVertexAttribPointerBounds in
 * https://github.com/android/platform_frameworks_base/blob/master/opengl/tools/glgen/stubs/gles11/GLES20cHeader.cpp
 * but is it not exported.
 */

#endif /* ANDROID */
''')
