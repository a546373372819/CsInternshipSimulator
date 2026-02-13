#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>

#include <iostream>
#include "Util.h"
#include "GameContext.h"
#include "StateManager.h"





// screen
int screenWidth = 800;
int screenHeight = 800;


// cursors
GLFWcursor* cursor = nullptr;
GLFWcursor* cursorPressed = nullptr;

// shader and VAOs
unsigned int rectShader = 0;
unsigned int VAOrect = 0;
unsigned int VBOrect = 0;

unsigned int VAOpatty = 0;
unsigned int VBOpatty = 0;

unsigned int VAOpic = 0;
unsigned int VBOpic = 0;

unsigned int barShader = 0;
unsigned int VAObar = 0;
unsigned int VBObar = 0;

// button bounds 
float btnLeft = 0.0f;
float btnRight = 0.0f;
float btnTop = 0.0f;
float btnBottom = 0.0f;

// patty data
float pattyX = 0.0f;
float pattyY = -0.1f;
float pattyHalfWidth = 0.07f;
float pattyHalfHeight = 0.07f;

// pic data
float picHalfWidth = 0.05f;
float picHalfHeight = 0.1f;

unsigned int indexTex = 0;

int endProgram(const char* message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

static void printCWD()
{
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    std::cout << "CWD = " << buf << "\n";
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }


}

// -------------------------------------------------------
// Geometry init
// -------------------------------------------------------

void initRectGeometry() {

    // Fullscreen background quad + centered button quad in one VBO

    float bgLeft = -1.0f;
    float bgRight = 1.0f;
    float bgTop = 1.0f;
    float bgBottom = -1.0f;

    float btnWidth = 0.4f; 
    float btnHeight = 0.4f;

    btnLeft = -btnWidth / 2.0f;
    btnRight = btnWidth / 2.0f;
    btnBottom = -btnHeight / 2.0f;
    btnTop = btnHeight / 2.0f;

    float verticesRect[] = {
        bgLeft,  bgTop,    0.0f, 1.0f,
        bgLeft,  bgBottom, 0.0f, 0.0f,
        bgRight, bgBottom, 1.0f, 0.0f,
        bgRight, bgTop,    1.0f, 1.0f,

        btnLeft,  btnTop,    0.0f, 1.0f,
        btnLeft,  btnBottom, 0.0f, 0.0f,
        btnRight, btnBottom, 1.0f, 0.0f,
        btnRight, btnTop,    1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAOrect);
    glGenBuffers(1, &VBOrect);

    glBindVertexArray(VAOrect);
    glBindBuffer(GL_ARRAY_BUFFER, VBOrect);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRect), verticesRect, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void initPattyGeometry() {
    float verticesPatty[] = {
        -pattyHalfWidth,  pattyHalfHeight,  0.0f, 1.0f,
        -pattyHalfWidth, -pattyHalfHeight,  0.0f, 0.0f,
         pattyHalfWidth, -pattyHalfHeight,  1.0f, 0.0f,
         pattyHalfWidth,  pattyHalfHeight,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAOpatty);
    glGenBuffers(1, &VBOpatty);

    glBindVertexArray(VAOpatty);
    glBindBuffer(GL_ARRAY_BUFFER, VBOpatty);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPatty), verticesPatty, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void initBarGeometry() {

    float verticesBar[] = {
        0.0f, 1.0f,  
        0.0f, 0.0f,  
        1.0f, 0.0f,  
        1.0f, 1.0f   
    };

    glGenVertexArrays(1, &VAObar);
    glGenBuffers(1, &VBObar);

    glBindVertexArray(VAObar);
    glBindBuffer(GL_ARRAY_BUFFER, VBObar);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesBar), verticesBar, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void initPictureGeometry() {
    
    float verticesPicture[] = {
       -picHalfWidth,  picHalfHeight,  0.0f, 1.0f,
        -picHalfWidth, -picHalfHeight,  0.0f, 0.0f,
         picHalfWidth, -picHalfHeight,  1.0f, 0.0f,
         picHalfWidth,  picHalfHeight,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAOpic);
    glGenBuffers(1, &VBOpic);

    glBindVertexArray(VAOpic);
    glBindBuffer(GL_ARRAY_BUFFER, VBOpic);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPicture), verticesPicture, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}



int main() {
    printCWD();
    if (!glfwInit())
        return endProgram("GLFW nije uspeo da se inicijalizuje.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // fullscreen
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    screenWidth = mode->width;
    screenHeight = mode->height;

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight,
        "CsInternshipSimulator", monitor, nullptr);
    if (!window) return endProgram("Prozor nije uspeo da se kreira.");

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    //glfwSetMouseButtonCallback(window, mouse_button_callback);

    cursor = loadImageToCursor("text/spatulacursor.png");
    if (cursor) glfwSetCursor(window, cursor);

    if (glewInit() != GLEW_OK)
        return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // shader
    barShader = createShader("bar.vert", "bar.frag");
    rectShader = createShader("rect.vert", "rect.frag");
    glUseProgram(rectShader);
    glUniform1i(glGetUniformLocation(rectShader, "uTex0"), 0);
    glUniform1i(glGetUniformLocation(rectShader, "uTex1"), 1);

    //tex
    preprocessTexture(indexTex, "text/index.png");


    // geometry
    initRectGeometry();
    initPattyGeometry();
    initBarGeometry();
    initPictureGeometry();


    //fps
    const double TARGET_FPS = 75.0;
    const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;
    double lastTime = glfwGetTime();

    //game context set
    GameContext ctx;
    StateManager manager(ctx);

    ctx.rectShader = rectShader;
    ctx.barShader = barShader;

    ctx.VAOrect = VAOrect;
    ctx.VAObar = VAObar;
    ctx.VAOpatty = VAOpatty;
    ctx.VAOpic = VAOpic;

    ctx.screenHeight = screenHeight;
    ctx.screenWidth = screenWidth;



    while (!glfwWindowShouldClose(window)) {

        
        

        double now = glfwGetTime();
        double dt = now - lastTime;
        lastTime = now;

        

        manager.update(window,dt);
        glClear(GL_COLOR_BUFFER_BIT);
        manager.render();


        //drawing index
        glUseProgram(rectShader);
        glBindVertexArray(VAOpatty); 

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, indexTex);

        glUniform1f(glGetUniformLocation(rectShader, "uCookProgress"), 0);
        glUniform1f(glGetUniformLocation(rectShader, "uX"), 0.8);
       glUniform1f(glGetUniformLocation(rectShader, "uY"), 0.8);
        glUniform1f(glGetUniformLocation(rectShader, "uS"), 2.0f);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);



        glfwSwapBuffers(window);
        glfwPollEvents();

        // frame limiter
        double frameEnd = glfwGetTime();
        double frameTime = frameEnd - now;
        if (frameTime < TARGET_FRAME_TIME) {
            while (glfwGetTime() - now < TARGET_FRAME_TIME) {
                // busy wait; could use sleep_for 
            }
        }
    }

    glDeleteProgram(rectShader);
    glDeleteVertexArrays(1, &VAOrect);
    glDeleteBuffers(1, &VBOrect);
    glDeleteVertexArrays(1, &VAOpatty);
    glDeleteBuffers(1, &VBOpatty);
    glDeleteVertexArrays(1, &VAObar);
    glDeleteBuffers(1, &VBObar);
    glDeleteVertexArrays(1, &VAOpic);
    glDeleteBuffers(1, &VBOpic);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
