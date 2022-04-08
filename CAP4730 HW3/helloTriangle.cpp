// template based on material from learnopengl.com
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace glm;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void loadObj(const char* objFileName, std::vector<vec3>& vs, std::vector<int>& faces, std::vector<vec3>& vns, std::vector<int>& nFaces);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char* vertexShaderSource;
const char* fragmentShaderSource;

vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//const char *vertexShaderSource = "#version 330 core\n"
//    "layout (location = 0) in vec3 aPos;\n"
//    "uniform mat4 m;\n"
//    "void main()\n"
//    "{\n"
//    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0) * m;\n"
//    "}\0";
//const char *fragmentShaderSource = "#version 330 core\n"
//    "out vec4 FragColor;\n"
//    "void main()\n"
//    "{\n"
//    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n" //
//    "}\n\0";
float scaler = 0.002f;
float theta = 0.0f;
vec3 axis = vec3(0.0, 0.0, 1.0);

vec3 lightPos(800.0f, 300.0f, 500.0f);

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "viewGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    // // glew: load all OpenGL function pointers
    glewInit();

    // Read shader sources from file
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::stringstream vShaderStream;
    std::stringstream fShaderStream;

    vShaderFile.open("phong.vs");
    fShaderFile.open("phong.fs");

    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    vShaderFile.close();
    fShaderFile.close();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

    vertexShaderSource = vertexCode.c_str();
    fragmentShaderSource = fragmentCode.c_str();


    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    float colorData[]{
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------    
    //float vertices[] = {
    //    -0.5f, -.25f, 0.0f, 1.0f, 0.5f, 0.2f, // left  
    //     0.5f, -.75f, 0.0f, 1.0f, 0.5f, 0.2f, // right 
    //     0.0f,  0.5f, 0.0f, 1.0f, 0.5f, 0.2f, // top   

    //    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // left  
    //     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // right 
    //     0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f // bottom
    //};

    std::vector<vec3> vs;
    std::vector<vec3> vns;
    std::vector<int> faces;
    std::vector<int> nFaces;
    loadObj("data/pawn.obj", vs, faces, vns, nFaces);

    std::vector<float> orderV;
    std::vector<float> orderVN;
    int count = 1;
    for (int i = 0; i < faces.size(); i++) {
        int vertexIndex = faces[i];
        int normIndex = nFaces[i];

        orderV.push_back(vs[vertexIndex - 1].x);
        orderV.push_back(vs[vertexIndex - 1].y);
        orderV.push_back(vs[vertexIndex - 1].z);

        // Gouraud / Phong shading
        if (vns.size() != 0) {
            orderV.push_back(vns[normIndex - 1].x);
            orderV.push_back(vns[normIndex - 1].y);
            orderV.push_back(vns[normIndex - 1].z);
        }
        else {
            vec3 temp = normalize(vs[vertexIndex - 1]);
            orderV.push_back(temp.x);
            orderV.push_back(temp.y);
            orderV.push_back(temp.z);
        }
        // --------------------------

        /*if (count == 1) {
            orderV.push_back(1.0f);
            orderV.push_back(0.0f);
            orderV.push_back(0.0f);
            count++;
        }
        else if (count == 2) {
            orderV.push_back(0.0f);
            orderV.push_back(1.0f);
            orderV.push_back(0.0f);
            count++;
        }
        else if (count == 3) {
            orderV.push_back(0.0f);
            orderV.push_back(0.0f);
            orderV.push_back(1.0f);
            count = 1;
        }*/
    }


    //float *vertices = new float[orderV.size()];
    unsigned int numVertices = orderV.size() / 3;

    //for (int i = 0; i < orderV.size(); i++) {
    //    //vertices[i] = orderV[i];
    //    std::cout << orderV[i] << std::endl;
    //    if ((i + 1) % 3 == 0)
    //        std::cout << std::endl;
    //}
    
    //EBO
    //float vertices[] = {
    //     0.0f,  0.5f, 0.0f, 1.0f, 0.5f, 0.2f, // top right
    //     0.0f, -0.5f, 0.0f, 1.0f, 0.5f, 0.2f, // bottom right
    //    -1.0f, -0.5f, 0.0f, 1.0f, 0.5f, 0.2f, // bottom left
    //    -1.0f,  0.5f, 0.0f, 1.0f, 0.5f, 0.2f // top left 
    //};
    //unsigned int indices[] = {
    //    0, 1, 3,   // first triangle
    //    1, 2, 3    // second triangle
    //};

    unsigned int VBO, VAO, EBO;
    //unsigned int colorHandle = glGetAttribLocation(shaderProgram, "aColor");
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    //glGenBuffers(1, &colorHandle);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, orderV.size() * sizeof(float), &orderV[0], GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    //glBindBuffer(GL_ARRAY_BUFFER, colorHandle);
    //glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), colorData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        vec3 objColor = vec3(1.0f, 0.5f, 0.3f);
        vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &objColor[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, &lightColor[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &cameraPos[0]);

        // Scale down the obj
        mat4 trans = mat4(1.0f);
        trans = scale(trans, vec3(scaler, scaler, scaler));
        trans = rotate(trans, radians(theta), axis);
        unsigned int matLoc = glGetUniformLocation(shaderProgram, "m");
        glUniformMatrix4fv(matLoc, 1, GL_FALSE, &trans[0][0]);

        mat4 projection = perspective(radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        mat4 view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

        // draw our first triangle
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, numVertices);
        //glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // unbind our VA no need to unbind it every time 
 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //delete[] vertices;

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    //glDeleteBuffers(1, &colorHandle);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void loadObj(const char* objFileName, std::vector<vec3>& vs, std::vector<int>& faces, std::vector<vec3>& vns, std::vector<int>& nFaces)
{
    std::ifstream objFile(objFileName);
    std::string line;
    std::string tempVertex;

    while (std::getline(objFile, line)) {
        if (line.substr(0, 2) == "v ") {
            std::istringstream s(line.substr(2));
            vec3 v;
            std::string temp;
            s >> v.x;
            //vs.push_back(std::stof(temp));
            s >> v.y;
            //vs.push_back(std::stof(temp));
            s >> v.z;
            vs.push_back(v);
        }

        if (line.substr(0, 3) == "vn ") {
            std::istringstream s(line.substr(3));
            vec3 vn;
            s >> vn.x;
            s >> vn.y;
            s >> vn.z;
            vns.push_back(vn);
        }

        if (line.substr(0, 2) == "f ") {
            std::istringstream s(line.substr(2));
            std::string temp;
            std::string altTemp;
            //vec3 f;
            s >> temp;
            std::size_t pos = temp.find("/");
            if (temp[pos + 1] == '/')
                altTemp = temp.substr(pos + 2);
            else
                altTemp = temp.substr(pos + 1);
            
            std::size_t altPos = altTemp.find("/");
            nFaces.push_back(std::stoi(altTemp.substr(0, altPos)));
            
            faces.push_back(std::stoi(temp.substr(0, pos)));
            s >> temp;
            pos = temp.find("/");
            if (temp[pos + 1] == '/')
                altTemp = temp.substr(pos + 2);
            else
                altTemp = temp.substr(pos + 1);

            altPos = altTemp.find("/");
            nFaces.push_back(std::stoi(altTemp.substr(0, altPos)));
            faces.push_back(std::stoi(temp.substr(0, pos)));
            s >> temp;
            pos = temp.find("/");
            if (temp[pos + 1] == '/')
                altTemp = temp.substr(pos + 2);
            else
                altTemp = temp.substr(pos + 1);

            altPos = altTemp.find("/");
            nFaces.push_back(std::stoi(altTemp.substr(0, altPos)));
            faces.push_back(std::stoi(temp.substr(0, pos)));
        }
    }
    objFile.close();
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS && scaler >= 0)
        scaler -= 0.0000001f;

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
        scaler += 0.0000001f;

    if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
        theta += 0.08f;

    if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
        theta -= 0.08f;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        axis = vec3(1.0, 0.0, 0.0);

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        axis = vec3(0.0, 1.0, 0.0);

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        axis = vec3(0.0, 0.0, 1.0);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(2.3 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}