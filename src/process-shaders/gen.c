#include <glsl-processor/defs.h>
#include <glsl-processor/logging.h>
#include <glsl-processor/memory.h>
#include <glsl-processor/parse.h>

#ifdef _MSC_VER
#include <Windows.h>
static void make_directory_if_not_exists(const char *dirpath)
{
        BOOL ret = CreateDirectory(dirpath, NULL);
        if (!ret && GetLastError() != ERROR_ALREADY_EXISTS)
                gp_fatal_f("Failed to create directory %s", dirpath);
}
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
static void make_directory_if_not_exists(const char *dirpath)
{
        int r = mkdir(dirpath, 0770);
        if (r == -1 && errno != EEXIST)
                gp_fatal_f("Failed to create directory %s: %s",
                           dirpath, strerror(errno));
}
#endif

#define INDENT "        "

struct MemoryBuffer {
        char *data;
        size_t length;
};

struct WriteCtx {
        struct GP_Ctx *ctx;
        struct MemoryBuffer hFilepath;
        struct MemoryBuffer cFilepath;
        struct MemoryBuffer hFile;
        struct MemoryBuffer cFile;
};

static void commit_to_file(struct MemoryBuffer *mb, const char *filepath)
{
        // writing only if file is different. This is to avoid the IDE unnecessary noticing File changes.
        int different = 0;
        {
                FILE *f = fopen(filepath, "rb");
                if (f == NULL)
                        different = 1;
                else {
                        fseek(f, 0, SEEK_END);
                        long contentsLength = ftell(f);
                        if (contentsLength == -1)
                                gp_fatal_f("Failed to ftell() file '%s': %s", filepath, strerror(errno));
                        if (contentsLength != mb->length)
                                different = 1;
                        else {
                                fseek(f, 0, SEEK_SET);
                                char *contents;
                                ALLOC_MEMORY(&contents, contentsLength + 1);
                                size_t readBytes = fread(contents, 1, contentsLength + 1, f);
                                if (readBytes != contentsLength)
                                        gp_fatal_f("File '%s' seems to have changed during the commit");
                                if (memcmp(contents, mb->data, mb->length) != 0)
                                        different = 1;
                                FREE_MEMORY(&contents);
                        }
                        fclose(f);
                }
        }
        if (different) {
                FILE *f = fopen(filepath, "wb");
                if (f == NULL)
                        gp_fatal_f("Failed to open '%s' for writing", filepath);
                fwrite(mb->data, 1, mb->length, f);
                fflush(f);
                if (ferror(f))
                        gp_fatal_f("I/O error while writing '%s'", filepath);
                fclose(f);
        }
}

static void append_to_buffer(struct MemoryBuffer *mb, const char *data, size_t size)
{
        size_t oldLength = mb->length;
        mb->length += size;
        REALLOC_MEMORY(&mb->data, mb->length + 1);
        memcpy(mb->data + oldLength, data, size);
        mb->data[mb->length] = '\0';
}

static void append_to_buffer_fv(struct MemoryBuffer *mb, const char *fmt, va_list ap)
{
        /* We're not allowed to use "ap" twice, and on gcc using it twice
        actually resulted in a segfault. So we make a backup copy before giving
        it to vsnprintf(). */
        va_list ap2;
        va_copy(ap2, ap);
        size_t need = vsnprintf(NULL, 0, fmt, ap);
        REALLOC_MEMORY(&mb->data, mb->length + need + 1);
        vsnprintf(mb->data + mb->length, need + 1, fmt, ap2);
        mb->data[mb->length + need] = '\0';
        mb->length += need;
}

static void append_to_buffer_f(struct MemoryBuffer *mb, const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        append_to_buffer_fv(mb, fmt, ap);
        va_end(ap);
}

static void append_filepath_component(struct MemoryBuffer *mb, const char *comp)
{
        // TODO: make code more correct, at least for Windows and Linux
        if (!(mb->length == 0 || (mb->data[mb->length - 1] == '/'
#ifdef _MSC_VER
            || mb->data[mb->length - 1] == '\\'
#endif
           ))) {
                append_to_buffer_f(mb, "/");
        }
        append_to_buffer_f(mb, "%s", comp);
}

