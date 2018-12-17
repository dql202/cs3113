#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
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
int graphicsIndex;

glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
GLuint LoadTexture(const char *filePath);

GLuint sheetSpriteTexture;
GLuint sheetSpriteTexture2;
GLuint fontTexture;
class Entity;

int Mix_OpenAudio(int frequency, Uint16 format, int channels,
                  int chunksize);

Mix_Chunk *sound1;
Mix_Chunk *sound2;

int levelTracker=1;

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL1,STATE_GAME_LEVEL2,STATE_GAME_LEVEL3, STATE_GAME_OVER,STATE_NEXT_LEVEL};
GameMode mode;
class GameState;
//GameState state;

float animationElapsed = 0.0f;
float framesPerSecond = 10.0f;
int currentIndex = 0;
float accumulator = 0.0f;

float screenShakeValue=1.2f;
float screenShakeSpeed=1.2f;
float screenShakeIntensity=100.2f;

float bulletSpeed=4;

glm::vec3 bulletVelocity[4]={glm::vec3(0,-bulletSpeed,0),glm::vec3(-bulletSpeed,0,0),glm::vec3(bulletSpeed,0,0),glm::vec3(0,bulletSpeed,0)};

std::vector<Entity> entities;

void setup();

//entities
int spriteCountX = 12;
int spriteCountY = 8;
int spriteCountX2 = 16;
int spriteCountY2 = 8;
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
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
void DrawSpriteSheetSprite2(ShaderProgram &program, int index, int spriteCountX,
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
    glBindTexture(GL_TEXTURE_2D, sheetSpriteTexture2);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
class Platform {
public:
    bool alive=false;
    void Draw();
    glm::vec3 position;
    glm::vec3 size;
    int spriteIndex;
};

void Platform::Draw(){
    modelMatrix=glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix=glm::scale(modelMatrix, size);
    DrawSpriteSheetSprite2(program, spriteIndex, spriteCountX2, spriteCountY2);
}

class Entity {
public:
    bool CheckCol(Entity &e);
    bool CheckCol(Platform &p);
    void Draw();
    glm::vec3 position;
    glm::vec3 velocity;
    float xvel;
    float yvel;
    float direct;
    glm::vec3 size;
    int spriteBase;
    int spriteIndex;
    int spriteDirection;
    std::vector<std::vector<int>> runAnimation;
    bool alive;
    bool left;
    bool right;
    bool top;
    bool bottom;
    Entity(){
        
    }
};

class Player:public Entity{
public:
    void Shoot();
    bool collide(Platform &p);
    bool collideX(Platform &p);
    bool collideY(Platform &p);
    bool moveLeft=false;
    bool moveRight=false;
    bool moveUp=false;
    bool moveDown=false;
    Player(int si){
        velocity=glm::vec3(0.0f,0.0f,0.0f);
        size=glm::vec3(1.0f,1.0f,0.0f);
        alive=true;
        spriteBase=si;
        spriteIndex=si;
        spriteDirection=0;
        runAnimation.push_back({si, si+2});
        runAnimation.push_back({si+12,si+14});
        runAnimation.push_back({si+24,si+26});
        runAnimation.push_back({si+36,si+38});
    }
};

bool Player::collide(Platform &p){
    float xcol=fabs(position[0]-p.position[0])-1;
    float ycol=fabs(position[1]-p.position[0])-1;
    
    return (xcol<=0&&ycol<=0);
    if (xcol<=0&&ycol<=0){
        printf("collided\n");
    }

}

bool Player::collideX(Platform &p){
    if (collide(p)){
        float xcol=fabs(position[0]-p.position[0])-1;
        return true;
    }
    else{
        return false;
    }
}
bool Player::collideY(Platform &p){
    if (collide(p)){
        float ycol=fabs(position[1]-p.position[1])-1;
        return true;
    }
    else{
        return false;
    }

}



class Enemy:public Entity{
public:
    Enemy(int si,float x,float y){
        velocity=glm::vec3(x,y,0.0f);
        size=glm::vec3(1.0f,1.0f,0.0f);
        alive=true;
        spriteBase=si;
        spriteIndex=si;
        spriteDirection=0;
        runAnimation.push_back({si, si+2});
        runAnimation.push_back({si+12,si+14});
        runAnimation.push_back({si+24,si+26});
        runAnimation.push_back({si+36,si+38});
    }
};

class Bullet:public Entity{
public:
    void Draw();
    bool player1;
    Bullet(){
        size=glm::vec3(1.0f,1.0f,0.0f);
        spriteIndex=40;
        alive=true;
    }
};
//draw bullet
void Bullet::Draw(){
    modelMatrix=glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix=glm::scale(modelMatrix, size);
    DrawSpriteSheetSprite2(program, spriteIndex, spriteCountX2, spriteCountY2);
    
}

//draw Entity
void Entity::Draw(){
    modelMatrix=glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix=glm::scale(modelMatrix, size);
    DrawSpriteSheetSprite(program, spriteIndex, spriteCountX, spriteCountY);
}

//draw Text
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
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

//GameState class
class GameState {
public:
    bool versus=false;
    bool l2=false;
    Player player1=Player(9);
    Player player2=Player(48);
    vector<Bullet> bullets;
    vector<Platform> platforms;
    vector<Platform> edges;
    vector<Platform> blocks;
    vector<Enemy> enemies;
    Entity gameOver;
    bool moveLeft=false;
    bool moveRight=false;
    bool moveUp=false;
    bool moveDown=false;
    bool moveLeft2=false;
    bool moveRight2=false;
    bool moveUp2=false;
    bool moveDown2=false;
    int gi=3;
    int vi=1;
    GameState(){
        player1.position=glm::vec3(1.0f,0.0f,0.0f);
        player2.position=glm::vec3(-0.1f,0.0f,0.0f);

        float y_start=4.0f;
        for (int ys=0;ys<9;ys++){
            float xstart=-7.0f;
            
            for (int i=0;i<15;i+=2){
                Platform newPlat1,newPlat2;
                newPlat1.spriteIndex=106;
                newPlat1.size=glm::vec3(1.0f,1.0f,0.0f);
                newPlat1.position=glm::vec3(xstart,y_start,0.0f);
                platforms.push_back(newPlat1);
                //index++;
                xstart+=1.0f;
                newPlat2.spriteIndex=107;
                newPlat2.size=glm::vec3(1.0f,1.0f,0.0f);
                newPlat2.position=glm::vec3(xstart,y_start,0.0f);
                platforms.push_back(newPlat2);
                //index++;
                xstart+=1.0f;
            }
            y_start-=1.0;
        }
        y_start=4.0f;
        float xstart=-7.0f;
        for (int i=0;i<15;i++){
            Platform edge;
            edge.alive=true;
            edge.spriteIndex=3;
            edge.size=glm::vec3(1.0f,1.0f,0.0f);
            edge.position=glm::vec3(xstart,4,0.0f);
            edges.push_back(edge);
            //index++;
            //xstart+=1.0f;
            edge.size=glm::vec3(1.0f,1.0f,0.0f);
            edge.position=glm::vec3(xstart,-4,0.0f);
            edges.push_back(edge);
            //index++;
            xstart+=1.0f;
        }
        for (int i=0;i<15;i++){
            Platform edge;
            edge.spriteIndex=3;
            edge.size=glm::vec3(1.0f,1.0f,0.0f);
            edge.position=glm::vec3(7.1,y_start,0.0f);
            edges.push_back(edge);
            //index++;
            //xstart+=1.0f;
            edge.size=glm::vec3(1.0f,1.0f,0.0f);
            edge.position=glm::vec3(-7.1,y_start,0.0f);
            edges.push_back(edge);
            //index++;
            y_start-=1.0f;
        }
        
    }
    
    void level1(){
        Enemy newEnemy= Enemy(3,0,0);
        newEnemy.position=glm::vec3(-2.0,0.0f,0.0f);
        enemies.push_back(newEnemy);
    }
    void level2(){
        l2=true;
        float start=-6.1f;
        int initidirect=2;
        for (int i=0;i<2;i++){
            for (int j=0;j<4;j++){
                float r3 = -3.0 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(6)));
                Enemy newEnemy= Enemy(gi,vi,r3);
                newEnemy.position=glm::vec3(start,0.0f,0.0f);
                newEnemy.spriteDirection=initidirect;
                enemies.push_back(newEnemy);
            }
            start=6.1f;
            initidirect=1;
            vi=-1;
        }
    }
    void level3(){
        for (int i=0;i<platforms.size();i++){
            platforms[i].spriteIndex=35;
        }
        float startx=3;
        float starty=3;
        glm::vec3 ssize=glm::vec3(0.9f,0.9f,0.9f);
        for (int i=0;i<3;i++){
            Platform edge;
            edge.spriteIndex=49;
            edge.size=ssize;
            edge.position=glm::vec3(startx,starty,0.0f);
            blocks.push_back(edge);
            //index++;
            //xstart+=1.0f;
            edge.size=ssize;
            edge.position=glm::vec3(-startx,-starty,0.0f);
            blocks.push_back(edge);
            //edge.spriteIndex=32;
            edge.size=ssize;
            edge.position=glm::vec3(-startx,starty,0.0f);
            blocks.push_back(edge);
            //index++;
            //xstart+=1.0f;
            edge.size=ssize;
            edge.position=glm::vec3(startx,-starty,0.0f);
            blocks.push_back(edge);
            //index++;
            starty--;
            startx--;
        }
        versus=true;
        player1.position=glm::vec3(5.7,2.8,0);
        player2.position=glm::vec3(-5.7,-2.8,0);
    }
};

