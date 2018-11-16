#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "FlareMap.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

using namespace std;
//setup-declare objects
SDL_Window* displayWindow;
ShaderProgram program;
ShaderProgram uprogram;

glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
GLuint LoadTexture(const char *filePath);

GLuint sheetSpriteTexture;
GLuint fontTexture;
class Entity;

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER};
GameMode mode;

float animationElapsed = 0.0f;
float framesPerSecond = 10.0f;
int currentIndex = 0;
float accumulator = 0.0f;

std::vector<Entity> entities;

void setup();

//entities
int spriteCountX = 16;
int spriteCountY = 8;

void DrawSpriteSheetSprite(ShaderProgram &program, int index, int spriteCountX,
                           int spriteCountY) {
    float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;
    float v = (float)(((int)index) / spriteCountX) / (float) spriteCountY;
    float spriteWidth = 1.0/(float)spriteCountX;
    float spriteHeight = 1.0/(float)spriteCountY;
    
    float texCoords[] = {
        u, v+spriteHeight,
        u+spriteWidth, v,
        u, v,
        u+spriteWidth, v,
        u, v+spriteHeight,
        u+spriteWidth, v+spriteHeight
    };
    
    float vertices[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f};
    // draw this data
    glBindTexture(GL_TEXTURE_2D, sheetSpriteTexture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}
class Entity {
public:
    void Draw();
    glm::vec3 position;
    glm::vec3 velocity;
    float xvel;
    float yvel;
    float direct;
    glm::vec3 size;
    int spriteIndex;
    
    bool alive=false;
};
void Entity::Draw(){
    modelMatrix=glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix=glm::scale(modelMatrix, size);
    DrawSpriteSheetSprite(program, spriteIndex, spriteCountX, spriteCountY);
}
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
class GameState {
public:
    Entity player;
    Entity platforms[9];
    Entity gameOver;
    bool moveLeft=false;
    bool moveRight=false;
    bool jump=false;
    bool grounded=false;
    GameState(){
        player.velocity=glm::vec3(0.0f,0.0f,0.0f);
        player.spriteIndex=98;
        player.alive=true;
        player.size=glm::vec3(1.0f,1.0f,0.0f);
        player.position=glm::vec3(0.0f,3.0f,0.0f);
        
        gameOver.velocity=glm::vec3(0.0f,0.0f,0.0f);
        gameOver.spriteIndex=80;
        gameOver.alive=true;
        gameOver.size=glm::vec3(1.0f,1.0f,0.0f);
        gameOver.position=glm::vec3(4.0f,2.0f,0.0f);
        
        float xstart=-3.0f;
        int index=0;
        for (int i=0;i<3;i++){
            platforms[index].spriteIndex=32;
            platforms[index].alive=true;
            platforms[index].size=glm::vec3(1.0f,1.0f,0.0f);
            platforms[index].position=glm::vec3(xstart,-1.0f,0.0f);
            index++;
            xstart+=1.0f;
        }
        for (int i=0;i<3;i++){
            platforms[index].spriteIndex=32;
            platforms[index].alive=true;
            platforms[index].size=glm::vec3(1.0f,1.0f,0.0f);
            platforms[index].position=glm::vec3(xstart,0.0f,0.0f);
            index++;
            xstart+=1.0f;
        }
        for (int i=0;i<3;i++){
            platforms[index].spriteIndex=114;
            platforms[index].alive=false;
            platforms[index].size=glm::vec3(1.0f,1.0f,0.0f);
            platforms[index].position=glm::vec3(xstart,1.0f,0.0f);
            index++;
            xstart+=0.6f;
        }
    }
    
};

void RenderGame(GameState &state) {
    
    modelMatrix=glm::mat4(1.0f);
    state.player.Draw();
    
    modelMatrix=glm::mat4(1.0f);
    for (int i=0;i<9;i++){
        state.platforms[i].Draw();
    }
    modelMatrix=glm::mat4(1.0f);
    state.gameOver.Draw();
    
}
void checkCollision(GameState &state){
    float gxcol=fabs(state.player.position[0]-state.gameOver.position[0])-1;
    float gycol=fabs(state.player.position[1]-state.gameOver.position[1])-1;
    if (gxcol<0&&gycol<0){
        mode=STATE_GAME_OVER;
    }
    
    for (Entity i:state.platforms){
        if (i.alive){
            float xcol=fabs(state.player.position[0]-i.position[0])-1;
            float ycol=fabs(state.player.position[1]-i.position[1])-1;
            if (xcol<0&&ycol<0&&state.player.position[0]+0.5f>i.position[0]-0.5f&&state.player.position[1]-0.5f>=i.position[1]+0.5f){
                state.player.position[0]-=fabs(state.player.position[0]-i.position[0]-1);
            }
            if (xcol<0&&ycol<0&&state.player.position[0]-0.5f<i.position[0]+0.5f&&state.player.position[1]-0.5f>=i.position[1]+0.5f){

                state.player.position[0]+=fabs(state.player.position[0]-i.position[0]-1);
            }
            if (ycol<0&&xcol<0){
                //state.player.position[1]+=0.01f;
                state.grounded=true;
            }
        }
        
    }
}
void collisionX(GameState &state){
    for (Entity i:state.platforms){
        if (i.alive){
            float xcol=fabs(state.player.position[0]-i.position[0])-1;
            float ycol=fabs(state.player.position[1]-i.position[1])-1;
            if (xcol<0&&ycol<0&&state.player.position[0]+0.5f>i.position[0]-0.5f&&state.player.position[1]-0.5f>=i.position[1]+0.5f){
                state.player.position[0]-=fabs(state.player.position[0]-i.position[0]-1);
            }
            if (xcol<0&&ycol<0&&state.player.position[0]-0.5f<i.position[0]+0.5f&&state.player.position[1]-0.5f>=i.position[1]+0.5f){
                
                state.player.position[0]+=fabs(state.player.position[0]-i.position[0]-1);
            }
            if (ycol<0&&xcol<0){
                //state.player.position[1]+=0.01f;
                state.grounded=true;
            }
        }
        
    }
}
void collisionY(GameState &state){
    for (Entity i:state.platforms){
        if (i.alive){
            float xcol=fabs(state.player.position[0]-i.position[0])-1;
            float ycol=fabs(state.player.position[1]-i.position[1])-1;
            if (xcol<0&&ycol<0&&state.player.position[0]+0.5f>i.position[0]-0.5f&&state.player.position[1]-0.5f>=i.position[1]+0.5f){
                state.player.position[0]-=5+fabs(state.player.position[0]-i.position[0]-1);
            }
            if (xcol<0&&ycol<0&&state.player.position[0]-0.5f<i.position[0]+0.5f&&state.player.position[1]-0.5f>=i.position[1]+0.5f){
                
                state.player.position[0]+=fabs(state.player.position[0]-i.position[0]-1);
            }
            if (ycol<0&&xcol<0){
                //state.player.position[1]+=0.01f;
                state.grounded=true;
            }
        }
        
    }
}
float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}
void UpdateGame(GameState &state, float elapsed) {
    projectionMatrix = glm::ortho(state.player.position[0]-7.1f,state.player.position[0]+7.1f,state.player.position[1] -4.0f, state.player.position[1]+4.0f, -1.0f, 1.0f);
    state.grounded=false;
    checkCollision(state);
    state.player.xvel=0.0f;
    state.player.yvel=0.0f;
    if (state.moveLeft){
        state.player.xvel=-3.4f;
        //state.player.velocity+=glm::vec3(-2.4f,0,0);

        
    }
    else if (state.moveRight){
        state.player.xvel=3.4f;
        //state.player.velocity+=glm::vec3(2.4f,0,0);
        
    }
    if (state.jump){
        state.player.yvel=20.0f;
        //state.player.velocity+=glm::vec3(0,5,0);
    }
    checkCollision(state);
    state.player.xvel = lerp(state.player.xvel, 0.0f, elapsed * 5.0f);
    state.player.velocity=glm::vec3(state.player.xvel,state.player.yvel,0);
    if (!state.grounded){
        glm::vec3 gravity= glm::vec3(0.0f,-400.0f,0.0f);
        state.player.velocity+=gravity*elapsed;
    }
    state.player.position+=state.player.velocity*elapsed;
    

}

