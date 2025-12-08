#pragma once
struct Puddle {
    float x, y;
    float halfW, halfH;
    unsigned int texture;
    bool active;
    bool falling;
};