//Collision Functions
float velocityChange= -1.01f;

void collideEnemy(Enemy &e,Platform &ed){
    float dx=(e.position[0]+1/2)-(ed.position[0]+1/2);
    float dy=(e.position[1]+1/2)-(ed.position[1]+1/2);
    float width=1;
    float height=1;
    float crossWidth=width*dy;
    float crossHeight=height*dx;
    int collision=-1;
    if(fabs(dx)<=width && fabs(dy)<=height){
        if(crossWidth>crossHeight){
            collision=(crossWidth>(-crossHeight))?0:2; //bottom, left
        }else{
            collision=(crossWidth>-(crossHeight))?3:1; //right top.
        }
    }
    if (collision==1){
        e.position-=glm::vec3(0,.1,0);
        e.velocity[0]*=-velocityChange;
        e.velocity[1]*=velocityChange;
    }
    else if (collision==3){
        e.position+=glm::vec3(.1,0,0);
        e.spriteDirection=2;
        e.velocity[0]*=velocityChange;
        e.velocity[1]*=-velocityChange;
    }
    else if (collision==0){
        e.position+=glm::vec3(0,.1,0);
        e.velocity[1]*=velocityChange;
        e.velocity[0]*=-velocityChange;
    }
    else if (collision==2){
        e.position-=glm::vec3(.1,0,0);
        e.spriteDirection=1;
        e.velocity[0]*=velocityChange;
        e.velocity[1]*=-velocityChange;
        
    }
    else{
        //printf("none\n");
    }
}
//Check Bullets and Platforms
void checkCollision(GameState &state,float elapsed){
    
    for (Enemy &i:state.enemies){
        for (Bullet &a:state.bullets){
            if (i.alive){
                if (a.alive){
                    float bxcol=fabs(a.position[0]-i.position[0])-.5;
                    float bycol=fabs(a.position[1]-i.position[1])-.5;
                    if ((bxcol<0&&bycol<0)){
                        printf("test\n");
                        i.alive=false;
                        a.alive=false;
                        
                        Mix_PlayChannel( -1, sound2, 0);
                    }
                }
            }
            
            
        }
        if (i.alive){
            float gxcol=fabs(state.player1.position[0]-i.position[0])-.5;
            float gycol=fabs(state.player1.position[1]-i.position[1])-.5;
            float gxcol2=fabs(state.player2.position[0]-i.position[0])-.5;
            float gycol2=fabs(state.player2.position[1]-i.position[1])-.5;
            if ((gxcol<0&&gycol<0)||(gxcol2<0&&gycol2<0)){
                viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, sin(screenShakeValue *screenShakeSpeed)* screenShakeIntensity, 0.0f));
                viewMatrix=glm::mat4(1.0f);
                SDL_Delay(900);
                if (levelTracker>3){
                    
                    mode=STATE_GAME_OVER;
                    
                }
                else{
                    mode=STATE_NEXT_LEVEL;
                    //levelTracker++;
                }
                
            }
        }
        
    }
    for (Platform &i:state.edges){
        state.player1.CheckCol(i);
        //collidePlayer(state.player1, i);
        for (Enemy &a:state.enemies){
            collideEnemy(a, i);
        }
        for (Bullet &b:state.bullets){
            float bxcol=fabs(b.position[0]-i.position[0])-.5;
            float bycol=fabs(b.position[1]-i.position[1])-.5;
            if ((bxcol<0&&bycol<0)){
                
                b.alive=false;

            }
        }
        
    }
    for (Platform &i:state.blocks){
        state.player1.CheckCol(i);
        //collidePlayer(state.player1, i);
        for (Enemy &a:state.enemies){
            collideEnemy(a, i);
        }
        for (Bullet &b:state.bullets){
            float bxcol=fabs(b.position[0]-i.position[0])-.5;
            float bycol=fabs(b.position[1]-i.position[1])-.5;
            if ((bxcol<0&&bycol<0)){
                
                b.alive=false;
                
            }
        }
        
    }
    
    if (state.versus==true){
        for (Bullet &a:state.bullets){
            if (a.alive){
                float bxcol=fabs(a.position[0]-state.player1.position[0])-.5;
                float bycol=fabs(a.position[1]-state.player1.position[1])-.5;
                float bxcol2=fabs(a.position[0]-state.player2.position[0])-.5;
                float bycol2=fabs(a.position[1]-state.player2.position[1])-.5;
                if ((bxcol<0&&bycol<0)&&a.player1==false){
                    
                    a.alive=false;
                    
                    Mix_PlayChannel( -1, sound2, 0);
                    mode=STATE_GAME_OVER;
                }
                else if (((bxcol2<0&&bycol2<0)&&a.player1==true)){
                    a.alive=false;
                    
                    Mix_PlayChannel( -1, sound2, 0);
                    mode=STATE_GAME_OVER;
                }
            }
        }
    }

    
}
//Check if Player is at the edge or at a barrier
void playercolX(GameState &state){
    for (Platform &e:state.edges){
        bool isCol=false;
        float xcol=fabs(state.player1.position[0]-e.position[0])-.9;
        float ycol=fabs(state.player1.position[1]-e.position[1])-.9;

        if (xcol<=0&&ycol<=0){
            isCol=true;
            //printf("collided\n");
        }
        if (isCol){
            float pen=fabs(xcol)+.001;
            if (state.player1.velocity[0]<0){
                state.player1.position[0]+=pen;
                state.player1.velocity[0]=0;
                state.player1.moveLeft=false;
            }
            else if (state.player1.velocity[0]>0){
                state.player1.position[0]-=pen;
                state.player1.velocity[0]=0;
                state.player1.moveRight=false;
            }
        }
    }
    for (Platform &e:state.blocks){
        bool isCol=false;
        float xcol=fabs(state.player1.position[0]-e.position[0])-.6;
        float ycol=fabs(state.player1.position[1]-e.position[1])-.6;
        
        if (xcol<=0&&ycol<=0){
            isCol=true;
            //printf("collided\n");
        }
        if (isCol){
            float pen=fabs(xcol)+.001;
            if (state.player1.velocity[0]<0){
                state.player1.position[0]+=pen;
                state.player1.velocity[0]=0;
                state.player1.moveLeft=false;
            }
            else if (state.player1.velocity[0]>0){
                state.player1.position[0]-=pen;
                state.player1.velocity[0]=0;
                state.player1.moveRight=false;
            }
        }
    }
}

