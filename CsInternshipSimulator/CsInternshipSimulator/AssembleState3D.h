#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include "Puddle.h"
#include "GameState.h"
#include "shader.hpp"
#include "model.hpp"
#include "Ingredient.h"

class GameContext;
class StateManager;

class AssembleState3D : public GameState
{
public:
    AssembleState3D(GameContext& ctx, StateManager& manager);

    void update(GLFWwindow* window, float dt) override;
    void render() override;

    void updatePuddles();
    void initIngredients();
    void moveCurrentIngredient3D( GLFWwindow* window, float dt);
    void spawnPuddleFor3D(const Ingredient3D& ing);
    

    float mRoomHalfExtent;     // room extends [-6..6] in X and Z
    float mRoomHeight;     // y=0 floor, y=4 ceiling
    float mRoomUVTiling;
    int currentIg;

    Ingredient3D ingredients[10];

    Puddle3D puddles[20];


    GLuint roomVAO = 0;
    GLuint roomVBO = 0;

    GLuint wallTex = 0;
    GLuint floorTex = 0;
    GLuint ceilingTex = 0;

    int puddleCount = 0;

    bool lightOn = true;

private:
    // --- shader / lighting ---
    Shader mShader3D;

    glm::vec3 mCamPos = glm::vec3(0.0f, 2.5f, 5.0f);
    glm::vec3 mCamTarget = glm::vec3(0.0f, 0.9f, 0.0f);

    glm::vec3 mCamFront = glm::vec3(0, 0, -1);
    glm::vec3 mCamUp = glm::vec3(0, 1, 0);





    // Light
    glm::vec3 mLightPos = glm::vec3(0.0f, 2.2f, 0.0f);


    bool   mCamInputInit = false;
    double mLastMouseX = 0.0;
    double mLastMouseY = 0.0;

    float  mYaw = -90.0f;
    float  mPitch = 0.0f;

    float  mMouseSensitivity = 0.12f;
    float  mCamMoveSpeed = 3.5f;

    // --- scene models ---
    Model mTable;
    Model mPlate;

    Model patty;
    Model bottomBun;
    Model ketchupBottle;
    Model mustardBottle;
    Model pickles;
    Model onion;
    Model lettuce;
    Model cheese;
    Model tomato;
    Model topBun;
    Model ketchupPuddle;
    Model mustardPuddle;
    unsigned int prijatnoTex = 0;

    glm::vec3 mTablePos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mTableScale = glm::vec3(0.01f);

    glm::vec3 mPlatePos = glm::vec3(0.5f, 0.0f, 0.5f);
    glm::vec3 mPlateScale = glm::vec3(1.5f);

    // You will likely need to tweak these depending on model origins (Blender export).
    float mTableTopY = 0.90f;   // where the tabletop is in world Y
    float mPlateLift = 0.03f;   // small lift so it doesn't z-fight the table

    //room

    float ingredientHalfWidth = 0.1f;
    float ingredientHalfHeight = 0.07f;
    float ingredientSpeed = 0.02f;

    glm::vec3 mIngPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mIngScale = glm::vec3(0.1f);
    

private:
    glm::mat4 makeModelMatrix(const glm::vec3& pos, const glm::vec3& scale) const;
};
