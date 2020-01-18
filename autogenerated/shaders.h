#ifndef AUTOGENERATED_SHADERS_H_INCLUDED
#define AUTOGENERATED_SHADERS_H_INCLUDED

#include <glsl-processor.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
        PROGRAM_line,
        PROGRAM_circle,
        NUM_PROGRAM_KINDS,
};

enum {
        SHADER_line_vert,
        SHADER_line_frag,
        SHADER_circle_vert,
        SHADER_circle_frag,
        NUM_SHADER_KINDS,
};

enum {
        NUM_UNIFORM_KINDS,
};

enum {
        ATTRIBUTE_line_color,
        ATTRIBUTE_line_deflection,
        ATTRIBUTE_line_normal,
        ATTRIBUTE_line_position,
        ATTRIBUTE_circle_centerPoint,
        ATTRIBUTE_circle_color,
        ATTRIBUTE_circle_diff,
        ATTRIBUTE_circle_radius,
        NUM_ATTRIBUTE_KINDS,
};

extern const struct SM_ShaderInfo smShaderInfo[NUM_SHADER_KINDS];
extern const struct SM_ProgramInfo smProgramInfo[NUM_PROGRAM_KINDS];
extern const struct SM_LinkInfo smLinkInfo[];
extern const int numLinkInfos;
extern const struct SM_UniformInfo smUniformInfo[NUM_UNIFORM_KINDS];
extern const struct SM_AttributeInfo smAttributeInfo[NUM_ATTRIBUTE_KINDS];
extern const struct SM_Description smDescription;

extern GfxShader gfxShader[NUM_SHADER_KINDS];
extern GfxProgram gfxProgram[NUM_PROGRAM_KINDS];
extern GfxUniformLocation gfxUniformLocation[NUM_UNIFORM_KINDS];
extern GfxAttributeLocation gfxAttributeLocation[NUM_ATTRIBUTE_KINDS];



#ifdef __cplusplus

#endif // #ifdef __cplusplus

#ifdef __cplusplus
} // extern "C" {
#endif
#endif
