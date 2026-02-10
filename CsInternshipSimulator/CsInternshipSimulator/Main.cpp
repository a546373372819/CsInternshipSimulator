#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "Util.h"

struct Vec3 { float x, y, z; };
struct Mat4 { float m[16]; };

static Mat4 identity() {
    Mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

static Mat4 mul(const Mat4& a, const Mat4& b) {
    Mat4 r{};
    for (int c = 0; c < 4; ++c) {
        for (int rI = 0; rI < 4; ++rI) {
            r.m[c * 4 + rI] =
                a.m[0 * 4 + rI] * b.m[c * 4 + 0] +
                a.m[1 * 4 + rI] * b.m[c * 4 + 1] +
                a.m[2 * 4 + rI] * b.m[c * 4 + 2] +
                a.m[3 * 4 + rI] * b.m[c * 4 + 3];
        }
    }
    return r;
}

static Mat4 translate(float x, float y, float z) {
    Mat4 r = identity();
    r.m[12] = x; r.m[13] = y; r.m[14] = z;
    return r;
}

static Mat4 scale(float x, float y, float z) {
    Mat4 r = identity();
    r.m[0] = x; r.m[5] = y; r.m[10] = z;
    return r;
}

static Mat4 perspective(float fovyRad, float aspect, float nearZ, float farZ) {
    Mat4 r{};
    float f = 1.0f / std::tan(fovyRad * 0.5f);
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (farZ + nearZ) / (nearZ - farZ);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    return r;
}

static Vec3 sub(const Vec3& a, const Vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static Vec3 cross(const Vec3& a, const Vec3& b) { return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
static float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Vec3 norm(const Vec3& v) { float l = std::sqrt(std::max(0.0001f, dot(v, v))); return { v.x / l, v.y / l, v.z / l }; }

static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = norm(sub(center, eye));
    Vec3 s = norm(cross(f, up));
    Vec3 u = cross(s, f);

    Mat4 r = identity();
    r.m[0] = s.x; r.m[4] = s.y; r.m[8] = s.z;
    r.m[1] = u.x; r.m[5] = u.y; r.m[9] = u.z;
    r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
    r.m[12] = -dot(s, eye);
    r.m[13] = -dot(u, eye);
    r.m[14] = dot(f, eye);
    return r;
}

struct Mesh { GLuint vao = 0, vbo = 0; GLsizei count = 0; };

static Mesh makeMesh(const std::vector<float>& v) {
    Mesh m;
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    m.count = static_cast<GLsizei>(v.size() / 8);
    return m;
}

static Mesh createCube() {
    std::vector<float> v = {
        -0.5f,-0.5f, 0.5f, 0,0,1, 0,0,  0.5f,-0.5f, 0.5f, 0,0,1, 1,0,  0.5f,0.5f,0.5f, 0,0,1, 1,1,
        -0.5f,-0.5f,0.5f,0,0,1,0,0, 0.5f,0.5f,0.5f,0,0,1,1,1, -0.5f,0.5f,0.5f,0,0,1,0,1,
        -0.5f,-0.5f,-0.5f,0,0,-1,1,0, -0.5f,0.5f,-0.5f,0,0,-1,1,1, 0.5f,0.5f,-0.5f,0,0,-1,0,1,
        -0.5f,-0.5f,-0.5f,0,0,-1,1,0, 0.5f,0.5f,-0.5f,0,0,-1,0,1, 0.5f,-0.5f,-0.5f,0,0,-1,0,0,
        -0.5f,0.5f,-0.5f,0,1,0,0,0, -0.5f,0.5f,0.5f,0,1,0,0,1, 0.5f,0.5f,0.5f,0,1,0,1,1,
        -0.5f,0.5f,-0.5f,0,1,0,0,0, 0.5f,0.5f,0.5f,0,1,0,1,1, 0.5f,0.5f,-0.5f,0,1,0,1,0,
        -0.5f,-0.5f,-0.5f,0,-1,0,1,1, 0.5f,-0.5f,0.5f,0,-1,0,0,0, -0.5f,-0.5f,0.5f,0,-1,0,1,0,
        -0.5f,-0.5f,-0.5f,0,-1,0,1,1, 0.5f,-0.5f,-0.5f,0,-1,0,0,1, 0.5f,-0.5f,0.5f,0,-1,0,0,0,
        -0.5f,-0.5f,-0.5f,-1,0,0,0,0, -0.5f,-0.5f,0.5f,-1,0,0,1,0, -0.5f,0.5f,0.5f,-1,0,0,1,1,
        -0.5f,-0.5f,-0.5f,-1,0,0,0,0, -0.5f,0.5f,0.5f,-1,0,0,1,1, -0.5f,0.5f,-0.5f,-1,0,0,0,1,
        0.5f,-0.5f,-0.5f,1,0,0,1,0, 0.5f,0.5f,0.5f,1,0,0,0,1, 0.5f,-0.5f,0.5f,1,0,0,0,0,
        0.5f,-0.5f,-0.5f,1,0,0,1,0, 0.5f,0.5f,-0.5f,1,0,0,1,1, 0.5f,0.5f,0.5f,1,0,0,0,1,
    };
    return makeMesh(v);
}

static Mesh createCylinder(float r, float h, int seg, bool coneBottom) {
    std::vector<float> v;
    const float y0 = -h * 0.5f, y1 = h * 0.5f;
    for (int i = 0; i < seg; ++i) {
        float a0 = (2.0f * 3.1415926f * i) / seg;
        float a1 = (2.0f * 3.1415926f * (i + 1)) / seg;
        float x0 = std::cos(a0), z0 = std::sin(a0), x1 = std::cos(a1), z1 = std::sin(a1);
        v.insert(v.end(), { r*x0,y0,r*z0,x0,0,z0,0,0, r*x1,y0,r*z1,x1,0,z1,1,0, r*x1,y1,r*z1,x1,0,z1,1,1,
                            r*x0,y0,r*z0,x0,0,z0,0,0, r*x1,y1,r*z1,x1,0,z1,1,1, r*x0,y1,r*z0,x0,0,z0,0,1 });
    }
    for (int i = 0; i < seg; ++i) {
        float a0 = (2.0f * 3.1415926f * i) / seg;
        float a1 = (2.0f * 3.1415926f * (i + 1)) / seg;
        float x0 = std::cos(a0), z0 = std::sin(a0), x1 = std::cos(a1), z1 = std::sin(a1);
        if (coneBottom) {
            v.insert(v.end(), { 0,y0,0,0,-1,0,0.5f,0.5f, r*x1,y0,r*z1,0,-1,0,1,0, r*x0,y0,r*z0,0,-1,0,0,0 });
        } else {
            v.insert(v.end(), { 0,y0,0,0,-1,0,0.5f,0.5f, r*x0,y0,r*z0,0,-1,0,0,0, r*x1,y0,r*z1,0,-1,0,1,0 });
        }
        v.insert(v.end(), { 0,y1,0,0,1,0,0.5f,0.5f, r*x1,y1,r*z1,0,1,0,1,1, r*x0,y1,r*z0,0,1,0,0,1 });
    }
    return makeMesh(v);
}

enum class AppState { Menu, Cooking, Assembling, Done };

static bool gDepth = true;
static bool gCull = true;
static bool gLight = true;
static float gYaw = -90.0f, gPitch = -20.0f;
static bool gFirstMouse = true;
static double gLastMx = 0.0, gLastMy = 0.0;

static void key_callback(GLFWwindow* w, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(w, GLFW_TRUE);
    if (action == GLFW_PRESS && key == GLFW_KEY_F5) gDepth = !gDepth;
    if (action == GLFW_PRESS && key == GLFW_KEY_F6) gCull = !gCull;
    if (action == GLFW_PRESS && key == GLFW_KEY_L) gLight = !gLight;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "CsInternshipSimulator 3D", monitor, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    glewInit();
    glViewport(0, 0, mode->width, mode->height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint shader3d = createShader("scene3d.vert", "scene3d.frag");
    GLuint rectShader = createShader("rect.vert", "rect.frag");
    GLuint barShader = createShader("bar.vert", "bar.frag");

    Mesh cube = createCube();
    Mesh cylinder = createCylinder(0.5f, 1.0f, 32, false);
    Mesh coneCyl = createCylinder(0.45f, 0.45f, 32, true);

    GLuint stoveTex = 0, tableTex = 0, pattyTex = 0, ketchupTex = 0, mustardTex = 0, btnTex = 0, bgTex = 0, prijatnoTex = 0;
    preprocessTexture(stoveTex, "text/stove.png");
    preprocessTexture(tableTex, "text/table.png");
    preprocessTexture(pattyTex, "text/cooked.png");
    preprocessTexture(ketchupTex, "text/ketchupbottle.png");
    preprocessTexture(mustardTex, "text/mustardbottle.png");
    preprocessTexture(btnTex, "text/startbutton.png");
    preprocessTexture(bgTex, "text/mcdonalds.jpg");
    preprocessTexture(prijatnoTex, "text/prijatno.png");

    float quad[] = {
        -1,1,0,1, -1,-1,0,0, 1,-1,1,0, 1,1,1,1,
        -0.3f,0.2f,0,1, -0.3f,-0.2f,0,0, 0.3f,-0.2f,1,0, 0.3f,0.2f,1,1
    };
    GLuint vao2d = 0, vbo2d = 0;
    glGenVertexArrays(1, &vao2d); glGenBuffers(1, &vbo2d);
    glBindVertexArray(vao2d); glBindBuffer(GL_ARRAY_BUFFER, vbo2d); glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); glEnableVertexAttribArray(1);

    Vec3 camPos{ 0.0f, 1.8f, 4.5f };
    Vec3 lightPos{ 1.0f, 2.5f, 1.0f };
    AppState state = AppState::Menu;
    float cookProgress = 0.0f;
    Vec3 pattyPos{ 0.0f, 0.65f, 0.4f };
    int ingredientStep = 0;
    Vec3 ingredientPos{ -0.4f, 1.2f, 0.0f };

    auto draw3d = [&](const Mesh& m, GLuint tex, const Mat4& model, const Mat4& view, const Mat4& proj) {
        glUseProgram(shader3d);
        glUniformMatrix4fv(glGetUniformLocation(shader3d, "uModel"), 1, GL_FALSE, model.m);
        glUniformMatrix4fv(glGetUniformLocation(shader3d, "uView"), 1, GL_FALSE, view.m);
        glUniformMatrix4fv(glGetUniformLocation(shader3d, "uProj"), 1, GL_FALSE, proj.m);
        glUniform3f(glGetUniformLocation(shader3d, "uLightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shader3d, "uViewPos"), camPos.x, camPos.y, camPos.z);
        glUniform1i(glGetUniformLocation(shader3d, "uLightEnabled"), gLight ? 1 : 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindVertexArray(m.vao);
        glDrawArrays(GL_TRIANGLES, 0, m.count);
    };

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = static_cast<float>(now - last); last = now;

        if (gDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (gCull) { glEnable(GL_CULL_FACE); glCullFace(GL_BACK); } else glDisable(GL_CULL_FACE);

        if (state != AppState::Menu && state != AppState::Done) {
            float speed = 2.5f * dt;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) camPos.x -= speed;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camPos.x += speed;
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) camPos.z -= speed;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) camPos.z += speed;

            double mx, my; glfwGetCursorPos(window, &mx, &my);
            if (gFirstMouse) { gLastMx = mx; gLastMy = my; gFirstMouse = false; }
            float dx = static_cast<float>(mx - gLastMx) * 0.12f;
            float dy = static_cast<float>(gLastMy - my) * 0.12f;
            gLastMx = mx; gLastMy = my;
            gYaw += dx; gPitch = std::clamp(gPitch + dy, -80.0f, 80.0f);
        }

        if (state == AppState::Cooking) {
            float ms = 1.3f * dt;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pattyPos.z -= ms;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pattyPos.z += ms;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pattyPos.x -= ms;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pattyPos.x += ms;
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) pattyPos.y -= ms;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) pattyPos.y += ms;
            if (pattyPos.x > -0.8f && pattyPos.x < 0.8f && pattyPos.z > -0.9f && pattyPos.z < -0.2f && pattyPos.y < 0.45f) cookProgress = std::min(1.0f, cookProgress + dt * 0.18f);
            if (cookProgress >= 1.0f) state = AppState::Assembling;
        } else if (state == AppState::Assembling) {
            float ms = 1.3f * dt;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) ingredientPos.z -= ms;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) ingredientPos.z += ms;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) ingredientPos.x -= ms;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) ingredientPos.x += ms;
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ingredientPos.y -= ms;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) ingredientPos.y += ms;
            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && ingredientPos.y < 0.8f) {
                ingredientStep++;
                ingredientPos = { -0.4f + 0.15f * ingredientStep, 1.2f, 0.0f };
                if (ingredientStep > 5) state = AppState::Done;
            }
        }

        glClearColor(0.08f, 0.1f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Vec3 front{
            std::cos(gYaw * 3.1415926f / 180.0f) * std::cos(gPitch * 3.1415926f / 180.0f),
            std::sin(gPitch * 3.1415926f / 180.0f),
            std::sin(gYaw * 3.1415926f / 180.0f) * std::cos(gPitch * 3.1415926f / 180.0f)
        };
        Mat4 view = lookAt(camPos, { camPos.x + front.x, camPos.y + front.y, camPos.z + front.z }, { 0,1,0 });
        Mat4 proj = perspective(60.0f * 3.1415926f / 180.0f, static_cast<float>(mode->width) / mode->height, 0.1f, 100.0f);

        if (state != AppState::Menu) {
            draw3d(cube, stoveTex, mul(translate(0, 0.2f, -0.55f), scale(1.7f, 0.5f, 0.9f)), view, proj);
            draw3d(cube, tableTex, mul(translate(0, 0.55f, 0.0f), scale(2.2f, 0.12f, 1.4f)), view, proj);
            draw3d(cube, tableTex, mul(translate(-0.95f, 0.25f, -0.55f), scale(0.15f, 0.6f, 0.15f)), view, proj);
            draw3d(cube, tableTex, mul(translate(0.95f, 0.25f, -0.55f), scale(0.15f, 0.6f, 0.15f)), view, proj);
            draw3d(cube, tableTex, mul(translate(-0.95f, 0.25f, 0.55f), scale(0.15f, 0.6f, 0.15f)), view, proj);
            draw3d(cube, tableTex, mul(translate(0.95f, 0.25f, 0.55f), scale(0.15f, 0.6f, 0.15f)), view, proj);
            draw3d(cylinder, pattyTex, mul(translate(pattyPos.x, pattyPos.y, pattyPos.z), scale(0.35f, 0.1f, 0.35f)), view, proj);
            draw3d(cylinder, ketchupTex, mul(translate(-0.7f, 0.8f, 0.5f), scale(0.12f, 0.45f, 0.12f)), view, proj);
            draw3d(coneCyl, ketchupTex, mul(translate(-0.7f, 0.48f, 0.5f), scale(0.11f, 0.25f, 0.11f)), view, proj);
            draw3d(cylinder, mustardTex, mul(translate(-0.4f, 0.8f, 0.5f), scale(0.12f, 0.45f, 0.12f)), view, proj);
            draw3d(coneCyl, mustardTex, mul(translate(-0.4f, 0.48f, 0.5f), scale(0.11f, 0.25f, 0.11f)), view, proj);

            if (state == AppState::Assembling) {
                draw3d(cube, pattyTex, mul(translate(ingredientPos.x, ingredientPos.y, ingredientPos.z), scale(0.28f, 0.01f, 0.28f)), view, proj);
            }
        }

        glDisable(GL_DEPTH_TEST);
        glUseProgram(rectShader);
        glBindVertexArray(vao2d);
        glActiveTexture(GL_TEXTURE0);
        glUniform1f(glGetUniformLocation(rectShader, "uCookProgress"), 0.0f);
        glUniform1f(glGetUniformLocation(rectShader, "uX"), 0.0f);
        glUniform1f(glGetUniformLocation(rectShader, "uY"), 0.0f);
        glUniform1f(glGetUniformLocation(rectShader, "uS"), 1.0f);
        if (state == AppState::Menu) {
            glBindTexture(GL_TEXTURE_2D, bgTex);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glBindTexture(GL_TEXTURE_2D, btnTex);
            glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                double mx, my; glfwGetCursorPos(window, &mx, &my);
                float xn = static_cast<float>(mx / mode->width) * 2.0f - 1.0f;
                float yn = -(static_cast<float>(my / mode->height) * 2.0f - 1.0f);
                if (xn >= -0.3f && xn <= 0.3f && yn >= -0.2f && yn <= 0.2f) state = AppState::Cooking;
            }
        }

        if (state == AppState::Cooking) {
            glUseProgram(barShader);
            glBindVertexArray(vao2d);
            glUniform1f(glGetUniformLocation(barShader, "uLeft"), -0.7f);
            glUniform1f(glGetUniformLocation(barShader, "uRight"), 0.7f);
            glUniform1f(glGetUniformLocation(barShader, "uTop"), 0.9f);
            glUniform1f(glGetUniformLocation(barShader, "uBottom"), 0.84f);
            glUniform1f(glGetUniformLocation(barShader, "uFill"), 1.0f);
            glUniform4f(glGetUniformLocation(barShader, "uColor"), 0.1f, 0.1f, 0.1f, 0.8f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glUniform1f(glGetUniformLocation(barShader, "uFill"), cookProgress);
            glUniform4f(glGetUniformLocation(barShader, "uColor"), 0.2f, 0.9f, 0.3f, 1.0f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        if (state == AppState::Done) {
            glUseProgram(rectShader);
            glBindTexture(GL_TEXTURE_2D, prijatnoTex);
            glUniform1f(glGetUniformLocation(rectShader, "uX"), 0.0f);
            glUniform1f(glGetUniformLocation(rectShader, "uY"), 0.0f);
            glUniform1f(glGetUniformLocation(rectShader, "uS"), 0.9f);
            glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
