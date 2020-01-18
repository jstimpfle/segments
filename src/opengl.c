#include <segments/defs.h>
#include <segments/memoryalloc.h>
#include <segments/logging.h>
#include <segments/window.h>
#include <segments/opengl.h>
#include <segments/shaders.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glu.h>

#define MAKE(type, name) type const name;
#include <segments/openglprocs.h>
#undef MAKE

static const struct {
        void **funcptr;
        const char *name;
} procsToLoad[] = {
#define MAKE(type, name) { (void**)&name, #name },
#include <segments/openglprocs.h>
#undef MAKE
};

static const int shadertypeMap[NUM_SHADERTYPE_KINDS] = {
        [SHADERTYPE_VERTEX] = GL_VERTEX_SHADER,
        [SHADERTYPE_FRAGMENT] = GL_FRAGMENT_SHADER,
};

static void NORETURN fatal_gl_error(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        message_fv(fmt, ap);
        va_end(ap);
        abort();
}

static void check_gl_errors(const char *filename, int line)
{
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
                fatal_f("In %s line %d: GL error %s\n", filename, line,
                        gluErrorString(err));
}

#define CHECK_GL_ERRORS() check_gl_errors(__FILE__, __LINE__)

static int get_compile_status(int shaderIndex)
{
	GLint shader = gfxShader[shaderIndex];
	const char *name = smShaderInfo[shaderIndex].name;
        GLint compileStatus;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
        if (compileStatus != GL_TRUE) {
                GLchar errorBuf[1024];
                GLsizei length;
                glGetShaderInfoLog(shader, sizeof errorBuf, &length, errorBuf);
                message_f("Warning: shader '%s' failed to compile: %s\n",
			  name, errorBuf);
        }
        return compileStatus == GL_TRUE;
}

static int get_link_status(int programIndex)
{
	GLint program = gfxProgram[programIndex];
	const char *name = smProgramInfo[programIndex].name;
        GLint linkStatus;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
                GLsizei length;
                GLchar errorBuf[128];
                glGetProgramInfoLog(program, sizeof errorBuf,
                        &length, errorBuf);
                message_f("Warning: Failed to link shader program '%s': %s\n",
			  name, errorBuf);
        }
        return linkStatus == GL_TRUE;
}


void setup_opengl(void)
{
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glDisable(GL_MULTISAMPLE);

        for (int i = 0; i < sizeof procsToLoad / sizeof procsToLoad[0]; i++) {
                const char *name = procsToLoad[i].name;
                *procsToLoad[i].funcptr = load_opengl_pointer(name);
        }
                CHECK_GL_ERRORS();
        for (int i = 0; i < NUM_SHADER_KINDS; i++) {
                int kind = shadertypeMap[smShaderInfo[i].shadertypeKind];
                gfxShader[i] = glCreateShader(kind);
        }
        for (int i = 0; i < NUM_SHADER_KINDS; i++) {
		const char *filepath = smShaderInfo[i].filepath;
		static char source[1024*1024];
		FILE *f = fopen(filepath, "rb");
		if (f == NULL)
			fatal_f("Failed to open '%s' for reading", filepath);
		fseek(f, 0, SEEK_END);
		long fileSize = ftell(f);
		if (fileSize == -1)
			fatal_f("failed to ftell(): %s", strerror(errno));
		fseek(f, 0, SEEK_SET);
		if (fileSize + 1 > sizeof source)
			fatal_f("File '%s' is too large", filepath);
		size_t nread = fread(source, 1, fileSize + 1, f);
		if (nread != fileSize)
			fatal_f("Did not read the expected amount of bytes from '%s'", filepath);
		source[fileSize] = '\0';
		if (ferror(f))
			fatal_f("I/O errors reading from %s", filepath);
		fclose(f);
		const char *ptr = source;
		glShaderSource(gfxShader[i], 1, &ptr, NULL);
        }
        for (int i = 0; i < NUM_SHADER_KINDS; i++) {
                glCompileShader(gfxShader[i]);
        }
        for (int i = 0; i < NUM_SHADER_KINDS; i++) {
		get_compile_status(i);
        }
                CHECK_GL_ERRORS();
        for (int i = 0; i < NUM_PROGRAM_KINDS; i++) {
                gfxProgram[i] = glCreateProgram();
                if (gfxProgram[i] == 0) {
                        fatal_gl_error("glCreateProgram() failed");
                }
        }
                CHECK_GL_ERRORS();
        for (int i = 0; i < numLinkInfos; i++) {
                int pi = smLinkInfo[i].programIndex;
                int si = smLinkInfo[i].shaderIndex;
                glAttachShader(gfxProgram[pi], gfxShader[si]);
        }
                CHECK_GL_ERRORS();
        for (int i = 0; i < NUM_PROGRAM_KINDS; i++) {
                glLinkProgram(gfxProgram[i]);
        }
        for (int i = 0; i < NUM_PROGRAM_KINDS; i++) {
		get_link_status(i);
        }
                CHECK_GL_ERRORS();
        for (int i = 0; i < NUM_ATTRIBUTE_KINDS; i++) {
                int pi = smAttributeInfo[i].programIndex;
                const char *name = smAttributeInfo[i].name;
                gfxAttributeLocation[i] = glGetAttribLocation(gfxProgram[pi], name);
        }
                CHECK_GL_ERRORS();
        for (int i = 0; i < NUM_UNIFORM_KINDS; i++) {
                int pi = smUniformInfo[i].programIndex;
                const char *name = smAttributeInfo[i].name;
                gfxAttributeLocation[i] = glGetUniformLocation(gfxProgram[pi], name);
        }
                CHECK_GL_ERRORS();
}

