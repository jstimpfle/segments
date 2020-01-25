#include <shaders.h>

const struct SM_ProgramInfo smProgramInfo[NUM_PROGRAM_KINDS] = {
        [PROGRAM_arc] = { "arc" },
        [PROGRAM_circle] = { "circle" },
        [PROGRAM_line] = { "line" },
        [PROGRAM_v3] = { "v3" },
};

const struct SM_ShaderInfo smShaderInfo[NUM_SHADER_KINDS] = {
        [SHADER_arc_frag] = { SHADERTYPE_FRAGMENT, "arc_frag", "glsl/arc.frag" },
        [SHADER_arc_vert] = { SHADERTYPE_VERTEX, "arc_vert", "glsl/arc.vert" },
        [SHADER_circle_frag] = { SHADERTYPE_FRAGMENT, "circle_frag", "glsl/circle.frag" },
        [SHADER_circle_vert] = { SHADERTYPE_VERTEX, "circle_vert", "glsl/circle.vert" },
        [SHADER_line_frag] = { SHADERTYPE_FRAGMENT, "line_frag", "glsl/line.frag" },
        [SHADER_line_vert] = { SHADERTYPE_VERTEX, "line_vert", "glsl/line.vert" },
        [SHADER_v3_frag] = { SHADERTYPE_FRAGMENT, "v3_frag", "glsl/v3.frag" },
        [SHADER_v3_vert] = { SHADERTYPE_VERTEX, "v3_vert", "glsl/v3.vert" },
};

const struct SM_LinkInfo smLinkInfo[] = {
        { PROGRAM_arc, SHADER_arc_vert },
        { PROGRAM_arc, SHADER_arc_frag },
        { PROGRAM_circle, SHADER_circle_vert },
        { PROGRAM_circle, SHADER_circle_frag },
        { PROGRAM_line, SHADER_line_vert },
        { PROGRAM_line, SHADER_line_frag },
        { PROGRAM_v3, SHADER_v3_vert },
        { PROGRAM_v3, SHADER_v3_frag },
};

const int numLinkInfos = sizeof smLinkInfo / sizeof smLinkInfo[0];

const struct SM_UniformInfo smUniformInfo[NUM_UNIFORM_KINDS] = {
        [UNIFORM_arc_screenTransform] = { PROGRAM_arc, GRAFIKUNIFORMTYPE_MAT4, "screenTransform" },
        [UNIFORM_circle_screenTransform] = { PROGRAM_circle, GRAFIKUNIFORMTYPE_MAT4, "screenTransform" },
        [UNIFORM_line_screenTransform] = { PROGRAM_line, GRAFIKUNIFORMTYPE_MAT4, "screenTransform" },
        [UNIFORM_v3_screenTransform] = { PROGRAM_v3, GRAFIKUNIFORMTYPE_MAT4, "screenTransform" },
};

const struct SM_AttributeInfo smAttributeInfo[NUM_ATTRIBUTE_KINDS] = {
        [ATTRIBUTE_arc_centerPoint] = { PROGRAM_arc, GRAFIKATTRTYPE_VEC2, "centerPoint" },
        [ATTRIBUTE_arc_color] = { PROGRAM_arc, GRAFIKATTRTYPE_VEC3, "color" },
        [ATTRIBUTE_arc_diffAngle] = { PROGRAM_arc, GRAFIKATTRTYPE_FLOAT, "diffAngle" },
        [ATTRIBUTE_arc_orientation] = { PROGRAM_arc, GRAFIKATTRTYPE_INT, "orientation" },
        [ATTRIBUTE_arc_position] = { PROGRAM_arc, GRAFIKATTRTYPE_VEC2, "position" },
        [ATTRIBUTE_arc_radius] = { PROGRAM_arc, GRAFIKATTRTYPE_FLOAT, "radius" },
        [ATTRIBUTE_arc_startPoint] = { PROGRAM_arc, GRAFIKATTRTYPE_VEC2, "startPoint" },
        [ATTRIBUTE_circle_centerPoint] = { PROGRAM_circle, GRAFIKATTRTYPE_VEC2, "centerPoint" },
        [ATTRIBUTE_circle_color] = { PROGRAM_circle, GRAFIKATTRTYPE_VEC3, "color" },
        [ATTRIBUTE_circle_diff] = { PROGRAM_circle, GRAFIKATTRTYPE_VEC2, "diff" },
        [ATTRIBUTE_circle_radius] = { PROGRAM_circle, GRAFIKATTRTYPE_FLOAT, "radius" },
        [ATTRIBUTE_line_color] = { PROGRAM_line, GRAFIKATTRTYPE_VEC3, "color" },
        [ATTRIBUTE_line_normal] = { PROGRAM_line, GRAFIKATTRTYPE_VEC2, "normal" },
        [ATTRIBUTE_line_position] = { PROGRAM_line, GRAFIKATTRTYPE_VEC2, "position" },
        [ATTRIBUTE_v3_color] = { PROGRAM_v3, GRAFIKATTRTYPE_VEC3, "color" },
        [ATTRIBUTE_v3_normal] = { PROGRAM_v3, GRAFIKATTRTYPE_VEC3, "normal" },
        [ATTRIBUTE_v3_position] = { PROGRAM_v3, GRAFIKATTRTYPE_VEC3, "position" },
};

GfxProgram gfxProgram[NUM_PROGRAM_KINDS];
GfxShader gfxShader[NUM_SHADER_KINDS];
GfxUniformLocation gfxUniformLocation[NUM_UNIFORM_KINDS];
GfxAttributeLocation gfxAttributeLocation[NUM_ATTRIBUTE_KINDS];

const struct SM_Description smDescription = {
        .programInfo = smProgramInfo,
        .shaderInfo = smShaderInfo,
        .linkInfo = smLinkInfo,
        .uniformInfo = smUniformInfo,
        .attributeInfo = smAttributeInfo,

        .gfxProgram = gfxProgram,
        .gfxShader = gfxShader,
        .gfxUniformLocation = gfxUniformLocation,
        .gfxAttributeLocation = gfxAttributeLocation,

        .numPrograms = NUM_PROGRAM_KINDS,
        .numShaders = NUM_SHADER_KINDS,
        .numUniforms = NUM_UNIFORM_KINDS,
        .numAttributes = NUM_ATTRIBUTE_KINDS,
        .numLinks = sizeof smLinkInfo / sizeof smLinkInfo[0],
};