void RenderMainMenu(){
    modelMatrix=glm::mat4(1.0f);

    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.7f,0.3f,0.0f));
    DrawText(program, fontTexture, "PLATFORMER DEMO", 0.5f, 0.0001f);
    program.SetModelMatrix(modelMatrix);
    
    modelMatrix=glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.7f,-0.4f,0.0f));
    DrawText(program, fontTexture, "Press Space", 0.4f, 0.0001f);
    program.SetModelMatrix(modelMatrix);
}
void UpdateGameOver(float elapsed){
    //no updates
}
void UpdateMainMenu(float elapsed){
    //no updates
}
void RenderGameOver(){
    modelMatrix=glm::mat4(1.0f);
    //program.SetModelMatrix(modelMatrix);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.7f,0.3f,0.0f));
    DrawText(program, fontTexture, "GAME OVER", 0.6f, 0.0001f);
    program.SetModelMatrix(modelMatrix);
}

GameState state;
void Render() {
    switch(mode) {
        case STATE_MAIN_MENU:
            RenderMainMenu();
            break;
        case STATE_GAME_LEVEL:
            RenderGame(state);
            break;
        case STATE_GAME_OVER:
            RenderGameOver();
            break;
    } }
void Update(float elapsed) {
    switch(mode) {
        case STATE_MAIN_MENU:
            UpdateMainMenu(elapsed);
            break;
        case STATE_GAME_LEVEL:
            UpdateGame(state, elapsed);
            break;
        case STATE_GAME_OVER:
            UpdateGameOver(elapsed);
            break;
    } }

