// CookingState.h
#pragma once
#include "GameState.h"

class CookingState : public GameState
{
private:



public:
    CookingState(GameContext& ctx, StateManager& manager);

    void update(GLFWwindow* window, float dt) override;
    void render() override;

    unsigned int cookingBackgroundTexture = 0;
    unsigned int pattyTexture = 0;
    unsigned int employeeTexture = 0;
    // patty data
    float pattyX = 0.0f;
    float pattyY = 0.2f;

    float cookProgress = 0.0f;
    bool isCooked = false;

    float pattyHalfWidth = 0.07f;
    float pattyHalfHeight = 0.07f;
    float pattySpeed = 0.02f;


    // pozicija loading bara u NDC
    float barLeft = -0.8f;
    float barRight = 0.8f;
    float barTop = 0.9f;
    float barBottom = 0.85f;


    // stove AABB in NDC – TWEAK to match your cooking background stove
    float stoveLeft = -1.0f;
    float stoveRight = 1.0f;
    float stoveTop = -0.15f;
    float stoveBottom = -1.0f;


};
