#include <shaders.h>

const struct SM_ProgramInfo smProgramInfo[NUM_PROGRAM_KINDS] = {
        [PROGRAM_line] = { "line" },
        [PROGRAM_circle] = { "circle" },
};

const struct SM_ShaderInfo smShaderInfo[NUM_SHADER_KINDS] = {
        [SHADER_line_vert] = { SHADERTYPE_VERTEX, "line_vert", "glsl/line.vert" },
        [SHADER_line_frag] = { SHADERTYPE_FRAGMENT, "line_frag", "glsl/line.frag" },
        [SHADER_circle_vert] = { SHADERTYPE_VERTEX, "circle_vert", "glsl/circle.vert" },
        [SHADER_circle_frag] = { SHADERTYPE_FRAGMENT, "circle_frag", "glsl/circle.frag" },
};

const struct SM_LinkInfo smLinkInfo[] = {
        { PROGRAM_line, SHADER_line_vert },
        { PROGRAM_line, SHADER_line_frag },
        { PROGRAM_circle, SHADER_circle_vert },
        { PROGRAM_circle, SHADER_circle_frag },
};

const int numLinkInfos = sizeof smLinkInfo / sizeof smLinkInfo[0];

const struct SM_UniformInfo smUniformInfo[NUM_UNIFORM_KINDS] = {
};

const struct SM_AttributeInfo smAttributeInfo[NUM_ATTRIBUTE_KINDS] = {
        [ATTRIBUTE_line_color] = { PROGRAM_line, GRAFIKATTRTYPE_VEC3, "color" },
        [ATTRIBUTE_line_deflection] = { PROGRAM_line, GRAFIKATTRTYPE_FLOAT, "deflection" },
        [ATTRIBUTE_line_normal] = { PROGRAM_line, GRAFIKATTRTYPE_VEC2, "normal" },
        [ATTRIBUTE_line_position] = { PROGRAM_line, GRAFIKATTRTYPE_VEC2, "position" },
        [ATTRIBUTE_circle_centerPoint] = { PROGRAM_circle, GRAFIKATTRTYPE_VEC2, "centerPoint" },
        [ATTRIBUTE_circle_color] = { PROGRAM_circle, GRAFIKATTRTYPE_VEC3, "color" },
        [ATTRIBUTE_circle_diff] = { PROGRAM_circle, GRAFIKATTRTYPE_VEC2, "diff" },
        [ATTRIBUTE_circle_radius] = { PROGRAM_circle, GRAFIKATTRTYPE_FLOAT, "radius" },
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
