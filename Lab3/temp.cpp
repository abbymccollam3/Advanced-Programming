/*
Author: Abby McCollam
Class: ECE4122 Section A
Last Date Modified: 12/12/23
Description: Main file for implementing 4 suzanne objects moving at random speeds on textured rectangle
*/

// header files
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random>
#include <thread>
#include <mutex>
#include <GL/glew.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

using namespace glm;
GLFWwindow* window;
std::mutex mtx;

struct object // structure for all objects
{
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 rotationVec;
    float rotate;
    float val;
};

object Objects[5]; // 5 objects

glm::vec3 getRandomVelocity() // function returning random velocities for all directions
{
    return glm::vec3
            (
                    -0.01 + static_cast<double>(rand()) / RAND_MAX * 0.02, // ranging between -.01 and .01
                    -0.01 + static_cast<double>(rand()) / RAND_MAX * 0.02,
                    -0.01 + static_cast<double>(rand()) / RAND_MAX * 0.02
            );
}

glm::vec3 getRandomRotation() // function returning random rotations for all directions
{
    return glm::vec3
            (
                    -1.0 + static_cast<double>(std::rand()) / RAND_MAX * 2.0, // ranging between -1 and 1
                    -1.0 + static_cast<double>(std::rand()) / RAND_MAX * 2.0,
                    -1.0 + static_cast<double>(std::rand()) / RAND_MAX * 2.0
            );
}

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.3f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for our buffers
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
    GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
    GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

    // Load the texture
    GLuint Texture = loadDDS("uvmap.DDS");
    GLuint RectangleTexture = loadBMP_custom("WoodPlanksOld.bmp"); // loading rectangle texture

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    // Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

    // Load it into a VBO
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);

    GLuint FirstSignalID = glGetUniformLocation(programID, "toggleSpecular");
    GLuint LightID = glGetUniformLocation(programID, "LightColor");
    GLuint SignalID = glGetUniformLocation(programID, "toggleSpecular");
    GLuint PowerID = glGetUniformLocation(programID, "LightPower");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(0);

    //------------------------------------------------------------------------------
    //RECTANGLE VERTICES
    GLfloat rectangleVertices[] =
            {
                    -20.0f, -20.0f, 0.0f,
                    -20.0f,  20.0f, 0.0f,
                    20.0f,	20.0f, 0.0f,
                    20.0f, -20.0f, 0.0f,
            };
    //RECTANGLE NORMALS
    GLfloat rectangleNormals[] =
            {
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
            };

    //RECTANGLE INDICES
    unsigned int rectangleIndices[] =
            {
                    0, 1, 3,
                    1, 2, 3
            };

    GLuint squareEBO; //stores ID for element buffer object
    glGenBuffers(1,&squareEBO); //generates 1 EBO
    glBindBuffer(GL_ARRAY_BUFFER, squareEBO); //binds EBO to target
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleIndices), rectangleIndices, GL_STATIC_DRAW); //copies data from rectangleIndices and binds

    //SAME IDEA FOR REST
    GLuint squareNorm;
    glGenBuffers(1,&squareNorm);
    glBindBuffer(GL_ARRAY_BUFFER, squareNorm);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleNormals), rectangleNormals, GL_STATIC_DRAW);

    GLuint squareVBO;
    glGenBuffers(1,&squareVBO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    bool beforeG = false;

    // initial positions for objects
    Objects[0].pos = glm::vec3(0.0f, 1.0f, 2.0f);
    Objects[1].pos = glm::vec3(-2.0f, 1.0f, 0.0f);
    Objects[2].pos = glm::vec3(2.0f, 1.0f, 0.0f);
    Objects[3].pos = glm::vec3(0.0f, 1.0f, -2.0f);
    Objects[4].pos = glm::vec3(0.1f, 0.0f, 0.0f);

    for (int i = 0; i < 4; ++i) // random velocities
    {
        Objects[i].vel = getRandomVelocity();
    }

    for (int i = 0; i < 4; ++i)
    {
        Objects[i].val = -1.0 + static_cast<double>(rand()) / RAND_MAX * 2;
    }

    for (int i = 0; i < 4; ++i)
    {
        Objects[i].rotationVec = getRandomRotation();
    }

    do
    {
#pragma omp parallel sections
        {
#pragma omp section
            {
                // Measure speed
                double currentTime = glfwGetTime();
                nbFrames++;
                if (currentTime - lastTime >= 1.0)
                { // If last prinf() was more than 1sec ago
                    // printf and reset
                    printf("%f ms/frame\n", 1000.0 / double(nbFrames));
                    nbFrames = 0;
                    lastTime += 1.0;
                }
                // FROM LAB 3 - MOST STAYS THE SAME
                glm::mat4 ModelMatrix1 = glm::mat4(1.0);
                glm::mat4 ModelMatrix2 = glm::mat4(1.0);
                glm::mat4 ModelMatrix3 = glm::mat4(1.0);
                glm::mat4 ModelMatrix4 = glm::mat4(1.0);
                glm::mat4 ModelMatrix5 = glm::mat4(1.0);
                glm::mat4 ModelMatrix6 = glm::mat4(1.0);
                glm::mat4 ModelMatrix7 = glm::mat4(1.0);
                glm::mat4 ModelMatrix8 = glm::mat4(1.0);

                // Clear the screen
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Compute the MVP matrix from keyboard and mouse input
                computeMatricesFromInputs();
                glm::mat4 ProjectionMatrix = getProjectionMatrix();
                glm::mat4 ViewMatrix = getViewMatrix();

                // Check to see if g is pressed
                if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
                {
                    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
                    {
                        beforeG = true;
                    }
                }

                //BEGINNING OF FIRST OBJECT
                glUseProgram(programID);

                if (beforeG) // user presses g and light changes
                {
                    glUniform1i(FirstSignalID, 1); //setting toggleSpecular to 1
                    glUniform3f(LightID, Objects[0].pos.x, Objects[0].pos.y, Objects[0].pos.z);
                    glUniform1i(SignalID, 1); //setting toggleSpecular to 1
                    glUniform1f(PowerID, rand()%100); //setting LightPower to random number
                }

                glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

                // setting up initial positions
                ModelMatrix1 = glm::translate(ModelMatrix1, Objects[0].pos);
                ModelMatrix1 = glm::rotate(ModelMatrix1, glm::radians(0.0f), Objects[0].rotationVec);

                glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

                // Send our transformation to the currently bound shader, k
                // in the "MVP" uniform
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);

                // Bind our texture in Texture Unit 0
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, Texture);
                glUniform1i(TextureID, 0); // Set our "myTextureSampler" sampler to user Texture Unit 0

                // 1rst attribute buffer : vertices
                glEnableVertexAttribArray(vertexPosition_modelspaceID);
                glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                glVertexAttribPointer (vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // 2nd attribute buffer : UVs
                glEnableVertexAttribArray(vertexUVID);
                glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
                glVertexAttribPointer (vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // 3rd attribute buffer : normals
                glEnableVertexAttribArray(vertexNormal_modelspaceID);
                glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
                glVertexAttribPointer (vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // Index buffer
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

                // Draw the triangles!
                glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);

                //BEGINNING OF SECOND OBJECT
                glUseProgram(programID);

                if (beforeG)
                {
                    glUniform1i(FirstSignalID, 1); //setting toggleSpecular to 1
                    glUniform3f(LightID, Objects[1].pos.x, Objects[1].pos.y, Objects[1].pos.z);
                    glUniform1i(SignalID, 1); //setting toggleSpecular to 1
                    glUniform1f(PowerID, rand()%100); //setting LightPower to random number
                }

                glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

                // setting up initial positions
                ModelMatrix2 = glm::translate(ModelMatrix2, Objects[1].pos);
                ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(0.0f), Objects[1].rotationVec);

                glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;

                // Send our transformation to the currently bound shader, k
                // in the "MVP" uniform
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);

                // Bind our texture in Texture Unit 0
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, Texture);
                glUniform1i(TextureID, 0); // Set our "myTextureSampler" sampler to user Texture Unit 0

                // 1rst attribute buffer : vertices
                glEnableVertexAttribArray(vertexPosition_modelspaceID);
                glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                glVertexAttribPointer (vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // 2nd attribute buffer : UVs
                glEnableVertexAttribArray(vertexUVID);
                glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
                glVertexAttribPointer (vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // 3rd attribute buffer : normals
                glEnableVertexAttribArray(vertexNormal_modelspaceID);
                glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
                glVertexAttribPointer (vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // Index buffer
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

                // Draw the triangles!
                glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);

                //BEGINNING OF THIRD OBJECT
                glUseProgram(programID);

                if (beforeG)
                {
                    glUniform1i(FirstSignalID, 1); //setting toggleSpecular to 1
                    glUniform3f(LightID, Objects[2].pos.x, Objects[2].pos.y, Objects[2].pos.z);
                    glUniform1i(SignalID, 1); //setting toggleSpecular to 1
                    glUniform1f(PowerID, rand()%100); //setting LightPower to random number
                }

                glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

                // setting up initial positions
                ModelMatrix3 = glm::translate(ModelMatrix3, Objects[2].pos);
                ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(0.0f), Objects[2].rotationVec);

                glm::mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrix3;

                // Send our transformation to the currently bound shader, k
                // in the "MVP" uniform
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix3[0][0]);

                // Bind our texture in Texture Unit 0
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, Texture);
                glUniform1i(TextureID, 0); // Set our "myTextureSampler" sampler to user Texture Unit 0

                // 1rst attribute buffer : vertices
                glEnableVertexAttribArray(vertexPosition_modelspaceID);
                glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                glVertexAttribPointer (vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // 2nd attribute buffer : UVs
                glEnableVertexAttribArray(vertexUVID);
                glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
                glVertexAttribPointer (vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // 3rd attribute buffer : normals
                glEnableVertexAttribArray(vertexNormal_modelspaceID);
                glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
                glVertexAttribPointer (vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // Index buffer
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

                // Draw the triangles!
                glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);

                //BEGINNING OF FOURTH OBJECT
                glUseProgram(programID);

                if (beforeG)
                {
                    glUniform1i(FirstSignalID, 1); //setting toggleSpecular to 1
                    glUniform3f(LightID, Objects[3].pos.x, Objects[3].pos.y, Objects[3].pos.z);
                    glUniform1i(SignalID, 1); //setting toggleSpecular to 1
                    glUniform1f(PowerID, rand()%100); //setting LightPower to random number
                }

                glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

                // setting up initial positions
                ModelMatrix4 = glm::translate(ModelMatrix4, Objects[3].pos);
                ModelMatrix4 = glm::rotate(ModelMatrix4, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                ModelMatrix4 = glm::rotate(ModelMatrix4, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

                glm::mat4 MVP4 = ProjectionMatrix * ViewMatrix * ModelMatrix4;

                // Send our transformation to the currently bound shader, k
                // in the "MVP" uniform
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP4[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix4[0][0]);

                // Bind our texture in Texture Unit 0
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, Texture);
                glUniform1i(TextureID, 0); // Set our "myTextureSampler" sampler to user Texture Unit 0

                // 1rst attribute buffer : vertices
                glEnableVertexAttribArray(vertexPosition_modelspaceID);
                glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                glVertexAttribPointer (vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // 2nd attribute buffer : UVs
                glEnableVertexAttribArray(vertexUVID);
                glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
                glVertexAttribPointer (vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // 3rd attribute buffer : normals
                glEnableVertexAttribArray(vertexNormal_modelspaceID);
                glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
                glVertexAttribPointer (vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                // Index buffer
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

                // Draw the triangles!
                glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);

                //STATIC OBJECTS ----------------------------------------------------------------------------------------
                // Assuming you have an array of transformation data
                std::vector<std::tuple<glm::vec3, float, glm::mat4*>> transformations =
                        {
                                // translations for each matrix
                                std::make_tuple(glm::vec3(10.0f, 1.0f, 1.0f), glm::radians(90.0f), &ModelMatrix5),
                                std::make_tuple(glm::vec3(-10.0f, 1.0f, 1.0f), glm::radians(-90.0f), &ModelMatrix6),
                                std::make_tuple(glm::vec3(0.0f, 1.0f, 10.0f), glm::radians(0.0f), &ModelMatrix7),
                                std::make_tuple(glm::vec3(0.0f, 1.0f, -10.0f), glm::radians(180.0f), &ModelMatrix8)
                        };

                for (const auto& transform : transformations)  //repeating for each static object
                {
                    glUseProgram(programID);

                    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

                    glm::mat4& currentModelMatrix = *std::get<2>(transform);

                    // Setting up initial positions
                    currentModelMatrix = glm::translate(glm::mat4(1.0f), std::get<0>(transform));
                    currentModelMatrix = glm::rotate(currentModelMatrix, std::get<1>(transform), glm::vec3(0.0f, 1.0f, 0.0f));

                    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * currentModelMatrix;

                    // Send our transformation to the currently bound shader in the "MVP" uniform
                    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &currentModelMatrix[0][0]);

                    // Bind our texture in Texture Unit 0
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, Texture);
                    glUniform1i(TextureID, 0); // Set our "myTextureSampler" sampler to user Texture Unit 0

                    // 1rst attribute buffer : vertices
                    glEnableVertexAttribArray(vertexPosition_modelspaceID);
                    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                    glVertexAttribPointer (vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                    // 2nd attribute buffer : UVs
                    glEnableVertexAttribArray(vertexUVID);
                    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
                    glVertexAttribPointer (vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                    // 3rd attribute buffer : normals
                    glEnableVertexAttribArray(vertexNormal_modelspaceID);
                    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
                    glVertexAttribPointer (vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // attribute to configure, size, type, normalized?, stride, array buffer offset

                    // Index buffer
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

                    // Draw the triangles!
                    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
                }


                //BEGINNING OF RECTANGLE
                glUseProgram(programID);

                // Give rectangle texture
                glActiveTexture(GL_TEXTURE1); // Use a different texture unit, e.g., GL_TEXTURE1
                glBindTexture(GL_TEXTURE_2D, RectangleTexture);
                glUniform1i(TextureID, 1);

                glm::mat4 squareModelMatrix = glm::mat4(1.0);
                squareModelMatrix = glm::translate(squareModelMatrix, glm::vec3(0.1f, 0.0f, 0.0f));
                squareModelMatrix = glm::rotate(squareModelMatrix, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
                glm::mat4 squareMVP = ProjectionMatrix * ViewMatrix * squareModelMatrix;

                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &squareMVP[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &squareModelMatrix[0][0]);

                //repeat process for rectangle object
                glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
                glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //attribute to configure, size, type, normalized?, stride, array buffer offset

                glBindBuffer(GL_ARRAY_BUFFER, squareNorm);
                glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); //attribute to configure, size, type, normalized?, stride, array buffer offset

                glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
                glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //attribute to configure, size, type, normalized?, stride, array buffer offset

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareEBO);
                glDrawArrays(GL_QUADS, 0, 4);
                //END OF  RENDERING RECTANGLE

                glDisableVertexAttribArray(vertexPosition_modelspaceID);
                glDisableVertexAttribArray(vertexUVID);
                glDisableVertexAttribArray(vertexNormal_modelspaceID);

                // Swap buffers
                glfwSwapBuffers(window);
                glfwPollEvents();
            }

#pragma omp parallel for
            for (int i = 0; i < 5; ++i)
            {
                std::unique_lock<std::mutex> lock(mtx, std::defer_lock);

                if (beforeG) // when g is pressed, random movements created for each object
                {
                    if (Objects[i].vel.x + Objects[i].pos.x >= 20.0f || Objects[i].vel.x + Objects[i].pos.x <= -20.0f) // x movements
                    {
                        Objects[i].vel.x = -Objects[i].vel.x;
                        Objects[i].rotationVec.x *= -1;
                        Objects[i].val = -1.0 + static_cast<double>(rand()) / RAND_MAX * 2.0;
                    }
                    if (Objects[i].vel.y + Objects[i].pos.y >= 10.0f || Objects[i].vel.y + Objects[i].pos.y <= 1.0f) // y movements
                    {
                        Objects[i].vel.y = -Objects[i].vel.y;
                        Objects[i].rotationVec.y *= -1;
                        Objects[i].val = -1.0 + static_cast<double>(rand()) / RAND_MAX * 2.0;
                    }
                    if (Objects[i].vel.z + Objects[i].pos.z >= 20.0f || Objects[i].vel.z + Objects[i].pos.z <= -20.0f) // z movements
                    {
                        Objects[i].vel.z = -Objects[i].vel.z;
                        Objects[i].rotationVec.z *= -1;
                        Objects[i].val = -1.0 + static_cast<double>(rand()) / RAND_MAX * 2.0;
                    }

                    // continuously updating position and rotation
                    Objects[i].pos += Objects[i].vel;
                    Objects[i].rotate += Objects[i].val;

                    for (int j = 0; j < 5; ++j)
                    {
                        // check if there was a collision
                        if (i == j)
                        {
                            continue;
                        }

                        float distance = glm::distance(Objects[i].pos, Objects[j].pos); // distance between the 2 objects of interest

                        if (distance < 1.0f)
                        {
                            // computing normalized vector
                            glm::vec3 direction = glm::normalize(Objects[j].pos - Objects[i].pos);
                            glm::vec3 newVec = (1.0f - distance) * direction;

                            // new positions
                            Objects[i].pos -= 0.5f * newVec;
                            Objects[j].pos += 0.5f * newVec;

                            // stop objects on collision
                            Objects[i].vel = glm::vec3(0.0f);
                            Objects[j].vel = glm::vec3(0.0f);
                        }
                    }
                }
            }
        }
    }

        // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}