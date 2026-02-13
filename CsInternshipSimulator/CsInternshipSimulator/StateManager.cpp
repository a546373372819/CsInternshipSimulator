#include "StateManager.h"
#include "MenuState.h"
#include "CookingState.h"
#include "CookingState3D.h"
#include "AssembleState.h"
#include <iostream>
#include "AssembleState3D.h"

StateManager::StateManager(GameContext& c)
    : ctx(c), currentId(StateID::Menu)
{
    changeState(StateID::Menu);
}



void StateManager::changeState(StateID id)
{
    currentId = id;
    switch (id)
    {
    case StateID::Menu:
        std::cout << "Changed";
        current = std::make_unique<MenuState>(ctx, *this);
        break;
    case StateID::Cooking:
        std::cout << "Changed";
        current = std::make_unique<CookingState3D>(ctx, *this);
        break;
    case StateID::Assembling:
        current = std::make_unique<AssembleState3D>(ctx, *this);
        break;
    }
}

void StateManager::update(GLFWwindow* window, float dt)
{
    if (current) current->update(window, dt);
}

void StateManager::render()
{
    if (current) current->render();
}
