// GameState.h
#pragma once
#include <GL/glew.h>        // MUST be first
#include <GLFW/glfw3.h>

struct GameContext;
class StateManager;

class GameState
{
protected:
    GameContext& ctx;
    StateManager& manager;

public:

    GameState(GameContext& ctx, StateManager& manager)
        : ctx(ctx), manager(manager) {}
    virtual ~GameState() = default;

    virtual void update(GLFWwindow* window, float dt) = 0;
    virtual void render() = 0;
};
