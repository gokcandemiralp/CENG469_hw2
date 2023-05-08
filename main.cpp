#include "main.h"

bool isWireframe = false;
string cubeMapDirs[6] ={
    "textures/right.png",
    "textures/left.png",
    "textures/top.png",
    "textures/bottom.png",
    "textures/front.png",
    "textures/back.png"
};
Scene scene = Scene(800, 450);

Sprite skyBoxSprite(&scene,"objects/cube.obj",cubeMapDirs);;
Sprite groundSprite = Sprite(&scene,"objects/ground.obj",
                             "textures/water.jpeg");
Sprite buoySprite = Sprite(&scene, "objects/buoy_ps.obj",
                              "textures/buoy.png");
Sprite vehicleSprite = Sprite(&scene,"objects/Yatch_ps.obj",
                              "textures/Yatch_DIF.png");

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if(key == GLFW_KEY_F && action == GLFW_PRESS){
        scene.staticMouse = !scene.staticMouse;
        if(scene.staticMouse){
            scene.eyeFront = glm::normalize(scene.calculateDirection(-90.0f,-5.0f));
        }
    }
    else if(key == GLFW_KEY_Q && action == GLFW_PRESS){
        scene.eyePos = glm::vec3(-12.0f, 4.0f,  0.0f);
        scene.eyeFront = glm::normalize(scene.calculateDirection(0.0f,-5.0f));
    }
    else if(key == GLFW_KEY_E && action == GLFW_PRESS){
        scene.eyePos = glm::vec3(12.0f, 4.0f,  0.0f);
        scene.eyeFront = glm::normalize(scene.calculateDirection(180.0f,-5.0f));
    }
    else if(key == GLFW_KEY_R && action == GLFW_PRESS){
        scene.eyePos = glm::vec3(0.0f, 4.0f,  12.0f);
        scene.eyeFront = glm::normalize(scene.calculateDirection(-90.0f,-5.0f));
    }
    else if(key == GLFW_KEY_T && action == GLFW_PRESS){
        scene.eyePos = glm::vec3(0.0f, 4.0f,  -12.0f);
        scene.eyeFront = glm::normalize(scene.calculateDirection(90.0f,-5.0f));
    }
    else if(key == GLFW_KEY_L && action == GLFW_PRESS){
        isWireframe = !isWireframe;
        if(isWireframe){glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );}
        else{glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );}
    }
}

void mouse(GLFWwindow* window, double xpos, double ypos){
    if(scene.staticMouse){
        return;
    }
    
    float xoffset = xpos - scene.mouseLastX;
    float yoffset = scene.mouseLastY - ypos; // reversed since y-coordinates range from bottom to top
    scene.mouseLastX = xpos;
    scene.mouseLastY = ypos;

    xoffset *= scene.sensitivity;
    yoffset *= scene.sensitivity;
    scene.yaw   += xoffset;
    scene.pitch += yoffset;
    
    if(scene.pitch > 89.0f)
        scene.pitch = 89.0f;
    if(scene.pitch < -89.0f)
        scene.pitch = -89.0f;
}

void cleanBuffers(){
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void init(){
    glEnable(GL_DEPTH_TEST);
    scene.initWindowShape();
    
    vehicleSprite.isVehicle = true;
    vehicleSprite.initReflection();
    
    skyBoxSprite.initShader("shaders/skyboxVert.glsl","shaders/skyboxFrag.glsl");
    groundSprite.initShader("shaders/groundVert.glsl","shaders/groundFrag.glsl");
    buoySprite.initShader("shaders/statueVert.glsl","shaders/statueFrag.glsl");
    vehicleSprite.initShader("shaders/vehicleVert.glsl","shaders/vehicleFrag.glsl");
    
    skyBoxSprite.initSkyBoxBuffer();
    groundSprite.initBuffer(600.0f, glm::vec3(0.0f,0.0f,0.0f));
    buoySprite.initBuffer(0.05f, glm::vec3(10.0f,-0.9f,10.0f));
    vehicleSprite.initBuffer(3.0f, glm::vec3(0.0f,0.0f,0.0f));
    
}

void display(){
    scene.eyeFront = glm::normalize(scene.calculateDirection(scene.yaw,scene.pitch));
    scene.lookAt();
    scene.sprites[3]->reflect();
    scene.sprites[0]->renderCubeMap();
    scene.sprites[1]->render();
    scene.sprites[2]->render();
    scene.sprites[3]->render();
}


void mainLoop(GLFWwindow* window){
    while (!glfwWindowShouldClose(window)){
        scene.movementKeys(window);
        scene.calculateFrameTime();
        cleanBuffers();
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv){
    
    init();
    
    glfwSetInputMode(scene.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(scene.window, keyboard);
    glfwSetCursorPosCallback(scene.window, mouse);
    
    mainLoop(scene.window); // this does not return unless the window is closed

    glfwDestroyWindow(scene.window);
    glfwTerminate();

    return 0;
}
