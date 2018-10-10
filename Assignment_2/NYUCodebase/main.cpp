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

//setup-declare objects
SDL_Window* displayWindow;
ShaderProgram program;

glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
GLuint LoadTexture(const char *filePath);

void setup();

//entities
class Player{
public:
    float up,down,left,right,pos;
    Player(float u,float d,float l,float r,float pos):up(u),down(d),left(l),right(r){}
    Player(float p){
        pos=p;
        up=0.2f;
        down=-0.2f;
        left=0.02+pos;
        right=0.05f+pos;
    }
    void replace(){
        up=0.2f;
        down=-0.2f;
        left=0.02+pos;
        right=0.05f+pos;
    }
};

//ball class with dimensions, x and y velocities, and x and y directions
class Ball{
public:
    float up,down,left,right,xv,yv,xd,yd;
    Ball(){
        up=0.04f;
        down=-0.04f;
        left=-0.04f;
        right=0.04f;
        xv=0.0006;
        yv=0.0006;
        xd=1;
        yd=1;
    }
    void move(float factor =1.0){
        up+=yv*yd*factor;
        down+=yv*yd*factor;
        left+=xv*xd*factor;
        right+=xv*xd*factor;
        
    }
    void replace(){
        up=0.04f;
        down=-0.04f;
        left=-0.04f;
        right=0.04f;
        xv=0.0006;
        yv=0.0006;
    }
    
};
int main(int argc, char *argv[])
{
    setup();
    float lastFrameTicks = 0.0f;

    Player left=Player(-1.5);
    Player right=Player(1.5);
    Ball ball=Ball();
    
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        ball.move(elapsed*1000.0f);
        float move=0.2f+elapsed;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            } else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_UP &&right.up<1) {
                    right.up+=move;
                    right.down+=move;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_DOWN && right.down>-1){
                    right.up-=move;
                    right.down-=move;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_W && left.up<1){
                    left.up+=move;
                    left.down+=move;
                }
                else if (event.key.keysym.scancode==SDL_SCANCODE_S && left.down>-1){
                    left.up-=move;
                    left.down-=move;
                }
            }
        }
        //inloop setup
        glClear(GL_COLOR_BUFFER_BIT);
        program.SetColor(0.5f, 0.4f, 0.0f, 1.0f);
        
        //inloop operations
        
        //right paddle
        modelMatrix=glm::mat4(1.0f);
        program.SetModelMatrix(modelMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        float rvertices[] = {right.left, right.down, right.right, right.down, right.right, right.up, right.left, right.down, right.left, right.up, right.right, right.up};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rvertices);
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //left paddle
        modelMatrix=glm::mat4(1.0f);
        program.SetModelMatrix(modelMatrix);
        float lvertices[] = {left.left, left.down, left.right, left.down, left.right, left.up, left.left, left.down, left.left, left.up, left.right, left.up};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, lvertices);
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // draw ball
        modelMatrix=glm::mat4(1.0f);
        program.SetModelMatrix(modelMatrix);
        float bvertices[] = {ball.left, ball.down, ball.right, ball.down, ball.right, ball.up, ball.left, ball.down, ball.left, ball.up, ball.right, ball.up};

        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, bvertices);
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        SDL_GL_SwapWindow(displayWindow);
        
        //paddle reflects and increases speed by .01
        if (ball.right>=right.left&&ball.up>=right.down&&ball.down<=right.up){
            ball.xd*=-1;
            ball.xv*=1.01;
        }
        else if (ball.left<=left.right&&ball.up>=left.down&&ball.down<=left.up){
            ball.xd*=-1;
            ball.xv*=1.01;
        }
        //win conditions
        else if (ball.right>=1.77f){
            printf("Player 1 wins\n");
            ball.replace();
            left.replace();
            right.replace();
        }
        else if (ball.left<=-1.77f){
            printf("Player 2 wins\n");
            ball.replace();
            left.replace();
            right.replace();
            
        }
        //if ball hits top or bottom of screen
        else if (ball.up>=1){
            ball.yv*=-1;
        }
        else if (ball.up<-.95){
            ball.yv*=-1;
        }

        
        
        

    }
    
    SDL_Quit();
    return 0;
}
//
//GLuint LoadTexture(const char *filePath) {
//    int w,h,comp;
//    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
//    if(image == NULL) {
//        std::cout << "Unable to load image. Make sure the path is correct\n";
//        assert(false);
//    }
//    GLuint retTexture;
//    glGenTextures(1, &retTexture);
//    glBindTexture(GL_TEXTURE_2D, retTexture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    stbi_image_free(image);
//    return retTexture;
//}

void setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
    
    //initialize objects out of loop
    program.Load(RESOURCE_FOLDER "vertex.glsl", RESOURCE_FOLDER "fragment.glsl" ) ;
    //program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl" ) ;
    //GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER "emoji.png");
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


