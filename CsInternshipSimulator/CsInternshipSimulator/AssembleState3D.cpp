#include "AssembleState3D.h"

#include <iostream>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "GameContext.h"
#include "StateManager.h"
#include "Puddle.h"

AssembleState3D::AssembleState3D(GameContext& ctx, StateManager& manager)
    : GameState(ctx, manager)
    , mShader3D("mesh3d.vert", "mesh3d.frag")
    , mTable("text/3d/Vintage_Furniture_Coffee_Table_OBJ.obj")
    , mPlate("text/3d/plate.obj")
    , bottomBun("text/3d/bottombun.obj")
    , patty("text/3d/patty.obj")
    , lettuce("text/3d/salad.obj")
    , onion("text/3d/onion.obj")
    , pickles("text/3d/pickle.obj")
    , topBun("text/3d/topbun.obj")
    , tomato("text/3d/tomato.obj")
    , ketchupBottle("text/3d/ketchup.obj")
    , mustardBottle("text/3d/mustard.obj")
    , ketchupPuddle("text/3d/ketchuppuddle.obj")
    , mustardPuddle("text/3d/mustardpuddle.obj")
    , cheese("text/3d/cheese.obj")
{

    mRoomHalfExtent = 6.0f;     // room extends 
    mRoomHeight = 4.0f;     // y=0 floor, y=4 ceiling
    mRoomUVTiling = 6.0f;     // how many times textures repeat on each face

    // Camera + light 
    mCamPos = glm::vec3(0.0345848f, 1.05756f, 1.18858);
    mCamTarget = glm::vec3(0.028343f, 0.941237f, 0.19539);
    mLightPos = glm::vec3(-0.5f, 1.5f, 0.5f);



    // Room geometry: one quad on XZ plane

    const float t = mRoomUVTiling; // uv tiling amount

    static float roomQuad[] = {
        // pos              // normal (up)   // uv
        -1, 0, -1,          0, 1, 0,         0, 0,
         1, 0, -1,          0, 1, 0,         t, 0,
         1, 0,  1,          0, 1, 0,         t, t,

        -1, 0, -1,          0, 1, 0,         0, 0,
         1, 0,  1,          0, 1, 0,         t, t,
        -1, 0,  1,          0, 1, 0,         0, t,
    };

    glGenVertexArrays(1, &roomVAO);
    glGenBuffers(1, &roomVBO);

    glBindVertexArray(roomVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roomVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomQuad), roomQuad, GL_STATIC_DRAW);

    // layout(location=0) vec3 position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // layout(location=1) vec3 normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // layout(location=2) vec2 uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // -----------------------------
    prijatnoTex = loadImageToTexture("text/prijatno.png");
    wallTex = loadImageToTexture("text/walls.png");
    floorTex = loadImageToTexture("text/floor.png");
    ceilingTex = loadImageToTexture("text/walls.png");

    // 3D essentials
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mShader3D.use();
    mShader3D.setInt("uDiffMap1", 0);

    // cook uniform (patty)
    mShader3D.setFloat("uCookProgress", 0.0f);

    mCamFront = glm::normalize(mCamTarget - mCamPos);

    initIngredients();

    currentIg = 0;
    ingredients[currentIg].active = true;
}

void AssembleState3D::initIngredients()
{
    glm::vec3 startPos = mTablePos + glm::vec3(0.0f, 1.0f, 0.0f);

    ingredients[0] = { IngredientType::BottomBun,startPos,mIngScale , false, false, bottomBun };
    ingredients[1] = { IngredientType::Patty,startPos,mIngScale, false, false, patty };
    ingredients[2] = { IngredientType::Ketchup,startPos ,mIngScale * glm::vec3(4) , false, false, ketchupBottle };
    ingredients[3] = { IngredientType::Mustard,startPos ,mIngScale * glm::vec3(4) , false, false, mustardBottle };
    ingredients[4] = { IngredientType::Pickles,startPos ,mIngScale , false, false, pickles };
    ingredients[5] = { IngredientType::Onion,startPos ,mIngScale , false, false, onion };
    ingredients[6] = { IngredientType::Lettuce,startPos ,mIngScale , false, false, lettuce };;
    ingredients[7] = { IngredientType::Cheese,startPos ,mIngScale , false, false, cheese };;
    ingredients[8] = { IngredientType::Tomato,startPos ,mIngScale , false, false, tomato };;
    ingredients[9] = { IngredientType::TopBun,startPos ,mIngScale , false, false, topBun };;


    currentIg = -1;
}

