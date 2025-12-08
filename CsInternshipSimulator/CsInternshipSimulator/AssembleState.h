#pragma once
#include "GameState.h"
#include "Ingredient.h"
#include "Puddle.h"



struct Ingredient;
struct Puddle;

class AssembleState : public GameState
{
public:
    AssembleState(GameContext& ctx, StateManager& manager);

     Ingredient gIngredients[10];
     int gCurrentIngredient = -1;
     Puddle gPuddles[50];
     int gPuddleCount = 0;

     void updateAssembling(GLFWwindow* window);
     void updatePuddles();
     void initIngredients();
     void spawnPuddleFor(const Ingredient& ing);


     // textures 
     unsigned int pattyCookedTexture = 0;
     unsigned int bottomBunTex = 0;
     unsigned int ketchupTex = 0;
     unsigned int mustardTex = 0;
     unsigned int ketchupBottleTex = 0;
     unsigned int mustardBottleTex = 0;
     unsigned int picklesTex = 0;
     unsigned int onionTex = 0;
     unsigned int lettuceTex = 0;
     unsigned int cheeseTex = 0;
     unsigned int tomatoTex = 0;
     unsigned int topBunTex = 0;
     unsigned int ketchupPuddleTex = 0;
     unsigned int mustardPuddleTex = 0;
     unsigned int tableBackgroundTexture = 0;
     unsigned int prijatnoTex = 0;


     float puddleFallSpeed = 0.02f;

     float tableSurfaceY = -0.55f;

     float puddleHalfW = 0.25f;
     float puddleHalfH = 0.08f;

     float plateLeft = -0.3f;
     float plateRight = 0.3f;
     float plateTop = -0.24f;
     float plateBottom = -0.24f;

     float tableLeft = -0.9f;
     float tableRight = 0.9f;
     float tableTop = -0.4f;  
     float tableBottom = -0.45f;   


     float ingredientHalfWidth = 0.25f;
     float ingredientHalfHeight = 0.08f;
     float ingredientSpeed = 0.02f;

    void update(GLFWwindow* window, float dt) override;
    void render() override;
};
