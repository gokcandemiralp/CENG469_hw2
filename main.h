#ifndef MAIN_H
#define MAIN_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/quaternion.hpp>

using namespace std;
using namespace tinyobj;

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
          string& data){     ///< [out] The contents of the file
    fstream myfile;

    myfile.open(fileName.c_str(), std::ios::in); // Open the input

    if (myfile.is_open()){
        string curLine;

        while (getline(myfile, curLine)){
            data += curLine;
            if (!myfile.eof()){
                data += "\n";
            }
        }

        myfile.close();
    }
    else{
        return false;
    }
    return true;
}


GLuint createVS(const char* shaderName){
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource)){
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = (GLint)shaderSource.length();
    const GLchar* shader = (const GLchar*)shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = { 0 };
    glGetShaderInfoLog(vs, 1024, &length, output);
    if(strcmp(output,"")){printf("VS compile log: %s\n", output);}

    return vs;
}

GLuint createFS(const char* shaderName){
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource)){
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = (GLint)shaderSource.length();
    const GLchar* shader = (const GLchar*)shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = { 0 };
    glGetShaderInfoLog(fs, 1024, &length, output);
    if(strcmp(output,"")){printf("FS compile log: %s\n", output);}

    return fs;
}

class Sprite;

class Scene{
    public:
    
    float mouseLastX,mouseLastY;
    float deltaTime,lastFrame;
    float sensitivity;
    float yaw,pitch;
    bool staticMouse;
    
    GLuint UBO;
    GLFWwindow* window;
    int gWidth, gHeight;
    glm::vec3 movementOffset;
    float eyeSpeedCoefficientZ;
    float eyeSpeedCoefficientR;
    float vehicleAngle;
    glm::vec3 eyePos;
    glm::vec3 eyeFront;
    glm::vec3 eyeUp;
    
    int spriteCount;
    vector<Sprite*> sprites;
    
    Scene();
    Scene(int inputWidth, int inputHeight);
    void renderWithoutVehicle();
    glm::vec3 calculateDirection(float inputYaw, float inputPitch);
    void calculateFrameTime();
    void movementKeys(GLFWwindow* window);
    void initWindowShape();
    void lookAt();
};

class Sprite{
    public:
    
    Scene *scene;
    int spriteId;
    
    string objDir;
    string texDir;
    string cubeMapDirs[6];
    GLuint VAO, VBO, EBO, FBO, RBO;
    GLuint gProgram, textureID, box2CubeMap;
    GLuint vertexDataSize, normalDataSize, indexDataSize, texCoordDataSize;
    GLuint vertexEntries, texCoordEntries, faceEntries;
    fastObjMesh* model;
    
    GLfloat* vertexData;
    GLfloat* normalData;
    GLfloat* texCoordData;
    GLuint* indexData;
    
    bool isVehicle;
    float scaleFactor;
    glm::vec3 positionOffset;
    glm::mat4 refViewingMatrices[6];
    
    bool doesShake;
    float shakeAngle;
    
    Sprite();
    Sprite(Scene *inputScene, string inputObjDir, string inputTexDir);
    Sprite(Scene *inputScene, string inputObjDir, string inputCubeTexDirs[6]);
    bool writeVertexNormal(GLfloat* normalData, int vertexIndex, int normalIndex);
    void initRefViewingMatrices();
    bool writeVertexTexCoord(GLfloat* texCoordData, int vertexIndex, int texCoordIndex);
    void addFaceElements(int index);
    void initShader(string vertDir, string fragDir);
    void initReflection();
    void initBuffer(float scaleFactorInput, glm::vec3 positionOffsetInput);
    void initSkyBoxBuffer();
    glm::mat4 shake();
    void reflect();
    void render();
    void renderVariation(glm::vec3 positionOffsetInput);
    void renderCubeMap();
};

Scene::Scene(){;}

