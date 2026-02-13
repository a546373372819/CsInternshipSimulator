#pragma once
#include "model.hpp"

struct Puddle {
    float x, y;
    float halfW, halfH;
    unsigned int texture;
    bool active;
    bool falling;
};

struct Puddle3D {
    glm::vec3 mPos;
    float halfW, halfH,halfZ;
    Model model = Model("");
    bool active;
    bool falling;
};