#include "main.h"

GLuint gProgram;
int gWidth = 800, gHeight = 450;

GLint modelingMatrixLoc;
GLint viewingMatrixLoc;
GLint projectionMatrixLoc;
GLint eyePosLoc;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;

glm::vec3 eyePos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 eyeFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 eyeUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::vec3 movementOffset(0.0f, 0.0f, -8.0f);
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

GLuint createVS(const char* shaderName){
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource)){
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*)shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = { 0 };
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    return vs;
}

GLuint createFS(const char* shaderName){
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource)){
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*)shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = { 0 };
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    return fs;
}

void initShaders(){

    gProgram = glCreateProgram();

    GLuint vs1 = createVS("vert.glsl");
    GLuint fs1 = createFS("frag.glsl");

    glAttachShader(gProgram, vs1);
    glAttachShader(gProgram, fs1);

    glLinkProgram(gProgram);
    GLint status;
    glGetProgramiv(gProgram, GL_LINK_STATUS, &status);

    if (status != GL_TRUE){
        cout << "Program link failed" << endl;
        exit(-1);
    }
    
    // Get the locations of the uniform variables from both programs
    modelingMatrixLoc = glGetUniformLocation(gProgram, "modelingMatrix");
    viewingMatrixLoc = glGetUniformLocation(gProgram, "viewingMatrix");
    projectionMatrixLoc = glGetUniformLocation(gProgram, "projectionMatrix");
    eyePosLoc = glGetUniformLocation(gProgram, "eyePos");
}

void initTexture(){
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load("objects/bicycle.jpg", &width, &height, &nrChannels, 0);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cout << "Failed to load texture" << std::endl;
    }
    
    glUseProgram(gProgram);
    glUniform1i(glGetUniformLocation(gProgram, "ourTexture"), 0); // set it manually
    
    stbi_image_free(data);
}

void writeVertexNormal(GLfloat* normalData, int vertexIndex, int normalIndex){
    normalData[3 * vertexIndex] = m->normals[3 * normalIndex];
    normalData[3 * vertexIndex + 1] = m->normals[3 * normalIndex + 1];
    normalData[3 * vertexIndex + 2] = m->normals[3 * normalIndex + 2];
}

void writeVertexTexCoord(GLfloat* texCoordData, int vertexIndex, int texCoordIndex){
    texCoordData[2 * vertexIndex] = m->texcoords[2 * texCoordIndex];
    texCoordData[2 * vertexIndex + 1] = m->texcoords[2 * texCoordIndex + 1];
}

//void writeVertexTexCoord(GLfloat* texCoordData, int vertexIndex, int texCoordIndex){
//    texCoordData[2 * vertexIndex] = 1.0f;
//    texCoordData[2 * vertexIndex + 1] = 1.0f;
//}

void initVBO(){
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer);
    glGenBuffers(1, &gIndexBuffer);

    assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    vertexEntries = m->position_count * 3;
    texCoordEntries = m->position_count * 2;
    faceEntries = m->face_count * 6;
    
    gVertexDataSizeInBytes = vertexEntries * sizeof(GLfloat);
    gNormalDataSizeInBytes = vertexEntries * sizeof(GLfloat);
    gTexCoordDataSizeInBytes = texCoordEntries * sizeof(GLfloat);
    indexDataSizeInBytes = faceEntries * sizeof(GLuint);
    GLfloat* normalData = new GLfloat[vertexEntries];
    GLfloat* texCoordData = new GLfloat[texCoordEntries];
    GLuint* indexData = new GLuint[faceEntries];
    
    for (int i = 0; i < m->face_count; ++i){
        indexData[6 * i] = m->indices[4 * i].p;
        indexData[6 * i + 1] = m->indices[4 * i + 1].p;
        indexData[6 * i + 2] = m->indices[4 * i + 2].p;
        
        indexData[6 * i + 3] = m->indices[4 * i].p;
        indexData[6 * i + 4] = m->indices[4 * i + 2].p;
        indexData[6 * i + 5] = m->indices[4 * i + 3].p;
        
        writeVertexNormal(normalData, m->indices[4 * i    ].p, m->indices[4 * i    ].n);
        writeVertexNormal(normalData, m->indices[4 * i + 1].p, m->indices[4 * i + 1].n);
        writeVertexNormal(normalData, m->indices[4 * i + 2].p, m->indices[4 * i + 2].n);
        writeVertexNormal(normalData, m->indices[4 * i + 3].p, m->indices[4 * i + 3].n);
        
        writeVertexTexCoord(texCoordData, m->indices[4 * i    ].p, m->indices[4 * i    ].t);
        writeVertexTexCoord(texCoordData, m->indices[4 * i + 1].p, m->indices[4 * i + 1].t);
        writeVertexTexCoord(texCoordData, m->indices[4 * i + 2].p, m->indices[4 * i + 2].t);
        writeVertexTexCoord(texCoordData, m->indices[4 * i + 3].p, m->indices[4 * i + 3].t);
        if(i == 1){cout << m->indices[4 * i    ].t  << "\n";}
    }


    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes + gTexCoordDataSizeInBytes, 0, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, m->positions);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, gTexCoordDataSizeInBytes, texCoordData);
    // texCoordData
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] indexData;
    delete[] normalData;
    fast_obj_destroy(m);
}

void initWindowShape(){
    glViewport(0, 0, gWidth, gHeight);
    float fovyRad = (float)(45.0 / 180.0) * M_PI;
    projectionMatrix = glm::perspective(fovyRad, gWidth/(float) gHeight, 1.0f, 100.0f);
    viewingMatrix = glm::lookAt(eyePos, eyePos + eyeFront, eyeUp);
    
    glUseProgram(gProgram);
    glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
}


void init(){
    m = fast_obj_read("objects/bicycle.obj");
    // m = fast_obj_read("cat.obj");
    cout << "m->normal_count: " << m->normal_count << "\n" ;
    cout << "m->position_count: " << m->position_count << "\n" ;
    cout << "m->face_count: " << m->face_count << "\n" ;
    cout << "m->texcoord_count: " << m->texcoord_count << "\n" ;

    glEnable(GL_DEPTH_TEST);
    initShaders();
    initTexture();
    initVBO();
    initWindowShape();
}

void drawModel(){
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes + gNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, faceEntries , GL_UNSIGNED_INT, 0);
}

void display(){
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    viewingMatrix = glm::lookAt(eyePos, eyePos + eyeFront, eyeUp);
    glm::mat4 matR = glm::rotate(glm::mat4(1.f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
    glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(0.02f ,0.02f ,0.02f));
    glm::mat4 matT = glm::translate(glm::mat4(1.0), movementOffset);
    modelingMatrix = matT * matS * matR;

    // Set the active program and the values of its uniform variables
    glUseProgram(gProgram);
    glUniformMatrix4fv(viewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(modelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelingMatrix));
    glUniform3fv(eyePosLoc, 1, glm::value_ptr(eyePos));

    // Draw the scene
    drawModel();
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

void mainLoop(GLFWwindow* window){
    while (!glfwWindowShouldClose(window)){
        movementKeys(window);
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
    
    window = glfwCreateWindow(gWidth, gHeight, "Simple Example", NULL, NULL);

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