Scene::Scene(int inputWidth, int inputHeight){
    gWidth = inputWidth;
    gHeight = inputHeight;
    spriteCount = 0;
    mouseLastX=gWidth/2;
    mouseLastY=gHeight/2;
    deltaTime = 0.0f;
    lastFrame = 0.0f;
    sensitivity = 0.1;
    yaw = -90.0f;
    pitch = -5.0f;
    staticMouse = true;
    
    eyeSpeedCoefficientZ = 0.0f;
    eyeSpeedCoefficientR = 0.0f;
    movementOffset = glm::vec3(0.0f,0.0f,0.0f); // initial position
    vehicleAngle = 0.0f;
    eyePos   = glm::vec3(0.0f, 4.0f,  12.0f);
    eyeFront = glm::vec3(0.0f, 0.0f, -1.0f);
    eyeUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    
    if (!glfwInit()){
        exit(-1);
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    window = glfwCreateWindow(gWidth, gHeight, "CENG469_HW2", NULL, NULL);

    if (!window){
        glfwTerminate();
        exit(-1);
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit()){
        std::cout << "Failed to initialize GLEW" << std::endl;
        exit(-1);
    }

    char rendererInfo[512] = { 0 };
    strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);
    
    glGenBuffers(1, &UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
      
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));
}

void Scene::renderWithoutVehicle(){
    sprites[0]->renderCubeMap();
    sprites[1]->render();
    sprites[2]->renderVariation(glm::vec3(20.0f,-4.6f,-20.0f));
    sprites[2]->renderVariation(glm::vec3(-30.0f,-4.6f,-25.0f));
    sprites[4]->render();
}

glm::vec3 Scene::calculateDirection(float inputYaw, float inputPitch){
    glm::vec3 direction;
    yaw = inputYaw;
    pitch = inputPitch;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    return direction;
}

void Scene::calculateFrameTime(){
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Scene::movementKeys(GLFWwindow* window){
    int sign = 1;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        eyeSpeedCoefficientZ = max(-1.0f,eyeSpeedCoefficientZ - (float)deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        eyeSpeedCoefficientZ = min(1.0f,eyeSpeedCoefficientZ + (float)deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        eyeSpeedCoefficientR = eyeSpeedCoefficientR - (float)deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        eyeSpeedCoefficientR = eyeSpeedCoefficientR + (float)deltaTime;
    }
    (eyeSpeedCoefficientZ > 0) ? sign = -1 : sign = 1;
    movementOffset += glm::vec3(glm::sin(glm::radians(vehicleAngle)),0.0f,-glm::cos(glm::radians(vehicleAngle))) * eyeSpeedCoefficientZ * (0.1f);
    vehicleAngle += eyeSpeedCoefficientR * sign * (0.5f);
    eyeSpeedCoefficientZ /= 1.01;
    eyeSpeedCoefficientR /= 1.01;
}

void Scene::initWindowShape(){
    glViewport(0, 0, gWidth, gHeight);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), gWidth/(float) gHeight, 1.0f, 400.0f);
    
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMatrix));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Scene::lookAt(){
    glm::mat4 viewingMatrix = glm::lookAt(eyePos, eyePos + eyeFront, eyeUp);
    
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(viewingMatrix));
    glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4), sizeof(glm::vec3), glm::value_ptr(eyePos));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

Sprite::Sprite() {
    ;
}

Sprite::Sprite(Scene *inputScene, string inputObjDir, string inputTexDir) {
    isVehicle = false;
    doesShake = false;
    scene = inputScene;
    objDir = inputObjDir;
    texDir = inputTexDir;
    spriteId = scene->spriteCount;
    scene->sprites.push_back(this);
    initRefViewingMatrices();
    cout << "Sprite Created - spriteId:" << spriteId << "\n";
    scene->spriteCount += 1;
}

Sprite::Sprite(Scene *inputScene, string inputObjDir, string inputCubeTexDirs[6]) {
    isVehicle = false;
    doesShake = false;
    scene = inputScene;
    objDir = inputObjDir;
    for(int i = 0 ; i < 6 ; ++i){
        cubeMapDirs[i] = inputCubeTexDirs[i];
    }
    spriteId = scene->spriteCount;
    scene->sprites.push_back(this);
    cout << "Sprite Created - spriteId:" << spriteId << "\n";
    scene->spriteCount += 1;
}

