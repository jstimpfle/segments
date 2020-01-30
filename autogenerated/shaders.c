#include <shaders.h>

const struct SM_ProgramInfo smProgramInfo[NUM_PROGRAM_KINDS] = {
        [PROGRAM_arc] = { "arc" },
        [PROGRAM_circle] = { "circle" },
        [PROGRAM_line] = { "line" },
        [PROGRAM_v3] = { "v3" },
};

const struct SM_ShaderInfo smShaderInfo[NUM_SHADER_KINDS] = {
        [SHADER_arc_frag] = { "arc_frag",
"#version 130\n"
"\n"
"in vec2 startPointF;\n"
"in vec2 centerPointF;\n"
"in vec2 positionF;\n"
"in vec3 colorF;\n"
"in float diffAngleF;\n"
"in float radiusF;\n"
"flat in int orientationF;\n"
"\n"
"int compute_winding_order(vec2 p, vec2 q, vec2 r)\n"
"{\n"
"        float area = 0.0;\n"
"        area += (q.x - p.x) * (q.y + p.y);\n"
"        area += (r.x - q.x) * (r.y + q.y);\n"
"        area += (p.x - r.x) * (p.y + r.y);\n"
"        return int(area > 0) - int(area < 0);\n"
"}\n"
"\n"
"float compute_angle(vec2 p, vec2 q)\n"
"{\n"
"        return acos(dot(p, q) / (length(p) * length(q)));\n"
"}\n"
"\n"
"float compute_signed_angle(vec2 p, vec2 q)\n"
"{\n"
"        float diffAngle = compute_angle(p, q);\n"
"        if (compute_winding_order(p, vec2(0,0), q) < 0)\n"
"                diffAngle = -diffAngle;\n"
"        return diffAngle;\n"
"}\n"
"\n"
"float compute_full_angle(vec2 p, vec2 q)\n"
"{\n"
"	float PI = 3.14159265359;\n"
"	float angle = compute_angle(p, q);\n"
"	if (compute_winding_order(p, vec2(0, 0), q) == -1)\n"
"		angle = 2 * PI - angle;\n"
"	return angle;\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"	float PI = 3.14159265359;\n"
"	if (distance(positionF, centerPointF) > radiusF)\n"
"		discard;\n"
"	if (diffAngleF < 0.0) {\n"
"		float angle = -compute_full_angle(positionF - centerPointF, startPointF - centerPointF);\n"
"		if (angle < diffAngleF || angle > 0)\n"
"			discard;\n"
"	}\n"
"	else {\n"
"		float angle = compute_full_angle(startPointF - centerPointF, positionF - centerPointF);\n"
"		if (angle < 0 || angle > diffAngleF)\n"
"			discard;\n"
"	}\n"
"	gl_FragColor = vec4(colorF, 0.2);\n"
"}", 1622, SHADERTYPE_FRAGMENT},
        [SHADER_arc_vert] = { "arc_vert",
"#version 130\n"
"\n"
"uniform mat4 screenTransform;\n"
"\n"
"in vec2 startPoint;\n"
"in vec2 centerPoint;\n"
"in vec2 position;\n"
"in vec3 color;\n"
"in float diffAngle;\n"
"in float radius;\n"
"in int orientation;\n"
"\n"
"out vec2 startPointF;\n"
"out vec2 centerPointF;\n"
"out vec2 positionF;\n"
"out vec3 colorF;\n"
"out float diffAngleF;\n"
"out float radiusF;\n"
"flat out int orientationF;\n"
"\n"
"void main()\n"
"{\n"
"	startPointF = startPoint;\n"
"	centerPointF = centerPoint;\n"
"	positionF = position;\n"
"	colorF = color;\n"
"	diffAngleF = diffAngle;\n"
"	radiusF = radius;\n"
"	orientationF = orientation;\n"
"	gl_Position = screenTransform * vec4(position, 0, 1);\n"
"}", 689, SHADERTYPE_VERTEX},
        [SHADER_circle_frag] = { "circle_frag",
"#version 130\n"
"\n"
"in vec2 diffF;\n"
"in vec3 colorF;\n"
"in float radiusF;\n"
"\n"
"void main()\n"
"{\n"
"	float d = length(diffF);\n"
"	if (d <= radiusF)\n"
"		gl_FragColor = vec4(colorF, 1);\n"
"	else\n"
"		discard;\n"
"}", 229, SHADERTYPE_FRAGMENT},
        [SHADER_circle_vert] = { "circle_vert",
"#version 130\n"
"\n"
"uniform mat4 screenTransform;\n"
"\n"
"in vec2 centerPoint;\n"
"in vec2 diff;\n"
"in vec3 color;\n"
"in float radius;\n"
"\n"
"out vec2 diffF;\n"
"out vec3 colorF;\n"
"out float radiusF;\n"
"\n"
"void main()\n"
"{\n"
"	diffF = radius * diff;\n"
"	colorF = color;\n"
"	radiusF = radius;\n"
"	vec2 position = centerPoint + radius * diff;\n"
"	gl_Position = screenTransform * vec4(position, 0, 1);\n"
"}", 424, SHADERTYPE_VERTEX},
        [SHADER_line_frag] = { "line_frag",
"#version 130\n"
"\n"
"in vec2 positionF;\n"
"in vec2 normalF;\n"
"in vec3 colorF;\n"
"\n"
"void main()\n"
"{\n"
"	float w = length(fwidth(positionF));\n"
"	gl_FragColor = vec4(colorF, 1);\n"
"}", 195, SHADERTYPE_FRAGMENT},
        [SHADER_line_vert] = { "line_vert",
"#version 130\n"
"\n"
"uniform mat4 screenTransform;\n"
"\n"
"in vec2 position;\n"
"in vec2 normal;\n"
"in vec3 color;\n"
"\n"
"out vec2 positionF;\n"
"out vec2 normalF;\n"
"out vec3 colorF;\n"
"\n"
"void main()\n"
"{\n"
"	positionF = position;\n"
"	normalF = normal;\n"
"	colorF = color;\n"
"	gl_Position = screenTransform * vec4(position + normal, 0, 1);\n"
"}", 363, SHADERTYPE_VERTEX},
        [SHADER_v3_frag] = { "v3_frag",
"#version 130\n"
"/*HELLO*/\n"
"uniform mat4 test;\n"
"\n"
"uniform mat4 screenTransform;\n"
"\n"
"in vec3 positionF;\n"
"in vec3 normalF;\n"
"in vec3 colorF;\n"
"\n"
"float compute_specular_strength(vec3 lightPos, vec3 surfacePoint, vec3 normalizedSurfaceNormal, vec3 spectatorPosition) {\n"
"	vec3 lightToSurface = surfacePoint - lightPos;\n"
"	vec3 reflectVector = normalize(lightToSurface - 2.0 * dot(lightToSurface, normalizedSurfaceNormal) * normalizedSurfaceNormal);\n"
"	vec3 spectateDirection = normalize(spectatorPosition - surfacePoint);\n"
"	float strength = pow(clamp(dot(reflectVector, spectateDirection), 0, 1), 2.0);\n"
"	return strength;\n"
"}\n"
"\n"
"float compute_diffuse_strength(vec3 lightPos, vec3 surfacePoint, vec3 normalizedSurfaceNormal)\n"
"{\n"
"	vec3 lightToSurface = normalize(surfacePoint - lightPos);\n"
"	return clamp(dot(lightToSurface, normalizedSurfaceNormal), 0, 1);\n"
"}\n"
"\n"
"\n"
"void main()\n"
"{\n"
"	vec3 spec = vec3(0,0,-3.5);\n"
"	vec3 light = (vec4(5,5,-3.5,1) * screenTransform).xyz;\n"
"	vec3 normal = normalize(normalF);\n"
"	\n"
"	float diffuseStrength = compute_diffuse_strength(light, positionF, normal);  //compute_specular_strength(light, positionF, normalF, spec);\n"
"	diffuseStrength = clamp(diffuseStrength, 0, 1);\n"
"	float lightIntensity = 0.5 + diffuseStrength;\n"
"	if (lightIntensity < 0) lightIntensity = 0;\n"
"	if (lightIntensity > 1) lightIntensity = 1;\n"
"	gl_FragColor = vec4(lightIntensity * colorF, 1);\n"
"	//gl_FragColor = vec4(positionF, 1);\n"
"}", 1529, SHADERTYPE_FRAGMENT},
        [SHADER_v3_vert] = { "v3_vert",
"#version 130\n"
"\n"
"uniform mat4 screenTransform;\n"
"\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec3 color;\n"
"\n"
"out vec3 positionF;\n"
"out vec3 colorF;\n"
"out vec3 normalF;\n"
"\n"
"void main()\n"
"{\n"
"	positionF = position;\n"
"	colorF = color;\n"
"	normalF = normal;\n"
"	gl_Position = screenTransform * vec4(position, 1);\n"
"}", 351, SHADERTYPE_VERTEX},
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
        [UNIFORM_v3_test] = { PROGRAM_v3, GRAFIKUNIFORMTYPE_MAT4, "test" },
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

