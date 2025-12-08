#pragma once
#include "GameState.h"

class MenuState : public GameState
{
public:
    float btnWidth = 0.4f; 
    float btnHeight = 0.4f;


    float btnLeft = -btnWidth / 2.0f;
    float btnRight = btnWidth / 2.0f;
    float btnBottom = -btnHeight / 2.0f;
    float btnTop = btnHeight / 2.0f;

    unsigned int backgroundTexture = 0;

    unsigned int buttonTexture = 0;

    MenuState(GameContext& ctx, StateManager& manager);

    void update(GLFWwindow* window, float dt) override;
    void render() override;
};