glm::mat4 AssembleState3D::makeModelMatrix(const glm::vec3& pos, const glm::vec3& scale) const
{
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::scale(M, scale);
    return M;


}

void AssembleState3D::update(GLFWwindow* window, float dt)
{
    // -------- exit ----------
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
    if (currentIg < 10) {

        // -------- mouse look init ----------
        if (!mCamInputInit) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &mLastMouseX, &mLastMouseY);
            mCamInputInit = true;
        }

        // -------- mouse look ----------
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        double xoffset = mx - mLastMouseX;
        double yoffset = mLastMouseY - my; // reversed y

        mLastMouseX = mx;
        mLastMouseY = my;

        mYaw += float(xoffset) * mMouseSensitivity;
        mPitch += float(yoffset) * mMouseSensitivity;

        if (mPitch > 89.0f)  mPitch = 89.0f;
        if (mPitch < -89.0f) mPitch = -89.0f;

        glm::vec3 front;
        front.x = std::cos(glm::radians(mYaw)) * std::cos(glm::radians(mPitch));
        front.y = std::sin(glm::radians(mPitch));
        front.z = std::sin(glm::radians(mYaw)) * std::cos(glm::radians(mPitch));
        mCamFront = glm::normalize(front);

        // -------- arrow key movement ----------
        const float speed = mCamMoveSpeed * dt;
        glm::vec3 right = glm::normalize(glm::cross(mCamFront, mCamUp));

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    mCamPos += mCamFront * speed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  mCamPos -= mCamFront * speed;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  mCamPos -= right * speed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) mCamPos += right * speed;

        // Optional vertical movement:
        if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)   mCamPos += mCamUp * speed;
        if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) mCamPos -= mCamUp * speed;

        mCamTarget = mCamPos + mCamFront;
    }

    static bool zWasDown = false;
    bool zDown = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;
    if (zDown && !zWasDown) ctx.gDepthTestOn = !ctx.gDepthTestOn;
    zWasDown = zDown;

    static bool cWasDown = false;
    bool cDown = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    if (cDown && !cWasDown) ctx.gCullOn = !ctx.gCullOn;
    cWasDown = cDown;

    moveCurrentIngredient3D(window, dt);
    updatePuddles();
    static bool lWasDown = false;
    bool lDown = (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS);

    if (lDown && !lWasDown) {
        lightOn = !lightOn;
    }

    lWasDown = lDown;


}

static bool overlapXZ(const glm::vec3& aPos, const glm::vec3& aHalf,
    const glm::vec3& bPos, const glm::vec3& bHalf,
    float shrinkA = 1.0f, float shrinkB = 1.0f)
{
    const float ax0 = aPos.x - aHalf.x * shrinkA;
    const float ax1 = aPos.x + aHalf.x * shrinkA;
    const float az0 = aPos.z - aHalf.z * shrinkA;
    const float az1 = aPos.z + aHalf.z * shrinkA;

    const float bx0 = bPos.x - bHalf.x * shrinkB;
    const float bx1 = bPos.x + bHalf.x * shrinkB;
    const float bz0 = bPos.z - bHalf.z * shrinkB;
    const float bz1 = bPos.z + bHalf.z * shrinkB;

    const bool xOverlap = (ax1 >= bx0) && (ax0 <= bx1);
    const bool zOverlap = (az1 >= bz0) && (az0 <= bz1);
    return xOverlap && zOverlap;
}

