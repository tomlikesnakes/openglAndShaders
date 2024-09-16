#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define numVAOs 1

GLuint renderingProgram;
GLuint vao[numVAOs];

GLuint createShaderProgram() {
	
	const char *vshaderSource = 
	"#version 430 \n"
	"void main(void) \n"
	"{ gl_Position = vec4(0.0,0.0,0.0,1.0);} \n";
	
	/*
	const char *vshaderSource = 
	"#version 430 \n"
	"void main(void) \n"
	"{ gl_Position = vec4(gl_VertexID == 0 ? -0.5 : (gl_VertexID == 1 ? 0.5 : 0.0), \n"
	"gl_VertexID == 2 ? 0.5 : -0.5, 0.0, 1.0); \n"
	"}";
	*/

	const char *fshaderSource = 
	"#version 430 \n"
	"out vec4 color; \n"
	"void main(void) \n"
	"{ if(gl_FragCoord.x < 295) color = vec4(1.0,0.0,1.0,1.0); else color = vec4(0.0,1.0,0.0,1.0)}";

GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

glShaderSource(vShader, 1, &vshaderSource, NULL);
glShaderSource(fShader,1,&fshaderSource,NULL);
glCompileShader(vShader);
glCompileShader(fShader);

GLuint vfProgram = glCreateProgram();
glAttachShader(vfProgram, vShader);
glAttachShader(vfProgram, fShader);
glLinkProgram(vfProgram);

return vfProgram;
}
void init(GLFWwindow *window) {
    renderingProgram = createShaderProgram();
    glGenVertexArrays(numVAOs, vao);
    glBindVertexArray(vao[0]);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
}
/*
void init(GLFWwindow *window) {
	renderingProgram = createShaderProgram();
	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);
	glEnable(GL_PROGRAM_POINT_SIZE);
}
*/
void display(GLFWwindow *window, double currentTime) {
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(renderingProgram);	
	//glDrawArrays(GL_TRIANGLES, 0, 3);
	glPointSize(30.0f);
	glDrawArrays(GL_POINTS, 0, 1);
}

int main(void) {
	if (!glfwInit()) return 1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(600,600,"Chapter2 - program1", NULL,NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) return 1;
	glfwSwapInterval(1);

	init(window);

	while(!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
