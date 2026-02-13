// IMPORTANT INCLUDE ORDER IN .cpp:
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <windows.h>
#include <iostream>
#include <algorithm>

#include "CookingState3D.h"

// Your app headers:
#include "GameContext.h"
#include "Util.h"
#include "StateManager.h"

// GLM extras:
#include <glm/gtc/matrix_transform.hpp>

static glm::mat4 MakeTRS(const glm::vec3& pos, const glm::vec3& rotDegXYZ, const glm::vec3& scale)
{
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::rotate(M, glm::radians(rotDegXYZ.x), glm::vec3(1, 0, 0));
    M = glm::rotate(M, glm::radians(rotDegXYZ.y), glm::vec3(0, 1, 0));
    M = glm::rotate(M, glm::radians(rotDegXYZ.z), glm::vec3(0, 0, 1));
    M = glm::scale(M, scale);
    return M;
}

CookingState3D::CookingState3D(GameContext& ctx, StateManager& manager)
    : GameState(ctx, manager)
    , mShader3D("mesh3d.vert", "mesh3d.frag")
    , mStove("text/3d/stove.obj")
    , mPatty("text/3d/patty.obj")
{
    // -----------------------------
    // Room parameters (tweak freely)
    // -----------------------------
    mRoomHalfExtent = 6.0f;     // room extends [-6..6] in X and Z
    mRoomHeight = 4.0f;     // y=0 floor, y=4 ceiling
    mRoomUVTiling = 6.0f;     // how many times textures repeat on each face

    // Camera + light (tweak freely)
    mCamPos = glm::vec3(-0.5f, 1.5f, 0.5f);
    mCamTarget = glm::vec3(-0.5f, 1.5f, -1.0f);
    mLightPos = glm::vec3(-0.5f, 1.5f, 0.5f);

    // Stove placement (tweak freely)
    mStovePos = glm::vec3(0.0f, 0.0f, -2.5f);
    mStoveScale = glm::vec3(0.80f);

    // Patty placement (tweak freely)
    mPattyScale = glm::vec3(0.1f);
    mPattyPos = glm::vec3(0.0f, 1.05f, -2.5f); // start above stove

    // Cooking/collision constants (you MUST tune these to your model sizes)
    // These are WORLD units. If your stove model is not aligned, adjust.
    stoveTopY = 0.665966f;     // y of stove top plane (world)
    pattyHalfHeight = 0.0f;     // half thickness used for snapping (world)

    // Stove top bounds in world XZ where patty is allowed to cook.
    // Centered around stove position; tune to your stove mesh.
    stoveMinX = mStovePos.x - 1.0321f;
    stoveMaxX = mStovePos.x  -0.528058f;
    stoveMinZ = mStovePos.z - 0.20f;
    stoveMaxZ = mStovePos.z + 0.30f;

    // Movement/cooking
    mMoveSpeed = 1.8f;
    mCookRate = 0.15f;
    cookProgress = 0.0f;
    isCooked = false;

    // -----------------------------------------
    // Room geometry: one quad on XZ plane (y=0)
    // We'll rotate/translate it to make floor/walls/ceiling.
    // IMPORTANT: it includes UVs at location 2.
    // -----------------------------------------
    const float t = mRoomUVTiling; // uv tiling amount

    static float roomQuad[] = {
        // pos              // normal (up)   // uv
        -1, 0, -1,          0, 1, 0,         0, 0,
         1, 0, -1,          0, 1, 0,         t, 0,
         1, 0,  1,          0, 1, 0,         t, t,

        -1, 0, -1,          0, 1, 0,         0, 0,
         1, 0,  1,          0, 1, 0,         t, t,
        -1, 0,  1,          0, 1, 0,         0, t,
    };

    glGenVertexArrays(1, &roomVAO);
    glGenBuffers(1, &roomVBO);

    glBindVertexArray(roomVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roomVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomQuad), roomQuad, GL_STATIC_DRAW);

    // layout(location=0) vec3 position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // layout(location=1) vec3 normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // layout(location=2) vec2 uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // -----------------------------
    // Load textures for the room
    // -----------------------------
    wallTex = loadImageToTexture("text/walls.png");
    floorTex = loadImageToTexture("text/floor.png");
    ceilingTex = loadImageToTexture("text/walls.png");

    // 3D essentials
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Make sure the shader uses texture unit 0 for room + models by default.
    mShader3D.use();
    mShader3D.setInt("uDiffMap1", 0);

    // cook uniform (patty)
    mShader3D.setFloat("uCookProgress", 0.0f);

    mCamFront = glm::normalize(mCamTarget - mCamPos);

}



