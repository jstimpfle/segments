#include <segments/defs.h>
#include <segments/memory.h>
#include <segments/logging.h>
#include <segments/window.h>
#include <segments/opengl.h>
#include <segments/gfx.h>
#include <shaders.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define M_PI 3.14159265358979323846f
#include <GL/glu.h>

#define MAKE(type, name) type const name;
#include <segments/openglprocs.h>
#undef MAKE

struct LineVertex {
        struct Vec2 position;
        struct Vec2 normal;
        struct Vec3 color;
};

struct CircleVertex {
        struct Vec2 centerPoint;
        struct Vec2 diff;
        struct Vec3 color;
        float radius;
};

struct ArcVertex {
        struct Vec2 startPoint;
        struct Vec2 centerPoint;
        struct Vec2 position;
        struct Vec3 color;
        float diffAngle;
        float radius;
};

struct V3Vertex {
        struct Vec3 position;
        struct Vec3 normal;
        struct Vec3 color;
};


static struct LineVertex *lineVertices;
static struct CircleVertex *circleVertices;
static struct ArcVertex *arcVertices;
static struct V3Vertex *v3Vertices;

static int numLineVertices;
static int numCircleVertices;
static int numArcVertices;
static int numV3Vertices;

static int obtuseArcAngle;
static float currentX;
static float currentY;
static float arcX;
static float arcY;

static float viewingAngleX;
static float viewingAngleY;
static float zoomFactor = 1.f;
static struct Mat4 screenTransform;

static float mouseX;  // in OpenGL coordinates
static float mouseY;

static int windowWidth;
static int windowHeight;

static const struct Vec3 lineColor = { 0.4f, 0.8f, 0.8f };

static struct Vec2 sub(struct Vec2 p, struct Vec2 q)
{
        return (struct Vec2) { p.x - q.x, p.y - q.y };
}

static float dot(struct Vec2 v, struct Vec2 w)
{
        return v.x * w.x + v.y * w.y;
}

static float length(struct Vec2 v)
{
        return sqrtf(v.x * v.x + v.y * v.y);
}

static float vec3_length(struct Vec3 v)
{
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

struct Vec2 normalize(struct Vec2 v)
{
	float d = sqrtf(v.x * v.x + v.y * v.y);
	v.x /= d;
	v.y /= d;
	return v;
}

static int compute_winding_order(struct Vec2 p, struct Vec2 q, struct Vec2 r)
{
        float area = 0.0f;
        area += (q.x - p.x) * (q.y + p.y);
        area += (r.x - q.x) * (r.y + q.y);
        area += (p.x - r.x) * (p.y + r.y);
        return (area > 0) - (area < 0);
}

/* Returns value in the range [0, PI) */
static float compute_angle(struct Vec2 p, struct Vec2 q)
{
        return acosf(dot(p, q) / (length(p) * length(q)));
}

static struct Vec2 vec2_rotate_cw(struct Vec2 v, float angle)
{
        float ct = cosf(angle);
        float st = sinf(angle);
        return (struct Vec2) {
                v.x * ct - v.y * st,
                v.x * st + v.y * st,
        };
}

static struct Vec3 vec3_rotate_x(struct Vec3 v, float angle)
{
        float ct = cosf(angle);
        float st = sinf(angle);
        return (struct Vec3) {
                v.x,
                v.y * ct - v.z * st,
                v.y * st + v.z * ct,
        };
}

static struct Vec3 vec3_rotate_y(struct Vec3 v, float angle)
{
        float ct = cosf(angle);
        float st = sinf(angle);
        return (struct Vec3) {
                v.x * ct - v.z * st,
                v.y,
                v.x * st + v.z * ct,
        };
}

static struct Vec3 vec3_rotate_z(struct Vec3 v, float angle)
{
        float ct = cosf(angle);
        float st = sinf(angle);
        return (struct Vec3) {
                v.x * ct - v.y * st,
                v.x * st + v.y * ct,
                v.z,
        };
}

static struct Mat4 mat4_mul(struct Mat4 a, struct Mat4 b)
{
        struct Mat4 result = {0};
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        for (int k = 0; k < 4; k++) {
                                result.mat[i][j] += a.mat[i][k] * b.mat[k][j];
                        }
                }
        }
        return result;
}

