// AssemblingState.cpp
#include "AssembleState.h"
#include "StateManager.h"
#include "GameContext.h"
#include "AssembleState.h"
#include <GL/glew.h>
#include "Util.h"
#include <iostream>

#include <windows.h>
#include <mmsystem.h>



AssembleState::AssembleState(GameContext& ctx, StateManager& manager)
    : GameState(ctx, manager)
{
    preprocessTexture(tableBackgroundTexture, "text/table.png");
    preprocessTexture(bottomBunTex, "text/lowerbun.png");
    preprocessTexture(ketchupBottleTex, "text/ketchupbottle.png");
    preprocessTexture(mustardBottleTex, "text/mustardbottle.png");
    preprocessTexture(picklesTex, "text/pickles.png");
    preprocessTexture(onionTex, "text/onion.png");
    preprocessTexture(lettuceTex, "text/lettuce.png");
    preprocessTexture(cheeseTex, "text/cheese.png");
    preprocessTexture(tomatoTex, "text/tomatoes.png");
    preprocessTexture(topBunTex, "text/topbun.png");
    preprocessTexture(pattyCookedTexture, "text/cooked.png");
    preprocessTexture(prijatnoTex, "text/prijatno.png");



    preprocessTexture(ketchupPuddleTex, "text/ketchup.png");
    preprocessTexture(mustardPuddleTex, "text/mustard.png");

    initIngredients();

    for (int i = 0; i < ctx.NUM_INGREDIENTS; ++i) {
                        gIngredients[i].placed = false;
                        gIngredients[i].active = false;
                    }

                    gCurrentIngredient = 0;
                    gIngredients[gCurrentIngredient].active = true;
}

void AssembleState::update(GLFWwindow* window, float /*dt*/)
{
    updatePuddles();
    updateAssembling(window);
}

void AssembleState::render()
{
    glUseProgram(ctx.rectShader);

    // 1) background (table)
    glBindVertexArray(ctx.VAOrect);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tableBackgroundTexture);

    glUniform1f(glGetUniformLocation(ctx.rectShader, "uCookProgress"), 0.0f);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uX"), 0.0f);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uY"), 0.0f);
    glUniform1f(glGetUniformLocation(ctx.rectShader, "uS"), 1.0f);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // 2) ingredients (all placed + current active)
 // reuse patty quad for all ingredients

    for (int i = 0; i < ctx.NUM_INGREDIENTS; ++i) {
        glBindVertexArray(ctx.VAOpatty);
        const Ingredient& ing = gIngredients[i];

        if (!ing.placed && !ing.active)
            continue; // not yet spawned or forgotten

        if (ing.type == IngredientType::Ketchup ||
            ing.type == IngredientType::Mustard) {

            glBindVertexArray(ctx.VAOpic);
            glUniform1f(glGetUniformLocation(ctx.rectShader, "uS"), 2.0f);

        }
        else {
            glUniform1f(glGetUniformLocation(ctx.rectShader, "uS"), 1.0f);

        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ing.texture);




        glUniform1f(glGetUniformLocation(ctx.rectShader, "uCookProgress"), 0);
        glUniform1f(glGetUniformLocation(ctx.rectShader, "uX"), ing.x);
        glUniform1f(glGetUniformLocation(ctx.rectShader, "uY"), ing.y);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    glBindVertexArray(ctx.VAOpatty); // isti quad koristimo za sve
    for (int i = 0; i < gPuddleCount; ++i) {
        const Puddle& p = gPuddles[i];
        if (!p.active)continue;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, p.texture);

        glUniform1f(glGetUniformLocation(ctx.rectShader, "uCookProgress"), 0.0f);
        glUniform1f(glGetUniformLocation(ctx.rectShader, "uX"), p.x);
        glUniform1f(glGetUniformLocation(ctx.rectShader, "uY"), p.y);
        glUniform1f(glGetUniformLocation(ctx.rectShader, "uS"), 1.0f);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    if (gCurrentIngredient >= ctx.NUM_INGREDIENTS) {

                glBindVertexArray(ctx.VAOpatty); 

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, prijatnoTex);

                glUniform1f(glGetUniformLocation(ctx.rectShader, "uCookProgress"), 0);
                glUniform1f(glGetUniformLocation(ctx.rectShader, "uX"), 0.0);
                glUniform1f(glGetUniformLocation(ctx.rectShader, "uY"), 0.6);
                glUniform1f(glGetUniformLocation(ctx.rectShader, "uS"), 4.0f);

                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            }
}

