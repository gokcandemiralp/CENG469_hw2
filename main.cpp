#include "main.h"

GLuint gProgram;
int gWidth, gHeight;

GLint modelingMatrixLoc;
GLint viewingMatrixLoc;
GLint projectionMatrixLoc;
GLint eyePosLoc;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint gVertexAttribBuffer, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

fastObjMesh* m;

bool ParseObj(const string& fileName){
    fstream myfile;

    // Open the input
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open()){
        string curLine;

        while (getline(myfile, curLine)){
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            string tmp;

            if (curLine.length() >= 2){
                if (curLine[0] == 'v'){
                    if (curLine[1] == 't'){ // texture
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n'){ // normal
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals.push_back(Normal(c1, c2, c3));
                    }
                    else{ // vertex
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f'){ // face
                    str >> tmp; // consume "f"
                    char c;
                    int vIndex[3], nIndex[3], tIndex[3];
                    str >> vIndex[0]; str >> c >> c; // consume "//"
                    str >> nIndex[0];
                    str >> vIndex[1]; str >> c >> c; // consume "//"
                    str >> nIndex[1];
                    str >> vIndex[2]; str >> c >> c; // consume "//"
                    str >> nIndex[2];

                    assert(vIndex[0] == nIndex[0] &&
                        vIndex[1] == nIndex[1] &&
                        vIndex[2] == nIndex[2]); // a limitation for now

                    // make indices start from 0
                    for (int c = 0; c < 3; ++c){
                        vIndex[c] -= 1;
                        nIndex[c] -= 1;
                        tIndex[c] -= 1;
                    }

                    gFaces.push_back(Face(vIndex, tIndex, nIndex));
                }
                else{
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }
            //data += curLine;
            if (!myfile.eof()){
                //data += "\n";
            }
        }

        myfile.close();
    }
    else{
        return false;
    }

    /*
    for (int i = 0; i < gVertices.size(); ++i)
    {
        Vector3 n;

        for (int j = 0; j < gFaces.size(); ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                if (gFaces[j].vIndex[k] == i)
                {
                    // face j contains vertex i
                    Vector3 a(gVertices[gFaces[j].vIndex[0]].x,
                              gVertices[gFaces[j].vIndex[0]].y,
                              gVertices[gFaces[j].vIndex[0]].z);

                    Vector3 b(gVertices[gFaces[j].vIndex[1]].x,
                              gVertices[gFaces[j].vIndex[1]].y,
                              gVertices[gFaces[j].vIndex[1]].z);

                    Vector3 c(gVertices[gFaces[j].vIndex[2]].x,
                              gVertices[gFaces[j].vIndex[2]].y,
                              gVertices[gFaces[j].vIndex[2]].z);

                    Vector3 ab = b - a;
                    Vector3 ac = c - a;
                    Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
                    n += normalFromThisFace;
                }

            }
        }

        n.normalize();

        gNormals.push_back(Normal(n.x, n.y, n.z));
    }
    */

    assert(gVertices.size() == gNormals.size());

    return true;
}

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

    gVertexDataSizeInBytes = m->position_count * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes = m->normal_count * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = m->face_count * 3 * sizeof(GLuint);
    GLuint* indexData = new GLuint[gFaces.size() * 3];
    
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
}

void init(){
    m = fast_obj_read("bunny.obj");
    cout << "m->normal_count: " << m->normal_count << "\n" ;
    cout << "m->position_count: " << m->position_count << "\n" ;
    cout << "m->face_count: " << m->face_count << "\n" ;
    // tinyObj o;
    // bool success = read_tiny_obj("bunny.obj", &o);
    
    ParseObj("bunny.obj");

    glEnable(GL_DEPTH_TEST);
    initShaders();
    initVBO();
}

void drawModel(){
    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, gFaces.size() * 3, GL_UNSIGNED_INT, 0);
}

void display(){
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    static float angle = 0;

    float angleRad = (float)(angle / 180.0) * M_PI;

    // Compute the modeling matrix
    glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(-0.1f, -0.2f, -7.0f));
    glm::mat4 matRx = glm::rotate<float>(glm::mat4(1.0), (0. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
    glm::mat4 matRy = glm::rotate<float>(glm::mat4(1.0), (-180. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 matRz = glm::rotate<float>(glm::mat4(1.0), angleRad, glm::vec3(0.0, 0.0, 1.0));
    modelingMatrix = matT * matRz * matRy * matRx;

    // Set the active program and the values of its uniform variables
    glUseProgram(gProgram);
    glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewingMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(modelingMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelingMatrix));
    glUniform3fv(eyePosLoc, 1, glm::value_ptr(eyePos));

    // Draw the scene
    drawModel();

    angle += 0.5;
}

void reshape(GLFWwindow* window, int w, int h){
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);

    float fovyRad = (float)(45.0 / 180.0) * M_PI;
    projectionMatrix = glm::perspective(fovyRad, w/(float) h, 1.0f, 100.0f);

    viewingMatrix = glm::mat4(1);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_F && action == GLFW_PRESS){
        //glShadeModel(GL_FLAT);
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

    int width = 640, height = 640;
    window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

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
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, width, height); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
