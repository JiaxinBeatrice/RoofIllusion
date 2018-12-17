#include "../glad/glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
 #include <SDL2/SDL.h>
 #include <SDL2/SDL_opengl.h>
#else
 #include <SDL.h>
 #include <SDL_opengl.h>
#endif
#include <cstdio>

#define GLM_FORCE_RADIANS
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <math.h>
#define PI M_PI
using namespace std;

int VertParams = 6;
int screenWidth = 800;
int screenHeight = 600;

float linSpeed = .1;
glm::vec3 pos_reset = glm::vec3(-3.779011, -3.737073, 5.000001);
glm::vec3 pos = pos_reset;
std::vector<glm::vec3> ball_origin;
std::vector<glm::vec3> balls;
std::vector<float> t0;
std::vector<float> v1;
std::vector<float> a;
std::vector<int> state;
float ballr=0.25;
float sin_ = .51/glm::length(glm::vec3(-0.92f, 0.f, .51f));
float cos_ = sqrt(1 - sin_*sin_);
glm::vec3 n_long = glm::normalize(glm::cross(glm::vec3(0.f, -1.7f, 0.f), glm::vec3(2.3f, -0.6f, -1.28f)));
glm::vec3 n_short = glm::normalize(glm::cross(glm::vec3(-0.2f, 0.4f, -1.89f), glm::vec3(0.f, -2.f, 0.f)));
glm::vec3 pt_on_long = glm::vec3(-.92f, -.6f, 1.51f);
glm::vec3 pt_on_short = glm::vec3(0.2f, -0.4f, 1.89f);
bool sound_on = false;

bool DEBUG_ON = true;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
void drawGeometry(int shaderProgram, std::vector<int> start);
void ballFalling();
void playSound();

bool fullscreen = false;