void playercolY(GameState &state){
    for (Platform &e:state.edges){
        bool isCol=false;
        float xcol=fabs(state.player1.position[0]-e.position[0])-.9;
        float ycol=fabs(state.player1.position[1]-e.position[1])-.9;

        if (xcol<=0&&ycol<=0){
            isCol=true;
            //printf("collided\n");
        }
        if (isCol){
            float pen=fabs(ycol)+.001;
            if (state.player1.velocity[1]<0){
                state.player1.position[1]+=pen;
                state.player1.velocity[1]=0;
                state.player1.moveUp=false;
            }
            else if (state.player1.velocity[1]>0){
                state.player1.position[1]-=pen;
                state.player1.velocity[1]=0;
                state.player1.moveDown=false;
            }
        }
    }
    for (Platform &e:state.blocks){
        bool isCol=false;
        float xcol=fabs(state.player1.position[0]-e.position[0])-.6;
        float ycol=fabs(state.player1.position[1]-e.position[1])-.6;
        

        if (xcol<=0&&ycol<=0){
            isCol=true;
            //printf("collided\n");
        }
        if (isCol){
            float pen=fabs(ycol)+.001;
            if (state.player1.velocity[1]<0){
                state.player1.position[1]+=pen;
                state.player1.velocity[1]=0;
                state.player1.moveUp=false;
            }
            else if (state.player1.velocity[1]>0){
                state.player1.position[1]-=pen;
                state.player1.velocity[1]=0;
                state.player1.moveDown=false;
            }
        }
    }
    
}

