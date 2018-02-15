#define GL_GLEXT_PROTOTYPES

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/random.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderScene(const Shader &shader);
void renderCube();
void renderQuad();

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(5.0f, 5.0f, 5.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// meshes
unsigned int planeVAO;


#define NUM_PARTICLES (1024 * 128)
#define WORK_GROUP_SIZE 256

// total number of particles to move
// # work-items per work-group
struct pos
{
    float x, y, z, w;
};
struct vel
{
    float vx, vy, vz, vw;
};
struct color
{
    float r, g, b, a;
};
// need to do the following for both position, velocity, and colors of the particles:
GLuint posSSbo;
GLuint velSSbo;
GLuint colSSbo;
GLuint plane;

int current = 0;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = 0;
    data = stbi_load(FileSystem::getPath("texture/map.jpg").c_str(), &width, &height, &nrChannels, 0);
    //stbi_image_free(data);
    if (data == 0) {
        cerr << "ERROR";
    }
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    int step = 2;
    int w = width / step;
    int h = height / step;

    glm::vec4 planeVertices[(w + 1) * (h + 1) * 2];
    for (int i = 0; i <= w; i++) {
        for (int j = 0; j <= h; j++) {
            planeVertices[(i * (h + 1) + j) * 2].x = float(i);
            planeVertices[(i * (h + 1) + j) * 2].z = float(j);
            planeVertices[(i * (h + 1) + j) * 2].y = float(data[((i * step) + j * step * width)]) / 5;
            planeVertices[(i * (h + 1) + j) * 2].w = 0;
        }
    }

    for (int i = 0; i <= w; i++) {
        for (int j = 0; j <= h; j++) {
            glm::vec3 a = glm::vec3(0);
            glm::vec3 b = glm::vec3(0);

            if (i != 0 && j != 0) {
                glm::vec3 c = glm::vec3(planeVertices[(i * (h + 1) + j - 1) * 2]);
                glm::vec3 d = glm::vec3(planeVertices[((i - 1) * (h + 1) + j) * 2]);
                glm::vec3 e = glm::vec3(planeVertices[(i * (h + 1) + j) * 2]);
                a = glm::normalize(glm::cross(e - c, d - c));
            }

            if (i != w && j != h) {
                glm::vec3 c = glm::vec3(planeVertices[(i * (h + 1) + j + 1) * 2]);
                glm::vec3 d = glm::vec3(planeVertices[((i + 1) * (h + 1) + j) * 2]);
                glm::vec3 e = glm::vec3(planeVertices[(i * (h + 1) + j) * 2]);
                b = glm::normalize(glm::cross(e - c, d - c));
            }
            planeVertices[(i * (h + 1) + j) * 2 + 1] = glm::vec4(glm::normalize(a + b), 0);
        }
    }

    glGenBuffers( 1, &plane);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, plane);
    glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec4), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));
    glBindVertexArray(0);

    // plane VAO
    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec4), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));
    glBindVertexArray(0);

    int indexes[w * h * 6];
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            indexes[(i * h + j) * 6] = i * (h + 1) + j;
            indexes[(i * h + j) * 6 + 1] = i * (h + 1) + j + 1;
            indexes[(i * h + j) * 6 + 2] = (i + 1) * (h + 1) + j;
            indexes[(i * h + j) * 6 + 3] = (i + 1) * (h + 1) + j;
            indexes[(i * h + j) * 6 + 4] = i * (h + 1) + j + 1;
            indexes[(i * h + j) * 6 + 5] = (i + 1) * (h + 1) + j + 1;
        }
    }

    GLuint IBO;

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    Shader shader("src/task4/shader.vs", "src/task4/shader.fs");
    Shader shaderLightBox("src/task4/deferred_light_box.vs", "src/task4/deferred_light_box.fs");
    Shader computeShader("src/task4/compute_shader.glsl");


    glGenBuffers(1, &posSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, posSSbo );
    glBufferData( GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct pos), NULL, GL_STATIC_DRAW);

    glGenBuffers( 1, &velSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, velSSbo);
    glBufferData( GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct vel), NULL, GL_STATIC_DRAW );

    glGenBuffers( 1, &colSSbo);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, colSSbo);
    glBufferData( GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct color) * 6, NULL, GL_DYNAMIC_DRAW );

//    glm::vec3 cube = glm::vec3(5, 5, 5);
//    glm::vec3 speed = glm::vec3(0);
//    glm::vec3 g = glm::vec3(0, -0.000001, 0);
//    glm::vec3 posA, posB, posC, normA, normB, normC;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))  {
        // input
        // -----
        processInput(window);
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 4, posSSbo );
        glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 5, velSSbo );
        glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 6, colSSbo );
        glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 7, plane );

        computeShader.use();
        computeShader.setMat4("view", camera.GetViewMatrix());
        computeShader.setInt("h", h);
        glDispatchCompute( NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1 );
        glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        glm::mat4 model = glm::mat4();
        model = glm::scale(model, glm::vec3(0.05));
        shader.setMat4("model", model);
        shader.setVec3("light", camera.Position);

        glBindVertexArray(planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glDrawElements(GL_TRIANGLES, sizeof(indexes), GL_UNSIGNED_INT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);


        model = glm::mat4();
        model = glm::scale(model, glm::vec3(0.05));

        //glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shaderLightBox.use();
        shaderLightBox.setMat4("projection", projection);
        shaderLightBox.setMat4("view", view);
        shaderLightBox.setMat4("model", model);
        glBindVertexArray(plane);
        glBindBuffer(GL_ARRAY_BUFFER, colSSbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
        glDrawArrays(GL_TRIANGLES, 0, NUM_PARTICLES * 3);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        glDisable(GL_BLEND);
        //glEnable(GL_DEPTH_TEST);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    glfwTerminate();
    return 0;
}

void createBalls() {

    GLint bufMask = GL_MAP_WRITE_BIT ;
// the invalidate makes a big difference when re-writing
    float cff = -0.0;
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, posSSbo);
    struct pos *points = (struct pos *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct pos), bufMask);

    glm::vec3 center = camera.Position * 20.0f + camera.Front * 60.0f;

    for( int i = 0; i < WORK_GROUP_SIZE; i++ )
    {
        points[ (i + current) % NUM_PARTICLES ].x = center.x + glm::linearRand ( -cff, cff );
        points[ (i + current) % NUM_PARTICLES ].y = center.y + glm::linearRand ( -cff, cff );
        points[ (i + current) % NUM_PARTICLES ].z = center.z + glm::linearRand ( -cff, cff );
        points[ (i + current) % NUM_PARTICLES ].w = 1.0f;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );

    glBindBuffer( GL_SHADER_STORAGE_BUFFER, velSSbo);
    struct vel *vels = (struct vel *) glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct vel), bufMask );
    for( int i = 0; i < WORK_GROUP_SIZE; i++ )
    {
        vels[ (i + current) % NUM_PARTICLES ].vx = glm::linearRand ( -1.0f, 1.0f );
        vels[ (i + current) % NUM_PARTICLES ].vy = glm::linearRand ( -1.0f, 1.0f );
        vels[ (i + current) % NUM_PARTICLES ].vz = glm::linearRand ( -1.0f, 1.0f );
        vels[ (i + current) % NUM_PARTICLES ].vw = 0.0f;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );

    current += WORK_GROUP_SIZE;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        createBalls();
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}