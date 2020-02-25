#ifndef TGTFRAME_H
#define TGTFRAME_H
#include "SDL.h"
#pragma once

// Data type used to store trial data for data writer
struct TargetFrame
{
	int trial;
	int redo;
	double starttime;
	Uint32 time;

	int vid1;
	int vid2;
	int vidtype;		//same, shared-trajectory, shared-body config, different
	int vidstatus;
	int instruct;

	int cresp;			//correct response (0 = same, 1 = different)
	int resp;
	int correct;
	int score;

	int practice;	

	int TrType;

	char key;

	int lat;
	int dur;

};

#endif