void playercolX2(GameState &state){
    for (Platform &e:state.edges){
        bool isCol=false;
        float xcol=fabs(state.player2.position[0]-e.position[0])-.9;
        float ycol=fabs(state.player2.position[1]-e.position[1])-.9;
        
        if (xcol<=0&&ycol<=0){
            isCol=true;
            //printf("collided\n");
        }
        if (isCol){
            float pen=fabs(xcol)+.001;
            if (state.player2.velocity[0]<0){
                state.player2.position[0]+=pen;
                state.player2.velocity[0]=0;
                state.player2.moveLeft=false;
            }
            else if (state.player2.velocity[0]>0){
                state.player2.position[0]-=pen;
                state.player2.velocity[0]=0;
                state.player2.moveRight=false;
            }
        }
    }
    for (Platform &e:state.blocks){
        bool isCol=false;
        float xcol=fabs(state.player2.position[0]-e.position[0])-.6;
        float ycol=fabs(state.player2.position[1]-e.position[1])-.6;
        

        if (xcol<=0&&ycol<=0){
            isCol=true;
            //printf("collided\n");
        }
        if (isCol){
            float pen=fabs(xcol)+.001;
            if (state.player2.velocity[0]<0){
                state.player2.position[0]+=pen;
                state.player2.velocity[0]=0;
                state.player2.moveLeft=false;
            }
            else if (state.player2.velocity[0]>0){
                state.player2.position[0]-=pen;
                state.player2.velocity[0]=0;
                state.player2.moveRight=false;
            }
        }
    }
}