static void text_to_cstring_literal(struct MemoryBuffer *mb, const char *buf, int size)
{
        int started = 0;
        for (int i = 0; i < size; i++) {
                if (!started) {
                        append_to_buffer_f(mb, "\"");
                        started = 1;
                }
                int c = buf[i];
                if (c == '\n') {
                        append_to_buffer_f(mb, "\\n\"\n");
                        started = 0;
                }
                else if (c == '\r') {
                        // ignore \r characters. Is this safe for the intended usage?
                }
                else if (c == '\t') {
                        // output tab characters literally. Will be much easier to read than a \t sequence.
                        append_to_buffer_f(mb, "\t");
                }
                else if (c == '"')
                        append_to_buffer_f(mb, "\\\"");
                else if (c == '\\')
                        append_to_buffer_f(mb, "\\'");
                else if (32 <= c && c <= 127 && c != '\\' && c != '"') {
                        char cc = (char) c;
                        append_to_buffer(mb, &cc, 1);
                }
                else
                        gp_fatal_f("Encoding of byte 0x%x in a C string literal is not yet implemented", c);
        }
        if (started)
                append_to_buffer_f(mb, "\"");
}

static void teardown_buffer(struct MemoryBuffer *mb)
{
        FREE_MEMORY(&mb->data);
        memset(mb, 0, sizeof *mb);
}

static void begin_enum(struct WriteCtx *wc)
{
        append_to_buffer_f(&wc->hFile, "enum {\n");
}

static void end_enum(struct WriteCtx *wc)
{
        append_to_buffer_f(&wc->hFile, "};\n\n");
}

static void add_enum_item(struct WriteCtx *wc, const char *name1)
{
        append_to_buffer_f(&wc->hFile, INDENT "%s,\n", name1);
}

static void add_enum_item_2(struct WriteCtx *wc, const char *name1, const char *name2)
{
        append_to_buffer_f(&wc->hFile, INDENT "%s_%s,\n", name1, name2);
}

static void add_enum_item_3(struct WriteCtx *wc, const char *name1, const char *name2, const char *name3)
{
        append_to_buffer_f(&wc->hFile, INDENT "%s_%s_%s,\n", name1, name2, name3);
}

static const char *const typeKind_to_real_type[GP_NUM_TYPE_KINDS] = {
        [GP_TYPE_BOOL] = "int",
        [GP_TYPE_INT] = "int",
        [GP_TYPE_UINT] = "unsigned int",
        [GP_TYPE_FLOAT] = "float",
        [GP_TYPE_DOUBLE] = "double",
        [GP_TYPE_VEC2] = "struct Vec2",
        [GP_TYPE_VEC3] = "struct Vec3",
        [GP_TYPE_VEC4] = "struct Vec4",
        [GP_TYPE_MAT2] = "struct Mat2",
        [GP_TYPE_MAT3] = "struct Mat3",
        [GP_TYPE_MAT4] = "struct Mat4",
};

static const char *const typeKind_to_GRAFIKATTRTYPE[GP_NUM_TYPE_KINDS] = {
        [GP_TYPE_BOOL] = "GRAFIKATTRTYPE_BOOL",
        [GP_TYPE_INT] = "GRAFIKATTRTYPE_INT",
        [GP_TYPE_UINT] = "GRAFIKATTRTYPE_UINT",
        [GP_TYPE_FLOAT] = "GRAFIKATTRTYPE_FLOAT",
        [GP_TYPE_DOUBLE] = "GRAFIKUNIFORMTYPE_DOUBLE",
        [GP_TYPE_VEC2] = "GRAFIKATTRTYPE_VEC2",
        [GP_TYPE_VEC3] = "GRAFIKATTRTYPE_VEC3",
        [GP_TYPE_VEC4] = "GRAFIKATTRTYPE_VEC4",
};

