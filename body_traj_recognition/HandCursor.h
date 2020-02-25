#ifndef HANDCURSOR_H
#define HANDCURSOR_H
#pragma once

#include "Circle.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "Object2D.h"
#define NCHIST 5

// object to keep track of hand position/velocity and display cursor
class HandCursor : public Circle
{
private:
	GLfloat xTrue;  //this is the true hand position, sans the cursor gain/rotation manipulations
	GLfloat yTrue;
	GLfloat zTrue;
	GLfloat xhist[NCHIST]; // x velocity history
	GLfloat yhist[NCHIST]; // y velocity history
	GLfloat zhist[NCHIST]; // y velocity history
	GLfloat xhistScr[NCHIST]; // x screen position history 
	GLfloat yhistScr[NCHIST]; // y screen position history
	GLfloat zhistScr[NCHIST]; // y screen position history

	GLfloat x0;  //this is the defined origin for any rotations or gain changes of the cursor
	GLfloat y0;
	GLfloat z0;

	//GLfloat zvel;
	//GLfloat zpos;

	GLfloat xgain; 
	GLfloat ygain;
	GLfloat zgain;

	GLfloat rotMat[4]; //rotation matrix

	GLfloat hitMargin;

	GLint clamp; // controls whether clamp is on
	GLfloat thetaClamp; // clamp angle

	using Circle::Explode;  //make function private

public:
	//constructor function call: inherit from Circle
	HandCursor(GLfloat x, GLfloat y, GLfloat diam, GLfloat clr[]);
	HandCursor(GLfloat x, GLfloat y, GLfloat z, GLfloat diam, GLfloat clr[]);

	// destructor
	~HandCursor(){}

	GLfloat GetTrueX();  //returns the actual current hand position, non-distorted
	GLfloat GetTrueY();  //returns the actual current hand position, non-distorted
	GLfloat GetLastX();  //returns the last hand-cursor position, non-distorted
	GLfloat GetLastY();  //returns the last hand-cursor position, non-distorted
	GLfloat GetMeanX();  //returns the mean saved hand-cursor position, non-distorted
	GLfloat GetMeanY();  //returns the mean saved hand-cursor position, non-distorted
	GLfloat GetXVel();   //returns the actual x hand velocity, non-distorted
	GLfloat GetYVel();   //returns the actual y hand velocity, non-distorted
	GLfloat GetVel2D();    //returns the actual 2D hand velocity, non-distorted
	GLfloat GetHitMargin();  //returns the multiplication factor for the target radius used to determine a Hit

	//void UpdatePos(GLfloat x, GLfloat y);  //input is the actual hand position (non-distorted); saves both the true hand position and updates the potentially distorted cursor position. also recalculates the instantaneous hand velocity. overrides "UpdatePos" in Circle class
	//void UpdatePos(GLfloat x, GLfloat y, GLfloat z = 0.0f);  //input is the actual hand position (non-distorted); saves both the true hand position and updates the potentially distorted cursor position. also recalculates the instantaneous hand velocity. overrides "UpdatePos" in Circle class
	void UpdatePos(GLfloat x, GLfloat y);  //input is the actual hand position (non-distorted); saves both the true hand position and updates the potentially distorted cursor position. also recalculates the instantaneous hand velocity. overrides "UpdatePos" in Circle class
	void UpdatePos(GLfloat x, GLfloat y, GLfloat z);  //input is the actual hand position (non-distorted); saves both the true hand position and updates the potentially distorted cursor position. also recalculates the instantaneous hand velocity. overrides "UpdatePos" in Circle class

	void SetHitMargin(GLfloat m = 1.0f);  //set the multiplication factor for the target radius used to determine a Hit (default = 1.0f)
	void SetRotation(GLfloat theta); // set rotation angle in degrees CW
	void SetOrigin(GLfloat x, GLfloat y); // set origin around which rotation occurs

	GLfloat Distance(GLfloat x, GLfloat y, GLfloat z = 0.0f); // get distance from the potentially distorted cursor to an (x,y) position
	GLfloat Distance(Circle* c);            // get distance from the potentially distorted cursor to a circle object
	GLfloat Distance(Object2D* obj);		// get distance from the potentially distorted cursor to an Object2D

	void SetGain(GLfloat xg, GLfloat yg);  // set gain
	
	void SetClamp(GLfloat th);
	void SetClampC(Circle* c1, Circle* c2); // sets a clamp through the center of two points
	void ClampOff(); //turn off the cursor clamp

	void Null();  //reset set the rotation matrix such that no rotation is present

	bool HitTarget(Circle* targ);  //compute if the potentially distorted cursor hit a circle object
	bool HitTarget(GLfloat x, GLfloat y, GLfloat rad);  //compute if the potentially distorted cursor is within rad distance of a given (x,y) position


	
	//*****************
	//new functions that work in 3D
	GLfloat GetTrueZ();  //returns the actual current hand position, non-distorted
	GLfloat GetMeanZ();  //returns the mean saved hand-cursor position, non-distorted
	GLfloat GetVel3D();    //returns the actual 3D hand velocity, non-distorted

	//UpdatePos(GLfloat x, GLfloat y, GLfloat z = 0.0f);
	//Distance(GLfloat x, GLfloat y, GLfloat z = 0.0f); -- but the ones referencing circle or object do not!


};

#endif