void playercolY2(GameState &state){
    for (Platform &e:state.edges){
        bool isCol=false;
        float xcol=fabs(state.player2.position[0]-e.position[0])-.9;
        float ycol=fabs(state.player2.position[1]-e.position[1])-.9;
        

        if (xcol<=0&&ycol<=0){
            isCol=true;
            //printf("collided\n");
        }
        if (isCol){
            float pen=fabs(ycol)+.001;
            if (state.player2.velocity[1]<0){
                state.player2.position[1]+=pen;
                state.player2.velocity[1]=0;
                state.player2.moveUp=false;
            }
            else if (state.player2.velocity[1]>0){
                state.player2.position[1]-=pen;
                state.player2.velocity[1]=0;
                state.player2.moveDown=false;
            }
        }
    }
    for (Platform &e:state.blocks){
        bool isCol=false;
        float xcol=fabs(state.player2.position[0]-e.position[0])-.6;
        float ycol=fabs(state.player2.position[1]-e.position[1])-.6;

        if (xcol<=0&&ycol<=0){
            isCol=true;
            //printf("collided\n");
        }
        if (isCol){
            float pen=fabs(ycol)+.001;
            if (state.player2.velocity[1]<0){
                state.player2.position[1]+=pen;
                state.player2.velocity[1]=0;
                state.player2.moveUp=false;
            }
            else if (state.player2.velocity[1]>0){
                state.player2.position[1]-=pen;
                state.player2.velocity[1]=0;
                state.player2.moveDown=false;
            }
        }
    }
    
}


