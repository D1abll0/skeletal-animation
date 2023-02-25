#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm\gtx\dual_quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../include/shader.h"
#include "../include/camera.h"
#include "../include/stb_image.h"
#include "../include/mesh.h"
#include "../include/model.h"

#include <iostream>

using namespace glm;

const int W = 800;
const int H = 600;

Camera camera(vec3(0.0f, 0.0f, 3.0f));

float lastX = W / 2;
float lastY = H / 2;
bool firstMouse = 1;

vector<mat4> Transforms;
vector<mat2x4> DQs;
vector<fdualquat> dualQuaternions;
float dt = 0;
float lastFrame = 0;
float animationTime = 0;

vec3 lightPos(1.2f, 1.0f, 2.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    	glfwSetWindowShouldClose(window, true);
    
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    	camera.ProcessKeyboard(FORWARD, dt);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    	camera.ProcessKeyboard(BACKWARD, dt);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
     	camera.ProcessKeyboard(LEFT, dt);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
     	camera.ProcessKeyboard(RIGHT, dt);
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, dt);
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, dt);  
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if(firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = 0;
	}
	float ox = xpos - lastX;
	float oy = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	
	camera.ProcessMouseMovement(ox, oy);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

int main()
{	
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWwindow* window = glfwCreateWindow(W, H, "", NULL, NULL);
	if (!window)
	{
	    glfwTerminate();
	    return -1;
	}
	
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	gladLoadGL();
	
	glEnable(GL_DEPTH_TEST);
	
	Shader shader("./shaders/shader.vs", "./shaders/shader.fs");
//	Shader shader("./shaders/1.model_loading.vs", "./shaders/1.model_loading.fs");

	Model mdl("./resources/man/model.dae");
	
	float startFrame = glfwGetTime();
	int a = 0;
    while (!glfwWindowShouldClose(window))
    {
        float curFrame = glfwGetTime();
        dt = curFrame - lastFrame;
        lastFrame = curFrame;
        
        animationTime = curFrame - startFrame;
        
    	processInput(window);
    	
    	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                
		shader.use();
        
		mat4 projection = perspective(radians(camera.Zoom), (float)W / (float)H, 0.001f, 100.0f);
		shader.setMat4("projection", projection);
		
    	mat4 view = camera.GetViewMatrix();
		shader.setMat4("view", view);
		
		mat4 model = mat4(1.0f);
		model = scale(model, vec3(.5, .5, .5));
		model = translate(model, vec3(0, 0, 0));
		const quat& rot = angleAxis(radians(-90.f), vec3(1.f, 0.f, 0.f));/*rotation*/
		model *= mat4_cast(rot);
		shader.setMat4("model", model);
		

		mdl.BoneTransform(animationTime, Transforms, dualQuaternions);
		
		for(unsigned int i = 0; i < Transforms.size(); ++i)
		{
			const string name = "gBones[" + to_string(i) + "]";
			GLuint boneTransform = glGetUniformLocation(shader.ID, name.c_str());
			shader.setMat4(name, Transforms[i]);
		}
		DQs.resize(dualQuaternions.size());
		
		for (unsigned int i = 0; i < dualQuaternions.size(); ++i)
		{
			DQs[i] = mat2x4_cast(dualQuaternions[i]);
			const string name = "dqs[" + to_string(i) + "]";
			shader.setMat2x4(name, DQs[i]);
		}
		
		if(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
			shader.setBool("optimised", GL_TRUE);
		else
			shader.setBool("optimised", GL_FALSE);
				
		mdl.Draw(shader);
				        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;	
}
