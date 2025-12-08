#pragma once
#include <memory>
#include "GameState.h"
#include "GameContext.h"

enum class StateID {
    Menu,
    Cooking,
    Assembling
};

class StateManager
{
public:
    GameContext& ctx;

private:
    std::unique_ptr<GameState> current;
    StateID currentId;

public:
    StateManager(GameContext& ctx);


    void changeState(StateID id);

    void update(GLFWwindow* window, float dt);
    void render();
};