glm::mat4 CookingState3D::makeModelMatrix(const glm::vec3& pos, const glm::vec3& scale) const
{
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::scale(M, scale);
    return M;
}

void CookingState3D::clampPattyToTable()
{
    // keep patty above stove bounds in XZ only
    mPattyPos.x = std::max(stoveMinX, std::min(mPattyPos.x, stoveMaxX));
    mPattyPos.z = std::max(stoveMinZ, std::min(mPattyPos.z, stoveMaxZ));
}

void CookingState3D::update(GLFWwindow* window, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    glm::vec3 prevPos = mPattyPos;

    // Movement
    glm::vec3 dir(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dir.z -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dir.z += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dir.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir.x += 1.0f;

    // Up/Down
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) dir.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) dir.y -= 1.0f;

    if (glm::length(dir) > 0.0f) dir = glm::normalize(dir);
    mPattyPos += dir * (mMoveSpeed * dt);

    clampPattyToTable();

    // Stove collision + snap
    // Stove collision + snap
    float prevBottomY = prevPos.y - pattyHalfHeight;
    float bottomY = mPattyPos.y - pattyHalfHeight;

    bool overStove =
        (mPattyPos.x >= stoveMinX && mPattyPos.x <= stoveMaxX) &&
        (mPattyPos.z >= stoveMinZ && mPattyPos.z <= stoveMaxZ);

    bool touchingPlane = (bottomY <= stoveTopY);
    bool touchingStove = overStove && touchingPlane;
    
    // Snap only when moving downward onto the plane
    if (touchingStove ) {
        mPattyPos.y = stoveTopY + pattyHalfHeight;
        bottomY = stoveTopY;
    }

    // Cooking
    if (touchingStove && !isCooked) {
        cookProgress += mCookRate * dt;
        if (cookProgress >= 1.0f) {
            cookProgress = 1.0f;
            isCooked = true;
            manager.changeState(StateID::Assembling);
        }
    }

    //camera 

    if (!mCamInputInit) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &mLastMouseX, &mLastMouseY);
        mFirstMouse = false;
        mCamInputInit = true;
    }

    // mouse look (polling)
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    double xoffset = mx - mLastMouseX;
    double yoffset = mLastMouseY - my; // reversed Y

    mLastMouseX = mx;
    mLastMouseY = my;

    mYaw += float(xoffset) * mMouseSensitivity;
    mPitch += float(yoffset) * mMouseSensitivity;

    // clamp pitch to avoid flip
    if (mPitch > 89.0f)  mPitch = 89.0f;
    if (mPitch < -89.0f) mPitch = -89.0f;

    // recompute forward from yaw/pitch
    glm::vec3 front;
    front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    front.y = sin(glm::radians(mPitch));
    front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    mCamFront = glm::normalize(front);

    // update target from position + direction
    mCamTarget = mCamPos + mCamFront;

    //moving

    const float speed = mCamMoveSpeed * dt;

    // right vector
    glm::vec3 right = glm::normalize(glm::cross(mCamFront, mCamUp));

    // Arrow keys: move camera
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        mCamPos += mCamFront * speed;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        mCamPos -= mCamFront * speed;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        mCamPos -= right * speed;

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        mCamPos += right * speed;

    // optional vertical movement (PageUp/PageDown)
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        mCamPos += mCamUp * speed;

    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        mCamPos -= mCamUp * speed;

    // keep target consistent
    mCamTarget = mCamPos + mCamFront;

    //bar 

   

}