bool Entity::CheckCol(Platform &p){
    float gxcol=fabs(position[0]-p.position[0])-1;
    float gycol=fabs(position[1]-p.position[1])-1;
    if (gxcol<=0&&gycol<=0){
        return true;
        printf("Collided\n");
    }
    else{
        return false;
    }
}


void  checkCollisionEnemies(GameState &state){
    for (Enemy &a:state.enemies){
        if (a.position[0]>6.6){
            a.velocity[0]*=velocityChange;
        }
        if (a.position[0]<-6.6){
            a.velocity[0]*=velocityChange;
        }
        if (a.position[1]<3.5){
            a.velocity[1]*=velocityChange;
        }
        if (a.position[1]<-3.5){
            a.velocity[1]*=velocityChange;
        }
    }

}


//Update Functions
void RenderGame(GameState &state) {
    
    modelMatrix=glm::mat4(1.0f);
    for (int i=0;i<state.platforms.size();i++){
        state.platforms[i].Draw();
    }
    for (int i=0;i<state.enemies.size();i++){
        if (state.enemies[i].alive){
            state.enemies[i].Draw();
        }
    }
    modelMatrix=glm::mat4(1.0f);
    for (int i=0;i<state.edges.size();i++){
        state.edges[i].Draw();
    }
    modelMatrix=glm::mat4(1.0f);
    for (int i=0;i<state.bullets.size();i++){
        if (state.bullets[i].alive){
            state.bullets[i].Draw();
        }
        
    }
    modelMatrix=glm::mat4(1.0f);
    for (int i=0;i<state.blocks.size();i++){
        state.blocks[i].Draw();
        
    }
    modelMatrix=glm::mat4(1.0f);
    state.player1.Draw();
    state.player2.Draw();
    //checkCollision(state);
    
//    modelMatrix=glm::mat4(1.0f);
//    state.gameOver.Draw();
    
}

bool checkEnemies(GameState &state){
    for (Enemy &i:state.enemies){
        if(i.alive){
            return false;
        }
    }
    return true;
}

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

void UpdateGame(GameState &state, float elapsed) {

   
    state.player1.spriteIndex=state.player1.runAnimation[state.player1.spriteDirection][graphicsIndex];
    state.player2.spriteIndex=state.player2.runAnimation[state.player2.spriteDirection][graphicsIndex];

    //Player 1 Movements
    state.player1.xvel=0.0f;
    state.player1.yvel=0.0f;
    if (state.moveLeft){
        
        state.player1.spriteDirection=1;
        state.player1.xvel=-3.4f;
        //state.player.velocity+=glm::vec3(-2.4f,0,0);
        
    }
    else if (state.moveRight){
        state.player1.spriteDirection=2;
        state.player1.xvel=3.4f;
        //state.player.velocity+=glm::vec3(2.4f,0,0);
        
    }
    if (state.moveUp){
        state.player1.spriteDirection=3;
        state.player1.yvel=3.4f;
        //state.player.velocity+=glm::vec3(0,5,0);
    }
    if (state.moveDown){
        state.player1.spriteDirection=0;
        state.player1.yvel=-3.4f;
        //state.player.velocity+=glm::vec3(0,5,0);
    }
    
    //Player 2 Movements
    state.player2.xvel=0.0f;
    state.player2.yvel=0.0f;
    if (state.moveLeft2){
        state.player2.spriteDirection=1;
        state.player2.xvel=-3.4f;
        //state.player.velocity+=glm::vec3(-2.4f,0,0);
        
    }
    else if (state.moveRight2){
        state.player2.spriteDirection=2;
        state.player2.xvel=3.4f;
        //state.player.velocity+=glm::vec3(2.4f,0,0);
        
    }
    if (state.moveUp2){
        state.player2.spriteDirection=3;
        state.player2.yvel=3.4f;
        //state.player.velocity+=glm::vec3(0,5,0);
    }
    if (state.moveDown2){
        state.player2.spriteDirection=0;
        state.player2.yvel=-3.4f;
        //state.player.velocity+=glm::vec3(0,5,0);
    }
    
    //move enemies
    for (int i=0;i<state.enemies.size();i++){
        state.enemies[i].spriteIndex=state.enemies[i].runAnimation[state.enemies[i].spriteDirection][graphicsIndex];
        state.enemies[i].position+=state.enemies[i].velocity*elapsed;
        if (state.enemies[i].velocity==glm::vec3(0,0,0)){
            state.enemies[i].spriteIndex=state.enemies[i].runAnimation[state.enemies[i].spriteDirection][0]+1;
        }
    }
    //move bullets
    for (int i=0;i<state.bullets.size();i++){
        state.bullets[i].position+=state.bullets[i].velocity*elapsed;
    }
    //check sprite change in direction
    if (state.player1.velocity==glm::vec3(0,0,0)){
        state.player1.spriteIndex=state.player1.runAnimation[state.player1.spriteDirection][0]+1;
    }
    //endgame
    if ((state.versus==false)&&(checkEnemies(state))){
        mode=STATE_NEXT_LEVEL;
    }
    //check bullet and enemy collision
    checkCollision(state,elapsed);
    if (!state.l2){
        playercolX(state);
        playercolY(state);
        playercolX2(state);
        playercolY2(state);
    }
    
    //apply transformations
    state.player1.xvel = lerp(state.player1.xvel, 0.0f, elapsed * 5.0f);
    state.player1.velocity=glm::vec3(state.player1.xvel,state.player1.yvel,0);
    state.player2.xvel = lerp(state.player2.xvel, 0.0f, elapsed * 5.0f);
    state.player2.velocity=glm::vec3(state.player2.xvel,state.player2.yvel,0);
    state.player1.position+=state.player1.velocity*elapsed;
    state.player2.position+=state.player2.velocity*elapsed;
    

}