#if 0
static void push_triangle(struct Vec2 p, struct Vec2 q, struct Vec2 r)
{
        struct LineVertex verts[3] = {
                { p, {0}, {1,0,0} },
                { q, {0}, {1,0,0} },
                { r, {0}, {1,0,0} },
        };
        int i = numLineVertices;
        numLineVertices += 3;
        REALLOC_MEMORY(&lineVertices, numLineVertices);
        lineVertices[i + 0] = verts[0];
        lineVertices[i + 1] = verts[1];
        lineVertices[i + 2] = verts[2];
}
#endif

static void push_triangle_v3(struct Vec3 p, struct Vec3 q, struct Vec3 r,
                          struct Vec3 pn, struct Vec3 qn, struct Vec3 rn, struct Vec3 color)
{
        struct V3Vertex verts[3] = {
                { p, pn, color },
                { q, qn, color },
                { r, rn, color },
        };
        int i = numV3Vertices;
        numV3Vertices += 3;
        REALLOC_MEMORY(&v3Vertices, numV3Vertices);
        v3Vertices[i + 0] = verts[0];
        v3Vertices[i + 1] = verts[1];
        v3Vertices[i + 2] = verts[2];
        /*
        message_f("Triangle %f,%f,%f  %f,%f,%f  %f,%f,%f",
                  verts[0].position.x, verts[0].position.y, verts[0].position.z,
                  verts[1].position.x, verts[1].position.y, verts[1].position.z,
                  verts[2].position.x, verts[2].position.y, verts[2].position.z);
                  */
}

