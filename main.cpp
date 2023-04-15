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
glm::vec3 eyePos(0, 0, 2);

GLuint gVertexAttribBuffer, gIndexBuffer;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes, indexDataSizeInBytes;
int vertexEntries, normalEntries, faceEntries;

fastObjMesh* m;

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

void initVBO(){
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);
    cout << "vao = " << vao << endl;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer);
    glGenBuffers(1, &gIndexBuffer);

    assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    vertexEntries = m->position_count * 3;
    normalEntries = m->normal_count * 3;
    faceEntries = m->face_count * 3;
    
    gVertexDataSizeInBytes = vertexEntries * sizeof(GLfloat);
    gNormalDataSizeInBytes = normalEntries * sizeof(GLfloat);
    indexDataSizeInBytes = faceEntries * sizeof(GLuint);
    GLuint* indexData = new GLuint[faceEntries];
    
    for (int i = 0; i < m->face_count; ++i){
        indexData[3 * i] = m->indices[3 * i].p;
        indexData[3 * i + 1] = m->indices[3 * i + 1].p;
        indexData[3 * i + 2] = m->indices[3 * i + 2].p;
    }


    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, m->positions);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, m->normals);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] indexData;
    fast_obj_destroy(m);
}

void initWindowShape(){
    glViewport(0, 0, gWidth, gHeight);
    float fovyRad = (float)(45.0 / 180.0) * M_PI;
    projectionMatrix = glm::perspective(fovyRad, gWidth/(float) gHeight, 1.0f, 100.0f);
}


void init(){
    m = fast_obj_read("armadillo.obj");
    // m = fast_obj_read("cat.obj");
    cout << "m->normal_count: " << m->normal_count << "\n" ;
    cout << "m->position_count: " << m->position_count << "\n" ;
    cout << "m->face_count: " << m->face_count << "\n" ;

    glEnable(GL_DEPTH_TEST);
    initShaders();
    initVBO();
    initWindowShape();
}

void drawModel(){
    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, faceEntries , GL_UNSIGNED_INT, 0);
}

void display(){
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Compute the modeling matrix
    viewingMatrix = viewingMatrix = glm::lookAt(eyePos,glm::vec3(0,0,0),glm::vec3(0,1,0));
    glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(0.2f ,0.2f ,0.2f));
    modelingMatrix = matS * matT;

    // Set the active program and the values of its uniform variables
    glUseProgram(gProgram);
    glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(modelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelingMatrix));
    glUniform3fv(eyePosLoc, 1, glm::value_ptr(eyePos));

    // Draw the scene
    drawModel();
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_W){
        eyePos.z -= 0.05;
    }
    else if (key == GLFW_KEY_S){
        eyePos.z += 0.05;
    }
}

void mainLoop(GLFWwindow* window){
    while (!glfwWindowShouldClose(window)){
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

    glfwSetKeyCallback(window, keyboard);
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