void AssembleState::initIngredients()
{
    // bottom bun
    gIngredients[0] = { IngredientType::BottomBun, 0.0f, 0.8f, false, false, bottomBunTex };
    // patty
    gIngredients[1] = { IngredientType::Patty, 0.0f, 0.8f, false, false, pattyCookedTexture };
    // ketchup
    gIngredients[2] = { IngredientType::Ketchup, -0.6f, 0.8f, false, false, ketchupBottleTex };
    // mustard
    gIngredients[3] = { IngredientType::Mustard, -0.2f, 0.8f, false, false, mustardBottleTex };
    // pickles
    gIngredients[4] = { IngredientType::Pickles, 0.2f, 0.8f, false, false, picklesTex };
    // onion
    gIngredients[5] = { IngredientType::Onion, 0.6f, 0.8f, false, false, onionTex };
    // lettuce
    gIngredients[6] = { IngredientType::Lettuce, -0.6f, 0.6f, false, false, lettuceTex };
    // cheese
    gIngredients[7] = { IngredientType::Cheese, -0.2f, 0.6f, false, false, cheeseTex };
    // tomato
    gIngredients[8] = { IngredientType::Tomato, 0.2f, 0.6f, false, false, tomatoTex };
    // top bun
    gIngredients[9] = { IngredientType::TopBun, 0.6f, 0.6f, false, false, topBunTex };


    gCurrentIngredient = -1;
}

void AssembleState::spawnPuddleFor(const Ingredient& ing) {

    Puddle& p = gPuddles[gPuddleCount++];
    p.x = ing.x;
    // kree malo ispod flaice
    p.y = ing.y - 0.1; // ako nema collHalfH, koristi halfH

    p.halfW = puddleHalfW;
    p.halfH = puddleHalfH;

    if (ing.type == IngredientType::Ketchup)
        p.texture = ketchupPuddleTex;
    else
        p.texture = mustardPuddleTex;

    p.active = true;
    p.falling = true;
}

void AssembleState::updateAssembling(GLFWwindow* window)
{
    if (gCurrentIngredient < 0 || gCurrentIngredient >= 10)
        return;

    Ingredient& ing = gIngredients[gCurrentIngredient];
    if (!ing.active || ing.placed)
        return;

    bool isLiquidSource = (ing.type == IngredientType::Ketchup ||
        ing.type == IngredientType::Mustard);

    float halfW = ingredientHalfWidth;
    float halfH = ingredientHalfHeight;

    // 1) Zapamti prethodnu poziciju
    float prevX = ing.x;
    float prevY = ing.y;

    // 2) Predlozi novu poziciju na osnovu WASD
    float newX = ing.x;
    float newY = ing.y;



    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) newY += ingredientSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) newY -= ingredientSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) newX -= ingredientSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) newX += ingredientSpeed;



    // 3) Invisible walls – clamp kretanje na sto
    if (newX - halfW < tableLeft)   newX = tableLeft + halfW;
    if (newX + halfW > tableRight)  newX = tableRight - halfW;
    if (newY - halfH <= tableTop)    newY = tableTop + halfH;


    ing.x = newX;
    ing.y = newY;

    if (isLiquidSource) {
        // edge-detect SPACE, da ne puca deset puta u jednom dranju
        static bool spaceWasDown = false;
        int spaceState = glfwGetKey(window, GLFW_KEY_SPACE);

        if (spaceState == GLFW_PRESS && !spaceWasDown) {
            // kreiraj puddle
            spawnPuddleFor(ing);

            // ova flaica je "potroena"


        }

        spaceWasDown = (spaceState == GLFW_PRESS);

        // veoma bitno: za ketchup/mustard NE radimo jo nita dalje
        return;
    }

    // 4) Raunamo donje ivice pre i posle pomeranja
    float prevBottom = prevY - halfH;
    float currBottom = ing.y - halfH;

    // Ako se nismo pomerali nadole, nema "sputanja"
    bool movingDown = (currBottom < prevBottom);

    if (!movingDown)
        return;

    // 5) Tražimo NAJVIU podlogu ispod sastojka koju smo upravo dodirnuli
    bool  foundSupport = false;

    // Helper lambda za proveru kandidata
    auto trySupport = [&](float left, float right, float top) {
        //// mora horizontalno da pokriva centar sastojka
        float cx = ing.x;
        bool horizontalOverlap = (cx >= left && cx <= right);

        //// uslov: ranije je donja ivica bila iznad, sada je ispod ili na topu
        bool crossedFromAbove = (prevBottom > top && currBottom <= top);

        if (horizontalOverlap && crossedFromAbove) {
            foundSupport = true;
        }
    };



    // 5a) Tanjir kao potencijalna podloga
    trySupport(plateLeft, plateRight, plateTop);

    // 5b) Svaki ve postavljen ingredient kao podloga
    for (int i = 0; i < ctx.NUM_INGREDIENTS; ++i) {
        const Ingredient& base = gIngredients[i];
        if (!base.placed) continue;

        float left = base.x - halfW * 0.6;
        float right = base.x + halfW * 0.6;
        float top = base.y - halfH * 0.5;  // gornja ivica tog sastojka

        trySupport(left, right, top);
    }

    // 6) Ako smo nali podlogu koju smo upravo dotakli – "spusti" sastojak
    if (foundSupport) {
        // donja ivica sastojka tano na podlozi
        ing.placed = true;
        ing.active = false;

        // Aktiviraj sledei ingredient
        gCurrentIngredient++;
        if (gCurrentIngredient < ctx.NUM_INGREDIENTS) {
            gIngredients[gCurrentIngredient].active = true;
        }
        else {
            PlaySound(NULL, 0, 0);

            PlaySound(TEXT("sound/fairy-dust.wav"), NULL, SND_FILENAME | SND_ASYNC);

            std::cout << "Burger assembled!\n";
            // ovde kasnije: PRIJATNO state, ESC exit itd.
        }
    }
}

