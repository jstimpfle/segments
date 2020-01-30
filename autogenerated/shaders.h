#ifndef AUTOGENERATED_SHADERS_H_INCLUDED
#define AUTOGENERATED_SHADERS_H_INCLUDED

#include <segments/gfx.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
        PROGRAM_arc,
        PROGRAM_circle,
        PROGRAM_line,
        PROGRAM_v3,
        NUM_PROGRAM_KINDS,
};

enum {
        SHADER_arc_frag,
        SHADER_arc_vert,
        SHADER_circle_frag,
        SHADER_circle_vert,
        SHADER_line_frag,
        SHADER_line_vert,
        SHADER_v3_frag,
        SHADER_v3_vert,
        NUM_SHADER_KINDS,
};

enum {
        UNIFORM_arc_screenTransform,
        UNIFORM_circle_screenTransform,
        UNIFORM_line_screenTransform,
        UNIFORM_v3_screenTransform,
        UNIFORM_v3_test,
        NUM_UNIFORM_KINDS,
};

enum {
        ATTRIBUTE_arc_centerPoint,
        ATTRIBUTE_arc_color,
        ATTRIBUTE_arc_diffAngle,
        ATTRIBUTE_arc_orientation,
        ATTRIBUTE_arc_position,
        ATTRIBUTE_arc_radius,
        ATTRIBUTE_arc_startPoint,
        ATTRIBUTE_circle_centerPoint,
        ATTRIBUTE_circle_color,
        ATTRIBUTE_circle_diff,
        ATTRIBUTE_circle_radius,
        ATTRIBUTE_line_color,
        ATTRIBUTE_line_normal,
        ATTRIBUTE_line_position,
        ATTRIBUTE_v3_color,
        ATTRIBUTE_v3_normal,
        ATTRIBUTE_v3_position,
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

static inline void arcShader_set_screenTransform(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_arc], gfxUniformLocation[UNIFORM_arc_screenTransform], mat); }
static inline void circleShader_set_screenTransform(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_circle], gfxUniformLocation[UNIFORM_circle_screenTransform], mat); }
static inline void lineShader_set_screenTransform(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_line], gfxUniformLocation[UNIFORM_line_screenTransform], mat); }
static inline void v3Shader_set_screenTransform(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_v3], gfxUniformLocation[UNIFORM_v3_screenTransform], mat); }
static inline void v3Shader_set_test(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_v3], gfxUniformLocation[UNIFORM_v3_test], mat); }


#ifdef __cplusplus

static struct {
        static inline void set_screenTransform(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_arc], gfxUniformLocation[UNIFORM_arc_screenTransform], &mat->mat[0][0]); }
} arcShader;

static struct {
        static inline void set_screenTransform(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_circle], gfxUniformLocation[UNIFORM_circle_screenTransform], &mat->mat[0][0]); }
} circleShader;

static struct {
        static inline void set_screenTransform(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_line], gfxUniformLocation[UNIFORM_line_screenTransform], &mat->mat[0][0]); }
} lineShader;

static struct {
        static inline void set_screenTransform(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_v3], gfxUniformLocation[UNIFORM_v3_screenTransform], &mat->mat[0][0]); }
        static inline void set_test(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_v3], gfxUniformLocation[UNIFORM_v3_test], &mat->mat[0][0]); }
} v3Shader;

#endif // #ifdef __cplusplus

#ifdef __cplusplus
} // extern "C" {
#endif
#endif
