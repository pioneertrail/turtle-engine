/*
 * GLAD OpenGL loader
 * Version 4.5
 * Language: C/C++
 * Specification: gl
 * APIs: gl=4.5
 * Profile: core
 * Extensions: none
 * Generator: https://glad.dav1d.de/
 */

#ifndef __glad_h_
#define __glad_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GLAD_PLATFORM_H_
#define GLAD_PLATFORM_H_

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define GLAD_PLATFORM_WIN32
#endif

#ifndef GLAD_PLATFORM_WIN32
#define GLAD_API_PTR
#else
#if defined(GLAD_GLAPI_EXPORT)
#if defined(GLAD_GLAPI_EXPORT_BUILD)
#define GLAD_API_PTR __declspec(dllexport)
#else
#define GLAD_API_PTR __declspec(dllimport)
#endif
#else
#define GLAD_API_PTR
#endif
#endif

#endif /* GLAD_PLATFORM_H_ */

#ifndef GLAD_GL_H_
#define GLAD_GL_H_

#include <KHR/khrplatform.h>
#include <windows.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* GLADloadproc(const char* name);

GLAD_API_PTR int gladLoadGL(void);
GLAD_API_PTR int gladLoadGLLoader(GLADloadproc* proc);

/* Constants */
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_FLOAT                          0x1406
#define GL_UNSIGNED_INT                   0x1405
#define GL_TRIANGLES                      0x0004
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_BLEND                          0x0BE2
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE                            1
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DEPTH_TEST                     0x0B71
#define GL_TEXTURE_2D                     0x0DE1
#define GL_RGBA                           0x1908
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_LINEAR                         0x2601

/* Function pointers */
typedef void (GLAD_API_PTR* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (GLAD_API_PTR* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (GLAD_API_PTR* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void (GLAD_API_PTR* PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef void (GLAD_API_PTR* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
typedef void (GLAD_API_PTR* PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void (GLAD_API_PTR* PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (GLAD_API_PTR* PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);
typedef void (GLAD_API_PTR* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (GLAD_API_PTR* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (GLAD_API_PTR* PFNGLVERTEXATTRIBDIVISORPROC)(GLuint index, GLuint divisor);
typedef void (GLAD_API_PTR* PFNGLDRAWELEMENTSINSTANCEDPROC)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
typedef void (GLAD_API_PTR* PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void* indices);
typedef void (GLAD_API_PTR* PFNGLGENTEXTURESPROC)(GLsizei n, GLuint* textures);
typedef void (GLAD_API_PTR* PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void (GLAD_API_PTR* PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
typedef void (GLAD_API_PTR* PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
typedef void (GLAD_API_PTR* PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint* textures);
typedef void (GLAD_API_PTR* PFNGLENABLEPROC)(GLenum cap);
typedef void (GLAD_API_PTR* PFNGLDISABLEPROC)(GLenum cap);
typedef void (GLAD_API_PTR* PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);

/* Function declarations */
GLAD_API_PTR PFNGLGENBUFFERSPROC glad_glGenBuffers;
#define glGenBuffers glad_glGenBuffers
GLAD_API_PTR PFNGLBINDBUFFERPROC glad_glBindBuffer;
#define glBindBuffer glad_glBindBuffer
GLAD_API_PTR PFNGLBUFFERDATAPROC glad_glBufferData;
#define glBufferData glad_glBufferData
GLAD_API_PTR PFNGLBUFFERSUBDATAPROC glad_glBufferSubData;
#define glBufferSubData glad_glBufferSubData
GLAD_API_PTR PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers;
#define glDeleteBuffers glad_glDeleteBuffers
GLAD_API_PTR PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays;
#define glGenVertexArrays glad_glGenVertexArrays
GLAD_API_PTR PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray;
#define glBindVertexArray glad_glBindVertexArray
GLAD_API_PTR PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays;
#define glDeleteVertexArrays glad_glDeleteVertexArrays
GLAD_API_PTR PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
#define glEnableVertexAttribArray glad_glEnableVertexAttribArray
GLAD_API_PTR PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer;
#define glVertexAttribPointer glad_glVertexAttribPointer
GLAD_API_PTR PFNGLVERTEXATTRIBDIVISORPROC glad_glVertexAttribDivisor;
#define glVertexAttribDivisor glad_glVertexAttribDivisor
GLAD_API_PTR PFNGLDRAWELEMENTSINSTANCEDPROC glad_glDrawElementsInstanced;
#define glDrawElementsInstanced glad_glDrawElementsInstanced
GLAD_API_PTR PFNGLDRAWELEMENTSPROC glad_glDrawElements;
#define glDrawElements glad_glDrawElements
GLAD_API_PTR PFNGLGENTEXTURESPROC glad_glGenTextures;
#define glGenTextures glad_glGenTextures
GLAD_API_PTR PFNGLBINDTEXTUREPROC glad_glBindTexture;
#define glBindTexture glad_glBindTexture
GLAD_API_PTR PFNGLTEXIMAGE2DPROC glad_glTexImage2D;
#define glTexImage2D glad_glTexImage2D
GLAD_API_PTR PFNGLTEXPARAMETERIPROC glad_glTexParameteri;
#define glTexParameteri glad_glTexParameteri
GLAD_API_PTR PFNGLDELETETEXTURESPROC glad_glDeleteTextures;
#define glDeleteTextures glad_glDeleteTextures
GLAD_API_PTR PFNGLENABLEPROC glad_glEnable;
#define glEnable glad_glEnable
GLAD_API_PTR PFNGLDISABLEPROC glad_glDisable;
#define glDisable glad_glDisable
GLAD_API_PTR PFNGLBLENDFUNCPROC glad_glBlendFunc;
#define glBlendFunc glad_glBlendFunc

#ifdef __cplusplus
}
#endif

#endif /* GLAD_GL_H_ */

#ifdef __cplusplus
}
#endif

#endif /* __glad_h_ */ 