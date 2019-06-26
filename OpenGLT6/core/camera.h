#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include "glm.h"

#include <vector>
#define GLGH_CAMERA_PI 3.14159265f
#define GLGH_SMOOTH_COS 0xB1
#define GLGH_INSTANT_COS 0xB2
#define GLGH_EASE_IN_COS 0xB3
#define GLGH_EASE_OUT_COS 0xB4
#define GLGH_LINEAR 0xB5
// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP, 
	DOWN,
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

struct glgh_CameraMovement
{
	float time;
	int function;
	float x;
	float y;
	float z;
	float yaw;
	float pitch;
	float zoom;
	bool constrainPitch;
};

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float HandMoveSensitivity = 4.8f;
	float Zoom;
	//if the camera is moving back
	bool MovingBack = false;
	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		if (direction == UP)
			Position += Up * velocity;
		if (direction == DOWN)
			Position -= Up * velocity;
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}
		
		// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	void ProcessHandMovement(float xoffset, float yoffset, int width, int height) {
		Position += glm::vec3(-xoffset / height * HandMoveSensitivity, -yoffset / width * HandMoveSensitivity, 0);
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}
	void PassDeltaTime(float deltaTime) 
	{
		if (MovingBack == true) {
			BudgeCamera(deltaTime, 0);
		}
	}
	void BudgeCamera(float deltaTime, float time, int function = GLGH_SMOOTH_COS, float toX=0.0f, float toY=0.0f, float toZ=3.0f, float toYaw=-90.0f, float toPitch=-0.0f, float toZoom=45.0f, bool constrainPitch=true) 
	{
		static float Ax = 0;
		static float Ay = 0;
		static float Az = 0;
		static float omega;
		static float timeAccumulation = 0;
		static float maxTime = 0;
		static float Ayaw;
		static float Apitch;
		static float Azoom;
		static float mtoX, mtoY, mtoZ, mtoYaw, mtoPitch, mtoZoom;
		static int mfunction = function;
		static float cosLifter;
		static float phi;
		float temp_cos;
		if (function == GLGH_INSTANT_COS) {
			MovingBack = false;
			Position.x = toX;
			Position.y = toY;
			Position.z = toZ;
			if (toZoom <= 1.0f)
				toZoom = 1.0f;
			else if (toZoom >= 45.0f)
				toZoom = 45.0f;
			if (constrainPitch) {
				if (toPitch > 89.0f)
					toPitch = 89.0f;
				if (toPitch < -89.0f)
					toPitch = -89.0f;
			}
			Yaw = toYaw;
			Pitch = toPitch;
			Zoom = toZoom;
			updateCameraVectors();
			return;
		}
		if (MovingBack == false) {
			Ax = (Position.x - toX);
			Ay = (Position.y - toY);
			Az = (Position.z - toZ);

			Ayaw = (Yaw - toYaw);
			Apitch = (Pitch - toPitch);
			Azoom = (Zoom - toZoom);

			mfunction = function;
			if (function < 0xB1 || function > 0xB5) {
				mfunction = GLGH_SMOOTH_COS;
				printf("Fails to recognize ease function, will use default function: (cosine) smooth\n");
			}
				

			if (function == GLGH_LINEAR) {
				Ax     /= float(time);
				Ay     /= float(time);
				Az     /= float(time);
				Ayaw   /= float(time);
				Apitch /= float(time);
				Azoom  /= float(time);
			}
			else if(mfunction == GLGH_SMOOTH_COS || function == GLGH_EASE_OUT_COS)
			{
				Ax     /= 2.0f;
				Ay     /= 2.0f;
				Az     /= 2.0f;
				Ayaw   /= 2.0f;
				Apitch /= 2.0f;
				Azoom  /= 2.0f;
				cosLifter = 1.0f;
				phi = 0.0f;
			}
			omega = GLGH_CAMERA_PI / time;
			if (function == GLGH_EASE_IN_COS) {
				phi        = 0.0f;
				omega     /= 2;
				cosLifter  = 0.0f;
			}
			else if(function == GLGH_EASE_OUT_COS)
			{
				phi        = time / 2;
				omega     /= 2;
			}

			mtoX     = toX;
			mtoY     = toY;
			mtoZ     = toZ;
			mtoYaw   = toYaw;
			mtoPitch = toPitch;
			mtoZoom  = toZoom;
			if (mtoZoom <= 1.0f)
				mtoZoom = 1.0f;
			else if (mtoZoom >= 45.0f)
				mtoZoom = 45.0f;
			if (constrainPitch) {
				if (mtoPitch > 89.0f)
					mtoPitch = 89.0f;
				if (mtoPitch < -89.0f)
					mtoPitch = -89.0f;
			}
			
			timeAccumulation = deltaTime;
			
			maxTime = time;
			MovingBack = true;
			
		}
		else {
			timeAccumulation += deltaTime;
			if (timeAccumulation > maxTime) {
				MovingBack = false;
				Position.x = mtoX;
				Position.y = mtoY;
				Position.z = mtoZ;
				Yaw = mtoYaw;
				Pitch = mtoPitch;
				Zoom = mtoZoom;
			}
			else {
				if (mfunction == GLGH_SMOOTH_COS || mfunction == GLGH_EASE_IN_COS || mfunction == GLGH_EASE_OUT_COS) {
					temp_cos = cosf(omega * (timeAccumulation+phi)) + cosLifter;
					Position.x = Ax * temp_cos + mtoX;
					Position.y = Ay * temp_cos + mtoY;
					Position.z = Az * temp_cos + mtoZ;
					Yaw = Ayaw * temp_cos + mtoYaw;
					Pitch = Apitch * temp_cos + mtoPitch;
					Zoom = Azoom * temp_cos + mtoZoom;
				}
				else if (mfunction == GLGH_LINEAR) {
					Position.x = mtoX     + float(Ax     * (maxTime - timeAccumulation));
					Position.y = mtoY     + float(Ay     * (maxTime - timeAccumulation));
					Position.z = mtoZ     + float(Az     * (maxTime - timeAccumulation));
					Yaw        = mtoYaw   + float(Ayaw   * (maxTime - timeAccumulation));
					Pitch      = mtoPitch + float(Apitch * (maxTime - timeAccumulation));
					Zoom       = mtoZoom  + float(Azoom  * (maxTime - timeAccumulation));
				}
			}
			updateCameraVectors();
		}
	}

private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};
#endif