static void clampToTableXZ(glm::vec3& p,
    float tableLeft, float tableRight,
    float tableBack, float tableFront,
    const glm::vec3& half)
{
    // table bounds in XZ
    if (p.x - half.x < tableLeft)  p.x = tableLeft + half.x;
    if (p.x + half.x > tableRight) p.x = tableRight - half.x;

    if (p.z - half.z < tableBack)  p.z = tableBack + half.z;
    if (p.z + half.z > tableFront) p.z = tableFront - half.z;

    if (p.y - 0.01 < 0.6)  p.y = 0.6 + 0.01;
}

void AssembleState3D::moveCurrentIngredient3D(GLFWwindow* window, float dt)
{
    if (currentIg < 0 || currentIg >= ctx.NUM_INGREDIENTS)
        return;

    Ingredient3D& ing = ingredients[currentIg];
    if (!ing.active || ing.placed)
        return;

    const bool isLiquidSource =
        (ing.type == IngredientType::Ketchup || ing.type == IngredientType::Mustard);

    // ---- movement (WASD moves on table in XZ) ----
    glm::vec3 prevPos = ing.mPos;
    glm::vec3 newPos = ing.mPos;

    const float move = 2.5 * dt;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) newPos.z -= move;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) newPos.z += move;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) newPos.x -= move;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) newPos.x += move;

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) newPos.y -= move; // down
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) newPos.y += move; // up

    // ---- clamp XZ to table ----
    clampToTableXZ(newPos,
        -0.6, 0.6,
        -0.3, 0.3,
        glm::vec3(0.05));



    ing.mPos = newPos;

    // ---- liquids: SPACE spawns puddle at current XZ ----
    if (isLiquidSource) {
        static bool spaceWasDown = false;
        bool spaceDown = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

        if (spaceDown && !spaceWasDown) {
#if defined(DEBUG_ASSEMBLE_LOGS)
#if defined(DEBUG_ASSEMBLE_LOGS)
#if defined(DEBUG_ASSEMBLE_LOGS)
            std::cout << "spawn";
#endif
#endif
#endif
            spawnPuddleFor3D(ing); 
        }
        spaceWasDown = spaceDown;
        return;
    }

    // ---- placement logic (crossing downward onto a support) ----
    const float prevBottomY = prevPos.y - ingredientHalfHeight;
    const float currBottomY = ing.mPos.y - ingredientHalfHeight;

    const bool movingDown = (currBottomY < prevBottomY);
    if (!movingDown)
        return;

    bool foundSupport = false;

    auto trySupportPlane = [&](const glm::vec3& supportCenter,
        float supportHalf,
        float supportTopY,
        float overlapShrink = 0.8f)
    {
        // need overlap in XZ

        if ((ing.mPos.x - ingredientHalfWidth < supportCenter.x + supportHalf) &&
            (ing.mPos.x + ingredientHalfWidth > supportCenter.x - supportHalf) &&
            (ing.mPos.z - ingredientHalfWidth < supportCenter.z + supportHalf) &&
            (ing.mPos.z + ingredientHalfWidth > supportCenter.z - supportHalf)) {

            if (prevBottomY > supportTopY && currBottomY <= supportTopY) {
                foundSupport = true;

                // snap so ingredient sits on top of support
                ing.mPos.y = supportTopY + ingredientHalfHeight;
            }

        }


    };

    // ---- plate support ----

    trySupportPlane(mPlatePos, 0.08f, mPlatePos.y - 0.07, 0.75f);
#if defined(DEBUG_ASSEMBLE_LOGS)
#if defined(DEBUG_ASSEMBLE_LOGS)
    std::cout << mPlatePos.x << "," << mPlatePos.y << "," << mPlatePos.z << "," << '\n';


