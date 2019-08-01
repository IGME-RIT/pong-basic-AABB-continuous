
#include <iostream>
#include <vector>

// We are using the glew32s.lib
// Thus we have a define statement
// If we do not want to use the static library, we can include glew32.lib in the project properties
// If we do use the non static glew32.lib, we need to include the glew32.dll in the solution folder
// The glew32.dll can be found here $(SolutionDir)\..\External Libraries\GLEW\bin\Release\Win32
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Transform.h"
#include "ShaderProgram.h"



// Variables for the Height and width of the window
const GLint WIDTH = 800, HEIGHT = 800;
const GLfloat ballVelocity = 1.0f;
const GLfloat playerVelocity = 1.5f;
glm::vec3 ballDirection = glm::normalize(glm::vec3(1, -1, 0));


GLuint VAO;
GLuint VBO;

//Move a paddle based on input
int MovePlayer(GLFWwindow* window, int pNum)
{
	if (pNum == 1)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			return 1;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			return -1;
		}

		return 0;

	}

	if (pNum == 2)
	{
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			return 1;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			return -1;
		}

		return 0;
	}

}

struct AABB
{
	glm::vec3 min;
	glm::vec3 max;

	AABB()
	{
		min = glm::vec3();
		max = glm::vec3();
	}

};

//Calculate min and max of a set of points
void CalcAABB(GLfloat vertices[], glm::mat4 worldMatrix, AABB* AABB)
{
	glm::vec4 c_min = worldMatrix * glm::vec4(vertices[0], vertices[1], vertices[2], 1.0);
	glm::vec4 c_max = c_min;
	glm::vec4 vertPos = glm::vec4();

	for (int i = 0; i < 18; i += 3)
	{
		vertPos = worldMatrix * glm::vec4(vertices[i], vertices[i + 1], vertices[i + 2], 1.0);

		if (vertPos.x > c_max.x)
			c_max.x = vertPos.x;
		if (vertPos.y > c_max.y)
			c_max.y = vertPos.y;

		if (vertPos.x < c_min.x)
			c_min.x = vertPos.x;
		if (vertPos.y < c_min.y)
			c_min.y = vertPos.y;

	}
	AABB->min= c_min;
	AABB->max = c_max;
}

//Check for axis overlap
bool AABBCollision(AABB box1, AABB box2)
{
	if (box1.max.x < box2.min.x || box1.min.x > box2.max.x)
		return false;

	if (box1.max.y < box2.min.y || box1.min.y > box2.max.y) 
		return false;

	return true;
}

//Taken from https://github.com/IGME-RIT/physics-AABB-AABB2D-VisualStudio
//mvm is the realative velocity vector from box1's space 
float AABBCollisionDynamic(AABB box1, AABB box2, glm::vec3 mvmt)
{
	if (AABBCollision(box1, box2))
		return 0.0f;

	float tFirst = 0.0f;
	float tLast = 1.0f;
	float tCurrent = 0.0f;

	//Checking each axis
	for (int axis = 0; axis < 2; axis++)
	{
		//relative velocity is negative	on this axis
		if (mvmt[axis] < 0.0f)
		{
			//max of box1 is less than min of box 2, so there is no overlap on this axis and they won't collide
			if (box1.max[axis] < box2.min[axis])
				return -1.0f;

			
			if (box1.min[axis] > box2.max[axis])	//Not yet collliding
			{
				tCurrent = (box2.max[axis] - box1.min[axis]) / mvmt[axis];	//Find out the point in this frame when they collide

				if (tCurrent > tFirst) tFirst = tCurrent;
			}

			if (box1.max[axis] > box2.min[axis])	//Not yet seperated
			{
				tCurrent = (box2.min[axis] - box1.max[axis]) / mvmt[axis];	//Find out the point in this frame when they seperate

				if (tCurrent < tLast) tLast = tCurrent;				
			}
		}
		//relative velocity is positive	on this axis
		else if (mvmt[axis] > 0.0f)
		{
			//min of box1 is more than max of box 2, so there is no overlap on this axis and they won't collide
			if (box1.min[axis] > box2.max[axis])
				return -1.0f;

			if (box1.max[axis] < box2.min[axis])	//Not yet collliding
			{
				tCurrent = (box2.min[axis] - box1.max[axis]) / mvmt[axis];	//Find out the point in this frame when they collide

				if (tCurrent > tFirst) tFirst = tCurrent;
			}

			if (box1.min[axis] < box2.max[axis])	//Not yet seperated
			{
				tCurrent = (box2.max[axis] - box1.min[axis]) / mtvmvmtaxis]; //Find out the point in this frame when they seperate

				if (tCurrent < tLast) tLast = tCurrent;
			}
		}
	}

	//If theres no overlap return -1
	if (tFirst > tLast)
		tFirst = -1;

	return tFirst;
}