int main(int argc, char *argv[])
{
    setup();
    float lastFrameTicks = 0.0f;
    mode=STATE_MAIN_MENU;
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue;
        }
        while(elapsed >= FIXED_TIMESTEP) {
            Update(FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }
        accumulator = elapsed;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    if (mode==STATE_MAIN_MENU){
                        mode=STATE_GAME_LEVEL;
                    }
                    else if (mode==STATE_GAME_LEVEL){
                        state.jump=true;
                    }
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_LEFT){
                    state.moveLeft=true;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_RIGHT){
                    
                    state.moveRight=true;
                    
                }
            }
            else if(event.type == SDL_KEYUP) {
                
                if (event.key.keysym.scancode==SDL_SCANCODE_LEFT){
                    state.moveLeft=false;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_RIGHT){
                    state.moveRight=false;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_SPACE){
                    state.jump=false;
                }
            }
        }
        //inloop setup
        glClear(GL_COLOR_BUFFER_BIT);
        
        program.SetColor(0.0f, 0.0f, 0.0f, 1.0f);
        Update(elapsed);
        Render();
        
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

void setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    //glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    glClearColor(0.52f, 0.8f, 0.92f, 1.0f);
    program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl" ) ;
    uprogram.Load(RESOURCE_FOLDER "vertex.glsl", RESOURCE_FOLDER "fragment.glsl" ) ;
    //initialize objects out of loop
    //program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl" ) ;
    //GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER "emoji.png");
    sheetSpriteTexture = LoadTexture(RESOURCE_FOLDER "arne_sprites.png");
    fontTexture=LoadTexture(RESOURCE_FOLDER "font1.png");
    projectionMatrix = glm::ortho(-7.1f, 7.1f, -4.0f, 4.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    glUseProgram(program.programID);
    glUseProgram(uprogram.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


