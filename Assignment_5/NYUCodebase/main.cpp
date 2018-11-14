#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include <vector>
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
int Mix_OpenAudio(int frequency, Uint16 format, int channels,
                  int chunksize);

glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
GLuint fontTexture;
GLuint LoadTexture(const char *filePath);

void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) {
    float character_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x + character_size, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x, texture_y + character_size,
        });
    }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    // draw this data (use the .data() method of std::vector to get pointer to data)
    // draw this yourself, use text.size() * 6 or vertexData.size()/2 to get number of vertices
    //modelMatrix=glm::mat4(1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program.SetModelMatrix(modelMatrix);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, text.size()*6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Assignment 5", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
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
    fontTexture=LoadTexture(RESOURCE_FOLDER "font1.png");
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
    
    //Load music and sounds
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    Mix_Chunk *sound1;
    sound1=Mix_LoadWAV("sound1.wav");
    Mix_Chunk *sound2;
    sound2=Mix_LoadWAV("sound2.wav");
    Mix_Music *music;
    music=Mix_LoadMUS("music.mp3");
    //start music
    Mix_PlayMusic(music, -1);
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            //if keys are pressed, play sounds
            else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    Mix_PlayChannel( -1, sound1, 0);
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_Q){
                    Mix_PlayChannel( -1, sound2, 0);
                }

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
        
        //Untextured Program
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
        
        modelMatrix=glm::mat4(1.0f);
        
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.7f,-1.7f,0.0f));
        DrawText(program, fontTexture, "Press Q", 0.3f, 0.0001f);
        program.SetModelMatrix(modelMatrix);
        
        modelMatrix=glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.7f,-1.3f,0.0f));
        DrawText(program, fontTexture, "Press Space", 0.3f, 0.0001f);
        program.SetModelMatrix(modelMatrix);
        
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
