#pragma once

// IMPORTANT: include GLEW before GLFW in any .cpp that includes these headers.
// In headers, try to avoid including GLFW/GLEW unless necessary.

#include <glm/glm.hpp>

class GameContext;
class StateManager;

#include "GameState.h"   // your base state with update/render

// Your 3D helpers (user-provided)
#include "shader.hpp"
#include "model.hpp"

class CookingState3D : public GameState
{
public:
    CookingState3D(GameContext& ctx, StateManager& manager);
    ~CookingState3D() override = default;

    void update(GLFWwindow* window, float dt) override;
    void render() override;

private:
    // 3D shader and models
    Shader mShader3D;
    Model  mStove;
    Model  mPatty;

    // Transforms / positions
    glm::vec3 mStovePos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mPattyPos = glm::vec3(0.0f, 1.2f, 0.0f);

    glm::vec3 mStoveScale = glm::vec3(1.0f);
    glm::vec3 mPattyScale = glm::vec3(1.0f);

    // Camera
    glm::vec3 mCamPos = glm::vec3(0.0f, 2.5f, 5.0f);
    glm::vec3 mCamTarget = glm::vec3(0.0f, 0.9f, 0.0f);

    GLuint roomVAO = 0;
    GLuint roomVBO = 0;

    GLuint wallTex = 0;
    GLuint floorTex = 0;
    GLuint ceilingTex = 0;

    float mRoomHalfExtent ;     // room extends [-6..6] in X and Z
    float mRoomHeight;     // y=0 floor, y=4 ceiling
    float mRoomUVTiling;

    // Light
    glm::vec3 mLightPos = glm::vec3(2.0f, 4.0f, 2.0f);

    // Movement/cooking
    float mMoveSpeed = 2.0f;     // units/sec
    float mCookRate = 0.25f;    // progress/sec

    float cookProgress = 0.0f;
    bool  isCooked = false;

    // Stove collision "plane + bounds" (tune these to your model!)
    float stoveTopY = 0.75f;
    float stoveMinX = -0.7f, stoveMaxX = 0.7f;
    float stoveMinZ = -0.6f, stoveMaxZ = 0.6f;

    // Patty approximate size (tune to match model)
    float pattyHalfHeight = 0.08f;

private:
    glm::mat4 makeModelMatrix(const glm::vec3& pos, const glm::vec3& scale) const;
    void clampPattyToTable(); // invisible walls in XZ
};
