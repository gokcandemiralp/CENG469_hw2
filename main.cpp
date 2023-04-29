#include "main.h"

string cubeMapDirs[6] ={
    "textures/right.png",
    "textures/left.png",
    "textures/top.png",
    "textures/bottom.png",
    "textures/front.png",
    "textures/back.png"
};
Scene scene;

Sprite skyBoxSprite(&scene,"objects/cube.obj",cubeMapDirs);;
Sprite groundSprite = Sprite(&scene,"objects/ground.obj",
                             "textures/water.jpeg");
Sprite characterSprite = Sprite(&scene,"objects/Yatch_ps.obj",
                              "textures/Yatch_DIF.png");
Sprite buoySprite = Sprite(&scene, "objects/buoy_ps.obj",
                              "textures/buoy.png");

int gWidth = 800, gHeight = 450;
glm::vec3 eyePos   = glm::vec3(0.0f, 0.0f,  0.0f);
glm::vec3 eyeFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 eyeUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;


float mouseLastX=gWidth/2;
float mouseLastY=gHeight/2;
const float sensitivity = 0.1f;
float yaw = -90.0f;
float pitch = -30.0f;

void display(){
    scene.lookAt(eyePos, eyeFront, eyeUp);
    skyBoxSprite.renderCubeMap();
    groundSprite.render(300.0f, glm::vec3(0.0f,0.0f,0.0f));
    characterSprite.render(3.0f, glm::vec3(0.0f,0.0f,0.0f));
    buoySprite.render(0.05f, glm::vec3(10.0f,-0.9f,10.0f));
}

void movementKeys(GLFWwindow* window){
    int sign = 1;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        scene.eyeSpeedCoefficientZ = max(-1.0f,scene.eyeSpeedCoefficientZ - (float)deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        scene.eyeSpeedCoefficientZ = min(1.0f,scene.eyeSpeedCoefficientZ + (float)deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        scene.eyeSpeedCoefficientR = scene.eyeSpeedCoefficientR - (float)deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        scene.eyeSpeedCoefficientR = scene.eyeSpeedCoefficientR + (float)deltaTime;
    }
    (scene.eyeSpeedCoefficientZ > 0) ? sign = -1 : sign = 1;
    scene.movementOffset += glm::vec3(glm::sin(glm::radians(scene.vehicleAngle)),0.0f,-glm::cos(glm::radians(scene.vehicleAngle))) * scene.eyeSpeedCoefficientZ * (0.1f);
    scene.vehicleAngle += scene.eyeSpeedCoefficientR * sign * (0.5f);
    scene.eyeSpeedCoefficientZ /= 1.01;
    scene.eyeSpeedCoefficientR /= 1.01;
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if(key == GLFW_KEY_L && action == GLFW_PRESS){
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else if(key == GLFW_KEY_O && action == GLFW_PRESS){
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
}

void mouse(GLFWwindow* window, double xpos, double ypos){
    float xoffset = xpos - mouseLastX;
    float yoffset = mouseLastY - ypos; // reversed since y-coordinates range from bottom to top
    mouseLastX = xpos;
    mouseLastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw   += xoffset;
    pitch += yoffset;
    
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    eyeFront = glm::normalize(direction);
}

void calculateFrameTime(){
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void cleanBuffers(){
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void mainLoop(GLFWwindow* window){
    while (!glfwWindowShouldClose(window)){
        movementKeys(window);
        calculateFrameTime();
        cleanBuffers();
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void init(){
    glEnable(GL_DEPTH_TEST);
    scene.initWindowShape();

    skyBoxSprite.initShader("shaders/skyboxVert.glsl","shaders/skyboxFrag.glsl");
    groundSprite.initShader("shaders/groundVert.glsl","shaders/groundFrag.glsl");
    buoySprite.initShader("shaders/statueVert.glsl","shaders/statueFrag.glsl");
    characterSprite.initShader("shaders/statueVert.glsl","shaders/statueFrag.glsl");
    
    skyBoxSprite.initSkyBoxBuffer();
    groundSprite.initBuffer();
    buoySprite.initBuffer();
    characterSprite.initBuffer();
    characterSprite.isStatic = false;
}

int main(int argc, char** argv){
    
    scene = Scene(800, 450);
    init();
    
    glfwSetInputMode(scene.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(scene.window, keyboard);
    glfwSetCursorPosCallback(scene.window, mouse);
    
    mainLoop(scene.window); // this does not return unless the window is closed

    glfwDestroyWindow(scene.window);
    glfwTerminate();

    return 0;
}
