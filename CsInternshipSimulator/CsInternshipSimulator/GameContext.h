// GameContext.h
#pragma once

struct GameContext
{
    // Shaders
    unsigned int rectShader = 0;
    unsigned int barShader = 0;

    // Geometry
    unsigned int VAOpatty = 0;
    unsigned int VAOrect = 0;
    unsigned int VAObar = 0;
    unsigned int VAOpic = 0;
    // add others: VAOrect, VBOs… if shared
public:
    int screenWidth = 800;
    int screenHeight = 800;
    int NUM_INGREDIENTS = 10;

};
