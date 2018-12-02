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
#include <string>
#include <vector>
#include <math.h>
#define PI M_PI
using namespace std;

int VertParams = 6;
int screenWidth = 800;
int screenHeight = 600;

float linSpeed = .1;
// (-2.5, 5.8, 6.11||5.55)
float posx=-3.779011, posy=-3.737073, posz=5.000001;
float ballx=-.92f, bally=-.6f, ballz=-10;

bool DEBUG_ON = true;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
void drawGeometry(int shaderProgram, std::vector<int> start);
void ballFalling();

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

            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_UP){
                if (windowEvent.key.keysym.mod & KMOD_SHIFT) posz += .2;
                else{
                    posx += linSpeed*posx;
                    posy += linSpeed*posy;
                    posz += linSpeed*posz;
                }
                printf("x: %f, y: %f, z: %f\n", posx, posy, posz);
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_DOWN){
                if (windowEvent.key.keysym.mod & KMOD_SHIFT) posz -= .2; //Is shift pressed?
                else{
                    posx -= linSpeed*posx;
                    posy -= linSpeed*posy;
                    posz -= linSpeed*posz;
                }
                printf("x: %f, y: %f, z: %f\n", posx, posy, posz);
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_LEFT){
                posx = cos(PI/90.0)*posx - sin(PI/90.0)*posy;
                posy = sin(PI/90.0)*posx + cos(PI/90.0)*posy;
                printf("x: %f, y: %f, z: %f\n", posx, posy, posz);
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_RIGHT){
                posx = cos(PI/90.0)*posx + sin(PI/90.0)*posy;
                posy = -sin(PI/90.0)*posx + cos(PI/90.0)*posy;
                printf("x: %f, y: %f, z: %f\n", posx, posy, posz);
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_SPACE){
                ballz = 3;
            }
        }

        // Clear the screen to default color
        glClearColor(.2f, 0.4f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        ballFalling();

        glm::mat4 view = glm::lookAt(
        glm::vec3(posx, posy, posz),  //Cam Position
        glm::vec3(posx, posy, posz) - glm::normalize(glm::vec3(posx, posy, posz)),
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
    ballz -=.01;
    printf("ball: %f, %f, %f\n", ballx, bally, ballz);
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

    // Ball
    model = glm::mat4();
    color = glm::vec3(1,0,1);
    model = glm::translate(model, glm::vec3(ballx,bally,ballz));
    model = glm::scale(model, .5f*glm::vec3(1.f,1.f,1.f));
    glUniform3fv(uniColor, 1, glm::value_ptr(color));
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, start[1], start[2]-start[1]);

    // Ground
    model = glm::mat4();
    color = glm::vec3(0,.819,.698);
    model = glm::translate(model, glm::vec3(0,0,-1.f));
    model = glm::scale(model, glm::vec3(9.f,9.f,1.f));
    glUniform3fv(uniColor, 1, glm::value_ptr(color));
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, start[2], start[3]-start[2]);


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