void Sprite::initRefViewingMatrices(){
    // glm::mat4 deneme = glm::lookAt(<#const vec<3, T, Q> &eye#>, <#const vec<3, T, Q> &center#>, <#const vec<3, T, Q> &up#>)
    glm::vec3 centerPos = glm::vec3(0.0f,0.2f,0.0f);
    refViewingMatrices[0] = glm::lookAt(centerPos, centerPos + glm::vec3(1.0f,0.0f,0.0f), glm::vec3(0.0f,-1.0f,0.0f));
    refViewingMatrices[1] = glm::lookAt(centerPos, centerPos + glm::vec3(-1.0f,0.0f,0.0f), glm::vec3(0.0f,-1.0f,0.0f));
    refViewingMatrices[2] = glm::lookAt(centerPos, centerPos + glm::vec3(0.0f,1.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f));
    refViewingMatrices[3] = glm::lookAt(centerPos, centerPos + glm::vec3(0.0f,-1.0f,0.0f), glm::vec3(0.0f,0.0f,-1.0f));
    refViewingMatrices[4] = glm::lookAt(centerPos, centerPos + glm::vec3(0.0f,0.0f,1.0f), glm::vec3(0.0f,-1.0f,0.0f));
    refViewingMatrices[5] = glm::lookAt(centerPos, centerPos + glm::vec3(0.0f,0.0f,-1.0f), glm::vec3(0.0f,-1.0f,0.0f));
}

bool Sprite::writeVertexNormal(GLfloat* normalData, int vertexIndex, int normalIndex){
    if(normalData[3 * vertexIndex] == 0 &&
       normalData[3 * vertexIndex + 1] == 0 &&
       normalData[3 * vertexIndex + 2] == 0){
        normalData[3 * vertexIndex] = model->normals[3 * normalIndex];
        normalData[3 * vertexIndex + 1] = model->normals[3 * normalIndex + 1];
        normalData[3 * vertexIndex + 2] = model->normals[3 * normalIndex + 2];
        return false;
    }
    else if (normalData[3 * vertexIndex] == model->normals[3 * normalIndex] &&
             normalData[3 * vertexIndex + 1] == model->normals[3 * normalIndex + 1] &&
             normalData[3 * vertexIndex + 2] == model->normals[3 * normalIndex + 2]){
        return false;
    }
    return true;
}

bool Sprite::writeVertexTexCoord(GLfloat* texCoordData, int vertexIndex, int texCoordIndex){
    if(texCoordData[2 * vertexIndex] == 0 && texCoordData[2 * vertexIndex + 1] == 0){
        texCoordData[2 * vertexIndex] = model->texcoords[2 * texCoordIndex];
        texCoordData[2 * vertexIndex + 1] = 1.0f - model->texcoords[2 * texCoordIndex + 1];
        return false;
    }
    else if (texCoordData[2 * vertexIndex] == model->texcoords[2 * texCoordIndex] &&
             texCoordData[2 * vertexIndex + 1] == 1.0f - model->texcoords[2 * texCoordIndex + 1]){
        return false;
    }
    return true;
}

void Sprite::addFaceElements(int index){
    if(writeVertexNormal(normalData, model->indices[index].p,model->indices[index].n) ||
       writeVertexTexCoord(texCoordData, model->indices[index].p,model->indices[index].t)){
        vertexData[vertexEntries] = vertexData[3 * model->indices[index].p];
        vertexData[vertexEntries + 1] = vertexData[3 * model->indices[index].p + 1];
        vertexData[vertexEntries + 2] = vertexData[3 * model->indices[index].p + 2];
        
        normalData[vertexEntries] = model->normals[3 * model->indices[index].n];
        normalData[vertexEntries + 1] = model->normals[3 * model->indices[index].n + 1];
        normalData[vertexEntries + 2] = model->normals[3 * model->indices[index].n + 2];
        
        texCoordData[texCoordEntries] = model->texcoords[2 * model->indices[index].t];
        texCoordData[texCoordEntries + 1] = 1.0f - model->texcoords[2 * model->indices[index].t + 1];
        indexData[index] = vertexEntries/3;
        vertexEntries = vertexEntries + 3;
        texCoordEntries = texCoordEntries + 2;
    }
    else{
        indexData[index] = model->indices[index].p;
    }
}

