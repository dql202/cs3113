#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

//setup-declare objects out of main
SDL_Window* displayWindow;
ShaderProgram program;
ShaderProgram uprogram;

glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
GLuint LoadTexture(const char *filePath);



int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Assignment 1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    
    //initialize objects out of loop
    
    program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl" ) ;
    uprogram.Load(RESOURCE_FOLDER "vertex.glsl", RESOURCE_FOLDER "fragment.glsl" ) ;
    GLuint shipTexture = LoadTexture(RESOURCE_FOLDER "falcon.png");
    GLuint starTexture = LoadTexture(RESOURCE_FOLDER "dstar.png");
    GLuint headTexture=LoadTexture(RESOURCE_FOLDER "c3.png");
    
    //Ortho Options
    //projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    //projectionMatrix = glm::ortho(-3.554f, 3.554f, -2.0f, 2.0f, -1.0f, 1.0f);
    projectionMatrix = glm::ortho(-5.333f, 5.333f, -3.0f, 3.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    glUseProgram(uprogram.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float lastFrameTicks = 0.0f;
    float angle = 45.0f * (3.1415926f / 180.0f);
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        //inloop setup
        glClear(GL_COLOR_BUFFER_BIT);
        uprogram.SetColor(0.0f, 0.0f, 0.8f, 1.0f);

        
        //timer
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        //in-loop operations
        
        //Moving Falcon
        program.SetModelMatrix(modelMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        glBindTexture(GL_TEXTURE_2D, shipTexture);
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        //translation and animated rotation
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(1.5f,0.0f,0.0f));
        angle += elapsed;
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f,1.0f,0.0f));
        program.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        
        //Death Star
        glBindTexture(GL_TEXTURE_2D, starTexture);
        float vertices2[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords2[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
        glEnableVertexAttribArray(program.texCoordAttribute);
        //translation and scale
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(1.2f,0.0f,0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
        program.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //Untextured Square 1
        modelMatrix=glm::mat4(1.0f);
        uprogram.SetModelMatrix(modelMatrix);
        uprogram.SetProjectionMatrix(projectionMatrix);
        uprogram.SetViewMatrix(viewMatrix);
        float vertices4[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(uprogram.positionAttribute, 2, GL_FLOAT, false, 0, vertices4);
        glEnableVertexAttribArray(uprogram.positionAttribute);
        //translation
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-3.0f,0.0f,0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
        uprogram.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(uprogram.positionAttribute);
        
        //Change Color
        uprogram.SetColor(0.8f, 0.0f, 0.0f, 1.0f);
        //Untextured Square 2
        modelMatrix=glm::mat4(1.0f);
        uprogram.SetModelMatrix(modelMatrix);
        uprogram.SetProjectionMatrix(projectionMatrix);
        uprogram.SetViewMatrix(viewMatrix);
        float vertices5[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(uprogram.positionAttribute, 2, GL_FLOAT, false, 0, vertices5);
        glEnableVertexAttribArray(uprogram.positionAttribute);
        //translation
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-3.0f,0.0f,0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.8f, 1.8f, 1.0f));
        uprogram.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(uprogram.positionAttribute);
        
        //C3PO Head
        glBindTexture(GL_TEXTURE_2D, headTexture);
        float vertices3[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords3[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3);
        glEnableVertexAttribArray(program.texCoordAttribute);
        //translation
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-3.0f,0.0f,0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
        program.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    SDL_Quit();
    return 0;
}

GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}