struct Vec2 {
        float x;
        float y;
};

struct Vec3 {
        float x;
        float y;
        float z;
};

struct LineVertex {
        struct Vec2 position;
        struct Vec2 normal;
        struct Vec3 color;
};

static GLuint lineVBO;
static GLuint lineVAO;

struct CircleVertex {
        struct Vec2 centerPoint;
        struct Vec2 diff;
        struct Vec3 color;
        float radius;
};

static GLuint circleVBO;
static GLuint circleVAO;

#define SET_VERTEX_ATTRIB_POINTER(loc, numFloats, type, member) glVertexAttribPointer(loc, numFloats, GL_FLOAT, GL_FALSE, sizeof (type), (void *) offsetof(type, member))

struct Vec2 normalize(struct Vec2 v)
{
	float d = sqrtf(v.x * v.x + v.y * v.y);
	v.x /= d;
	v.y /= d;
	return v;
}



static struct LineVertex *lineVertices;
static struct CircleVertex *circleVertices;

static int numLineVertices;
static int numCircleVertices;

static float currentX;
static float currentY;

static float mouseX;  // in OpenGL coordinates
static float mouseY;

static int windowWidth = 800;  // in screen pixels. TODO: read from events
static int windowHeight = 600;  // in screen pixels

static const struct Vec3 lineColor = { 0.3, 0.8, 0.5 };

void add_line(float x1, float y1, float x2, float y2)
{
	struct Vec2 n = { x2 - x1, y2 - y1 };
	n = normalize(n);
	float dx = n.x / 32.f;
	float dy = n.y / 32.f;

        struct LineVertex verts[6] = {
                {{ x1, y1}, { dy, -dx }, lineColor, },
                {{ x1, y1}, { -dy, dx }, lineColor, },
                {{ x2, y2}, { dy, -dx }, lineColor, },
                {{ x1, y1}, { -dy, dx }, lineColor, },
                {{ x2, y2}, { dy, -dx }, lineColor, },
                {{ x2, y2}, { -dy, dx }, lineColor, },
        };
        
        int idx = numLineVertices;
        numLineVertices += 6;
        REALLOC_MEMORY(&lineVertices, numLineVertices);
        COPY_MEMORY(lineVertices + idx, verts, LENGTH(verts));
}

void add_circle(float x, float y)
{
        float d = 1.0 / 32.f;
        struct CircleVertex verts[6] = {
                {{ x, y }, { -1.0, -1.0 }, lineColor, d },
                {{ x, y }, { -1.0, 1.0 }, lineColor, d },
                {{ x, y }, { 1.0, 1.0 }, lineColor, d },
                {{ x, y }, { 1.0, 1.0 }, lineColor, d },
                {{ x, y }, { 1.0, -1.0 }, lineColor, d },
                {{ x, y }, { -1.0, -1.0 }, lineColor, d },
        };

        int idx = numCircleVertices;
        numCircleVertices += 6;
        REALLOC_MEMORY(&circleVertices, numCircleVertices);
        COPY_MEMORY(circleVertices + idx, verts, LENGTH(verts));
}

void move_to(float x, float y)
{
        currentX = x;
        currentY = y;
}

void line_to(float x, float y)
{
        add_line(currentX, currentY, x, y);
        add_circle(x, y);
        currentX = x;
        currentY = y;
}

