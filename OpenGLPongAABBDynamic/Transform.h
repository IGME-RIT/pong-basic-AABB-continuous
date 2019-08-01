#pragma once

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
class Transform
{
	glm::mat4 rotMatrix;
	glm::mat4 worldMatrix;

	//We make our quaternion private
	//Usually we only want our Euler angles accessible, and have quaternions work underneath
	glm::quat rotation;
	

	void CalculateWorldMatrix();
public:
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 eulerRotation;
	bool recalculateWMatrix = false;

	Transform();
	~Transform();

	glm::mat4 GetWorldMatrix();
	glm::vec3 GetForward();
	glm::vec3 GetRight();
	void MoveForward(GLfloat distance);
	void MoveRight(GLfloat distance);

	
	void Update();
};