#endif
#endif
    // ---- placed ingredients support ----
    if (!foundSupport) {
        for (int i = 0; i < 10; ++i) {

            const Ingredient3D& base = ingredients[i];
            if (!base.placed) continue;

            float topY = base.mPos.y;
            if (base.type == IngredientType::Puddle) {
                topY = base.mPos.y - 0.1;
            }
            else {
                topY = base.mPos.y - 0.08; // top surface of ingredient
            }
            // top surface of that ingredient:

            // support region is the base ingredient's footprint in XZ
            trySupportPlane(base.mPos, ingredientHalfWidth, topY, 0.75f);

            if (foundSupport) {
#if defined(DEBUG_ASSEMBLE_LOGS)
#if defined(DEBUG_ASSEMBLE_LOGS)
                std::cout << "ing";
#endif
#endif
                break;
            }
        }
    }
    else {
#if defined(DEBUG_ASSEMBLE_LOGS)
#if defined(DEBUG_ASSEMBLE_LOGS)
        std::cout << "plate";

#endif
#endif
    }

    if (foundSupport) {
        ing.placed = true;
        ing.active = false;

        currentIg++;
        if (currentIg < 10) {
            ingredients[currentIg].active = true;
        }
        else {
            // finished
#if defined(DEBUG_ASSEMBLE_LOGS)
#if defined(DEBUG_ASSEMBLE_LOGS)
            std::cout << "Burger assembled (3D)!\n";
#endif
#endif
        }
    }
}

void AssembleState3D::spawnPuddleFor3D(const Ingredient3D& ing)
{
    if (puddleCount >= 10)
        return;

    Puddle3D& p = puddles[puddleCount++];

    // Spawn directly under the bottle in XZ
    p.mPos = glm::vec3(
        ing.mPos.x,
        ing.mPos.y - 0.08f,   // slightly below bottle
        ing.mPos.z
    );

    // Flat puddle (lies on table)

    if (ing.type == IngredientType::Ketchup)
        p.model = ketchupPuddle;
    else
        p.model = mustardPuddle;

    p.active = true;
    p.falling = true;

    // units per second
}

static glm::mat4 MakeTRS(const glm::vec3& pos, const glm::vec3& rotDegXYZ, const glm::vec3& scale)
{
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::rotate(M, glm::radians(rotDegXYZ.x), glm::vec3(1, 0, 0));
    M = glm::rotate(M, glm::radians(rotDegXYZ.y), glm::vec3(0, 1, 0));
    M = glm::rotate(M, glm::radians(rotDegXYZ.z), glm::vec3(0, 0, 1));
    M = glm::scale(M, scale);
    return M;
}