int main(int argc, char *argv[]){

    SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)

    //Ask SDL to get a recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    //Create a window (offsetx, offsety, width, height, flags)
    SDL_Window* window = SDL_CreateWindow("Rooftop Illusion", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

    //Create a context to draw in
    SDL_GLContext context = SDL_GL_CreateContext(window);

    //Load OpenGL extentions with GLAD
    if (gladLoadGLLoader(SDL_GL_GetProcAddress)){
        printf("\nOpenGL loaded\n");
        printf("Vendor:   %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version:  %s\n\n", glGetString(GL_VERSION));
    }
    else {
        printf("ERROR: Failed to initialize OpenGL context.\n");
        return -1;
    }

    ifstream modelFile;
    int totalNumVerts = 0;
    std::vector<int> start;
    start.push_back(totalNumVerts);

    std::vector<float> v;
    std::vector<float> vn;
    string line;
    string command;
    float x, y, z;

    //Lode roof
    std::vector<float> roof;
    modelFile.open("models/roof.obj");
    while(modelFile >> command){
        if(command == "v"){
            modelFile >> x >> y >> z;
            v.push_back(x);v.push_back(y);v.push_back(z);
        }else if(command == "vn"){
            modelFile >> x >> y >> z;
            vn.push_back(x);vn.push_back(y);vn.push_back(z);
        }else if(command == "f"){
            string a;
            int num;
            for (int i=0; i<3; i++){
                modelFile >> a;
                num = std::stoi(a.substr(0, a.find('/')))-1;
                a = a.substr(a.find('/')+2);
                roof.push_back(v[num*3]);roof.push_back(v[num*3+1]);roof.push_back(v[num*3+2]);
                num = std::stoi(a)-1;
                roof.push_back(vn[num*3]);roof.push_back(vn[num*3+1]);roof.push_back(vn[num*3+2]);
            }

        }else{
            getline(modelFile, line);
        }
    }
    v.resize(0);vn.resize(0);
    modelFile.close();
    totalNumVerts += (int)roof.size()/VertParams;
    start.push_back(totalNumVerts);

    // Load sphere
    std::vector<float> sphere;
    modelFile.open("models/sphere.obj");
    while(modelFile >> command){
        if(command == "v"){
            modelFile >> x >> y >> z;
            v.push_back(x);v.push_back(y);v.push_back(z);
        }else if(command == "vn"){
            modelFile >> x >> y >> z;
            vn.push_back(x);vn.push_back(y);vn.push_back(z);
        }else if(command == "f"){
            string a;
            int num;
            for (int i=0; i<3; i++){
                modelFile >> a;
                num = std::stoi(a.substr(0, a.find('/')))-1;
                a = a.substr(a.find('/')+2);
                sphere.push_back(v[num*3]);sphere.push_back(v[num*3+1]);sphere.push_back(v[num*3+2]);
                num = std::stoi(a)-1;
                sphere.push_back(vn[num*3]);sphere.push_back(vn[num*3+1]);sphere.push_back(vn[num*3+2]);
            }

        }else{
            getline(modelFile, line);
        }
    }
    v.resize(0);vn.resize(0);
    modelFile.close();
    totalNumVerts += (int)sphere.size()/VertParams;
    start.push_back(totalNumVerts);

    // Load ground
    std::vector<float> ground;
    modelFile.open("models/ground.obj");
    while(modelFile >> command){
        if(command == "v"){
            modelFile >> x >> y >> z;
            v.push_back(x);v.push_back(y);v.push_back(z);
        }else if(command == "vn"){
            modelFile >> x >> y >> z;
            vn.push_back(x);vn.push_back(y);vn.push_back(z);
        }else if(command == "f"){
            string a;
            int num;
            for (int i=0; i<3; i++){
                modelFile >> a;
                num = std::stoi(a.substr(0, a.find('/')))-1;
                a = a.substr(a.find('/')+2);
                ground.push_back(v[num*3]);ground.push_back(v[num*3+1]);ground.push_back(v[num*3+2]);
                num = std::stoi(a)-1;
                ground.push_back(vn[num*3]);ground.push_back(vn[num*3+1]);ground.push_back(vn[num*3+2]);
            }

        }else{
            getline(modelFile, line);
        }
    }
    v.resize(0);vn.resize(0);
    modelFile.close();
    totalNumVerts += (int)ground.size()/VertParams;
    start.push_back(totalNumVerts);

    float* modelData = new float[totalNumVerts*VertParams];
    copy(roof.begin(), roof.end(), modelData+start[0]*VertParams);
    copy(sphere.begin(), sphere.end(), modelData+start[1]*VertParams);
    copy(ground.begin(), ground.end(), modelData+start[2]*VertParams);

    roof.resize(0);sphere.resize(0);ground.resize(0);

    //Build a Vertex Array Object (VAO) to store mapping of shader attributse to VBO
    GLuint vao;
    glGenVertexArrays(1, &vao); //Create a VAO
    glBindVertexArray(vao); //Bind the above created VAO to the current context

    //Allocate memory on the graphics card to store geometry (vertex buffer object)
    GLuint vbo[1];
    glGenBuffers(1, vbo);  //Create 1 buffer called vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
    glBufferData(GL_ARRAY_BUFFER, totalNumVerts*VertParams*sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo
    delete[] modelData;

    int shader = InitShader("src/vertex.glsl", "src/fragment.glsl");

    //Tell OpenGL how to set fragment shader input
    GLint posAttrib = glGetAttribLocation(shader, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, VertParams*sizeof(float), 0);
      //Attribute, vals/attrib., type, isNormalized, stride, offset
    glEnableVertexAttribArray(posAttrib);

    //Tell OpenGL how to set fragment shader input
    GLint norAttrib = glGetAttribLocation(shader, "inNormal");
    glVertexAttribPointer(norAttrib, 3, GL_FLOAT, GL_FALSE, VertParams*sizeof(float), (void*)((VertParams-3)*sizeof(float)));
      //Attribute, vals/attrib., type, isNormalized, stride, offset
    glEnableVertexAttribArray(norAttrib);

    GLint uniView = glGetUniformLocation(shader, "view");
    GLint uniProj = glGetUniformLocation(shader, "proj");

    glBindVertexArray(0); //Unbind the VAO in case we want to create a new one


    glEnable(GL_DEPTH_TEST);

    //Event Loop (Loop forever processing each event as fast as possible)
    SDL_Event windowEvent;
    bool quit = false;
    int cnt = -1;
    while (!quit){
        while (SDL_PollEvent(&windowEvent)){
            if (windowEvent.type == SDL_QUIT) quit = true;
            //List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
            //Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE) quit = true; //Exit event loop
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f){
                fullscreen = !fullscreen;
                SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Toggle fullscreen
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_r){
                pos = pos_reset;
            }            
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_p){
                sound_on = !sound_on;
            }  
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_UP){
                if (windowEvent.key.keysym.mod & KMOD_SHIFT) pos.z += .3;
                else{
                    pos += linSpeed * pos;
                }
                printf("x: %f, y: %f, z: %f\n", pos.x, pos.y, pos.z);
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_DOWN){
                if (windowEvent.key.keysym.mod & KMOD_SHIFT) pos.z -= .3; //Is shift pressed
                else{
                    pos -= linSpeed * pos;
                }
                printf("x: %f, y: %f, z: %f\n", pos.x, pos.y, pos.z);
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_LEFT){
                pos.x = cos(PI/90.0)*pos.x - sin(PI/90.0)*pos.y;
                pos.y = sin(PI/90.0)*pos.x + cos(PI/90.0)*pos.y;
                printf("x: %f, y: %f, z: %f\n", pos.x, pos.y, pos.z);
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_RIGHT){
                pos.x = cos(PI/90.0)*pos.x + sin(PI/90.0)*pos.y;
                pos.y = -sin(PI/90.0)*pos.x + cos(PI/90.0)*pos.y;
                printf("x: %f, y: %f, z: %f\n", pos.x, pos.y, pos.z);
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_SPACE){
                cnt++;
                // balls.push_back(glm::vec3(-1.8f, -.6f, 2.5f));
                if(cnt>4){
                    int tmp_c = cnt%5;
                    balls.at(tmp_c) = glm::vec3(-2.f+.3f*tmp_c, -1.1f + 0.4f*tmp_c, 2.5f-0.2f*tmp_c);
                    ball_origin.at(tmp_c) = glm::vec3(-2.f+.3f*tmp_c, -1.1f + 0.4f*tmp_c, 2.5f-0.2f*tmp_c);
                    t0.at(tmp_c)= SDL_GetTicks()/1000.f;
                    state.at(tmp_c) = 0;
                    v1.at(tmp_c) = 0;
                    a.at(tmp_c) = 4.9f;
                }
                else {
                    balls.push_back(glm::vec3(-2.f+.3f*cnt, -1.1f + 0.4f*cnt, 2.5f-0.2*cnt));
                    ball_origin.push_back(glm::vec3(-2.f+.3f*cnt, -1.1f + 0.4f*cnt, 2.5f-0.2*cnt));
                    t0.push_back(SDL_GetTicks()/1000.f);
                    state.push_back(0);
                    v1.push_back(0);
                    a.push_back(4.9f);
                }
                
            }
        }

        // Clear the screen to default color
        glClearColor(.2f, 0.4f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        ballFalling();

        glm::mat4 view = glm::lookAt(
        pos,  //Cam Position
        pos - glm::normalize(pos),
        glm::vec3(0.0f, 0.0f, 1.0f)); //Up
        glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 proj = glm::perspective(3.14f/4, screenWidth / (float) screenHeight, .1f, 20.0f); //FOV, aspect, near, far
        glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

        glBindVertexArray(vao);

        drawGeometry(shader, start);

        SDL_GL_SwapWindow(window); //Double buffering
    }
    //Clean Up
    glDeleteProgram(shader);
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, &vao);

    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}