void CookingState3D::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int w = 1280, h = 720;
    if (GLFWwindow* window = glfwGetCurrentContext()) {
        glfwGetFramebufferSize(window, &w, &h);
    }

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), float(w) / float(h), 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(mCamPos, mCamTarget, glm::vec3(0, 1, 0));

    mShader3D.use();
    mShader3D.setMat4("view", view);
    mShader3D.setMat4("projection", projection);
    mShader3D.setVec3("lightPos", mLightPos);
    mShader3D.setVec3("viewPos", mCamPos);

    // -----------------------------
    // Draw room (floor + ceiling + 4 walls)
    // Quad is centered at origin, XZ plane, scaled to room size and then rotated/translated.
    // -----------------------------


    auto drawQuadFace = [&](GLuint tex, const glm::mat4& model) {
        mShader3D.setBool("uHasDiffuseMap", true);
        mShader3D.setFloat("uCookProgress", 0.0f);
        mShader3D.setInt("uDiffMap1", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        mShader3D.setMat4("model", model);

        glBindVertexArray(roomVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    };

    const float H = mRoomHeight;
    const float E = mRoomHalfExtent;



    // Floor (XZ)
    drawQuadFace(
        floorTex,
        MakeTRS(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 0, 0), glm::vec3(E, 1.0f, E))
    );

    // Ceiling (XZ), flip to face downward
    drawQuadFace(
        ceilingTex,
        MakeTRS(glm::vec3(0.0f, H, 0.0f), glm::vec3(180, 0, 0), glm::vec3(E, 1.0f, E))
    );

    // Back wall (XY plane at z=-E), face inward
    drawQuadFace(
        wallTex,
        MakeTRS(glm::vec3(0.0f, H * 0.5f, -E), glm::vec3(90, 0, 0), glm::vec3(E, 1.0f, H * 0.5f))
    );

    // Front wall (XY plane at z=+E), face inward
    drawQuadFace(
        wallTex,
        MakeTRS(glm::vec3(0.0f, H * 0.5f, +E), glm::vec3(-90, 0, 0), glm::vec3(E, 1.0f, H * 0.5f))
    );

    // Left wall (ZY plane at x=-E), face inward
    drawQuadFace(
        wallTex,
        MakeTRS(glm::vec3(-E, H * 0.5f, 0), glm::vec3(-90, 0, -90), glm::vec3(E, 1.0f, H ))
    );

    // Right wall (ZY plane at x=+E), face inward
    drawQuadFace(
        wallTex,
        MakeTRS(glm::vec3(+E, H * 0.5f, 0), glm::vec3(90, 0, 90), glm::vec3(E, 1.0f, H))
    );


    // -----------------------------
    // Draw stove
    // -----------------------------
    {
        glm::mat4 M = makeModelMatrix(mStovePos, mStoveScale);
        mShader3D.setMat4("model", M);
        mShader3D.setFloat("uCookProgress", 0.0f);
        mStove.Draw(mShader3D);
    }

    // -----------------------------
    // Draw patty (with browning)
    // -----------------------------
    {
        mShader3D.setBool("uHasDiffuseMap", false);

        glm::mat4 M = makeModelMatrix(mPattyPos, mPattyScale);
        mShader3D.setMat4("model", M);

        float visualCook = std::min(cookProgress, 0.75f);
        mShader3D.setFloat("uCookProgress", visualCook);

        mPatty.Draw(mShader3D);
    }

    // Optional overlays (2D) go here after 3D
     //  loading bar 
    glUseProgram(ctx.barShader);
    glBindVertexArray(ctx.VAObar);

    // zajednicke granice bara
    glUniform1f(glGetUniformLocation(ctx.barShader, "uLeft"), barLeft);
    glUniform1f(glGetUniformLocation(ctx.barShader, "uRight"), barRight);
    glUniform1f(glGetUniformLocation(ctx.barShader, "uTop"), barTop);
    glUniform1f(glGetUniformLocation(ctx.barShader, "uBottom"), barBottom);

    

    //zeleni deo
    float t = cookProgress;

    std::cout << t << "\n";

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    glUniform1f(glGetUniformLocation(ctx.barShader, "uFill"), t);
    glUniform4f(glGetUniformLocation(ctx.barShader, "uColor"), 0.2f, 0.8f, 0.2f, 0.9f);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //  sivi prazan bar 
    glUniform1f(glGetUniformLocation(ctx.barShader, "uFill"), 1.0f);
    glUniform4f(glGetUniformLocation(ctx.barShader, "uColor"), 0.1f, 0.1f, 0.1f, 0.7f);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