void AssembleState3D::updatePuddles() {
    bool placedSomething = false;

    for (int i = 0; i < puddleCount; ++i) {
        Puddle3D& p = puddles[i];
        if (!p.active || !p.falling) continue;

        const float prevY = p.mPos.y;
        const float newY = p.mPos.y - 0.02;

        const float prevBottom = prevY - 0.01;
        const float currBottom = newY - 0.01;

        bool foundSupport = false;
        bool landedOnPlateOrIngredient = false;

        // helper: 2D overlap in XZ between puddle footprint and support rectangle
        auto overlapPuddleWithRectXZ = [&](float left, float right, float back, float front) -> bool {
            const float px0 = p.mPos.x - 0.05;
            const float px1 = p.mPos.x + 0.05;
            const float pz0 = p.mPos.z - 0.05;
            const float pz1 = p.mPos.z + 0.05;

            const bool xOverlap = (px1 >= left) && (px0 <= right);
            const bool zOverlap = (pz1 >= back) && (pz0 <= front);
            return xOverlap && zOverlap;
        };

        // helper: check crossing a horizontal plane "topY" from above with XZ overlap
        auto trySupportPlane = [&](float left, float right, float back, float front, float topY) {
            if (foundSupport) return;

            const bool overlapped = overlapPuddleWithRectXZ(left, right, back, front);
            const bool crossedFromAbove = (prevBottom > topY && currBottom <= topY);
#if defined(DEBUG_ASSEMBLE_LOGS)
#if defined(DEBUG_ASSEMBLE_LOGS)

            std::cout << overlapped << crossedFromAbove;

#endif
#endif
            if (overlapped && crossedFromAbove) {
                foundSupport = true;

                // snap puddle to sit on top
                p.mPos.y = topY + 0.01;
            }
        };

        //  Table surface support (always counts as support, but doesn't mark "placedSomething")
        trySupportPlane(-0.6, 0.6,
            -0.3, 0.3,
            0.6);

        // Plate support (if not already supported by table)
        if (!foundSupport) {
            const float plateLeft = mPlatePos.x - 0.08;
            const float plateRight = mPlatePos.x + 0.08;
            const float plateBack = mPlatePos.z - 0.08;
            const float plateFront = mPlatePos.z + 0.08;

            trySupportPlane(plateLeft, plateRight, plateBack, plateFront, mPlatePos.y);

            if (foundSupport) {
                placedSomething = true;
                landedOnPlateOrIngredient = true;
            }
        }

        //  Ingredient supports (if not already supported)
        if (!foundSupport) {
            for (int k = 0; k < ctx.NUM_INGREDIENTS; ++k) {
                const Ingredient3D& base = ingredients[k];
                if (!base.placed) continue;

                // shrink footprint like your 2D (0.6)

                const float left = base.mPos.x - 0.2;
                const float right = base.mPos.x + 0.2;
                const float back = base.mPos.z - 0.2;
                const float front = base.mPos.z + 0.2;

                float topY = base.mPos.y;
                if (base.type == IngredientType::Puddle) {
                    topY = base.mPos.y;
                }
                else {
                    topY = base.mPos.y + 0.07; // top surface of ingredient
                }
                trySupportPlane(left, right, back, front, topY);

                if (foundSupport) {
                    placedSomething = true;
                    landedOnPlateOrIngredient = true;
                    break;
                }
            }
        }

        if (foundSupport) {
            p.falling = false;


            if (landedOnPlateOrIngredient) {

                // Choose where to store puddles (your original uses [2] and [3])
                const bool ketchupFirstSlotFree = !ingredients[2].placed; // same as your condition

                if (ketchupFirstSlotFree) {
                    // Put ketchup puddle in slot 2
                    ingredients[2].type = IngredientType::Puddle;
                    ingredients[2].mPos = p.mPos;
                    ingredients[2].mScale = glm::vec3(0.1);
                    ingredients[2].placed = true;
                    ingredients[2].active = false;
                    ingredients[2].model = ketchupPuddle;
                }
                else {
                    // Put mustard puddle in slot 3
                    ingredients[3].type = IngredientType::Puddle;
                    ingredients[3].mPos = p.mPos;
                    ingredients[3].mScale = glm::vec3(0.1);
                    ingredients[3].placed = true;
                    ingredients[3].active = false;
                    ingredients[3].model = mustardPuddle;
                }

                currentIg++;
                if (currentIg < 10) {
                    ingredients[currentIg].active = true;
                }

                p.active = false;
            }

        }
        else {
            // keep falling
            p.mPos.y = newY;
        }
    }


}