void fire(GameState &state){
    Bullet newBullet= Bullet();
    newBullet.player1=true;
    newBullet.velocity=bulletVelocity[state.player1.spriteDirection];
    newBullet.position=state.player1.position;
    state.bullets.push_back(newBullet);
}

void fire2(GameState &state){
    Bullet newBullet= Bullet();
    newBullet.player1=false;
    newBullet.velocity=bulletVelocity[state.player2.spriteDirection];
    newBullet.position=state.player2.position;
    state.bullets.push_back(newBullet);

}

void RenderMainMenu(){
    modelMatrix=glm::mat4(1.0f);

    modelMatrix = glm::translate(modelMatrix, glm::vec3(-6.0f,0.3f,0.0f));
    DrawText(program, fontTexture, "The Alien and the Blob Demo", 0.49f, -0.0001f);
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

void RenderNextLevel(){
    modelMatrix=glm::mat4(1.0f);
    
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.7f,0.3f,0.0f));
    DrawText(program, fontTexture, "Continue", 0.4f, 0.0000001f);
    program.SetModelMatrix(modelMatrix);
    
    modelMatrix=glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.7f,-0.4f,0.0f));
    DrawText(program, fontTexture, "Press Space", 0.5f, 0.0001f);
    program.SetModelMatrix(modelMatrix);
}

void RenderGameOver(){
    modelMatrix=glm::mat4(1.0f);
    //program.SetModelMatrix(modelMatrix);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.7f,0.3f,0.0f));
    DrawText(program, fontTexture, "GAME OVER", 0.8f, 0.0001f);
    program.SetModelMatrix(modelMatrix);
}

GameState state;

void Render() {
    switch(mode) {
        case STATE_MAIN_MENU:
            RenderMainMenu();
            break;
        case STATE_GAME_LEVEL1:
            RenderGame(state);
            break;
        case STATE_GAME_LEVEL2:
            RenderGame(state);
            break;
        case STATE_GAME_LEVEL3:
            RenderGame(state);
            break;
        case STATE_NEXT_LEVEL:
            RenderNextLevel();
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
        case STATE_GAME_LEVEL1:
            UpdateGame(state, elapsed);
            break;
        case STATE_GAME_LEVEL2:
            UpdateGame(state, elapsed);
            break;
        case STATE_GAME_LEVEL3:
            UpdateGame(state, elapsed);
            break;
        case STATE_NEXT_LEVEL:
            UpdateMainMenu(elapsed);
        case STATE_GAME_OVER:
            UpdateGameOver(elapsed);
            break;
    } }

GameState newState;



