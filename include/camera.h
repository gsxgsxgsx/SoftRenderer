#ifndef _CAMERA_H
#define _CAMERA_H

#include <iostream>
#include "geometry.h"
#include "math.h"

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,

    UP,
    DOWN,
    ROTATE_LEFT,
    ROTATE_RIGHT
};
// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 0.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

struct Camera
{
    Vec3f eye;
    Vec3f center;
    Vec3f up;
    Vec3f front;
    Vec3f right;

    // euler Angles
    int pitch; //yz平面
    int yaw;   //xz平面
    int roll;  //xy平面

    // camera options
    float MovementSpeed;
    float MouseSensitivity = 0.1f;
    float zoom = 45.0f;

    Camera() : eye(Vec3f(0, 0, 1.f)), center(Vec3f(0)), up(Vec3f(0, 1.f, 0)), front(Vec3f(0, 0, -1.f)) {}

    Camera(Vec3f eye_, Vec3f center_, Vec3f up_) : eye(eye_), center(center_), up(up_), front(Vec3f(0, 0, -1.f)) {}

    Matrix getViewMat()
    {
        return lookat(this->eye, this->eye + front, this->up);
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = SPEED * deltaTime;
        switch (direction)
        {
        case FORWARD:
        {
            eye = eye + front * velocity;
        }
        break;
        case BACKWARD:
        {
            eye = eye - front * velocity;
        }
        break;
        case LEFT:
        {
            eye = eye - right * velocity;
        }
        break;
        case RIGHT:
        {
            eye = eye + right * velocity;
        }
        break;
            //pitch俯仰角，x轴
        case UP:
        {
            Matrix r = Matrix::identity();
            rotate(1, Vec3f(1, 0, 0), r);
            front = proj<3>(r * embed<4>(front)).normalize();
        }
        break;
        case DOWN:
        {
            pitch += 5;
            pitch = (pitch + 360) % 360;
            center.x = cos(radians(pitch));
            center.y = sin(radians(pitch));
            center.z = cos(radians(pitch));
            center.normalize();
        }
        break;
        case ROTATE_LEFT:
        {
            Matrix r = Matrix::identity();
            rotate(1, Vec3f(0, 1, 0), r);
            front = proj<3>(r * embed<4>(front)).normalize();
        }
        break;
        case ROTATE_RIGHT:
        {
            yaw += 5;
            //yaw = (yaw + 360) % 360;
            center.x = cos(radians(pitch)) * cos(radians(yaw));
            center.y = sin(radians(pitch));
            center.z = cos(radians(pitch)) * sin(radians(yaw));
        }
        break;
        }
        //updateCameraVectors();
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;



        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        zoom -= (float)yoffset;
        if (zoom < 1.0f)
            zoom = 1.0f;
        if (zoom > 45.0f)
            zoom = 45.0f;
    }

    //private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        Vec3f front;
        front.x = cos(radians(yaw)) * cos(radians(pitch));
        front.y = sin(radians(pitch));
        front.z = sin(radians(yaw)) * cos(radians(pitch));
        front.normalize();

        // also re-calculate the Right and Up vector
        right = cross(front, Vec3f(0, 1, 0)).normalize(); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        up = cross(right, front).normalize();
    }

    Matrix update(){
        return lookat(eye,center,up);
    }
};

#endif
