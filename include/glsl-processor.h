#ifndef GLSL_PROCESSOR_H_INCLUDED
#define GLSL_PROCESSOR_H_INCLUDED

enum {
        SHADERTYPE_VERTEX,
        SHADERTYPE_FRAGMENT,
        NUM_SHADERTYPE_KINDS
};

enum {
        GRAFIKATTRTYPE_BOOL,
        GRAFIKATTRTYPE_INT,
        GRAFIKATTRTYPE_UINT,
        GRAFIKATTRTYPE_FLOAT,
        GRAFIKATTRTYPE_DOUBLE,
        GRAFIKATTRTYPE_VEC2,
        GRAFIKATTRTYPE_VEC3,
        GRAFIKATTRTYPE_VEC4,
        GRAFIKATTRTYPE_MAT2,
        GRAFIKATTRTYPE_MAT3,
        GRAFIKATTRTYPE_MAT4,
};

typedef int GfxShader;
typedef int GfxProgram;
typedef int GfxUniformLocation;
typedef int GfxAttributeLocation;
typedef int GfxVBO;
typedef int GfxVAO;

struct SM_ShaderInfo {
        int shadertypeKind;
        const char *name;
        const char *filepath;
};

struct SM_ProgramInfo {
        const char *name;
};

struct SM_LinkInfo {
        int programIndex;
        int shaderIndex;
};

struct SM_UniformInfo {
        int programIndex;
        int typeKind;
        const char *name;
};

struct SM_AttributeInfo {
        int programIndex;
        int typeKind;
        const char *name;
};

struct SM_Description {
        // description of shaders to compile
        const struct SM_ProgramInfo *programInfo;
        const struct SM_ShaderInfo *shaderInfo;
        const struct SM_LinkInfo *linkInfo;
        const struct SM_AttributeInfo *attributeInfo;
        const struct SM_UniformInfo *uniformInfo;

        // output locations for compiled shaders / metadata
        GfxProgram *gfxProgram;
        GfxShader *gfxShader;
        GfxUniformLocation *gfxUniformLocation;
        GfxAttributeLocation *gfxAttributeLocation;

        int numPrograms;
        int numShaders;
        int numLinks;
        int numUniforms;
        int numAttributes;
};

#endif
