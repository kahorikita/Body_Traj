#include <cmath>
#include <limits>
#include <sstream>
#include <iostream>
#include <fstream>
#include <istream>

#include "Circle.h"

Circle::Circle(GLfloat x, GLfloat y, GLfloat diam, GLfloat clr[])
{
	
	startdiameter = diam;
	diameter = diam;
	SetColor(clr);
	xpos = x;
	ypos = y;
	zpos = 0.0f;
	startxpos = x;
	startypos = y;
	startzpos = 0.0f;
	xvel = 0.0f;
	yvel = 0.0f;
	zvel = 0.0f;
	CircTimer.Reset();
	timerdelay = 0;
	nVerts = 100;
	borderWidth = .001f;
	MakeVerts();
	border = 1;
	drawOn = 0;

	borderColor[0] = 0.0f;
	borderColor[1] = 0.0f;
	borderColor[2] = 0.0f;

}

Circle::Circle(GLfloat x, GLfloat y, GLfloat z, GLfloat diam, GLfloat clr[])
{
	startdiameter = diam;
	diameter = diam;
	SetColor(clr);
	xpos = x;
	ypos = y;
	zpos = z;
	startxpos = x;
	startypos = y;
	startzpos = z;
	xvel = 0.0f;
	yvel = 0.0f;
	zvel = 0.0f;
	CircTimer.Reset();
	timerdelay = 0;
	nVerts = 100;
	borderWidth = .001f;
	MakeVerts();
	border = 1;
	drawOn = 0;

	borderColor[0] = 0.0f;
	borderColor[1] = 0.0f;
	borderColor[2] = 0.0f;

}

void Circle::SetDiameter(GLfloat diam)
{
	diameter = diam;
	startdiameter = diam;
	MakeVerts();
}

void Circle::SetRadius(GLfloat rad)
{
	diameter = 2*rad;
	startdiameter = diameter;
	MakeVerts();
}

void Circle::MakeVerts()
{
	nVerts = 100; // use 100 vertices
	for (int i = 0; i < nVerts; i++)
	{
		GLfloat theta = 2*3.14159*i/(nVerts - 1);
		vertices[0][i] = (diameter/2)*sin(theta);
		vertices[1][i] = (diameter/2)*cos(theta);
		verticesInner[0][i] = ((diameter/2) - borderWidth) * sin(theta);
		verticesInner[1][i] = ((diameter/2) - borderWidth) * cos(theta);
	}
}

void Circle::SetColor(GLfloat clr[])
{
	color[0] = clr[0];
	color[1] = clr[1];
	color[2] = clr[2];
}

void Circle::SetBorderColor(GLfloat clr[])
{
	borderColor[0] = clr[0];
	borderColor[1] = clr[1];
	borderColor[2] = clr[2];
}


void Circle::SetPos(GLfloat x, GLfloat y, GLfloat z)
{
	xpos = x;
	ypos = y;
	zpos = z;
	startxpos = x;
	startypos = y;
	startzpos = z;

}

void Circle::SetVel(GLfloat vx, GLfloat vy)
{
	xvel = vx;
	yvel = vy;
}

void Circle::ResetTimer(int delay)
{
	CircTimer.Reset();
	timerdelay = delay;
}


Uint32 Circle::TgtTime()
{
	return (CircTimer.Elapsed());
}


void Circle::Draw()
{
	if(drawOn)
	{
		if(border)
		{
			// Draw border circle first
			glColor3f(borderColor[0], borderColor[1], borderColor[2]);
			glBegin(GL_TRIANGLE_FAN);
			glVertex3f(xpos, ypos, 0);	
			for (int i = 0; i < nVerts; i++)
			{
				glVertex3f(vertices[0][i] + xpos, vertices[1][i] + ypos, 0.0f);
			}
			glEnd();

			glColor3f(color[0], color[1], color[2]);
			glBegin(GL_TRIANGLE_FAN);
			glVertex3f(xpos, ypos, 0);	
			for (int i = 0; i < nVerts; i++)
			{
				glVertex3f(verticesInner[0][i] + xpos, verticesInner[1][i] + ypos, 0.0f);
			}

			glEnd();
			glColor3f(1.0f, 1.0f, 1.0f);
		}
		else
		{
			// Draw the polygon
			glColor3f(color[0], color[1], color[2]);
	
			glBegin(GL_TRIANGLE_FAN);

			glVertex3f(xpos, ypos, 0);
			for (int i = 0; i < nVerts; i++)
			{
				glVertex3f(vertices[0][i] + xpos, vertices[1][i] + ypos, 0.0f);
			}

			glEnd();
			glColor3f(1.0f, 1.0f, 1.0f);
		}
	}

}

void Circle::UpdatePosition()
{
	int dt = int(CircTimer.Elapsed())-timerdelay;
	
	if (dt > 0)
	{
		xpos = (float(dt)/1000.0f)*xvel + startxpos;
		ypos = (float(dt)/1000.0f)*yvel + startypos;
		MakeVerts();
	}

}

GLfloat Circle::GetX()
{
	return xpos;
}

GLfloat Circle::GetY()
{
	return ypos;
}

GLfloat Circle::GetZ()
{
	return zpos;
}

GLfloat Circle::GetDiam()
{
	return diameter;
}

void Circle::BorderOn()
{
	border = 1;
}

void Circle::BorderOff()
{
	border = 0;
}

void Circle::On()
{
	drawOn = 1;
}

void Circle::Off()
{
	drawOn = 0;
}

GLint Circle::drawState()
{
	return drawOn;
}

GLfloat Circle::Distance(Circle* c)
{
	//return sqrtf(powf(c->GetX() - xpos, 2.0f) + powf(c->GetY() - ypos, 2.0f));
	return sqrtf(powf(c->GetX() - xpos, 2.0f) + powf(c->GetY() - ypos, 2.0f) + powf(c->GetZ() - zpos, 2.0f));
}

GLfloat Circle::Distance(Object2D* c)
{
	return sqrtf(powf(c->GetX() - xpos, 2.0f) + powf(c->GetY() - ypos, 2.0f));
}


GLfloat Circle::GetRadius()
{
	return diameter/2;
}

void Circle::SetBorderWidth(GLfloat w)
{
	borderWidth = w;
}


bool Circle::Explode(GLfloat ntimes, GLfloat vel, Uint32 refTime)
{

	Uint32 curTime = SDL_GetTicks();

	diameter = float(curTime-refTime)/1000.0f*vel + startdiameter;
	if (diameter >= startdiameter*ntimes)
	{
		return(true);
	}
	else
	{
		UpdatePosition();
		//MakeVerts();
		return(false);
	}

}