int main(int argc, char *argv[])
{
    setup();
    float lastFrameTicks = 0.0f;
    mode=STATE_MAIN_MENU;
    
    SDL_Event event;
    bool done = false;
    float animationElapsed = 0.0f;
    float framesPerSecond = 240.0f;
    //int currentIndex = 0;
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
                        mode=STATE_GAME_LEVEL1;
                        state.level1();
                        levelTracker++;
                        
                    }
                    else if (mode==STATE_NEXT_LEVEL){
                        if (levelTracker==2){
                            state=newState;
                            mode=STATE_GAME_LEVEL2;
                            state.level2();
                            levelTracker++;
                        }
                        else if (levelTracker==3){
                            state=newState;
                            mode=STATE_GAME_LEVEL3;
                            state.level3();
                            levelTracker++;
                        }
                        else if(mode==STATE_GAME_OVER){
                            exit(EXIT_SUCCESS);
                        }
                    }
                    else if ((mode==STATE_GAME_LEVEL1)||(mode=STATE_GAME_LEVEL2)||(mode==STATE_GAME_LEVEL3)){
                        
                        fire(state);
                        Mix_PlayChannel( -1, sound1, 0);
//                        state.jump=true;
                    }
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_LEFT){
                    state.moveLeft=true;
                    state.player1.moveLeft=true;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_RIGHT){
                    state.player1.moveRight=true;
                    state.moveRight=true;
                    
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_UP){
                    state.moveUp=true;
                    state.player1.moveUp=true;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_DOWN){
                    state.player1.moveDown=true;
                    state.moveDown=true;
                    
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_A){
                    state.player2.moveLeft=true;
                    state.moveLeft2=true;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_D){
                    state.player2.moveRight=true;
                    state.moveRight2=true;
                    
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_W){
                    state.player2.moveUp=true;
                    state.moveUp2=true;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_S){
                    state.player2.moveDown=true;
                    state.moveDown2=true;
                    
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_C){
                    fire2(state);
                    Mix_PlayChannel( -1, sound1, 0);
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_Q){
                    exit(EXIT_SUCCESS);
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
//                    state.jump=false;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_UP){
                    state.moveUp=false;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_DOWN){
                    state.moveDown=false;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_A){
                    state.moveLeft2=false;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_D){
                    
                    state.moveRight2=false;
                    
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_W){
                    state.moveUp2=false;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_S){
                    
                    state.moveDown2=false;
                    
                }
            }
        }
        //inloop setup
        glClear(GL_COLOR_BUFFER_BIT);
        
        program.SetColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        animationElapsed += elapsed;
        if(animationElapsed > 1.0/framesPerSecond) {
            graphicsIndex++;
            animationElapsed = 0.0;
            if(graphicsIndex > 1) {
                graphicsIndex = 0;
            }
        }
        screenShakeValue += elapsed;
        
        Update(elapsed);
        Render();
        //DrawSpriteSheetSprite2(program, 22, spriteCountX2, spriteCountY2);
        
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
    displayWindow = SDL_CreateWindow("Final Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glClearColor(0.52f, 0.8f, 0.92f, 1.0f);
    program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl" ) ;
    uprogram.Load(RESOURCE_FOLDER "vertex.glsl", RESOURCE_FOLDER "fragment.glsl" ) ;
    //initialize objects out of loop
    //program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl" ) ;
    //GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER "emoji.png");
    sheetSpriteTexture = LoadTexture(RESOURCE_FOLDER "characters_1.png");
    sheetSpriteTexture2 = LoadTexture(RESOURCE_FOLDER "arne_sprites.png");
    fontTexture=LoadTexture(RESOURCE_FOLDER "font1.png");
    projectionMatrix = glm::ortho(-7.1f, 7.1f, -4.0f, 4.0f, -1.0f, 1.0f);
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    Mix_Music *music;
    music=Mix_LoadMUS("music.mp3");
    sound1=Mix_LoadWAV("sound1.wav");
    sound2=Mix_LoadWAV("sound2.wav");
    Mix_VolumeChunk(sound2, 35);
    Mix_PlayMusic(music, -1);
    glUseProgram(program.programID);
    glUseProgram(program.programID);
    glUseProgram(uprogram.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


