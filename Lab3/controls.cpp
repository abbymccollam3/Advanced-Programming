/*
Author: Abby McCollam
Class: ECE4122 Section A
Last Date Modified: 10/28/23
Description:

Controls file for keys that control movement of 4 suzanne objects + rectangle

*/

// Include GLFW
#include <glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

bool additionalLight = true;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}
bool getLight() { //need to use boolean function to use in main
    return additionalLight;
}

// Initial horizontal angle : toward -Z
float radius = 10.0f;
float horizontalAngle = glm::radians(45.0f);
// Initial vertical angle : none
float verticalAngle = glm::radians(45.0f);
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
//float mouseSpeed = 0.005f;

void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();
    static double lastSetTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

    static double lastTime_1 = glfwGetTime();

	// Get mouse position
	double xpos, ypos;
	//glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	//glfwSetCursorPos(window, 1024/2, 768/2);

	//changing position to Cartesian coordinates
	glm::vec3 position
    (
		radius * sin(verticalAngle) * cos(horizontalAngle),
        radius * cos(verticalAngle),
        radius * sin(verticalAngle) * sin(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3 (sin(horizontalAngle - 3.14f/2.0f), 0, cos(horizontalAngle - 3.14f/2.0f));
	
	// Up vector
	//glm::vec3 up = glm::cross( right, direction );
    glm::vec3 up(0, (sin(verticalAngle) > 0) ? 1 : -1, 0); //ternary operator to check if sin(verticalAngle) is greater than 0

	// up arrow zooms in
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
                radius -= deltaTime * speed * 3.0f;
        }

	// down arrow zooms out
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
                radius += deltaTime * speed * 3.0f;
        }

    //USING ANGLES FOR ROTATIONS OF CAMERA

	// left arrow rotates camera left
        if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
            horizontalAngle += deltaTime * speed;
        }

	// right arrow rotates camera right
        if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
            horizontalAngle -= deltaTime * speed;
        }

	// U rotates camera up
        if (glfwGetKey( window, GLFW_KEY_U ) == GLFW_PRESS){
                verticalAngle -= deltaTime * speed;
        }

	// D rotates camera down
        if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
                verticalAngle += deltaTime * speed;
        }

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(position, glm::vec3(0,0,0), up); // (position,position+direction,up); // camera is here and looks here : at the same position, plus "direction" and head is up (set to 0,-1,0 to look upside-down)
    
	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