void AssembleState3D::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int w = 1280, h = 720;
    if (GLFWwindow* window = glfwGetCurrentContext()) {
        glfwGetFramebufferSize(window, &w, &h);
    }

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), float(w) / float(h), 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(mCamPos, mCamTarget, glm::vec3(0, 1, 0));

    mShader3D.use();
    mShader3D.setMat4("view", view);
    mShader3D.setMat4("projection", projection);
    mShader3D.setVec3("lightPos", mLightPos);
    mShader3D.setVec3("viewPos", mCamPos);

    mShader3D.setFloat("uLightIntensity", lightOn ? 1.0f : 0.0f);

    if (ctx.gDepthTestOn) glEnable(GL_DEPTH_TEST);
    else              glDisable(GL_DEPTH_TEST);

    if (ctx.gCullOn) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    }
    else {
        glDisable(GL_CULL_FACE);
    }

    // Draw room (floor + ceiling + 4 walls)
    // Quad is centered at origin, XZ plane, scaled to room size and then rotated/translated.



    auto drawQuadFace = [&](GLuint tex, const glm::mat4& model) {
        mShader3D.setBool("uHasDiffuseMap", true);
        mShader3D.setFloat("uCookProgress", 0.0f);
        mShader3D.setInt("uDiffMap1", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        mShader3D.setMat4("model", model);

        glBindVertexArray(roomVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    };

    const float H = mRoomHeight;
    const float E = mRoomHalfExtent;



    // Floor (XZ)
    drawQuadFace(
        floorTex,
        MakeTRS(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 0, 0), glm::vec3(E, 1.0f, E))
    );

    // Ceiling (XZ), flip to face downward
    drawQuadFace(
        ceilingTex,
        MakeTRS(glm::vec3(0.0f, H, 0.0f), glm::vec3(180, 0, 0), glm::vec3(E, 1.0f, E))
    );

    // Back wall (XY plane at z=-E), face inward
    drawQuadFace(
        wallTex,
        MakeTRS(glm::vec3(0.0f, H * 0.5f, -E), glm::vec3(90, 0, 0), glm::vec3(E, 1.0f, H * 0.5f))
    );

    // Front wall (XY plane at z=+E), face inward
    drawQuadFace(
        wallTex,
        MakeTRS(glm::vec3(0.0f, H * 0.5f, +E), glm::vec3(-90, 0, 0), glm::vec3(E, 1.0f, H * 0.5f))
    );

    // Left wall (ZY plane at x=-E), face inward
    drawQuadFace(
        wallTex,
        MakeTRS(glm::vec3(-E, H * 0.5f, 0), glm::vec3(-90, 0, -90), glm::vec3(E, 1.0f, H))
    );

    // Right wall (ZY plane at x=+E), face inward
    drawQuadFace(
        wallTex,
        MakeTRS(glm::vec3(+E, H * 0.5f, 0), glm::vec3(90, 0, 90), glm::vec3(E, 1.0f, H))
    );

    // Ensure assemble scene doesn't apply "cooking" tint
    mShader3D.setFloat("uCookProgress", 0.0f);

    if (ctx.gCullOn) {
        glCullFace(GL_BACK);
    }
    

    // ---- draw table ----
    {
        glm::mat4 M = makeModelMatrix(mTablePos, mTableScale);
        mShader3D.setMat4("model", M);
        mTable.Draw(mShader3D);
    }

    // ---- draw plate (on top of table) ----
    {
        // Keep plate position updated (in case you tweak mTableTopY live)
        mPlatePos = glm::vec3(mTablePos.x, mTableTopY - 0.28, mTablePos.z);

        glm::mat4 M = makeModelMatrix(mPlatePos, mPlateScale);
        mShader3D.setMat4("model", M);
        mPlate.Draw(mShader3D);
    }

    //ingredients

    for (int i = 0; i < ctx.NUM_INGREDIENTS; ++i) {
        Ingredient3D& ing = ingredients[i];

        if (!ing.placed && !ing.active)
            continue;

        //ako su flase
        {
            glm::mat4 M = makeModelMatrix(ing.mPos, ing.mScale);
            mShader3D.setMat4("model", M);
            ing.model.Draw(mShader3D);
        }
    }
    //puddles

    for (int i = 0; i < puddleCount; ++i) {

        Puddle3D& p = puddles[i];

        if (!p.active)continue;

        {
            glm::mat4 M = makeModelMatrix(p.mPos, glm::vec3(0.1f));
            mShader3D.setMat4("model", M);
            p.model.Draw(mShader3D);
        }

    }


    // Cache rectShader uniform locations (avoid glGetUniformLocation every frame)
    static bool sRectCached = false;
    static GLint r_uCookProgress = -1, r_uX = -1, r_uY = -1, r_uS = -1;
    if (!sRectCached) {
        r_uCookProgress = glGetUniformLocation(ctx.rectShader, "uCookProgress");
        r_uX = glGetUniformLocation(ctx.rectShader, "uX");
        r_uY = glGetUniformLocation(ctx.rectShader, "uY");
        r_uS = glGetUniformLocation(ctx.rectShader, "uS");
        sRectCached = true;
    }

    if (currentIg >= ctx.NUM_INGREDIENTS) {

        glUseProgram(ctx.rectShader);

        glBindVertexArray(ctx.VAOpatty);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, prijatnoTex);

        glUniform1f(r_uCookProgress, 0.0f);
        glUniform1f(r_uX, 0.0f);
        glUniform1f(r_uY, 0.6f);
        glUniform1f(r_uS, 4.0f);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}
