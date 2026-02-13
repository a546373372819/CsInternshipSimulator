#pragma once
#include "model.hpp"
enum class IngredientType {
    BottomBun,
    Patty,
    Ketchup,
    Mustard,
    Pickles,
    Onion,
    Lettuce,
    Cheese,
    Tomato,
    TopBun,
    Puddle
};

struct Ingredient {
    IngredientType type;
    float x, y;
    bool active;
    bool placed;
    unsigned int texture;
};

struct Ingredient3D {
    IngredientType type;
    glm::vec3 mPos;
    glm::vec3 mScale;
    bool active;
    bool placed;
    Model model=Model("");
};