void Sprite::initShader(string vertDir, string fragDir){
    GLint status, vs, fs;
    
    gProgram = glCreateProgram();
    vs = createVS(vertDir.c_str());
    fs = createFS(fragDir.c_str());
    glAttachShader(gProgram, vs);
    glAttachShader(gProgram, fs);
    glLinkProgram(gProgram);
    glGetProgramiv(gProgram, GL_LINK_STATUS, &status);

    if (status != GL_TRUE){
        cout << "Program link failed for program" << gProgram << endl;
        exit(-1);
    }
}

void Sprite::initBuffer(float scaleFactorInput, glm::vec3 positionOffsetInput){
    scaleFactor = scaleFactorInput;
    positionOffset = positionOffsetInput;
    model = fast_obj_read(objDir.c_str());
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(texDir.c_str(), &width, &height, &nrChannels, 0);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cout << "Failed to load texture" << std::endl;
    }
    
    stbi_image_free(data);
    
    vertexEntries = model->position_count * 3;
    texCoordEntries = model->position_count * 2;
    faceEntries = model->face_count * 3;
    
    vertexDataSize = vertexEntries * sizeof(GLfloat);
    indexDataSize = faceEntries * sizeof(GLuint);
    
    vertexData  = new GLfloat[faceEntries * 3] ();
    memcpy(vertexData, model->positions, vertexDataSize);
    normalData = new GLfloat[faceEntries * 3] ();
    texCoordData = new GLfloat[faceEntries * 3] ();
    indexData = new GLuint[faceEntries] ();
    
    for (int i = 0; i < model->face_count; ++i){
        for(int j = 0 ; j < 3 ; ++j) {
            addFaceElements(3 * i + j);
        }
    }
    vertexDataSize = vertexEntries * sizeof(GLfloat);
    normalDataSize = vertexEntries * sizeof(GLfloat);
    texCoordDataSize = texCoordEntries * sizeof(GLfloat);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    
    glBufferData(GL_ARRAY_BUFFER, vertexDataSize + normalDataSize + texCoordDataSize, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexDataSize, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize, normalDataSize, normalData);
    glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize + normalDataSize, texCoordDataSize, texCoordData);
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertexDataSize));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertexDataSize + normalDataSize));

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] texCoordData;
    delete[] indexData;
    fast_obj_destroy(model);
}

void Sprite::initSkyBoxBuffer(){
    model = fast_obj_read(objDir.c_str());
    int vertexEntries, faceEntries;
    
    vertexEntries = model->position_count * 3;
    faceEntries = model->face_count * 3;
    
    vertexDataSize = vertexEntries * sizeof(GLfloat);
    indexDataSize = faceEntries * sizeof(GLuint);
    GLuint* indexData = new GLuint[faceEntries];
    
    for (int i = 0; i < model->face_count; ++i){
        indexData[3 * i] = model->indices[3 * i].p;
        indexData[3 * i + 1] = model->indices[3 * i + 1].p;
        indexData[3 * i + 2] = model->indices[3 * i + 2].p;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexDataSize, model->positions, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // Creates the cubemap texture object
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // These are very important to prevent seams
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // This might help with seams on some systems
    //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    // Cycles through all the textures and attaches them to the cubemap object
    for (unsigned int i = 0; i < 6; i++){
        int width, height, nrChannels;
        unsigned char* data = stbi_load(cubeMapDirs[i].c_str(), &width, &height, &nrChannels, 0);
        if (data){
            stbi_set_flip_vertically_on_load(false);
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_RGB,
                width,
                height,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                data
            );
            stbi_image_free(data);
        }
        else{
            std::cout << "Failed to load texture: " << cubeMapDirs[i] << std::endl;
            stbi_image_free(data);
        }
    }

    // done copying; can free now
    delete[] indexData;
    fast_obj_destroy(model);
}

