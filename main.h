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

struct Sprite{
    public:
        string objDir;
        string texDir;
        string cubeMapDirs[6];
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
    Sprite(string inputObjDir, string inputCubeTexDirs[6]) {
        objDir = inputObjDir;
        for(int i = 0 ; i < 6 ; ++i){
            cubeMapDirs[i] = inputCubeTexDirs[i];
        }
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
    
    void initShader(string vertDir, string fragDir){
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
        texCoordDataSize = vertexEntries * sizeof(GLfloat);
        indexDataSize = faceEntries * sizeof(GLuint);
        GLfloat* normalData = new GLfloat[vertexEntries];
        GLfloat* texCoordData = new GLfloat[texCoordDataSize];
        GLuint* indexData = new GLuint[faceEntries];
        
        for (int i = 0; i < model->face_count; ++i){
            for(int j = 0 ; j < 3 ; ++j) {
                indexData[3 * i + j] = model->indices[3 * i + j].p;
                writeVertexNormal(normalData, model->indices[3 * i + j].p,model->indices[3 * i + j].n);
                writeVertexTexCoord(texCoordData, model->indices[3 * i  + j].p,model->indices[3 * i + j].t);
            }
        }

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
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexDataSize, model->positions);
        glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize, normalDataSize, normalData);
        glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize + normalDataSize, texCoordDataSize, texCoordData);
        
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData, GL_STATIC_DRAW);

        // done copying; can free now
        delete[] normalData;
        delete[] texCoordData;
        delete[] indexData;
        fast_obj_destroy(model);
    }
    
    void initSkyBoxBuffer(){
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
    
    void render(float scaleFactor, glm::vec3 movementOffset, glm::mat4 &projectionMatrix, glm::mat4 &viewingMatrix){
        glm::mat4 matS = glm::scale(glm::mat4(1.f), glm::vec3(scaleFactor ,scaleFactor ,scaleFactor));
        glm::mat4 matT = glm::translate(glm::mat4(1.0f), movementOffset);
        glm::mat4 modelingMatrix = matT * matS;
        glm::vec3 eyePos   = glm::vec3(0.0f, 0.0f,  0.0f);
        
        glUseProgram(gProgram);
        glUniform1i(glGetUniformLocation(gProgram, "sampler"), 0); // set it manually
        glUniformMatrix4fv(glGetUniformLocation(gProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(glGetUniformLocation(gProgram, "viewingMatrix"), 1, GL_FALSE, glm::value_ptr(viewingMatrix));
        glUniformMatrix4fv(glGetUniformLocation(gProgram, "modelingMatrix"), 1, GL_FALSE, glm::value_ptr(modelingMatrix));
        glUniform3fv(glGetUniformLocation(gProgram, "eyePos"), 1, glm::value_ptr(eyePos));
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertexDataSize));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertexDataSize + normalDataSize));
        glDrawElements(GL_TRIANGLES, faceEntries , GL_UNSIGNED_INT, 0);
    }
};

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
