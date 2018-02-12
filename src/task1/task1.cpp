#define GL_GLEXT_PROTOTYPES

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>

#include <nanogui/nanogui.h>

#include <iostream>

using namespace nanogui;

Screen *screen = nullptr;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouseButton(GLFWwindow* window, int button, int action, int mods);
void mouse(GLFWwindow* window, double xpos, double ypos);
void scroll(GLFWwindow* window, double xs, double ys);

const unsigned int SCR_SIZE = 800;
float zoom = 1, x = 0, y = 0, xdrag = 0, ydrag = 0, mul = 3;
bool isPressed = false, firstTime = false;

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
	GLFWwindow* window = glfwCreateWindow(SCR_SIZE, SCR_SIZE, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse);
	glfwSetMouseButtonCallback(window, mouseButton);
	glfwSetScrollCallback(window, scroll);


    screen = new Screen();
    screen->initialize(window, true);

    Window *win = new Window(screen, "Multiplyer size");
    win->setPosition(Eigen::Vector2i(15, 15));
    win->setLayout(new GroupLayout());

    Widget *panel = new Widget(win);
    panel->setLayout(new BoxLayout(Orientation::Horizontal,
                                   Alignment::Middle, 0, 20));

    Slider *slider = new Slider(panel);
    slider->setValue(0.6f);
    slider->setFixedWidth(80);

    TextBox *textBox = new TextBox(panel);
    textBox->setFixedSize(Eigen::Vector2i(60, 25));
    textBox->setValue("300");
    textBox->setUnits("%");
    slider->setCallback([textBox](float value) {
        mul = (value * 2.5 + 1.5);
        textBox->setValue(std::to_string((int) (mul * 100)));
    });
    textBox->setFixedSize(Eigen::Vector2i(60,25));
    textBox->setFontSize(20);
    textBox->setAlignment(TextBox::Alignment::Right);

    screen->setVisible(true);
    screen->performLayout();


    // glad: load all OpenGL function pointers
	// ---------------------------------------
	#if defined(NANOGUI_GLAD)
    	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
        	throw std::runtime_error("Could not initialize GLAD!");
    	glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
	#endif

	// build and compile shaders
	// -------------------------
	Shader shader("src/task1/shader.vs", "src/task1/shader.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	float vertices[] = {
		// aPoss
		 1,  1, 0, // top right
		 1, -1, 0, // bottom right
		-1, -1, 0, // bottom left
		-1,  1, 0 // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// aPos attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

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
	unsigned char *data = stbi_load(FileSystem::getPath("texture/mickey.jpg").c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{

		// input
		// -----
		processInput(window);

		glClear(GL_COLOR_BUFFER_BIT);
		shader.use();
		shader.setFloat("zoom", zoom);
		shader.setFloat("x", x);
		shader.setFloat("y", y);
        shader.setFloat("mul", mul);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		shader.setInt("ourTexture", 0);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        screen->drawContents();
        screen->drawWidgets();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------

	glfwTerminate();
	return 0;
}



float scale(float x) {
	return (x - SCR_SIZE / 2) / (SCR_SIZE / 2);
}

void mouseButton(GLFWwindow* window, int button, int action, int mods) {
    if (screen->mouseButtonCallbackEvent(button, action, mods)) {
        return;
    }
	double _x, _y;
	glfwGetCursorPos(window, &_x, &_y);
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		isPressed = true;
		firstTime = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		isPressed = false;
	}
}

void scroll(GLFWwindow* window, double xs, double ys) {
	double _x, _y;
	glfwGetCursorPos(window, &_x, &_y);
	double mul;
	if (ys > 0) {
		mul = 1.1f;
	}
	else {
		mul = 1 / 1.1f;
	}
	zoom *= mul;
	x = (scale(_x) + x) * mul - scale(_x);
	y = (-scale(_y) + y) * mul + scale(_y);
}


void mouse(GLFWwindow* window, double xpos, double ypos) {
    if (screen->cursorPosCallbackEvent(xpos, ypos)) {
        return;
    }
	if (isPressed && firstTime) {
		xdrag = scale(xpos) + x;
		ydrag = -scale(ypos) + y;
		firstTime = false;
	}
	else if (isPressed) {
		x = xdrag - scale(xpos);
		y = ydrag + scale(ypos);
	}
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window){
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