void ballFalling(){
    /** finite state machine for the movements
     * 0: dropping from a height
     * 1: sliding down the slope
     * 2: climbing back until v < 0
     */
    for(int i=0; i<balls.size(); i++){
        float t1 = SDL_GetTicks()/1000.f;
        if(state[i] == 0){
            if(glm::dot(glm::normalize(balls[i] - 0.5f*ballr*n_long - pt_on_long), n_long) < 0.01){
                state[i] = 1;
                if(sound_on)playSound();
                ball_origin[i] = balls[i];
                t0[i] = SDL_GetTicks()/1000.f;
                a[i] = a[i]/3;
            }
            else{
                balls[i].z = ball_origin[i].z - a[i]*(t1-t0[i])*(t1-t0[i]);
                v1[i] = a[i]*(t1-t0[i]);
            }
        }
        else if(state[i] == 1){
            if(glm::dot(glm::normalize(balls[i] - 0.85f*ballr*n_short - pt_on_short), n_short) < 0.01) {
                state[i] = 2;
                if(sound_on)playSound();
                // v1[i] = (v1[i] + a[i]*(t1-t0[i]))/(3.5f+0.3*i);
                v1[i] = (v1[i] + a[i]*(t1-t0[i]))/3.5f;
                t0[i] = SDL_GetTicks()/1000.f;
                ball_origin[i] = balls[i];
                if(v1[i]<0.02)state[i]=3;
            }
            else {
                balls[i].x = ball_origin[i].x + v1[i]*(t1-t0[i])*(cos_/sin_) + a[i]*(t1-t0[i])*(t1-t0[i])*(cos_/sin_);
                balls[i].z = ball_origin[i].z - v1[i]*(t1-t0[i]) - a[i]*(t1-t0[i])*(t1-t0[i]);      
            }
        }
        else if(state[i] == 2){
            if((v1[i] - a[i]*(t1-t0[i]))<0.01){
                state[i] = 1;
                ball_origin[i] = balls[i];
                v1[i] = 0;
                t0[i] = SDL_GetTicks()/1000.f;
                
            }
            else {
                float tmp = balls[i].z;
                balls[i].x = ball_origin[i].x - v1[i]*(t1-t0[i])*(cos_/sin_) - a[i]*(t1-t0[i])*(t1-t0[i])*(cos_/sin_);
                balls[i].z = ball_origin[i].z + v1[i]*(t1-t0[i]) + a[i]*(t1-t0[i])*(t1-t0[i]);            
            }
        }
    }
}

