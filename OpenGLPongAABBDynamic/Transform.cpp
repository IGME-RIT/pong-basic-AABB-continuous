#include "Transform.h"



Transform::Transform()
{
	scale = glm::vec3(1.f, 1.f, 1.f);
	position = glm::vec3(0.0f, 0.0f, 0.0f);
	eulerRotation = glm::vec3(0.0f, 0.0f, 0.0f);
	CalculateWorldMatrix();
}


Transform::~Transform()
{
}

glm::mat4 Transform::GetWorldMatrix()
{
	return worldMatrix;
}

glm::vec3 Transform::GetForward()
{
	return rotation * glm::vec4(0, 0, -1, 1);
}

void Transform::MoveForward(GLfloat distance)
{
	position += distance * GetForward();
}

glm::vec3 Transform::GetRight()
{
	return rotation * glm::vec4(1, 0, 0, 1);
}

void Transform::MoveRight(GLfloat distance)
{
	position += distance * GetRight();
}


void Transform::CalculateWorldMatrix()
{
	//http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/#quaternions
	//Quaternions are one of the best ways to represent rotations in 3D space
	//At its core quaternions are 4 values
	//Euler angles are vulnerable to gimbal lock https://en.wikipedia.org/wiki/Gimbal_lock, whereas quaternions are free from it
	//This is becuase euler angles happen in sequence( Y rotation, then X rotation, then Z rotation) but quaternions
	//calculate the total rotation all at once

	rotation = glm::quat(eulerRotation);

	rotMatrix = glm::mat4_cast(rotation);

	//WORLD MATRIX CALCULATION
		//We want to create a 4x4 matrix for our triangle
		//This matrix will have 3 main components
		//Translation matrix - tells us where to move our vertices
		//Rotation matrix -	tells us how to rotate the model
		//Scale matrix - tells us the size of the model
		//World Matrix = Translation Matrix* Rotation Matrix *Scale Matrix  ***IN ORDER***
		//With matrix multiplication, order matters.
		//Otherwise we manipulate vertices incorrectly
	worldMatrix = glm::translate(position) *													//creates a translation matrix taking our position as the parameter
		rotMatrix*
		glm::scale(scale);															//creates a scale matrix taking our scale vector

	recalculateWMatrix = false;
}

void Transform::Update()
{
	//if (recalculateWMatrix)
		CalculateWorldMatrix();


}
