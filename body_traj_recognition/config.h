#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#include "SDL_opengl.h"

////////// Set these parameters only! //////////////////////////

//set this to the data path in which the current set of data for this block should be stored.

//set the subject number
#define SUBJECT_ID "9999"

//define the file that contains the name of the trial table
#define TRIALFILE "./TrialTables/SameDiffGest1.txt"  

//define the folder where the data will go (this folder must exist!)
#define DATAPATH "./Data/"




//define paths
#define INSTRUCTPATH "./Resources/Instructions"
#define NINSTRUCT 1

#define VIDEOPATH "Resources\\videos\\"
#define VPATH ""  //don't add any extra extension onto the VIDEOPATH path
#define NVIDEOS 56



////////////////////////////////////////////////////////////////




// Configurable constants


//screen dimensions
//   (note, the Elitebook wants dimensions that are slightly larger than the screen, with WINDOWED set to true!)
//#define SCREEN_WIDTH  1602
//#define SCREEN_HEIGHT  901
#define SCREEN_WIDTH  1440
#define SCREEN_HEIGHT  900

//video dimensions
//Videos are in 16:9 format
//#define VIDEO_WIDTH int(float(SCREEN_WIDTH)*0.95f)
#define VIDEO_RATIO (16.0f/9.0f)
//#define VIDEO_HEIGHT int(float(VIDEO_WIDTH)/VIDEO_RATIO)
#define VIDEO_WIDTH 1368
#define VIDEO_HEIGHT 770
//#define VIDEO_WIDTH 1536
//#define VIDEO_HEIGHT 864


// Physical dimensions of the screen in kilo-inches (code is requesting inches, not meters, but there is a scale error in TrackBird)
#define PHYSICAL_WIDTH  (0.307f/0.0254f/1000.0f)
#define PHYSICAL_HEIGHT  (0.173f/0.0254f/1000.0f)
//#define PHYSICAL_HEIGHT  0.192

// screen ratio, in meters per pixel
#define PHYSICAL_RATIO  (PHYSICAL_WIDTH / SCREEN_WIDTH)
//#define PHYSICAL_RATIOI  (SCREEN_WIDTH/PHYSICAL_WIDTH)

#define SCREEN_BPP  32
//switch WINDOWED and MIRRORED for kinereach/simulation runs
#define WINDOWED  true
#define MIRRORED  false

//define pi as a float
#define PI 3.14159265358979f

//#define TRIAL_DURATION 10000 //10 sec, in msec
#define MAX_TRIAL_DURATION 600000 //10 min, in msec
#define ITI 500 //time to wait between trials, in msec

#define SAMPRATE 100 //legacy argument


#endif