void upload_vertices(void)
{
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferData(GL_ARRAY_BUFFER, numLineVertices * sizeof *lineVertices, lineVertices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        CHECK_GL_ERRORS();

        glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
        glBufferData(GL_ARRAY_BUFFER, numCircleVertices * sizeof *circleVertices, circleVertices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        CHECK_GL_ERRORS();
}

void generate_vertices(void)
{
        numLineVertices = 0;
        numCircleVertices = 0;
        move_to(0.1, 0.1);
        line_to(0.5, 0.6);
        line_to(0.0, -0.5);
        move_to(-0.9, -0.9);
        line_to(0.5, -0.1);
        upload_vertices();

        /*
        for (int i = 0; i < numLineVertices; i++)
                message_f("Line vertex: { %f, %f }, { %f, %f }, %f ",
                          lineVertices[i].position.x,
                          lineVertices[i].position.y,
                          lineVertices[i].normal.y)
                          */
}

void make_draw_call(GLuint program, GLuint vao, int primitiveKind, int firstIndex, int count)
{
        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(primitiveKind, firstIndex, count);
        glBindVertexArray(0);
        glUseProgram(0);
}

void just_do_all_gl_stuff(void)
{
        GLuint multisampleTexture;
        GLuint multisampleFramebuffer;
        int width = 1024; //XXX
        int height = 768; //XXX

        glGenTextures(1, &multisampleTexture);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleTexture);
        static const int num_samples = 4;
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_RGBA8, width, height, GL_FALSE);

        glGenFramebuffers(1, &multisampleFramebuffer);

        GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );

        glGenBuffers(1, &lineVBO);
        glGenVertexArrays(1, &lineVAO);
        glBindVertexArray(lineVAO);
                CHECK_GL_ERRORS();
        glEnableVertexAttribArray(gfxAttributeLocation[ATTRIBUTE_line_position]);
                CHECK_GL_ERRORS();
        glEnableVertexAttribArray(gfxAttributeLocation[ATTRIBUTE_line_normal]);
                CHECK_GL_ERRORS();
        glEnableVertexAttribArray(gfxAttributeLocation[ATTRIBUTE_line_color]);
                CHECK_GL_ERRORS();
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
                CHECK_GL_ERRORS();
        SET_VERTEX_ATTRIB_POINTER(gfxAttributeLocation[ATTRIBUTE_line_position], 2, struct LineVertex, position);
                CHECK_GL_ERRORS();
        SET_VERTEX_ATTRIB_POINTER(gfxAttributeLocation[ATTRIBUTE_line_normal], 2, struct LineVertex, normal);
                CHECK_GL_ERRORS();
        SET_VERTEX_ATTRIB_POINTER(gfxAttributeLocation[ATTRIBUTE_line_color], 3, struct LineVertex, color);
                CHECK_GL_ERRORS();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
                CHECK_GL_ERRORS();

        glGenBuffers(1, &circleVBO);
        glGenVertexArrays(1, &circleVAO);
        glBindVertexArray(circleVAO);
        glEnableVertexAttribArray(gfxAttributeLocation[ATTRIBUTE_circle_centerPoint]);
        glEnableVertexAttribArray(gfxAttributeLocation[ATTRIBUTE_circle_diff]);
        glEnableVertexAttribArray(gfxAttributeLocation[ATTRIBUTE_circle_color]);
        glEnableVertexAttribArray(gfxAttributeLocation[ATTRIBUTE_circle_radius]);
        glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
        SET_VERTEX_ATTRIB_POINTER(gfxAttributeLocation[ATTRIBUTE_circle_centerPoint], 2, struct CircleVertex, centerPoint);
        SET_VERTEX_ATTRIB_POINTER(gfxAttributeLocation[ATTRIBUTE_circle_diff], 2, struct CircleVertex, diff);
        SET_VERTEX_ATTRIB_POINTER(gfxAttributeLocation[ATTRIBUTE_circle_color], 3, struct CircleVertex, color);
        SET_VERTEX_ATTRIB_POINTER(gfxAttributeLocation[ATTRIBUTE_circle_radius], 1, struct CircleVertex, radius);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        CHECK_GL_ERRORS();

        for (;;) {
                fetch_all_pending_events();

                while (have_events()) {
                        struct Event event;
                        dequeue_event(&event);
                        if (event.eventKind == EVENT_KEY) {
                                if (event.tKey.keyKind == KEY_ESCAPE) {
                                        close_window();
                                        exit(0);
                                }
                        }
                        if (event.eventKind == EVENT_MOUSEBUTTON) {
                                if (event.tMousebutton.mousebuttonKind == MOUSEBUTTON_1) {
                                        if (event.tMousebutton.mousebuttoneventKind == MOUSEBUTTONEVENT_PRESS) {
                                                line_to(mouseX, mouseY);
                                                upload_vertices(); // XXX: too early
                                        }
                                }
                        }
                        if (event.eventKind == EVENT_MOUSEMOVE) {
                                mouseX = (2.0f * event.tMousemove.x / windowWidth) - 1.0f;
                                mouseY = - ((2.0f * event.tMousemove.y / windowHeight) - 1.0f);
                        }
                }

                glBindFramebuffer(GL_FRAMEBUFFER, multisampleFramebuffer);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_2D_MULTISAMPLE, multisampleTexture, 0);

                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                make_draw_call(gfxProgram[PROGRAM_line], lineVAO, GL_TRIANGLES, 0, numLineVertices);
                make_draw_call(gfxProgram[PROGRAM_circle], circleVAO, GL_TRIANGLES, 0, numCircleVertices);

                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);   // Make sure no FBO is set as the draw framebuffer
                glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer); // Make sure your multisampled FBO is the read framebuffer
                glDrawBuffer(GL_BACK);                       // Set the back buffer as the draw buffer
                glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

                swap_buffers();
        }

        glDeleteBuffers(1, &lineVBO);
        glDeleteVertexArrays(1, &lineVAO);
}