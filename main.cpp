#include "main.h"

spriteInfo skyBoxSprite;
spriteInfo groundSprite;

int gWidth = 800, gHeight = 450;
glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::vec3 eyePos   = glm::vec3(0.0f, 0.0f,  0.0f);
glm::vec3 eyeFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 eyeUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::vec3 movementOffset(0.0f, -1.0f, 0.0f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float eyeSpeed = 1.0f;

float mouseLastX=gWidth/2;
float mouseLastY=gHeight/2;
const float sensitivity = 0.1f;
float yaw = -90.0f;
float pitch = 0.0f;

void initShaders(){
    GLint status;
    
    skyBoxSprite.gProgram = glCreateProgram();
    GLuint vs1 = createVS("shaders/vertSkybox.glsl");
    GLuint fs1 = createFS("shaders/fragSkybox.glsl");
    glAttachShader(skyBoxSprite.gProgram, vs1);
    glAttachShader(skyBoxSprite.gProgram, fs1);
    glLinkProgram(skyBoxSprite.gProgram);
    glGetProgramiv(skyBoxSprite.gProgram, GL_LINK_STATUS, &status);

    if (status != GL_TRUE){
        cout << "Program link failed" << endl;
        exit(-1);
    }
    
    groundSprite.gProgram = glCreateProgram();
    // GLuint vs2 = createVS("shaders/vertCharacter.glsl");
    GLuint fs2 = createFS("shaders/fragCharacter.glsl");
    glAttachShader(groundSprite.gProgram, vs1);
    glAttachShader(groundSprite.gProgram, fs2);
    glLinkProgram(groundSprite.gProgram);
    glGetProgramiv(groundSprite.gProgram, GL_LINK_STATUS, &status);

    if (status != GL_TRUE){
        cout << "Program link failed" << endl;
        exit(-1);
    }
}

void writeVertexNormal(GLfloat* normalData, int vertexIndex, int normalIndex){
    normalData[3 * vertexIndex] = skyBoxSprite.model->normals[3 * normalIndex];
    normalData[3 * vertexIndex + 1] = skyBoxSprite.model->normals[3 * normalIndex + 1];
    normalData[3 * vertexIndex + 2] = skyBoxSprite.model->normals[3 * normalIndex + 2];
}

void writeVertexTexCoord(GLfloat* texCoordData, int vertexIndex, int texCoordIndex){
    texCoordData[2 * vertexIndex] = skyBoxSprite.model->texcoords[2 * texCoordIndex];
    texCoordData[2 * vertexIndex + 1] = 1.0f - skyBoxSprite.model->texcoords[2 * texCoordIndex + 1];
}

void initSkyBoxBuffer(){
    int vertexEntries, faceEntries;
    
    vertexEntries = skyBoxSprite.model->position_count * 3;
    faceEntries = skyBoxSprite.model->face_count * 3;
    
    groundSprite.vertexDataSize = vertexEntries * sizeof(GLfloat);
    groundSprite.indexDataSize = faceEntries * sizeof(GLuint);
    GLuint* indexData = new GLuint[faceEntries];
    
    for (int i = 0; i < skyBoxSprite.model->face_count; ++i){
        indexData[3 * i] = skyBoxSprite.model->indices[3 * i].p;
        indexData[3 * i + 1] = skyBoxSprite.model->indices[3 * i + 1].p;
        indexData[3 * i + 2] = skyBoxSprite.model->indices[3 * i + 2].p;
    }

    glGenVertexArrays(1, &skyBoxSprite.VAO);
    glGenBuffers(1, &skyBoxSprite.VBO);
    glGenBuffers(1, &skyBoxSprite.EBO);
    
    glBindVertexArray(skyBoxSprite.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxSprite.VBO);
    glBufferData(GL_ARRAY_BUFFER, groundSprite.vertexDataSize, skyBoxSprite.model->positions, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyBoxSprite.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, groundSprite.indexDataSize, indexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // All the faces of the cubemap (make sure they are in this exact order)
    std::string facesCubemap[6]={
        "objects/right.png",
        "objects/left.png",
        "objects/top.png",
        "objects/bottom.png",
        "objects/front.png",
        "objects/back.png"
    };

    // Creates the cubemap texture object
    glGenTextures(1, &skyBoxSprite.textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxSprite.textureID);
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
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
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
            std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
            stbi_image_free(data);
        }
    }

    // done copying; can free now
    delete[] indexData;
    fast_obj_destroy(skyBoxSprite.model);
}