void Sprite::initReflection(){
    
    glGenTextures(1, &box2CubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, box2CubeMap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for(int i=0; i<6; ++i){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    }

    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 1024);
    
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    for(int i=0; i<6; ++i){
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, box2CubeMap, 0);
    }
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){exit(0);}
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

glm::mat4 Sprite::shake(){
    shakeAngle += (scene->deltaTime)/10;
    return glm::rotate(glm::mat4(1.0f), glm::radians(glm::sin(shakeAngle)*5), glm::vec3(0.6f,0.0f,0.8f));
}

void Sprite::reflect(){
    glm::mat4 reflectionProjectionMat = glm::perspective(glm::radians(90.0f), 1.0f , 0.1f, 100.0f);
    
    glViewport(0, 0, 1024, 1024);
    glBindBuffer(GL_UNIFORM_BUFFER, scene->UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(reflectionProjectionMat));
    
    
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    for(int i = 0 ; i <6 ; ++i){
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, box2CubeMap, 0);
        glClearColor(0, 0, 0, 1);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(refViewingMatrices[i]));
        scene->renderWithoutVehicle();
    }
    
    // Fix Buffers
    int width,height;
    glfwGetFramebufferSize(scene->window, &width, &height);
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), scene->gWidth/(float) scene->gHeight, 1.0f, 400.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMatrix));
    glm::mat4 viewingMatrix = glm::lookAt(scene->eyePos, scene->eyePos + scene->eyeFront, scene->eyeUp);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(viewingMatrix));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Sprite::render(){
    glm::mat4 matS,matT,matR,modelingMatrix;
    glUseProgram(gProgram);
    
    if(isVehicle) {
        matS = glm::scale(glm::mat4(1.f), glm::vec3(scaleFactor ,scaleFactor ,scaleFactor));
        modelingMatrix = matS;
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, box2CubeMap);
        glUniform1i(glGetUniformLocation(gProgram, "skybox"), 0);
    }
    else{
        matR = glm::rotate(glm::mat4(1.0f), glm::radians(scene->vehicleAngle), glm::vec3(0.0f,1.0f,0.0f));
        matS = glm::scale(glm::mat4(1.f), glm::vec3(scaleFactor ,scaleFactor ,scaleFactor));
        matT = glm::translate(glm::mat4(1.0f), positionOffset + scene->movementOffset);
        if(doesShake){
            glm::mat4 shakeMatR = shake();
            modelingMatrix = matR * matT * matS * shakeMatR;
        }
        else{modelingMatrix = matR * matT * matS;}
        
    }
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(gProgram, "sampler"), 1);
    
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(modelingMatrix));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, faceEntries , GL_UNSIGNED_INT, 0);
}

void Sprite::renderVariation(glm::vec3 positionOffsetInput){
    positionOffset = positionOffsetInput;
    render();
}

void Sprite::renderCubeMap(){
    glDisable(GL_DEPTH_TEST);
    
    glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(80.0f ,80.0f ,80.0f));
    glm::mat4 matT = glm::translate(glm::mat4(1.0f), scene->eyePos);
    glm::mat4 matR = glm::rotate(glm::mat4(1.0f), glm::radians(scene->vehicleAngle), glm::vec3(0.0f,1.0f,0.0f));
    glm::mat4 modelingMatrix = matT * matR * matS;
    
    glUseProgram(gProgram);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glUniform1i(glGetUniformLocation(gProgram, "sampler"), 0);
    
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(modelingMatrix));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glEnable(GL_DEPTH_TEST);
}

#endif
