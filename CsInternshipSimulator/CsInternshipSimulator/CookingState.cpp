// CookingState.cpp
#define NOMINMAX
#include "CookingState.h"
#include "StateManager.h"
#include "GameContext.h"
#include <windows.h>
#include <mmsystem.h>
#include "Util.h"
#include <algorithm>
#include <iostream>

#pragma comment(lib, "winmm.lib")



CookingState::CookingState(GameContext& ctx, StateManager& manager)
    : GameState(ctx, manager),
    cookProgress(0.0f),
    isCooked(false)
{
    preprocessTexture(employeeTexture, "text/employee.png");

    preprocessTexture(cookingBackgroundTexture, "text/stove.png");
    preprocessTexture(pattyTexture, "text/raw.png");
}

void CookingState::update(GLFWwindow* window, float dt)
{
    
            // zapamti prethodnu poziciju
            float prevY = pattyY;
            float prevX = pattyX;

            // WASD movement
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pattyY += pattySpeed;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pattyY -= pattySpeed;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pattyX -= pattySpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pattyX += pattySpeed;

            //  patty limits
            float pLeft = pattyX - pattyHalfWidth;
            float pRight = pattyX + pattyHalfWidth;
           float pTop = pattyY + pattyHalfHeight;
            float pBottom = pattyY - pattyHalfHeight;

            //left right bounds
            if (pattyX > 0.8)pattyX = prevX;
            if (pattyX < -0.8)pattyX = prevX;


            bool intersectsStove = pBottom <= stoveTop;

            if (intersectsStove) {

                //if going down bounds
                if (prevY > pattyY) {
                    pattyY = stoveTop + pattyHalfHeight;
                    pBottom = stoveTop;
                }

                cookProgress += 0.004f; 

                if (cookProgress >= 1.0f) {
                    cookProgress = 1.0f;
                    isCooked = true;
                    std::cout << "Pljeskavica je ispecena!\n";
                    PlaySound(NULL, 0, 0);
                    PlaySound(TEXT("sound/restaurant.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
                    manager.changeState(StateID::Assembling);
                }
            }
        
}

void CookingState::render() {
    // 1) cooking background (vec postoji)
    glUseProgram(ctx.rectShader);
    glBindVertexArray(ctx.VAOrect);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cookingBackgroundTexture);

    glUniform1f(glGetUniformLocation(ctx.rectShader, "uCookProgress"), 0.0f);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uX"), 0.0f);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uY"), 0.0f);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uS"), 1.0f);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //employee
    glBindVertexArray(ctx.VAOpic);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, employeeTexture);

    glUniform1f(glGetUniformLocation(ctx.rectShader, "uCookProgress"), 0);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uX"), 0.4);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uY"), 0.6);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uS"),1.7f);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // 2) patty
    glBindVertexArray(ctx.VAOpatty);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pattyTexture);

    glUniform1f(glGetUniformLocation(ctx.rectShader, "uX"), pattyX);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uY"), pattyY);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uS"), 1.0f);

    float visualCook = std::min(cookProgress, 0.75f);

    glUniform1f(glGetUniformLocation(ctx.rectShader, "uCookProgress"), visualCook);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);



    //  loading bar 
    glUseProgram(ctx.barShader);
    glBindVertexArray(ctx.VAObar);

    // zajednicke granice bara
    glUniform1f(glGetUniformLocation(ctx.barShader, "uLeft"), barLeft);
    glUniform1f(glGetUniformLocation(ctx.barShader, "uRight"), barRight);
    glUniform1f(glGetUniformLocation(ctx.barShader, "uTop"), barTop);
    glUniform1f(glGetUniformLocation(ctx.barShader, "uBottom"), barBottom);

    //  sivi prazan bar 
    glUniform1f(glGetUniformLocation(ctx.barShader, "uFill"), 1.0f);
    glUniform4f(glGetUniformLocation(ctx.barShader, "uColor"), 0.1f, 0.1f, 0.1f, 0.7f);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //zeleni deo
    float t = cookProgress;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    glUniform1f(glGetUniformLocation(ctx.barShader, "uFill"), t);
    glUniform4f(glGetUniformLocation(ctx.barShader, "uColor"), 0.2f, 0.8f, 0.2f, 0.9f);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    
}
