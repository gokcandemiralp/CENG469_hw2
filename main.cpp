#include "main.h"

spriteInfo skyBoxSprite;
spriteInfo characterSprite;


int gWidth = 800, gHeight = 450;
glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::vec3 eyePos   = glm::vec3(0.0f, 0.0f,  0.0f);
glm::vec3 eyeFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 eyeUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::vec3 movementOffset(0.0f, 0.0f, 0.0f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float eyeSpeed = 1.0f;

float mouseLastX=gWidth/2;
float mouseLastY=gHeight/2;
const float sensitivity = 0.1f;
float yaw = -90.0f;
float pitch = 0.0f;


GLuint gVertexAttribBuffer, gIndexBuffer;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes, indexDataSizeInBytes, gTexCoordDataSizeInBytes;
int vertexEntries, normalEntries, faceEntries, texCoordEntries;

fastObjMesh* m;

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
    
    characterSprite.gProgram = glCreateProgram();
    // GLuint vs2 = createVS("shaders/vertCharacter.glsl");
    GLuint fs2 = createFS("shaders/fragCharacter.glsl");
    glAttachShader(characterSprite.gProgram, vs1);
    glAttachShader(characterSprite.gProgram, fs2);
    glLinkProgram(characterSprite.gProgram);
    glGetProgramiv(characterSprite.gProgram, GL_LINK_STATUS, &status);

    if (status != GL_TRUE){
        cout << "Program link failed" << endl;
        exit(-1);
    }
}

void writeVertexNormal(GLfloat* normalData, int vertexIndex, int normalIndex){
    normalData[3 * vertexIndex] = m->normals[3 * normalIndex];
    normalData[3 * vertexIndex + 1] = m->normals[3 * normalIndex + 1];
    normalData[3 * vertexIndex + 2] = m->normals[3 * normalIndex + 2];
}

void writeVertexTexCoord(GLfloat* texCoordData, int vertexIndex, int texCoordIndex){
    texCoordData[2 * vertexIndex] = m->texcoords[2 * texCoordIndex];
    texCoordData[2 * vertexIndex + 1] = 1.0f - m->texcoords[2 * texCoordIndex + 1];
}

void initVBO(){
    vertexEntries = m->position_count * 3;
    texCoordEntries = m->position_count * 2;
    faceEntries = m->face_count * 3;
    
    gVertexDataSizeInBytes = vertexEntries * sizeof(GLfloat);
    gNormalDataSizeInBytes = vertexEntries * sizeof(GLfloat);
    gTexCoordDataSizeInBytes = texCoordEntries * sizeof(GLfloat);
    indexDataSizeInBytes = faceEntries * sizeof(GLuint);
    GLuint* indexData = new GLuint[faceEntries];
    
    for (int i = 0; i < m->face_count; ++i){
        indexData[3 * i] = m->indices[3 * i].p;
        indexData[3 * i + 1] = m->indices[3 * i + 1].p;
        indexData[3 * i + 2] = m->indices[3 * i + 2].p;
    }

    glGenVertexArrays(1, &skyBoxSprite.VAO);
    glGenBuffers(1, &skyBoxSprite.VBO);
    glGenBuffers(1, &skyBoxSprite.EBO);
    
    glBindVertexArray(skyBoxSprite.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxSprite.VBO);
    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, m->positions, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyBoxSprite.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);
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
    fast_obj_destroy(m);
}

void initWindowShape(){
    glViewport(0, 0, gWidth, gHeight);
    float fovyRad = (float)(45.0 / 180.0) * M_PI;
    projectionMatrix = glm::perspective(fovyRad, gWidth/(float) gHeight, 1.0f, 100.0f);
    
    glUseProgram(skyBoxSprite.gProgram);
    glUniformMatrix4fv(glGetUniformLocation(skyBoxSprite.gProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    
    glUseProgram(characterSprite.gProgram);
    glUniformMatrix4fv(glGetUniformLocation(characterSprite.gProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
}

void init(){
    m = fast_obj_read("objects/cube.obj");
    // m = fast_obj_read("cat.obj");
    cout << "m->normal_count: " << m->normal_count << "\n" ;
    cout << "m->position_count: " << m->position_count << "\n" ;
    cout << "m->face_count: " << m->face_count << "\n" ;
    cout << "m->texcoord_count: " << m->texcoord_count << "\n" ;

    glEnable(GL_DEPTH_TEST);
    initShaders();
    //initTexture();
    initVBO();
    initWindowShape();
}

void renderSkyBox(){
    glDepthFunc(GL_LEQUAL);
    
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

    glDepthFunc(GL_LESS);   // Switch back to the normal depth function
}

void renderCharacter(){
    glm::mat4 matR = glm::rotate(glm::mat4(1.f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
    glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(1.0f ,1.0f ,1.0f));
    glm::mat4 matT = glm::translate(glm::mat4(1.0), movementOffset);
    glm::mat4 modelingMatrix = matT * matS * matR;
    
    glUseProgram(characterSprite.gProgram);
    glUniform1i(glGetUniformLocation(characterSprite.gProgram, "skybox"), 0);
    glUniformMatrix4fv(glGetUniformLocation(characterSprite.gProgram, "viewingMatrix"), 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(glGetUniformLocation(characterSprite.gProgram, "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(modelingMatrix));
    glUniform3fv(glGetUniformLocation(characterSprite.gProgram, "eyePos"), 1, glm::value_ptr(eyePos));
    
    glBindVertexArray(skyBoxSprite.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxSprite.textureID);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void display(){
    viewingMatrix = glm::lookAt(eyePos, eyePos + eyeFront, eyeUp);
    renderSkyBox();
    renderCharacter();
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