void add_line(float x1, float y1, float x2, float y2)
{
	struct Vec2 n = { x2 - x1, y2 - y1 };
	n = normalize(n);
	float dx = n.x / 128.f;
	float dy = n.y / 128.f;

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
        float d = 1.0 / 32.0f;
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

void add_arc(struct Vec2 p, struct Vec2 q, struct Vec2 r)
{
        struct Vec2 qp = sub(p, q);
        struct Vec2 qr = sub(r, q);
        float diffAngle = compute_angle(qp, qr);
        int windingOrder = compute_winding_order(p, q, r);
        if (obtuseArcAngle)
                diffAngle = -(2 * M_PI - diffAngle);
        if (windingOrder == -1)
                diffAngle = -diffAngle;

        float radius = length(qp);
        struct ArcVertex verts[6] = {
                {p, q, { q.x - radius, q.y - radius }, lineColor, diffAngle, radius },
                {p, q, { q.x - radius, q.y + radius }, lineColor, diffAngle, radius },
                {p, q, { q.x + radius, q.y + radius }, lineColor, diffAngle, radius },
                {p, q, { q.x + radius, q.y + radius }, lineColor, diffAngle, radius },
                {p, q, { q.x + radius, q.y - radius }, lineColor, diffAngle, radius },
                {p, q, { q.x - radius, q.y - radius }, lineColor, diffAngle, radius },
        };

        int idx = numArcVertices;
        numArcVertices += 6;
        REALLOC_MEMORY(&arcVertices, numArcVertices);
        COPY_MEMORY(arcVertices + idx, verts, LENGTH(verts));
}

void move_to(float x, float y)
{
        arcX = currentX;
        arcY = currentY;
        currentX = x;
        currentY = y;
}

void line_to(float x, float y)
{
        add_line(currentX, currentY, x, y);
        add_circle(x, y);
        arcX = currentX;
        arcY = currentY;
        currentX = x;
        currentY = y;
}

static void make_sphere(void)
{
        float radius = 0.5f;
        struct Vec3 circle[60];
        for (int i = 0; i < LENGTH(circle); i++) {
                float angle = 2 * M_PI / LENGTH(circle) * i;
                circle[i] = (struct Vec3) { radius * cosf(angle), 0.f, radius * sinf(angle) };
                //message_f("circle: %f %f, length(%f)", circle[i].x, circle[i].z, vec3_length(circle[i]));
        }
#define CIRCLESTEPS 10
        for (int i = 0; i < CIRCLESTEPS; i++) {
                float angle1 = M_PI / 2 * i / CIRCLESTEPS;
                float sa1 = sinf(angle1);
                float ca1 = cosf(angle1);
                float angle2 = M_PI / 2 * (i+1) / CIRCLESTEPS;
                float sa2 = sinf(angle2);
                float ca2 = cosf(angle2);

                for (int j = 0; j < LENGTH(circle); j++) {
                        int k = j ? j - 1 : LENGTH(circle) - 1;
                        struct Vec3 p = circle[j];
                        struct Vec3 q = circle[k];
                        struct Vec3 p1 = { radius * ca1 * p.x, radius * sa1, radius * ca1 * p.z };
                        struct Vec3 q1 = { radius * ca1 * q.x, radius * sa1, radius * ca1 * q.z };
                        struct Vec3 p2 = { radius * ca2 * p.x, radius * sa2, radius * ca2 * p.z };
                        struct Vec3 q2 = { radius * ca2 * q.x, radius * sa2, radius * ca2 * q.z };
                        struct Vec3 color = { 0.f, 0.f, 1.f };
                        push_triangle_v3(p1, q1, p2,      p1, q1, p2,  color);
                        push_triangle_v3(q1, p2, q2,      q1, p2, q2,  color);
                        /*
                        message_f("%f %f %f %f",
                                  vec3_length(p1),
                                  vec3_length(p2),
                                  vec3_length(q1),
                                  vec3_length(q2));
                                  */
                }
        }
}

static void make_torus(void)
{
        float radius = 0.4f;
        float torusDiameter = 0.1f;
        struct Vec3 point = { torusDiameter, 0.0f, 0.0f };
        struct Vec3 normalVector = { 1.0f, 0.0f, 0.0f };
#define RINGPOINTS 30
        struct Vec3 ring[RINGPOINTS];
        struct Vec3 normal[RINGPOINTS];
        for (int i = 0; i < RINGPOINTS; i++) {
                float angle = 2 * M_PI / RINGPOINTS * i;
                ring[i] = vec3_rotate_z(point, angle);
                ring[i].x += radius;
                normal[i] = vec3_rotate_z(normalVector, angle);
        }
#define STEPS 60
        for (int i = STEPS/2; i <= STEPS; i++) {
                float angle[2] = {
                        2 * M_PI / STEPS * i,
                        2 * M_PI / STEPS * (i+1),
                };
                for (int j = 0; j < RINGPOINTS; j++) {
                        int k = j ? j - 1 : RINGPOINTS - 1;
                        int ri[2] = { j, k };
                        struct Vec3 p[2];
                        struct Vec3 q[2];
                        struct Vec3 pn[2];
                        struct Vec3 qn[2];
                        for (int pi = 0; pi < 2; pi++) {
                                p[pi] = vec3_rotate_y(ring[ri[pi]], angle[0]);
                                pn[pi] = vec3_rotate_y(normal[ri[pi]], angle[0]);
                        }
                        for (int qi = 0; qi < 2; qi++) {
                                q[qi] = vec3_rotate_y(ring[ri[qi]], angle[1]);
                                qn[qi] = vec3_rotate_y(normal[ri[qi]], angle[1]);
                        }
                        struct Vec3 color = { 0.f, 1.f, (float)i/STEPS };
                        //struct Vec3 color = { 0.f, 0.f, 1.f };
                        push_triangle_v3(p[0], q[0], p[1], pn[0], qn[0], pn[1], color);
                        push_triangle_v3(q[0], q[1], p[1], qn[0], qn[1], pn[1], color);
                }
        }

        // arrow tip
#define ARROWSTEPS 10
        float arrowStartDiameter = 1.4f * torusDiameter;
        float arrowAngularLength = 1.2f;
        for (int i = 0; i < ARROWSTEPS; i++) {
                float angle[2] = {
                        arrowAngularLength / ARROWSTEPS * i,
                        arrowAngularLength / ARROWSTEPS * (i + 1),
                };
                for (int j = 0; j < RINGPOINTS; j++) {
                        float a[2] = {
                                2 * M_PI / RINGPOINTS * j,
                                2 * M_PI / RINGPOINTS * (j + 1),
                        };
                        struct Vec3 p0[2];
                        for (int k = 0; k < 2; k++) {
                                p0[k] = (struct Vec3) { arrowStartDiameter, 0.f, 0.f };
                                p0[k] = vec3_rotate_z(p0[k], a[k]);
                        }
                        struct Vec3 p[2];
                        struct Vec3 q[2];
                        struct Vec3 pn[2] = {0};
                        struct Vec3 qn[2] = {0};
                        for (int pi = 0; pi < 2; pi++) {
                                struct Vec3 P = p0[pi];
                                float factor = (float) (ARROWSTEPS - i) / ARROWSTEPS;
                                P.x *= factor;
                                P.y *= factor;
                                P.x += radius;
                                P = vec3_rotate_y(P, angle[0]);
                                p[pi] = P;
                                //pn[pi] = vec3_rotate_y(normal[pi], angle[0]);
                        }
                        for (int qi = 0; qi < 2; qi++) {
                                struct Vec3 P = p0[qi];
                                float factor = (float) (ARROWSTEPS - i - 1) / ARROWSTEPS;
                                P.x *= factor;
                                P.y *= factor;
                                P.x += radius;
                                P = vec3_rotate_y(P, angle[1]);
                                q[qi] = P;
                                //qn[qi] = vec3_rotate_y(normal[ri[qi]], angle[1]);
                        }
                        struct Vec3 color = { 1.f, (float)i/STEPS, 0.f };
                        //struct Vec3 color = { 0.f, 0.f, 1.f };
                        push_triangle_v3(p[0], q[0], p[1], pn[0], qn[0], pn[1], color);
                        push_triangle_v3(q[0], q[1], p[1], qn[0], qn[1], pn[1], color);
                }
        }
}

static void compute_screen_transform(void)
{
        float cx = cosf(viewingAngleX);
        float sx = sinf(viewingAngleX);
        struct Mat4 tX = {{
                { 1.f, 0.f, 0.f, 0.f },
                { 0.f,  cx, -sx, 0.f },
                { 0.f,  sx,  cx, 0.f },
                { 0.f, 0.f, 0.f, 1.f },
        }};
        float cy = cosf(viewingAngleY);
        float sy = sinf(viewingAngleY);
        struct Mat4 tY = {{
                { cy,  0.f, -sy, 0.f },
                { 0.f, 1.f, 0.f, 0.f },
                { sy,  0.f, cy,  0.f },
                { 0.f, 0.f, 0.f, 1.f },
        }};
        struct Mat4 st = mat4_mul(tX, tY);
        for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                        st.mat[i][j] *= zoomFactor;
        memcpy(&screenTransform, &st, sizeof st);
}

static float add_modulo_2pi(float a, float b)
{
        ENSURE(0 <= a && a <= 2 * M_PI);
        ENSURE(0 <= b && b <= 2 * M_PI);
        a += b;
        if (a > 2 * M_PI)
                a -= 2 * M_PI;
        return a;
}

static float sub_modulo_2pi(float a, float b)
{
        ENSURE(0 <= a && a <= 2 * M_PI);
        ENSURE(0 <= b && b <= 2 * M_PI);
        a -= b;
        if (a < 0)
                a += 2 * M_PI;
        return a;
}




static void set_vertex_attrib_pointer(GfxVAO vao, GfxVBO vbo, GfxAttributeLocation loc, int numFloats, int stride, int offset)
{
        if (loc == -1)
                message_f("Warning: attribute not available");
        else {
                glBindVertexArray(vao);
                glEnableVertexAttribArray(loc);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glVertexAttribPointer(loc, numFloats, GL_FLOAT, GL_FALSE, stride, (char *) 0 + offset);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
        }
}

static void check_gl_errors(const char *filename, int line)
{
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
                fatal_f("In %s line %d: GL error %s\n", filename, line,
                        gluErrorString(err));
}

static void set_array_buffer_data(int bufferId, void *data, size_t numElems, size_t elemSize)
{
        glBindBuffer(GL_ARRAY_BUFFER, bufferId);
        glBufferData(GL_ARRAY_BUFFER, numElems * elemSize, data, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#define CHECK_GL_ERRORS() check_gl_errors(__FILE__, __LINE__)
#define SET_ARRAY_BUFFER_DATA(bufferId, data, numElems) set_array_buffer_data((bufferId), (data), (numElems), sizeof *(data))
#define SET_VERTEX_ATTRIB_POINTER(vao, vbo, loc, numFloats, type, member) set_vertex_attrib_pointer(vao, vbo, loc, numFloats, sizeof (type), offsetof(type, member))

static void make_draw_call(GLuint program, GLuint vao, int primitiveKind, int firstIndex, int count)
{
        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(primitiveKind, firstIndex, count);
        glBindVertexArray(0);
        glUseProgram(0);
}

void set_uniform_1f(int program, int location, float x)
{
        glUseProgram(program);
        glUniform1f(location, x);
        glUseProgram(0);
}

void set_uniform_2f(int program, int location, float x, float y)
{
        glUseProgram(program);
        glUniform2f(location, x, y);
        glUseProgram(0);
}

void set_uniform_3f(int program, int location, float x, float y, float z)
{
        glUseProgram(program);
        glUniform3f(location, x, y, z);
        glUseProgram(0);
}

void set_uniform_4f(int program, int location, float x, float y, float z, float w)
{
        glUseProgram(program);
        glUniform4f(location, x, y, z, w);
        glUseProgram(0);
}

void set_uniform_mat2f(int program, int location, const struct Mat2 *mat)
{
        glUseProgram(program);
        glUniformMatrix2fv(location, 1, GL_TRUE, &mat->mat[0][0]);
        glUseProgram(0);
}

void set_uniform_mat3f(int program, int location, const struct Mat3 *mat)
{
        glUseProgram(program);
        glUniformMatrix3fv(location, 1, GL_TRUE, &mat->mat[0][0]);
        glUseProgram(0);
}

void set_uniform_mat4f(int program, int location, const struct Mat4 *mat)
{
        glUseProgram(program);
        glUniformMatrix4fv(location, 1, GL_TRUE, &mat->mat[0][0]);
        glUseProgram(0);
}

static void change_mode(void)
{
static int mode;
mode++;
if (mode == 3) mode = 0;
if (mode == 0)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
else if (mode == 1) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
else if (mode == 2)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

static GLuint lineVBO;
static GLuint lineVAO;

static GLuint circleVBO;
static GLuint circleVAO;

static GLuint arcVBO;
static GLuint arcVAO;

static GLuint v3VBO;
static GLuint v3VAO;

void do_gfx(void)
{
        //make_3d_axes();
        make_torus();
        //make_sphere();

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
                                else if (event.tKey.keyKind == KEY_SPACE) {
                                        obtuseArcAngle = !obtuseArcAngle;
                                }
                                else if (event.tKey.keyKind == KEY_BACKSPACE) {
                                        change_mode();
                                }
                                else if (event.tKey.keyKind == KEY_LEFT) {
                                        viewingAngleY = add_modulo_2pi(viewingAngleY, 0.2f);
                                }
                                else if (event.tKey.keyKind == KEY_RIGHT) {
                                        viewingAngleY = sub_modulo_2pi(viewingAngleY, 0.2f);
                                }
                                else if (event.tKey.keyKind == KEY_UP) {
                                        viewingAngleX = add_modulo_2pi(viewingAngleX, 0.2f);
                                }
                                else if (event.tKey.keyKind == KEY_DOWN) {
                                        viewingAngleX = sub_modulo_2pi(viewingAngleX, 0.2f);
                                }
                        }
                        else if (event.eventKind == EVENT_MOUSEBUTTON) {
                                if (event.tMousebutton.mousebuttonKind == MOUSEBUTTON_1) {
                                        if (event.tMousebutton.mousebuttoneventKind == MOUSEBUTTONEVENT_PRESS) {
        CHECK_GL_ERRORS();
                                                line_to(mouseX, mouseY);
        CHECK_GL_ERRORS();
                                        }
                                }
                        }
                        else if (event.eventKind == EVENT_MOUSEMOVE) {
                                mouseX = (2.0f * event.tMousemove.x / windowWidth) - 1.0f;
                                mouseY = - ((2.0f * event.tMousemove.y / windowHeight) - 1.0f);
                        }
                        else if (event.eventKind == EVENT_SCROLL) {
                                zoomFactor += 0.25f * event.tScroll.amount;
                                if (zoomFactor < 1.f)
                                        zoomFactor = 1.f;
                                else if (zoomFactor > 3.f)
                                        zoomFactor = 3.f;
                        }
                        else if (event.eventKind == EVENT_WINDOWRESIZE) {
                                windowWidth = event.tWindowresize.w;
                                windowHeight = event.tWindowresize.h;
                                glViewport(0, 0, windowWidth, windowHeight);
                        }
                }

                add_line(currentX, currentY, mouseX, mouseY);
                add_circle(mouseX, mouseY);
                add_arc((struct Vec2) {arcX, arcY}, (struct Vec2) { currentX, currentY }, (struct Vec2) {mouseX, mouseY});

                SET_ARRAY_BUFFER_DATA(lineVBO, lineVertices, numLineVertices);
                SET_ARRAY_BUFFER_DATA(circleVBO, circleVertices, numCircleVertices);
                SET_ARRAY_BUFFER_DATA(arcVBO, arcVertices, numArcVertices);
                SET_ARRAY_BUFFER_DATA(v3VBO, v3Vertices, numV3Vertices);
                CHECK_GL_ERRORS();

                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                compute_screen_transform();

                lineShader_set_screenTransform(&screenTransform);
                circleShader_set_screenTransform(&screenTransform);
                arcShader_set_screenTransform(&screenTransform);
                v3Shader_set_screenTransform(&screenTransform);

                glDisable(GL_CULL_FACE);
                make_draw_call(gfxProgram[PROGRAM_line], lineVAO, GL_TRIANGLES, 0, numLineVertices);
                make_draw_call(gfxProgram[PROGRAM_circle], circleVAO, GL_TRIANGLES, 0, numCircleVertices);
                make_draw_call(gfxProgram[PROGRAM_arc], arcVAO, GL_TRIANGLES, 0, numArcVertices);

                //glEnable(GL_CULL_FACE);
                make_draw_call(gfxProgram[PROGRAM_v3], v3VAO, GL_TRIANGLES, 0, numV3Vertices);
                CHECK_GL_ERRORS();

                numLineVertices -= 6;
                numCircleVertices -= 6;
                numArcVertices -= 6;

                swap_buffers();
        }

        glDeleteBuffers(1, &lineVBO);
        glDeleteVertexArrays(1, &lineVAO);
}

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
        CHECK_GL_ERRORS();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glDisable(GL_MULTISAMPLE);

                        CHECK_GL_ERRORS();
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
		const char *source = smShaderInfo[i].shaderSource;
                int sourceSize = smShaderInfo[i].shaderSourceSize;
		glShaderSource(gfxShader[i], 1, &source, &sourceSize);
        }
        for (int i = 0; i < NUM_SHADER_KINDS; i++) {
                glCompileShader(gfxShader[i]);
        }
        for (int i = 0; i < NUM_SHADER_KINDS; i++) {
		if (!get_compile_status(i))
                        fatal_f("Failed to compile!");
        }
                CHECK_GL_ERRORS();
        for (int i = 0; i < NUM_PROGRAM_KINDS; i++) {
                gfxProgram[i] = glCreateProgram();
                if (gfxProgram[i] == 0) {
                        fatal_f("glCreateProgram() failed");
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
		if (!get_link_status(i))
                        fatal_f("Failed to link!");
        }
                CHECK_GL_ERRORS();
        for (int i = 0; i < NUM_ATTRIBUTE_KINDS; i++) {
                int pi = smAttributeInfo[i].programIndex;
                const char *name = smAttributeInfo[i].name;
                gfxAttributeLocation[i] = glGetAttribLocation(gfxProgram[pi], name);
                if (gfxAttributeLocation[i] == -1) {
                        message_f("Warning: Shader '%s', attribute '%s' not available",
                                  smProgramInfo[pi].name, name);
                }
        }
                CHECK_GL_ERRORS();
        for (int i = 0; i < NUM_UNIFORM_KINDS; i++) {
                int pi = smUniformInfo[i].programIndex;
                const char *name = smUniformInfo[i].name;
                gfxUniformLocation[i] = glGetUniformLocation(gfxProgram[pi], name);
                if (gfxUniformLocation[i] == -1) {
                        message_f("Warning: Shader '%s', attribute '%s' not available",
                                  smProgramInfo[pi].name, name);
                }
        }
                CHECK_GL_ERRORS();

                CHECK_GL_ERRORS();

        glGenBuffers(1, &lineVBO);
        glGenVertexArrays(1, &lineVAO);
        SET_VERTEX_ATTRIB_POINTER(lineVAO, lineVBO, gfxAttributeLocation[ATTRIBUTE_line_position], 2, struct LineVertex, position);
        SET_VERTEX_ATTRIB_POINTER(lineVAO, lineVBO, gfxAttributeLocation[ATTRIBUTE_line_normal], 2, struct LineVertex, normal);
        SET_VERTEX_ATTRIB_POINTER(lineVAO, lineVBO, gfxAttributeLocation[ATTRIBUTE_line_color], 3, struct LineVertex, color);

        glGenBuffers(1, &circleVBO);
        glGenVertexArrays(1, &circleVAO);
        SET_VERTEX_ATTRIB_POINTER(circleVAO, circleVBO, gfxAttributeLocation[ATTRIBUTE_circle_centerPoint], 2, struct CircleVertex, centerPoint);
        SET_VERTEX_ATTRIB_POINTER(circleVAO, circleVBO, gfxAttributeLocation[ATTRIBUTE_circle_diff], 2, struct CircleVertex, diff);
        SET_VERTEX_ATTRIB_POINTER(circleVAO, circleVBO, gfxAttributeLocation[ATTRIBUTE_circle_color], 3, struct CircleVertex, color);
        SET_VERTEX_ATTRIB_POINTER(circleVAO, circleVBO, gfxAttributeLocation[ATTRIBUTE_circle_radius], 1, struct CircleVertex, radius);

        CHECK_GL_ERRORS();

        glGenBuffers(1, &arcVBO);
        glGenVertexArrays(1, &arcVAO);
        SET_VERTEX_ATTRIB_POINTER(arcVAO, arcVBO, gfxAttributeLocation[ATTRIBUTE_arc_startPoint], 2, struct ArcVertex, startPoint);
        SET_VERTEX_ATTRIB_POINTER(arcVAO, arcVBO, gfxAttributeLocation[ATTRIBUTE_arc_centerPoint], 2, struct ArcVertex, centerPoint);
        SET_VERTEX_ATTRIB_POINTER(arcVAO, arcVBO, gfxAttributeLocation[ATTRIBUTE_arc_position], 2, struct ArcVertex, position);
        SET_VERTEX_ATTRIB_POINTER(arcVAO, arcVBO, gfxAttributeLocation[ATTRIBUTE_arc_color], 3, struct ArcVertex, color);
        SET_VERTEX_ATTRIB_POINTER(arcVAO, arcVBO, gfxAttributeLocation[ATTRIBUTE_arc_diffAngle], 1, struct ArcVertex, diffAngle);
        SET_VERTEX_ATTRIB_POINTER(arcVAO, arcVBO, gfxAttributeLocation[ATTRIBUTE_arc_radius], 1, struct ArcVertex, radius);
        CHECK_GL_ERRORS();

        glGenBuffers(1, &v3VBO);
        glGenVertexArrays(1, &v3VAO);
        SET_VERTEX_ATTRIB_POINTER(v3VAO, v3VBO, gfxAttributeLocation[ATTRIBUTE_v3_position], 3, struct V3Vertex, position);
        SET_VERTEX_ATTRIB_POINTER(v3VAO, v3VBO, gfxAttributeLocation[ATTRIBUTE_v3_normal], 3, struct V3Vertex, normal);
        SET_VERTEX_ATTRIB_POINTER(v3VAO, v3VBO, gfxAttributeLocation[ATTRIBUTE_v3_color], 3, struct V3Vertex, color);
        CHECK_GL_ERRORS();
}
