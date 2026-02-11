// IMPORTANT INCLUDE ORDER IN .cpp:
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <windows.h>
#include <iostream>

#include "CookingState3D.h"

// Your app headers:
#include "GameContext.h"
#include "Util.h"

#include "StateManager.h"

// GLM extras:
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm> // std::min/max

CookingState3D::CookingState3D(GameContext& ctx, StateManager& manager)
    : GameState(ctx, manager)
    // Paths are examples – set them to your real files:
    
    , mShader3D("mesh3d.vert", "mesh3d.frag")
    , mStove("text/stove.obj")
    , mPatty("text/patty.obj")
{

    static float roomQuad[] = {
        // pos            // normal        // uv
        -1,0,-1,          0,1,0,            0,0,
         1,0,-1,          0,1,0,            1,0,
         1,0, 1,          0,1,0,            1,1,
        -1,0,-1,          0,1,0,            0,0,
         1,0, 1,          0,1,0,            1,1,
        -1,0, 1,          0,1,0,            0,1,
    };

    glGenVertexArrays(1, &roomVAO);
    glGenBuffers(1, &roomVBO);

    glBindVertexArray(roomVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roomVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomQuad), roomQuad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // 2) Load textures
    wallTex = loadImageToTexture("text/wall.jpg");
    floorTex = loadImageToTexture("text/wall.jpg");
    ceilingTex = loadImageToTexture("text/wall.jpg");
    
    // Typical: model scale needs tuning (most models are huge or tiny).
    mStoveScale = glm::vec3(1.0f);
    mPattyScale = glm::vec3(1.0f);

    mPattyPos = glm::vec3(0.0f, 1.2f, 0.0f);

    cookProgress = 0.0f;
    isCooked = false;

    // 3D essentials:
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    // invisible walls: keep patty above stove area / table area (XZ only)
    mPattyPos.x = std::max(stoveMinX, std::min(mPattyPos.x, stoveMaxX));
    mPattyPos.z = std::max(stoveMinZ, std::min(mPattyPos.z, stoveMaxZ));
}

void CookingState3D::update(GLFWwindow* window, float dt)
{
    // Escape to exit (same behavior as your 2D)
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    // Save previous position to detect "coming from above"
    glm::vec3 prevPos = mPattyPos;

    // -------- Movement in 3 axes --------
    glm::vec3 dir(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dir.z -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dir.z += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dir.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir.x += 1.0f;

    // Up/Down (pick keys you like)
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) dir.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) dir.y -= 1.0f;

    if (glm::length(dir) > 0.0f) dir = glm::normalize(dir);

    mPattyPos += dir * (mMoveSpeed * dt);

    // Optional: limit XZ to stove/table bounds
    clampPattyToTable();

    // -------- Collision with stove top --------
    float prevBottomY = prevPos.y - pattyHalfHeight;
    float bottomY = mPattyPos.y - pattyHalfHeight;

    bool overStove =
        (mPattyPos.x >= stoveMinX && mPattyPos.x <= stoveMaxX) &&
        (mPattyPos.z >= stoveMinZ && mPattyPos.z <= stoveMaxZ);

    bool touchingPlane = (bottomY <= stoveTopY);
    bool touchingStove = overStove && touchingPlane;

    // Snap only if moving downward onto the plane (prevents "can't move up" bug)
    if (touchingStove && prevBottomY > stoveTopY) {
        mPattyPos.y = stoveTopY + pattyHalfHeight;
        bottomY = stoveTopY;
    }

    // -------- Cooking logic --------
    if (touchingStove && !isCooked) {
        cookProgress += mCookRate * dt;
        if (cookProgress >= 1.0f) {
            cookProgress = 1.0f;
            isCooked = true;

            // Transition to next state, like your 2D version
            // manager.changeState(StateID::Assembling);
            // (leave this line to your existing state manager setup)
        }
    }
}

void CookingState3D::render()
{
    // Clear color + depth (IMPORTANT in 3D)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- Build camera matrices ---
    int w, h;
    // If you store size in context, use that instead.
    // For GLFW, you can fetch framebuffer size:
    // (if you have a window pointer in ctx, use that; else skip and use fixed)
    // Here we assume 1280x720 fallback:
    w = 1280; h = 720;

    glm::mat4 projection = glm::perspective(glm::radians(60.0f),
        float(w) / float(h),
        0.1f, 100.0f);

    glm::mat4 view = glm::lookAt(mCamPos, mCamTarget, glm::vec3(0, 1, 0));

    


    // --- Use 3D shader ---
    mShader3D.use();
    mShader3D.setMat4("view", view);
    mShader3D.setMat4("projection", projection);
    mShader3D.setVec3("lightPos", mLightPos);
    mShader3D.setVec3("viewPos", mCamPos);


    //walls
    glBindVertexArray(roomVAO);

    float R = 6.0f;
    float floorY = 0.0f;
    float ceilY = 4.0f;

    // FLOOR
    glBindTexture(GL_TEXTURE_2D, floorTex);
    glm::mat4 M = glm::translate(glm::mat4(1), { 0, floorY, 0  });
    M = glm::scale(M, { R,1,R });
    mShader3D.setMat4("model", M);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // WALL (back)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wallTex);
    M = glm::translate(glm::mat4(1), { 0, 2.0f, -R });
    M = glm::rotate(M, glm::radians(90.f), { 1,0,0 });
    M = glm::scale(M, { R,1,2.0f });
    mShader3D.setMat4("model", M);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // --- Draw stove ---
    {
        glm::mat4 M = makeModelMatrix(mStovePos, mStoveScale);
        mShader3D.setMat4("model", M);
        mShader3D.setFloat("uCookProgress", 0.0f); // stove doesn't cook
        mStove.Draw(mShader3D);
    }

    // --- Draw patty with browning ---
    {
        glm::mat4 M = makeModelMatrix(mPattyPos, mPattyScale);
        mShader3D.setMat4("model", M);

        float visualCook = std::min(cookProgress, 0.75f);
        mShader3D.setFloat("uCookProgress", visualCook);

        mPatty.Draw(mShader3D);
    }

    // --- Draw 2D overlays (optional) ---
    // If you already have loading bar/index drawn in 2D, do it last:
    //
    // glDisable(GL_DEPTH_TEST);
    // renderLoadingBar2D(cookProgress);  // your existing 2D code
    // renderIndexOverlay2D();            // your existing 2D code
    // glEnable(GL_DEPTH_TEST);
}