void AssembleState::updatePuddles()
{
    bool placed = false;

    for (int i = 0; i < gPuddleCount; ++i) {
        Puddle& p = gPuddles[i];
        if (!p.active || !p.falling) continue;

        float prevY = p.y;
        float newY = p.y - puddleFallSpeed;

        float prevBottom = prevY - p.halfH;
        float currBottom = newY - p.halfH;

        // traimo NAJBLIU povrinu ispod, koju smo upravo presekli
        float bestTop = -2.0f;
        bool foundSupport = false;

        float cx = p.x;

        auto trySupport = [&](float left, float right, float top) {
            //// mora horizontalno da pokriva centar sastojka
            bool horizontalOverlap = (cx >= left && cx <= right);

            //// uslov: ranije je donja ivica bila iznad, sada je ispod ili na topu
            bool crossedFromAbove = (prevBottom > top && currBottom <= top);

            if (horizontalOverlap && crossedFromAbove) {
                foundSupport = true;
            }
        };

        // 1) povrina stola
        trySupport(tableLeft, tableRight, tableSurfaceY);

        // 2) tanjir
        if (!foundSupport) {
            trySupport(plateLeft, plateRight, plateTop);
            if (foundSupport) placed = true;
        }

        // 5b) Svaki ve postavljen ingredient kao podloga
        if (!foundSupport) {
            for (int i = 0; i < ctx.NUM_INGREDIENTS; ++i) {
                const Ingredient& base = gIngredients[i];
                if (!base.placed) continue;

                float left = base.x - ingredientHalfWidth * 0.6;
                float right = base.x + ingredientHalfWidth * 0.6;
                float top = base.y - ingredientHalfHeight * 0.5;  // gornja ivica tog sastojka

                trySupport(left, right, top);
            }
            if (foundSupport) placed = true;
        }

        if (foundSupport) {

            p.falling = false; // ostaje tu

            if (placed) {
                if (!gIngredients[2].placed) {
                    std::cout << "a";

                    gIngredients[2] = { IngredientType::Puddle, p.x, p.y, false, true, ketchupPuddleTex };


                }
                else {
                    std::cout << "b";

                    gIngredients[3] = { IngredientType::Puddle, p.x, p.y, false, true, mustardPuddleTex };

                }
                gCurrentIngredient++;
                gIngredients[gCurrentIngredient].active = true;
                p.active = false;


            }


        }
        else {
            // nema sudara – nastavlja da pada
            p.y = newY;
        }
    }
}
