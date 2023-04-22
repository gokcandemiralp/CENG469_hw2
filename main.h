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
#define CHECK(_c) check(_c, #_c)
#define VERTEX_PER_FACE 4

struct Sprite{
    public:
        string objDir;
        string texDir;
        GLuint gProgram, VAO, VBO, EBO, textureID;
        GLuint vertexDataSize, normalDataSize, indexDataSize, texCoordDataSize;
        GLuint vertexEntries, faceEntries;
        fastObjMesh* model;
    
    Sprite() {
        ;
    }
    
    Sprite(string inputObjDir, string inputTexDir) {
        objDir = inputObjDir;
        texDir = inputTexDir;
    }

    void writeVertexNormal(GLfloat* normalData, int vertexIndex, int normalIndex){
        normalData[3 * vertexIndex] = model->normals[3 * normalIndex];
        normalData[3 * vertexIndex + 1] = model->normals[3 * normalIndex + 1];
        normalData[3 * vertexIndex + 2] = model->normals[3 * normalIndex + 2];
    }

    void writeVertexTexCoord(GLfloat* texCoordData, int vertexIndex, int texCoordIndex){
        texCoordData[2 * vertexIndex] = model->texcoords[2 * texCoordIndex];
        texCoordData[2 * vertexIndex + 1] = 1.0f - model->texcoords[2 * texCoordIndex + 1];
    }
    
    void initBuffer(){
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
        faceEntries = model->face_count * 3;
        
        vertexDataSize = vertexEntries * sizeof(GLfloat);
        normalDataSize = vertexEntries * sizeof(GLfloat);
        indexDataSize = faceEntries * sizeof(GLuint);
        GLfloat* normalData = new GLfloat[vertexEntries];
        GLuint* indexData = new GLuint[faceEntries];
        
        for (int i = 0; i < model->face_count; ++i){
            indexData[3 * i] = model->indices[3 * i].p;
            indexData[3 * i + 1] = model->indices[3 * i + 1].p;
            indexData[3 * i + 2] = model->indices[3 * i + 2].p;
            
            this->writeVertexNormal(normalData, model->indices[3 * i].p,model->indices[3 * i].n);
            this->writeVertexNormal(normalData, model->indices[3 * i + 1].p,model->indices[3 * i + 1].n);
            this->writeVertexNormal(normalData, model->indices[3 * i + 2].p,model->indices[3 * i + 2].n);
        }

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        assert(glGetError() == GL_NONE);

        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        
        glBufferData(GL_ARRAY_BUFFER, vertexDataSize + normalDataSize, 0, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexDataSize, model->positions);
        glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize, normalDataSize, normalData);
        
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData, GL_STATIC_DRAW);

        // done copying; can free now
        delete[] normalData;
        delete[] indexData;
        fast_obj_destroy(model);
    }
    
    void render(glm::vec3 &movementOffset, glm::mat4 &viewingMatrix){
        glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(1.0f ,1.0f ,1.0f));
        glm::mat4 matT = glm::translate(glm::mat4(1.0f), movementOffset);
        glm::mat4 modelingMatrix = matT * matS;
        glm::vec3 eyePos   = glm::vec3(0.0f, 0.0f,  0.0f);
        
        glUseProgram(gProgram);
        glUniform1i(glGetUniformLocation(gProgram, "sampler"), 0); // set it manually
        glUniformMatrix4fv(glGetUniformLocation(gProgram, "viewingMatrix"), 1, GL_FALSE, glm::value_ptr(viewingMatrix));
        glUniformMatrix4fv(glGetUniformLocation(gProgram, "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(modelingMatrix));
        glUniform3fv(glGetUniformLocation(gProgram, "eyePos"), 1, glm::value_ptr(eyePos));
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertexDataSize));
        glDrawElements(GL_TRIANGLES, faceEntries , GL_UNSIGNED_INT, 0);
    }
};

struct Vertex{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Texture{
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
    GLfloat u, v;
};

struct Normal{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Face{
    Face(int v[], int t[], int n[]) {
        vIndex[0] = v[0];
        vIndex[1] = v[1];
        vIndex[2] = v[2];
        tIndex[0] = t[0];
        tIndex[1] = t[1];
        tIndex[2] = t[2];
        nIndex[0] = n[0];
        nIndex[1] = n[1];
        nIndex[2] = n[2];
    }
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

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

void initShader(string vsFile, string fsFile, Sprite &sprite, glm::mat4 &projectionMatrix){
    GLint status, vs, fs;
    
    sprite.gProgram = glCreateProgram();
    vs = createVS(vsFile.c_str());
    fs = createFS(fsFile.c_str());
    glAttachShader(sprite.gProgram, vs);
    glAttachShader(sprite.gProgram, fs);
    glLinkProgram(sprite.gProgram);
    glGetProgramiv(sprite.gProgram, GL_LINK_STATUS, &status);
    glUseProgram(sprite.gProgram);
    glUniformMatrix4fv(glGetUniformLocation(sprite.gProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    if (status != GL_TRUE){
        cout << "Program link failed for program" << sprite.gProgram << endl;
        exit(-1);
    }
}


#endif