void ResolveCollision(Transform* player, Transform* ball)
{
	//Collision with player 1
	if (ball->position.x < 0)
	{
		//Check if the ball is behind or in front of the paddle
		if(ball->position.x > player->position.x)
			ballDirection = glm::normalize(glm::reflect((player->position - ball->position), glm::vec3(1, 0, 0)));

		return;
	}

	//Collision with player 2

	//Check if the ball is behind or in front of the paddle
	if (ball->position.x < player->position.x)
		ballDirection = glm::normalize(glm::reflect((player->position - ball->position), glm::vec3(-1, 0, 0)));
}


int main()
{
#pragma region GL setup
	//Initializes the glfw
	glfwInit();

	// Setting the required options for GLFW

	// Setting the OpenGL version, in this case 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 99);
	// Setting the Profile for the OpenGL.

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	// Setting the forward compatibility of the application to true
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// We don't want the window to resize as of now.
	// Therefore we will set the resizeable window hint to false.
	// To make is resizeable change the value to GL_TRUE.
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create the window object
	// The first and second parameters passed in are WIDTH and HEIGHT of the window we want to create
	// The third parameter is the title of the window we want to create
	// NOTE: Fourth paramter is called monitor of type GLFWmonitor, used for the fullscreen mode.
	//		 Fifth paramter is called share of type GLFWwindow, here we can use the context of another window to create this window
	// Since we won't be using any of these two features for the current tutorial we will pass nullptr in those fields
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Shaders Tutorial", nullptr, nullptr);

	// We call the function glfwGetFramebufferSize to query the actual size of the window and store it in the variables.
	// This is useful for the high density screens and getting the window size when the window has resized.
	// Therefore we will be using these variables when creating the viewport for the window
	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	// Check if the window creation was successful by checking if the window object is a null pointer or not
	if (window == nullptr)
	{
		// If the window returns a null pointer, meaning the window creation was not successful
		// we print out the messsage and terminate the glfw using glfwTerminate()
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		// Since the application was not able to create a window we exit the program returning EXIT_FAILURE
		return EXIT_FAILURE;
	}

	// Creating a window does not make it the current context in the windows.
	// As a results if the window is not made the current context we wouldn't be able the perform the operations we want on it
	// So we make the created window to current context using the function glfwMakeContextCurrent() and passing in the Window object
	glfwMakeContextCurrent(window);

	// Enable GLEW, setting glewExperimental to true.
	// This allows GLEW take the modern approach to retrive function pointers and extensions
	glewExperimental = GL_TRUE;

	// Initialize GLEW to setup OpenGL function pointers
	if (GLEW_OK != glewInit())
	{
		// If the initalization is not successful, print out the message and exit the program with return value EXIT_FAILURE
		std::cout << "Failed to initialize GLEW" << std::endl;

		return EXIT_FAILURE;
	}

	// Setting up the viewport
	// First the parameters are used to set the top left coordinates
	// The next two parameters specify the height and the width of the viewport.
	// We use the variables screenWidth and screenHeight, in which we stored the value of width and height of the window,
	glViewport(0, 0, screenWidth, screenHeight);
#pragma endregion

#pragma region Game_Setup
	float ballWidth = 0.02f;
	float ballHeight = 0.02f;
	Transform* player1 = new Transform();
	Transform* player2 = new Transform();

	player1->scale = player2->scale = glm::vec3(ballWidth, ballHeight*4, 1.0);
	player1->position = glm::vec3(-0.8f, 0.0, 0.0);
	player2->position = glm::vec3(0.8f, 0.0, 0.0);
	player1->Update();
	player2->Update();

	Transform* ball = new Transform();
	ball->position = glm::vec3(0.0, 0.8, 0.0);
	ball->scale = glm::vec3(ballWidth, ballHeight, 1.0);
	ball->Update();

	glm::vec2 score = glm::vec2(0, 0);


	AABB p1Box = AABB();
	AABB p2Box = AABB();
	AABB ballBox = AABB();
#pragma endregion
	GLfloat vertices[] = {
		// Triangle 1		
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		//Triangle 2
		-0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f
	};



	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3* sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Shader*	vShader = new Shader((char*)"shaders/vShader.glsl", GL_VERTEX_SHADER);
	Shader*	fShader = new Shader((char*)"shaders/fShader.glsl", GL_FRAGMENT_SHADER);
	ShaderProgram* shader = new ShaderProgram();
	shader->AttachShader(vShader);
	shader->AttachShader(fShader);
	shader->Bind();

	GLint uniform = glGetUniformLocation(shader->GetGLShaderProgram(), (char*)"worldMatrix");
	// This is the game loop, the game logic and render part goes in here.
	// It checks if the created window is still open, and keeps performing the specified operations until the window is closed
	while (!glfwWindowShouldClose(window))
	{

		// Calculate delta time.
		float dt = glfwGetTime();
		// Reset the timer.
		glfwSetTime(0);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, screenWidth, screenHeight);

		
#pragma region Player_Updates
		player1->position.y += MovePlayer(window, 1) * playerVelocity * dt;
		player1->position.y = glm::clamp(player1->position.y, -0.9f, 0.9f);
		player1->Update();
		

		player2->position.y += MovePlayer(window, 2) * playerVelocity * dt;
		player2->position.y = glm::clamp(player2->position.y, -0.9f, 0.9f);
		player2->Update();
		
#pragma endregion

#pragma region Ball_Updates
     	CalcAABB(vertices, player1->GetWorldMatrix(),&p1Box);
		CalcAABB(vertices, player2->GetWorldMatrix(), &p2Box);
		CalcAABB(vertices, ball->GetWorldMatrix(), &ballBox);

		bool collision = false;
		//Since input velocity is relative to box1, negate ball velocity
		if (!AABBCollision(p1Box, ballBox))
		{
			float t = AABBCollisionDynamic(p1Box, ballBox, -ballDirection*ballVelocity * dt);
			//If there is a collision this frame
			if (t >= 0)
			{
				ResolveCollision(player1, ball);
				ball->position += ballDirection * ballVelocity * dt * t;
				collision = true;
			}

			
		}
		if (!AABBCollision(p2Box, ballBox))
		{
			float t = AABBCollisionDynamic(p2Box, ballBox, -ballDirection*ballVelocity * dt);
			//If there is a collision this frame
			if (t >= 0)
			{
				ResolveCollision(player2, ball);
				//Move ball out of 
				ball->position += ballDirection * ballVelocity * dt * t;
				collision = true;
			}

		}

		if (!collision)
			ball->position += ballDirection * ballVelocity * dt;
		else
			collision = false;
		
		if (ball->position.y >= 1 || ball->position.y <= -1)
			ballDirection.y *= -1;

		ball->Update();

		//Respawn ball
		if (ball->position.x >= 1)
		{
			score.x++;
			ball->position = glm::vec3(0.0, -0.8, 0.0);
			ballDirection = glm::normalize(glm::vec3(-1, 1, 0));

			printf("Player 1 Scored! Current score is %u to %u\n", (int)score.x, (int)score.y );
		}
		else if (ball->position.x <= -1)
		{
			score.y++;
			ball->position = glm::vec3(0.0, 0.8, 0.0);
			ballDirection = glm::normalize(glm::vec3(1, -1, 0));
			printf("Player 2 Scored! Current score is %u to %u\n", (int)score.x, (int)score.y);
		}


		ball->Update();
		//Check win condition
		if (score.x == 5)
		{
			printf("Player 1 Wins! Score is reset.\n");
			score.x = score.y = 0;
		}
		if (score.y == 5)
		{
			printf("Player 2 Wins! Score is reset.\n");
			score.x = score.y = 0;
		}
#pragma endregion

		glBindVertexArray(VAO);	

		glUniformMatrix4fv(uniform, 1, GL_FALSE, &ball->GetWorldMatrix()[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glUniformMatrix4fv(uniform, 1, GL_FALSE, &player1->GetWorldMatrix()[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glUniformMatrix4fv(uniform, 1, GL_FALSE, &player2->GetWorldMatrix()[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);


		// Swaps the front and back buffers of the specified window
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	delete player1, player2, ball, shader;
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	// Terminate all the stuff related to GLFW and exit the program using the return value EXIT_SUCCESS
	glfwTerminate();

	return EXIT_SUCCESS;
}