void drawGeometry(int shaderProgram, std::vector<int> start){

    GLint uniModel = glGetUniformLocation(shaderProgram, "model");
    GLint uniColor = glGetUniformLocation(shaderProgram, "inColor");
    glm::mat4 model;
    glm::vec3 color;

    // Roof
    model = glm::mat4();
    color = glm::vec3(1,1,1);
    glUniform3fv(uniColor, 1, glm::value_ptr(color));
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, start[0], start[1]-start[0]);

    glm::vec3 color_s[5] = {glm::vec3(1,0,0),glm::vec3(1,0.45,0.05),glm::vec3(1,1,0),glm::vec3(0,0.8,0.2),glm::vec3(0,0.1,1)};
    // Ball
    for(int i=0; i<balls.size(); i++){
        // color = glm::vec3(1,0,1);
        color = color_s[i];
        model = glm::mat4();
        float time = SDL_GetTicks()/1000.f;
        model = glm::translate(model, balls[i]);
        model = glm::scale(model, 2*ballr*glm::vec3(1.f,1.f,1.f));
        glUniform3fv(uniColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, start[1], start[2]-start[1]);
    }

    // Ground
    model = glm::mat4();
    color = glm::vec3(0,.819,.698);
    model = glm::translate(model, glm::vec3(0,0,-1.f));
    model = glm::scale(model, glm::vec3(9.f,9.f,1.f));
    glUniform3fv(uniColor, 1, glm::value_ptr(color));
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, start[2], start[3]-start[2]);


}

void playSound()
{
    // Load a sound buffer from a wav file
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("tennis_2.wav"))
        return;

    // Create a sound instance and play it
    sf::Sound sound(buffer);
    sound.play();

    // Loop while the sound is playing
    while (sound.getStatus() == sf::Sound::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(50));

    }
}

// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile){
    FILE *fp;
    long length;
    char *buffer;

    // open the file containing the text of the shader code
    fp = fopen(shaderFile, "r");

    // check for errors in opening the file
    if (fp == NULL) {
        printf("can't open shader source file %s\n", shaderFile);
        return NULL;
    }

    // determine the file size
    fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
    length = ftell(fp);  // return the value of the current position

    // allocate a buffer with the indicated number of bytes, plus one
    buffer = new char[length + 1];

    // read the appropriate number of bytes from the file
    fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
    fread(buffer, 1, length, fp); // read all of the bytes

    // append a NULL character to indicate the end of the string
    buffer[length] = '\0';

    // close the file
    fclose(fp);

    // return the string
    return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName){
    GLuint vertex_shader, fragment_shader;
    GLchar *vs_text, *fs_text;
    GLuint program;

    // check GLSL version
    printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Create shader handlers
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    // Read source code from shader files
    vs_text = readShaderSource(vShaderFileName);
    fs_text = readShaderSource(fShaderFileName);

    // error check
    if (vs_text == NULL) {
        printf("Failed to read from vertex shader file %s\n", vShaderFileName);
        exit(1);
    } else if (DEBUG_ON) {
        printf("Vertex Shader:\n=====================\n");
        printf("%s\n", vs_text);
        printf("=====================\n\n");
    }
    if (fs_text == NULL) {
        printf("Failed to read from fragent shader file %s\n", fShaderFileName);
        exit(1);
    } else if (DEBUG_ON) {
        printf("\nFragment Shader:\n=====================\n");
        printf("%s\n", fs_text);
        printf("=====================\n\n");
    }

    // Load Vertex Shader
    const char *vv = vs_text;
    glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
    glCompileShader(vertex_shader); // Compile shaders

    // Check for errors
    GLint  compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printf("Vertex shader failed to compile:\n");
        if (DEBUG_ON) {
            GLint logMaxSize, logLength;
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
            printf("printing error message of %d bytes\n", logMaxSize);
            char* logMsg = new char[logMaxSize];
            glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
            printf("%d bytes retrieved\n", logLength);
            printf("error message: %s\n", logMsg);
            delete[] logMsg;
        }
        exit(1);
    }

    // Load Fragment Shader
    const char *ff = fs_text;
    glShaderSource(fragment_shader, 1, &ff, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);

    //Check for Errors
    if (!compiled) {
        printf("Fragment shader failed to compile\n");
        if (DEBUG_ON) {
            GLint logMaxSize, logLength;
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
            printf("printing error message of %d bytes\n", logMaxSize);
            char* logMsg = new char[logMaxSize];
            glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
            printf("%d bytes retrieved\n", logLength);
            printf("error message: %s\n", logMsg);
            delete[] logMsg;
        }
        exit(1);
    }

    // Create the program
    program = glCreateProgram();

    // Attach shaders to program
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    // Link and set program to use
    glLinkProgram(program);

    return program;
}