static const char *const typeKind_to_GRAFIKUNIFORMTYPE[GP_NUM_TYPE_KINDS] = {
        [GP_TYPE_BOOL] = "GRAFIKUNIFORMTYPE_BOOL",
        [GP_TYPE_INT] = "GRAFIKUNIFORMTYPE_INT",
        [GP_TYPE_UINT] = "GRAFIKUNIFORMTYPE_UINT",
        [GP_TYPE_FLOAT] = "GRAFIKUNIFORMTYPE_FLOAT",
        [GP_TYPE_DOUBLE] = "GRAFIKUNIFORMTYPE_DOUBLE",
        [GP_TYPE_VEC2] = "GRAFIKUNIFORMTYPE_VEC2",
        [GP_TYPE_VEC3] = "GRAFIKUNIFORMTYPE_VEC3",
        [GP_TYPE_VEC4] = "GRAFIKUNIFORMTYPE_VEC4",
        [GP_TYPE_MAT2] = "GRAFIKUNIFORMTYPE_MAT2",
        [GP_TYPE_MAT3] = "GRAFIKUNIFORMTYPE_MAT3",
        [GP_TYPE_MAT4] = "GRAFIKUNIFORMTYPE_MAT4",
};

void write_c_interface(struct GP_Ctx *ctx, const char *autogenDirpath)
{
        struct WriteCtx mtsCtx = { 0 };
        struct WriteCtx *wc = &mtsCtx;

        append_filepath_component(&wc->hFilepath, autogenDirpath);
        append_filepath_component(&wc->cFilepath, autogenDirpath);
        append_filepath_component(&wc->hFilepath, "shaders.h");
        append_filepath_component(&wc->cFilepath, "shaders.c");

        append_to_buffer_f(&wc->hFile,
                "#ifndef AUTOGENERATED_SHADERS_H_INCLUDED\n"
                "#define AUTOGENERATED_SHADERS_H_INCLUDED\n"
                "\n"
                "#include <segments/gfx.h>\n"
                "\n"
                "#ifdef __cplusplus\n"
                "extern \"C\" {\n"
                "#endif\n"
                "\n");

        begin_enum(wc);
        for (int i = 0; i < ctx->desc.numPrograms; i++)
                add_enum_item_2(wc, "PROGRAM", ctx->desc.programInfo[i].programName);
        add_enum_item(wc, "NUM_PROGRAM_KINDS");
        end_enum(wc);

        begin_enum(wc);
        for (int i = 0; i < ctx->desc.numShaders; i++)
                add_enum_item_2(wc, "SHADER", ctx->desc.shaderInfo[i].shaderName);
        add_enum_item(wc, "NUM_SHADER_KINDS");
        end_enum(wc);

        begin_enum(wc);
        for (int i = 0; i < ctx->numProgramUniforms; i++) {
                int programIndex = ctx->programUniforms[i].programIndex;
                const char *programName = ctx->desc.programInfo[programIndex].programName;
                const char *uniformName = ctx->programUniforms[i].uniformName;
                add_enum_item_3(wc, "UNIFORM", programName, uniformName);
        }
        add_enum_item(wc, "NUM_UNIFORM_KINDS");
        end_enum(wc);

        begin_enum(wc);
        for (int i = 0; i < ctx->numProgramAttributes; i++) {
                int programIndex = ctx->programAttributes[i].programIndex;
                const char *programName = ctx->desc.programInfo[programIndex].programName;
                const char *attributeName = ctx->programAttributes[i].attributeName;
                add_enum_item_3(wc, "ATTRIBUTE", programName, attributeName);
        }
        add_enum_item(wc, "NUM_ATTRIBUTE_KINDS");
        end_enum(wc);

        append_to_buffer_f(&wc->hFile,
                "extern const struct SM_ShaderInfo smShaderInfo[NUM_SHADER_KINDS];\n"
                "extern const struct SM_ProgramInfo smProgramInfo[NUM_PROGRAM_KINDS];\n"
                "extern const struct SM_LinkInfo smLinkInfo[];\n"
                "extern const int numLinkInfos;\n"
                "extern const struct SM_UniformInfo smUniformInfo[NUM_UNIFORM_KINDS];\n"
                "extern const struct SM_AttributeInfo smAttributeInfo[NUM_ATTRIBUTE_KINDS];\n"
                "extern const struct SM_Description smDescription;\n"
                "\n"
        );

        append_to_buffer_f(&wc->cFile, "#include <shaders.h>\n\n");

        append_to_buffer_f(&wc->cFile, "const struct SM_ProgramInfo smProgramInfo[NUM_PROGRAM_KINDS] = {\n");
        for (int i = 0; i < ctx->desc.numPrograms; i++) {
                const char *programName = ctx->desc.programInfo[i].programName;
                append_to_buffer_f(&wc->cFile, INDENT "[PROGRAM_%s] = { \"%s\" },\n", programName, programName);
        }
        append_to_buffer_f(&wc->cFile, "};\n\n");

        append_to_buffer_f(&wc->cFile, "const struct SM_ShaderInfo smShaderInfo[NUM_SHADER_KINDS] = {\n");
        append_to_buffer_f(&wc->cFile, "#define SHADER_SOURCE(s) s, sizeof s - 1\n");
        for (int i = 0; i < ctx->desc.numShaders; i++) {
                struct GP_ShaderInfo *info = &ctx->desc.shaderInfo[i];
                struct GP_ShaderfileAst *sfa = &ctx->shaderfileAsts[i];

                append_to_buffer_f(&wc->cFile, INDENT "[SHADER_%s] = { \"%s\",\n",
                        info->shaderName, info->shaderName);
                {
                        struct MemoryBuffer mb = {0};
                        text_to_cstring_literal(&mb, "#version 130\n", 13); //XXX
                        text_to_cstring_literal(&mb, sfa->output, sfa->outputSize);
                                append_to_buffer_f(&wc->cFile, "SHADER_SOURCE(\n");
                                append_to_buffer(&wc->cFile, mb.data, mb.length);
                                append_to_buffer_f(&wc->cFile, "), %s},\n", gp_shadertypeKindString[info->shaderType]);
                        teardown_buffer(&mb);
                }
        }
        append_to_buffer_f(&wc->cFile, "};\n\n");

        append_to_buffer_f(&wc->cFile, "const struct SM_LinkInfo smLinkInfo[] = {\n");
        for (int i = 0; i < ctx->desc.numLinks; i++) {
                int programIndex = ctx->desc.linkInfo[i].programIndex;
                int shaderIndex = ctx->desc.linkInfo[i].shaderIndex;
                const char *programName = ctx->desc.programInfo[programIndex].programName;
                const char *shaderName = ctx->desc.shaderInfo[shaderIndex].shaderName;
                append_to_buffer_f(&wc->cFile, INDENT "{ PROGRAM_%s, SHADER_%s },\n", programName, shaderName);
        }
        append_to_buffer_f(&wc->cFile, "};\n\n");
        append_to_buffer_f(&wc->cFile, "const int numLinkInfos = sizeof smLinkInfo / sizeof smLinkInfo[0];\n\n");

        append_to_buffer_f(&wc->cFile, "const struct SM_UniformInfo smUniformInfo[NUM_UNIFORM_KINDS] = {\n");
        for (int i = 0; i < ctx->numProgramUniforms; i++) {
                int programIndex = ctx->programUniforms[i].programIndex;
                int typeKind = ctx->programUniforms[i].typeKind;
                const char *programName = ctx->desc.programInfo[programIndex].programName;
                const char *uniformName = ctx->programUniforms[i].uniformName;
                const char *typeName = typeKind_to_GRAFIKUNIFORMTYPE[typeKind];
                GP_ENSURE(typeName != NULL);
                append_to_buffer_f(&wc->cFile, INDENT "[UNIFORM_%s_%s] = { PROGRAM_%s, %s, \"%s\" },\n",
                        programName, uniformName, programName, typeName, uniformName);
        }
        append_to_buffer_f(&wc->cFile, "};\n\n");

        append_to_buffer_f(&wc->cFile, "const struct SM_AttributeInfo smAttributeInfo[NUM_ATTRIBUTE_KINDS] = {\n");
        for (int i = 0; i < ctx->numProgramAttributes; i++) {
                int programIndex = ctx->programAttributes[i].programIndex;
                int typeKind = ctx->programAttributes[i].typeKind;
                const char *programName = ctx->desc.programInfo[programIndex].programName;
                const char *attributeName = ctx->programAttributes[i].attributeName;
                const char *typeName = typeKind_to_GRAFIKATTRTYPE[typeKind];
                GP_ENSURE(typeName != NULL);
                append_to_buffer_f(&wc->cFile, INDENT "[ATTRIBUTE_%s_%s] = { PROGRAM_%s, %s, \"%s\" },\n",
                        programName, attributeName, programName, typeName, attributeName);
        }
        append_to_buffer_f(&wc->cFile, "};\n\n");

        append_to_buffer_f(&wc->hFile,
                "extern GfxShader gfxShader[NUM_SHADER_KINDS];\n"
                "extern GfxProgram gfxProgram[NUM_PROGRAM_KINDS];\n"
                "extern GfxUniformLocation gfxUniformLocation[NUM_UNIFORM_KINDS];\n"
                "extern GfxAttributeLocation gfxAttributeLocation[NUM_ATTRIBUTE_KINDS];\n"
                "\n"
        );

        append_to_buffer_f(&wc->cFile,
                "GfxProgram gfxProgram[NUM_PROGRAM_KINDS];\n"
                "GfxShader gfxShader[NUM_SHADER_KINDS];\n"
                "GfxUniformLocation gfxUniformLocation[NUM_UNIFORM_KINDS];\n"
                "GfxAttributeLocation gfxAttributeLocation[NUM_ATTRIBUTE_KINDS];\n"
                "\n"
        );

        append_to_buffer_f(&wc->cFile,
                "const struct SM_Description smDescription = {\n"
                INDENT ".programInfo = smProgramInfo,\n"
                INDENT ".shaderInfo = smShaderInfo,\n"
                INDENT ".linkInfo = smLinkInfo,\n"
                INDENT ".uniformInfo = smUniformInfo,\n"
                INDENT ".attributeInfo = smAttributeInfo,\n"
                "\n"
                INDENT ".gfxProgram = gfxProgram,\n"
                INDENT ".gfxShader = gfxShader,\n"
                INDENT ".gfxUniformLocation = gfxUniformLocation,\n"
                INDENT ".gfxAttributeLocation = gfxAttributeLocation,\n"
                "\n"
                INDENT ".numPrograms = NUM_PROGRAM_KINDS,\n"
                INDENT ".numShaders = NUM_SHADER_KINDS,\n"
                INDENT ".numUniforms = NUM_UNIFORM_KINDS,\n"
                INDENT ".numAttributes = NUM_ATTRIBUTE_KINDS,\n"
                INDENT ".numLinks = sizeof smLinkInfo / sizeof smLinkInfo[0],\n"
                "};\n\n"
        );

        for (int i = 0; i < ctx->numProgramUniforms; i++) {
                struct GP_ProgramUniform *uniform = &ctx->programUniforms[i];
                const char *programName = ctx->desc.programInfo[uniform->programIndex].programName;
                const char *uniformName = uniform->uniformName;
                append_to_buffer_f(&wc->hFile, "static inline void %sShader_set_%s", programName, uniformName);
                const char *fmt;
                switch (uniform->typeKind) {
                case GP_TYPE_FLOAT: fmt = "(float x) { set_uniform_1f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], x); }\n"; break;
                case GP_TYPE_VEC2: fmt = "(float x, float y) { set_uniform_2f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], x, y); }\n"; break;
                case GP_TYPE_VEC3: fmt = "(float x, float y, float z) { set_uniform_3f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], x, y, z); }\n"; break;
                case GP_TYPE_VEC4: fmt = "(float x, float y, float z, float w) { set_uniform_4f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], x, y, z, w); }\n"; break;
                case GP_TYPE_MAT2: fmt = "(const struct Mat2 *mat) { set_uniform_mat2f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], mat); }\n"; break;
                case GP_TYPE_MAT3: fmt = "(const struct Mat3 *mat) { set_uniform_mat3f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], mat); }\n"; break;
                case GP_TYPE_MAT4: fmt = "(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], mat); }\n"; break;
                case GP_TYPE_SAMPLER2D: continue;  // cannot be set, can it?
                default: gp_fatal_f("Not implemented!");
                }
                append_to_buffer_f(&wc->hFile, fmt, programName, programName, uniformName);
        }
        append_to_buffer_f(&wc->hFile, "\n\n");

        append_to_buffer_f(&wc->hFile, "#define SET_TYPED_ATTRIBPOINTER(attribKind, vao, vbo, structType, memberName, type) do {\\\n"
                        INDENT "{ type *MUSTBECOMPATIBLE = &((structType *)NULL)->memberName; (void) MUSTBECOMPATIBLE; }\\\n"
                        INDENT "set_attribpointer((attribKind), (vao), (vbo), sizeof (structType), offsetof(structType, memberName));\\\n"
                        "} while (0)\n\n");
        for (int i = 0; i < ctx->numProgramAttributes; i++) {
                struct GP_ProgramAttribute *attr = &ctx->programAttributes[i];
                const char *programName = ctx->desc.programInfo[attr->programIndex].programName;
                const char *attributeName = attr->attributeName;
                append_to_buffer_f(&wc->hFile, "#define SET_ATTRIBPOINTER_%s_%s(vao, vbo, structType, memberName) SET_TYPED_ATTRIBPOINTER(ATTRIBUTE_%s_%s, (vao), (vbo), structType, memberName, %s)\n",
                                   programName, attributeName,
                                   programName, attributeName,
                                   typeKind_to_real_type[attr->typeKind]);
        }

        append_to_buffer_f(&wc->hFile,
                "\n"
                "\n"
                "#ifdef __cplusplus\n\n");
        for (int i = 0; i < ctx->numProgramUniforms; i++) {
                int programIndex = ctx->programUniforms[i].programIndex;
                int typeKind = ctx->programUniforms[i].typeKind;
                const char *uniformName = ctx->programUniforms[i].uniformName;
                const char *programName = ctx->desc.programInfo[programIndex].programName;
                if (i == 0 || programIndex != ctx->programUniforms[i - 1].programIndex) {
                        append_to_buffer_f(&wc->hFile, "static struct {\n");
                }
                append_to_buffer_f(&wc->hFile, INDENT "static inline void set_%s", uniformName);
                const char *fmt;
                switch (typeKind) {
                case GP_TYPE_FLOAT: fmt = "(float x) { set_uniform_1f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], x); }\n"; break;
                case GP_TYPE_VEC2: fmt = "(float x, float y) { set_uniform_2f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], x, y); }\n"; break;
                case GP_TYPE_VEC3: fmt = "(float x, float y, float z) { set_uniform_3f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], x, y, z); }\n"; break;
                case GP_TYPE_VEC4: fmt = "(float x, float y, float z, float w) { set_uniform_4f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], x, y, z, w); }\n"; break;
                case GP_TYPE_MAT2: fmt = "(const struct Mat2 *mat) { set_uniform_mat2f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], &mat->mat[0][0]); }\n"; break;
                case GP_TYPE_MAT3: fmt = "(const struct Mat3 *mat) { set_uniform_mat3f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], &mat->mat[0][0]); }\n"; break;
                case GP_TYPE_MAT4: fmt = "(const struct Mat4 *mat) { set_uniform_mat4f(gfxProgram[PROGRAM_%s], gfxUniformLocation[UNIFORM_%s_%s], &mat->mat[0][0]); }\n"; break;
                default: gp_fatal_f("Not implemented!");
                }
                append_to_buffer_f(&wc->hFile, fmt, programName, programName, uniformName);
                if (i + 1 == ctx->numProgramUniforms || programIndex != ctx->programUniforms[i + 1].programIndex)
                        append_to_buffer_f(&wc->hFile, "} %sShader;\n\n", programName);
        }
        append_to_buffer_f(&wc->hFile, "#endif // #ifdef __cplusplus\n\n");

        append_to_buffer_f(&wc->hFile,
                "#ifdef __cplusplus\n"
                "} // extern \"C\" {\n"
                "#endif\n"
                "#endif\n");

        make_directory_if_not_exists(autogenDirpath);
        commit_to_file(&wc->hFile, wc->hFilepath.data);
        commit_to_file(&wc->cFile, wc->cFilepath.data);

        teardown_buffer(&wc->hFilepath);
        teardown_buffer(&wc->cFilepath);
        teardown_buffer(&wc->hFile);
        teardown_buffer(&wc->cFile);
}
