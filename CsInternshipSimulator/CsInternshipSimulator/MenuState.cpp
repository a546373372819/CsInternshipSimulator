#include "Util.h"
#include "MenuState.h"
#include "StateManager.h"

#include <windows.h>
#include <mmsystem.h>

#include "GameContext.h"
#include <iostream>

#pragma comment(lib, "winmm.lib")


MenuState::MenuState(GameContext& ctx, StateManager& manager)
    : GameState(ctx, manager)
{
    preprocessTexture(backgroundTexture, "text/mcdonalds.jpg");
    preprocessTexture(buttonTexture, "text/startbutton.png");
    PlaySound(TEXT("sound/intro.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

}

void MenuState::update(GLFWwindow* window, float /*dt*/)
{
    // mouse click on "Zapocni praksu"
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    bool clicked = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    float xN = static_cast<float>(xpos / ctx.screenWidth) * 2.0f - 1.0f;
    float yN = -(static_cast<float>(ypos / ctx.screenHeight) * 2.0f - 1.0f);


    if (clicked) {
        if (xN >= btnLeft && xN <= btnRight &&
            yN >= btnBottom && yN <= btnTop) {


            PlaySound(TEXT("sound/mcdonalds-beeping-sound.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
            manager.changeState(StateID::Assembling);


        }
    }
}

void MenuState::render()
{
    glUseProgram(ctx.rectShader);
    glBindVertexArray(ctx.VAOrect);

    // background
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);

    glUniform1f(glGetUniformLocation(ctx.rectShader, "uX"), 0.0f);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uY"), 0.0f);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uS"), 1.0f);
    glUniform1i(glGetUniformLocation(ctx.rectShader, "hasHat"), 0);
    glUniform1i(glGetUniformLocation(ctx.rectShader, "flipped"), 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // button
    glBindTexture(GL_TEXTURE_2D, buttonTexture);
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
}