void initGroundBuffer(){
    int vertexEntries, texCoordEntries, faceEntries;
    
    glGenVertexArrays(1, &groundSprite.VAO);
    glBindVertexArray(groundSprite.VAO);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &groundSprite.VBO);
    glGenBuffers(1, &groundSprite.EBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, groundSprite.VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundSprite.EBO);

    vertexEntries = groundSprite.model->position_count * 3;
    texCoordEntries = groundSprite.model->position_count * 2;
    faceEntries = groundSprite.model->face_count * 6;
    
    groundSprite.vertexDataSize = vertexEntries * sizeof(GLfloat);
    groundSprite.normalDataSize = vertexEntries * sizeof(GLfloat);
    groundSprite.texCoordDataSize = texCoordEntries * sizeof(GLfloat);
    groundSprite.indexDataSize = faceEntries * sizeof(GLuint);
    GLfloat* normalData = new GLfloat[vertexEntries];
    GLfloat* texCoordData = new GLfloat[texCoordEntries];
    GLuint* indexData = new GLuint[faceEntries];
    
    for (int i = 0; i < groundSprite.model->face_count; ++i){
        indexData[3 * i] = groundSprite.model->indices[3 * i].p;
        indexData[3 * i + 1] = groundSprite.model->indices[3 * i + 1].p;
        indexData[3 * i + 2] = groundSprite.model->indices[3 * i + 2].p;
    }


    glBufferData(GL_ARRAY_BUFFER, groundSprite.vertexDataSize +
                                  groundSprite.normalDataSize +
                                  groundSprite.texCoordDataSize, 0, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, groundSprite.vertexDataSize, groundSprite.model->positions);
    glBufferSubData(GL_ARRAY_BUFFER, groundSprite.vertexDataSize, groundSprite.normalDataSize, normalData);
    glBufferSubData(GL_ARRAY_BUFFER, groundSprite.vertexDataSize + groundSprite.normalDataSize, groundSprite.texCoordDataSize, texCoordData);
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, groundSprite.indexDataSize, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] indexData;
    delete[] texCoordData;
    delete[] normalData;
    fast_obj_destroy(groundSprite.model);
}

void initWindowShape(){
    glViewport(0, 0, gWidth, gHeight);
    float fovyRad = (float)(45.0 / 180.0) * M_PI;
    projectionMatrix = glm::perspective(fovyRad, gWidth/(float) gHeight, 1.0f, 100.0f);
    
    glUseProgram(skyBoxSprite.gProgram);
    glUniformMatrix4fv(glGetUniformLocation(skyBoxSprite.gProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    
    glUseProgram(groundSprite.gProgram);
    glUniformMatrix4fv(glGetUniformLocation(groundSprite.gProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
}

void init(){
    skyBoxSprite.model = fast_obj_read("objects/cube.obj");
    cout << "skyBoxSprite.model->normal_count: " << skyBoxSprite.model->normal_count << "\n" ;
    cout << "skyBoxSprite.model->position_count: " << skyBoxSprite.model->position_count << "\n" ;
    cout << "skyBoxSprite.model->face_count: " << skyBoxSprite.model->face_count << "\n" ;
    cout << "skyBoxSprite.model->texcoord_count: " << skyBoxSprite.model->texcoord_count << "\n" ;
    
    groundSprite.model = fast_obj_read("objects/ground.obj");
    cout << "groundSprite.model->normal_count: " << groundSprite.model->normal_count << "\n" ;
    cout << "groundSprite.model->position_count: " << groundSprite.model->position_count << "\n" ;
    cout << "groundSprite.model->face_count: " << groundSprite.model->face_count << "\n" ;
    cout << "groundSprite.model->texcoord_count: " << groundSprite.model->texcoord_count << "\n" ;

    glEnable(GL_DEPTH_TEST);
    initShaders();
    //initTexture();
    initSkyBoxBuffer();
    initGroundBuffer();
    initWindowShape();
}

void renderSkyBox(){
    glDisable(GL_DEPTH_TEST);
    
    glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(8.0f ,8.0f ,8.0f));
    glm::mat4 modelingMatrix = matS;
    
    glUseProgram(skyBoxSprite.gProgram);
    glUniform1i(glGetUniformLocation(skyBoxSprite.gProgram, "skybox"), 0);
    glUniformMatrix4fv(glGetUniformLocation(skyBoxSprite.gProgram, "viewingMatrix"), 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(glGetUniformLocation(skyBoxSprite.gProgram, "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(modelingMatrix));
    
    glBindVertexArray(skyBoxSprite.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxSprite.textureID);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);    // Switch the depth function back on
}

void renderGround(){
    glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(100.0f ,100.0f ,100.0f));
    glm::mat4 matT = glm::translate(glm::mat4(1.0), movementOffset);
    glm::mat4 modelingMatrix = matT * matS;
    
    glUseProgram(groundSprite.gProgram);
    glUniformMatrix4fv(glGetUniformLocation(groundSprite.gProgram, "viewingMatrix"), 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(glGetUniformLocation(groundSprite.gProgram, "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(modelingMatrix));
    glUniform3fv(glGetUniformLocation(groundSprite.gProgram, "eyePos"), 1, glm::value_ptr(eyePos));
    
    glBindVertexArray(groundSprite.VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(groundSprite.vertexDataSize));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(groundSprite.vertexDataSize + groundSprite.normalDataSize));

    glDrawElements(GL_TRIANGLES, 6 , GL_UNSIGNED_INT, 0);
}

void display(){
    viewingMatrix = glm::lookAt(eyePos, eyePos + eyeFront, eyeUp);
    renderSkyBox();
    renderGround();
}

void movementKeys(GLFWwindow* window){
    eyeSpeed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        movementOffset -= eyeSpeed * eyeFront;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        movementOffset += eyeSpeed * eyeFront;
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        movementOffset += glm::normalize(glm::cross(eyeFront, eyeUp)) * eyeSpeed;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        movementOffset -= glm::normalize(glm::cross(eyeFront, eyeUp)) * eyeSpeed;
    }
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

int main(int argc, char** argv){
    
    GLFWwindow* window;
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
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = { 0 };
    strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*)glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, mouse);
    
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
