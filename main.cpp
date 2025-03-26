#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include "square.h"
#include <iostream>
#include <ft2build.h>
#include <freetype/freetype.h>
#include "character.h"
#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
int SCORE = 0;
    
float allCircleVertices[102 * 3];
unsigned int VBO[5], VAO[5], EBO[1];

float* generateCircleVertices(float x, float y, float z, float radius, unsigned int numberOfSides){

    unsigned int numOfVertices = numberOfSides + 2;
    float doublePi = 2.0f * M_PI;

    memset(allCircleVertices, 0, sizeof(float) * numOfVertices * 3);

    float circleVertX[numOfVertices];
    float circleVertY[numOfVertices];
    float circleVertZ[numOfVertices];

    circleVertX[0] = x;
    circleVertY[0] = y;
    circleVertZ[0] = z;

    for(int i = 1; i < numOfVertices; i++){
        circleVertX[i] = x + (radius * cos( i * doublePi / numberOfSides ));
        circleVertY[i] = y + (radius * sin( i * doublePi / numberOfSides ));
        circleVertZ[i] = z;
    }

    for(int i = 0; i < numOfVertices; i++){
        allCircleVertices[i * 3] = circleVertX[i];
        allCircleVertices[(i * 3) + 1] = circleVertY[i];
        allCircleVertices[(i * 3) + 2] = circleVertZ[i];
    }

   return allCircleVertices;

}

bool checkCollisionPaddle(float squareX, float squareY, float circleX, float circleY){

    float squareRadius = 0.5f;
    float circleRadius = 0.5f;

    glm::vec2 center(circleX + circleRadius, circleY + circleRadius);
    glm::vec2 aabb_half_extents(squareRadius, squareRadius);
    glm::vec2 aabb_center(
        squareX + aabb_half_extents.x,
        squareY + aabb_half_extents.y
    );
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    glm::vec2 closest = aabb_center + clamped;

    difference = closest - center;

    return glm::length(difference) < circleRadius;

}


void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
    {
        // activate corresponding render state	
        shader.use();
        glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO[4]);

        // iterate through all characters
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) 
        {
            Character ch = Characters[*c];

            float xpos = x + ch.Bearing.x * scale;
            float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;
            // update VBO for each character
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },            
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }           
            };
            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            // update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
}

