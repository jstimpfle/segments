#ifndef SEGMENTS_GFX_H_INCLUDED
#define SEGMENTS_GFX_H_INCLUDED

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
};

enum {
        GRAFIKUNIFORMTYPE_BOOL,
        GRAFIKUNIFORMTYPE_INT,
        GRAFIKUNIFORMTYPE_UINT,
        GRAFIKUNIFORMTYPE_FLOAT,
        GRAFIKUNIFORMTYPE_DOUBLE,
        GRAFIKUNIFORMTYPE_VEC2,
        GRAFIKUNIFORMTYPE_VEC3,
        GRAFIKUNIFORMTYPE_VEC4,
        GRAFIKUNIFORMTYPE_MAT2,
        GRAFIKUNIFORMTYPE_MAT3,
        GRAFIKUNIFORMTYPE_MAT4,
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

void set_uniform_1f(int program, int location, float x);
void set_uniform_2f(int program, int location, float x, float y);
void set_uniform_3f(int program, int location, float x, float y, float z);
void set_uniform_4f(int program, int location, float x, float y, float z, float w);
void set_uniform_mat2f(int program, int location, const float *fourFloats);
void set_uniform_mat3f(int program, int location, const float *nineFloats);
void set_uniform_mat4f(int program, int location, const float *sixteenFloats);

struct Vec2 {
        float x;
        float y;
};

struct Vec3 {
        float x;
        float y;
        float z;
};

struct Vec4 {
        float x;
        float y;
        float z;
        float w;
};

struct Mat2 {
        float mat[2][2];
};

struct Mat3 {
        float mat[3][3];
};

struct Mat4 {
        float mat[4][4];
};

void set_uniform_1f(int program, int location, float x);
void set_uniform_2f(int program, int location, float x, float y);
void set_uniform_3f(int program, int location, float x, float y, float z);
void set_uniform_4f(int program, int location, float x, float y, float z, float w);
void set_uniform_mat2f(int program, int location, const float *fourFloats);
void set_uniform_mat3f(int program, int location, const float *nineFloats);
void set_uniform_mat4f(int program, int location, const float *sixteenFloats);

void do_gfx(void);

#endif
