#pragma once
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