int main()
{

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Shape Shift", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right 0
         0.5f, -0.5f, 0.0f,  // bottom right 1
        -0.5f, -0.5f, 0.0f,  // bottom left 2
    };

    float vertices2[] = {
        -0.5f,  -0.5f, 0.0f,  // bottom left
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
    };

    float targetVertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3,   // second Triangle
    };

    glGenVertexArrays(5, VAO);
    glGenBuffers(5, VBO);
    glGenBuffers(1, EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
   // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
  
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);



    float *circleVerts = generateCircleVertices(0.0f, 0.0f, 0.0f, 0.5f, 100);
    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 102 * 3, circleVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);



    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(targetVertices), targetVertices, GL_STATIC_DRAW);
    // EBO for target squares
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glEnableVertexAttribArray(0);

    // Text 
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compile and setup shader

    Shader shader("text.vs", "text.fs");
    Shader shader1("shader.vs", "fragment1.fs");
    Shader shader2("shader.vs", "fragment2.fs");

    glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

    FT_Library ft;

    if(FT_Init_FreeType(&ft)){
        std::cout << "ERROR::FREETYPE: Could not init FreeType library" << std::endl;
        return -1;
    }

    std::string font_name = "/Users/sameerqureshi/Documents/OpenGL/fonts/arial.ttf";

    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }
	
	// load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    glBindVertexArray(VAO[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0 );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
   // glBindVertexArray(0); 


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------

    float paddleX = -4.85f;
    float paddleY = 2.5f;
    float paddleVelocity = 0.035f;

    float circleX = -2.0f;
    float circleY = 0.0;
    float circleVelocityX = 0.035f;
    float circleVelocityY = 0.045f;

    // spacing 0.10f 

    Square targetCoords[] = {
        Square(4.75f, 2.4f, 0.0f, true), 
        Square(4.75f, 1.3f, 0.0f, true), 
        Square(4.75f, 0.2f, 0.0f, true), 
        Square(4.75f, -0.9f, 0.0f, true), 
        Square(4.75f, -2.0f, 0.0f, true), 
        
        Square(3.65f, 2.4f, 0.0f, true), 
        Square(3.65f, 1.3f, 0.0f, true), 
        Square(3.65f, 0.2f, 0.0f, true), 
        Square(3.65f, -0.9f, 0.0f, true), 
        Square(3.65f, -2.0f, 0.0f, true), 
    };


    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(paddleX, paddleY, 0.0f));

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    glm::mat4 modelCircle = glm::mat4(1.0f);
    modelCircle = glm::translate(modelCircle, glm::vec3(circleX, circleY, 0.0f));

    glm::mat4 modelTarget = glm::mat4(1.0f);
    glm::mat4 modelTarget2 = glm::mat4(1.0f);
    glm::mat4 modelTarget3 = glm::mat4(1.0f);
    glm::mat4 modelTarget4 = glm::mat4(1.0f);
    glm::mat4 modelTarget5 = glm::mat4(1.0f);
    glm::mat4 modelTarget6 = glm::mat4(1.0f);
    glm::mat4 modelTarget7 = glm::mat4(1.0f);
    glm::mat4 modelTarget8 = glm::mat4(1.0f);
    glm::mat4 modelTarget9 = glm::mat4(1.0f);
    glm::mat4 modelTarget10 = glm::mat4(1.0f);

    modelTarget = glm::translate(modelTarget, glm::vec3(targetCoords[0].getX(), targetCoords[0].getY(), targetCoords[0].getZ()));
    modelTarget2 = glm::translate(modelTarget2, glm::vec3(targetCoords[1].getX(), targetCoords[1].getY(), targetCoords[1].getZ()));
    modelTarget3 = glm::translate(modelTarget3, glm::vec3(targetCoords[2].getX(), targetCoords[2].getY(), targetCoords[2].getZ()));
    modelTarget4 = glm::translate(modelTarget4, glm::vec3(targetCoords[3].getX(), targetCoords[3].getY(), targetCoords[3].getZ()));
    modelTarget5 = glm::translate(modelTarget5, glm::vec3(targetCoords[4].getX(), targetCoords[4].getY(), targetCoords[4].getZ()));
    modelTarget6 = glm::translate(modelTarget6, glm::vec3(targetCoords[5].getX(), targetCoords[5].getY(), targetCoords[5].getZ()));
    modelTarget7 = glm::translate(modelTarget7, glm::vec3(targetCoords[6].getX(), targetCoords[6].getY(), targetCoords[6].getZ()));
    modelTarget8 = glm::translate(modelTarget8, glm::vec3(targetCoords[7].getX(), targetCoords[7].getY(), targetCoords[7].getZ()));
    modelTarget9 = glm::translate(modelTarget9, glm::vec3(targetCoords[8].getX(), targetCoords[8].getY(), targetCoords[8].getZ()));
    modelTarget10 = glm::translate(modelTarget10, glm::vec3(targetCoords[9].getX(), targetCoords[9].getY(), targetCoords[9].getZ()));

    std::map<int, glm::mat4> modelDict;
    modelDict[0] = modelTarget;
    modelDict[1] = modelTarget2;
    modelDict[2] = modelTarget3;
    modelDict[3] = modelTarget4;
    modelDict[4] = modelTarget5;
    modelDict[5] = modelTarget6;
    modelDict[6] = modelTarget7;
    modelDict[7] = modelTarget8;
    modelDict[8] = modelTarget9;
    modelDict[9] = modelTarget10;

    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        if(glfwGetKey(window, GLFW_KEY_S ) == GLFW_PRESS){
            if(paddleY > -2.5f){
                model = glm::translate(model, glm::vec3(0.0f, -1.0f * paddleVelocity, 0.0f));
                paddleY = paddleY + (-1.0f * paddleVelocity);
            }   
        }

        if(glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS){
            if(paddleY < 2.5f){
                model = glm::translate(model, glm::vec3(0.0f, 1.0f * paddleVelocity, 0.0f));
                paddleY = paddleY + (1.0f * paddleVelocity);
            }   
        }
        

        // first triangle model, view, projection

       // glUseProgram(shaderProgram);
       shader1.use();

        unsigned int modelLoc = glGetUniformLocation(shader1.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        unsigned int viewLoc = glGetUniformLocation(shader1.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        unsigned int projectionLoc = glGetUniformLocation(shader1.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // draw first triangle
        
        glBindVertexArray(VAO[0]); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, 3);


        // second triangle model, view, projection

       // glUseProgram(shaderProgram2);
       shader2.use();

        unsigned int modelLoc2 = glGetUniformLocation(shader2.ID, "model");
        glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model));
        unsigned int viewLoc2 = glGetUniformLocation(shader2.ID, "view");
        glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));
        unsigned int projectionLoc2 = glGetUniformLocation(shader2.ID, "projection");
        glUniformMatrix4fv(projectionLoc2, 1, GL_FALSE, glm::value_ptr(projection));

        // draw second triangle 

        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLES, 0 ,3);

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time 

        // circle model, view, projection

        //glUseProgram(shaderProgram2);
        shader2.use();

        unsigned int modelLoc3 = glGetUniformLocation(shader2.ID, "model");
        glUniformMatrix4fv(modelLoc3, 1, GL_FALSE, glm::value_ptr(modelCircle));
        unsigned int viewLoc3 = glGetUniformLocation(shader2.ID, "view");
        glUniformMatrix4fv(viewLoc3, 1, GL_FALSE, glm::value_ptr(view));
        unsigned int projectionLoc3 = glGetUniformLocation(shader2.ID, "projection");
        glUniformMatrix4fv(projectionLoc3, 1, GL_FALSE, glm::value_ptr(projection));

        // draw circle 

        glBindVertexArray(VAO[2]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 102);

        // view and projection matrix on square

        int targetCoordsLength = sizeof(targetCoords) / sizeof(targetCoords[0]);
        int modelCounter = 0;

        //glUseProgram(shaderProgram);
        shader1.use();
        
        for(int i = 0; i < targetCoordsLength; i++){  
            if(targetCoords[i].getActive() == true){
                unsigned int modelTargetLoc = glGetUniformLocation(shader1.ID, "model");
                glUniformMatrix4fv(modelTargetLoc, 1, GL_FALSE, glm::value_ptr(modelDict[modelCounter]));
                unsigned int targetViewLoc = glGetUniformLocation(shader1.ID, "view");
                glUniformMatrix4fv(targetViewLoc, 1, GL_FALSE, glm::value_ptr(view));
                unsigned int targetProjectionLoc = glGetUniformLocation(shader1.ID, "projection");
                glUniformMatrix4fv(targetProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
                glBindVertexArray(VAO[3]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            modelCounter += 1;
        }

    
        // circle velocity

        modelCircle = glm::translate(modelCircle, glm::vec3(1.0f * circleVelocityX, 1.0f * circleVelocityY, 0.0f));
        circleX = circleX + (1.0f * circleVelocityX);
        circleY = circleY + (1.0f * circleVelocityY);

        // check circle bounds

        if(circleY > 2.5f){
            modelCircle = glm::translate(modelCircle, glm::vec3(1.0f * circleVelocityX, 1.0f * -circleVelocityY, 0.0f));
            circleVelocityY = -circleVelocityY;
            circleX = circleX + (1.0f * circleVelocityX);
            circleY = circleY + (1.0f * circleVelocityY);
        }

        if(circleX > 4.85f){
            modelCircle = glm::translate(modelCircle, glm::vec3(1.0f * -circleVelocityX, 1.0f * circleVelocityY, 0.0f));
            circleVelocityX = -circleVelocityX;
            circleX = circleX + (1.0f * circleVelocityX);
            circleY = circleY + (1.0f * circleVelocityY);
        }

        if(circleY < -2.5f){
            modelCircle = glm::translate(modelCircle, glm::vec3(1.0f * circleVelocityX, 1.0f * -circleVelocityY, 0.0f));
            circleVelocityY = -circleVelocityY;
            circleX = circleX + (1.0f * circleVelocityX);
            circleY = circleY + (1.0f * circleVelocityY);
        }

        if(circleX < -4.85f){
            modelCircle = glm::translate(modelCircle, glm::vec3(1.0f * -circleVelocityX, 1.0f * circleVelocityY, 0.0f));
            circleVelocityX = -circleVelocityX;
            circleX = circleX + (1.0f * circleVelocityX);
            circleY = circleY + (1.0f * circleVelocityY);
        }

        // check for collision with paddle
 
        if(checkCollisionPaddle(paddleX, paddleY, circleX, circleY)){
            modelCircle = glm::translate(modelCircle, glm::vec3(1.0f * -circleVelocityX, 1.0f * -circleVelocityY, 0.0f));
            circleVelocityX = -circleVelocityX;
            circleVelocityY = -circleVelocityY;
            circleX = circleX + (1.0f * circleVelocityX);
            circleY = circleY + (1.0f * circleVelocityY);
        }

        // check for collision with target

        for(int i = 0; i < targetCoordsLength; i++){
            Square currSquare = targetCoords[i];
            if(targetCoords[i].getActive() == true && currSquare.checkCollisionTarget(targetCoords[i], targetCoords[i].getX(), targetCoords[i].getY(), circleX, circleY)){  
                targetCoords[i].setIsActive(false);
                modelCircle = glm::translate(modelCircle, glm::vec3(1.0f * -circleVelocityX, 1.0f * -circleVelocityY, 0.0f));
                circleVelocityX = -circleVelocityX;
                circleVelocityY = -circleVelocityY;
                circleX = circleX + (1.0f * circleVelocityX);
                circleY = circleY + (1.0f * circleVelocityY);
            }
        }

        std::string score = std::to_string(SCORE);

        RenderText(shader, "SCORE - ", 580.0f, 25.0f, 0.6f, glm::vec3(1.0f, 1.0f, 1.0f));
        RenderText(shader, score, 710.0f, 25.0f, 0.6f, glm::vec3(1.0f, 1.0f, 1.0f));
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
   // glDeleteVertexArrays(1, VAO[0]);
  //  glDeleteBuffers(1, VBO[0]);
  //  glDeleteBuffers(1, EBO[0]);
  //  glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
