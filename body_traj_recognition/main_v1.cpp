#include <cmath>
#include <sstream>
#include <iostream>
#include <fstream>
#include <istream>
#include <windows.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"

#include "MouseInput.h"
#include "TrackBird.h"

#include "Circle.h"
#include "DataWriter.h"
#include "HandCursor.h"
#include "Object2D.h"
#include "Path2D.h"
#include "Region2D.h"
#include "Sound.h"
#include "Timer.h"
#include "Image.h"
#include "vlcVideoPlayer.h"

#include "config.h"

#include <gl/GL.h>
#include <gl/GLU.h>

/*
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_mixer.lib")
#pragma comment(lib, "SDL2_ttf.lib")
#pragma comment(lib, "SDL2_image.lib")
#pragma comment(lib, "Bird.lib")
#pragma comment(lib, "ftd2xx.lib")
#pragma comment(lib, "ATC3DG.lib")
*/
#pragma push(1)

//state machine
enum GameState
{
	Idle = 0x01,       //00001
	Instruct = 0x02,   //00010
	WaitStim = 0x03,   //00011
	ShowContext = 0x04, //00100
	ShowStim = 0x05,   //00101
	Active = 0x06,     //00110
	EndTrial = 0x08, //01000
	Finished = 0x10    //10000
};



SDL_Event event;
SDL_Window *screen = NULL;
SDL_GLContext glcontext = NULL;

HandCursor* curs[BIRDCOUNT + 1];
HandCursor* player = NULL;
Circle* startCircle = NULL;  //circle to keep track of the "home" position
//Circle* handCircle = NULL;
//Object2D* items[NCONTEXTS];
Object2D* instructimages[2][NINSTRUCT];
Image* endtext = NULL;
Image* readytext = NULL;
Image* trialinstructtext = NULL;
Image* stoptext = NULL;
Image* holdtext = NULL;
Image* redotext = NULL;
Image* trialnum = NULL;
Image* recordtext = NULL;
Image* proceedtext = NULL;
Image* returntext = NULL;
Image* mousetext = NULL;
Image* contextText[NCONTEXTS];
Sound* startbeep = NULL;
Sound* itemsounds[NCONTEXTS];
SDL_Color textColor = {0, 0, 0, 1};
DataWriter* writer = NULL;
GameState state;
Timer* trialTimer;
Timer* hoverTimer;
Timer* movTimer;

//videos
Video *Vid[NVIDEOS0];
int NVids;
int textcontext; //flag to also create/show text along with audio context
int NcontextTexts;


//tracker variables
int trackstatus;
TrackSYSCONFIG sysconfig;
TrackDATAFRAME dataframe[BIRDCOUNT+1];
Uint32 DataStartTime = 0;
bool recordData = false;
bool didrecord = false;

//colors
float redColor[3] = {1.0f, 0.0f, 0.0f};
float greenColor[3] = {0.0f, 1.0f, 0.0f};
float blueColor[3] = {0.0f, 0.0f, 1.0f};
float cyanColor[3] = {0.0f, 0.5f, 1.0f};
float grayColor[3] = {0.6f, 0.6f, 0.6f};
float blkColor[3] = {0.0f, 0.0f, 0.0f};
float whiteColor[3] = {1.0f, 1.0f, 1.0f};
float orangeColor[3] = {1.0f, 0.5f, 0.0f};
float *cursColor = blueColor;


// Trial table structure, to keep track of all the parameters for each trial of the experiment
typedef struct {
	int trialType;		// Flag for trial type ( 0 = FreeStream
						//						Mless: 1 = Mless, 2 = MlessAwkward, 3 = MlessStatic; 
	                    //                       Mful: 4 = Mful,  5 = MfulAwkward,  6 = MfulStatic,   7 = Named)
	int item;			//item number, corresonding to a video file
	int showcontext;	//flag to show context or not (-1 = hide, 0 = play a number, 1 = show dynamic, 2 = show static)
	int context;		//context number (audio and/or picture and/or text) (>=0 = context number and show, < 0 = do not show)
	int contextdur;		//duration to show context (if -2, do context simultaneous with video playback; if -1, wait until done)
	int srdur;			//duration to wait after video playback before go cue (stim-response delay duration)
	int trdur;			//trial duration (max movement time)
	int practice;		//flag if the block is a practice block or not (changes the instructions disiplayed at the start of the block)
} TRTBL;

#define TRTBL_SIZE 100
TRTBL trtbl [TRTBL_SIZE];

int NTRIALS = 0;
int CurTrial = -1;

#define curtr trtbl[CurTrial]

//target structure; keep track of the target and other parameters, for writing out to data stream
TargetFrame Target;

//varibles that let the experimenter control the experiment flow
bool nextstateflag = false;
bool redotrialflag = false;
int numredos = 0;
//int dotrialflag = 0;

// Initializes everything and returns true if there were no errors
bool init();
// Sets up OpenGL
void setup_opengl();
// Performs closing operations
void clean_up();
// Draws objects on the screen
void draw_screen();
//file to load in trial table
int LoadTrFile(char *filename);
//file to load in a list of context strings and create a bunch of text images from them
int LoadContextFile(char *fname,Image* cText[]);
// Update loop (state machine)
void game_update();

bool quit = false;  //flag to cue exit of program



int main(int argc, char* args[])
{
	int a = 0;
	int flagoffsetkey = -1;
	Target.key = ' ';

	//redirect stderr output to a file
	freopen( "./Debug/errorlog.txt", "w", stderr); 
	freopen( "./Debug/outlog.txt", "w", stdout); 

	std::cerr << "Start main." << std::endl;

	SetPriorityClass(GetCurrentProcess(),ABOVE_NORMAL_PRIORITY_CLASS);
	//HIGH_PRIORITY_CLASS
	std::cerr << "Promote process priority to Above Normal." << std::endl;

	if (!init())
	{
		// There was an error during initialization
		std::cerr << "Initialization error." << std::endl;
		return 1;
	}

	DataStartTime = SDL_GetTicks();

	while (!quit)
	{
		int inputs_updated = 0;

		// Retrieve Flock of Birds data
		if (trackstatus>0)
		{
			// Update inputs from Flock of Birds
			inputs_updated = TrackBird::GetUpdatedSample(&sysconfig,dataframe);
		}

		// Handle SDL events
		while (SDL_PollEvent(&event))
		{
			// See http://www.libsdl.org/docs/html/sdlevent.html for list of event types
			if (event.type == SDL_MOUSEMOTION)
			{
				if (trackstatus <= 0)
				{
					MouseInput::ProcessEvent(event);
					inputs_updated = MouseInput::GetFrame(dataframe);

				}
			}
			else if (event.type == SDL_KEYDOWN)
			{
				// See http://www.libsdl.org/docs/html/sdlkey.html for Keysym definitions
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
				else if (event.key.keysym.sym == SDLK_0)
				{
					flagoffsetkey = 1;
					Target.key = '0';
					//std::cerr << "Zero requested" << std::endl;
				}
				else if (event.key.keysym.sym == SDLK_r)
				{
					redotrialflag = true;
					Target.key = 'r';

					//std::cerr << "Redo requested" << std::endl;
				}
				/*
				else if (event.key.keysym.sym == SDLK_o)
				{
					flagoffsetkey = 0;
					Target.key = 'O';
					//std::cerr << "Offsets requested" << std::endl;
				}
				*/
				else if (event.key.keysym.sym == SDLK_SPACE)
				{
					nextstateflag = true;
					Target.key = 's';
					//std::cerr << "Advance requested" << std::endl;
				}
				else //if( event.key.keysym.unicode < 0x80 && event.key.keysym.unicode > 0 )
				{
					Target.key = *SDL_GetKeyName(event.key.keysym.sym);  //(char)event.key.keysym.unicode;
					//std::cerr << Target.flag << std::endl;
				}
			}
			else if (event.type == SDL_KEYUP)
			{
				Target.key = '-1';
			}
			else if (event.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		if ((CurTrial >= NTRIALS) && (state == Finished) && (trialTimer->Elapsed() >= 10000))
			quit = true;

		// Get data from input devices
		if (inputs_updated > 0) // if there is a new frame of data
		{

			//updatedisplay = true;
			for (int a = ((trackstatus>0) ? 1 : 0); a <= ((trackstatus>0) ? BIRDCOUNT : 0); a++)
			{
				if (dataframe[a].ValidInput)
				{
					curs[a]->UpdatePos(float(dataframe[a].x),float(dataframe[a].y),float(dataframe[a].z));
					dataframe[a].vel = curs[a]->GetVel3D();

					//std::cerr << "Curs" << a << ": " << curs[a]->GetX() << " , " << curs[a]->GetY() << " , " << curs[a]->GetZ() << std::endl;
					//std::cerr << "Data" << a << ": " << dataframe[a].x << " , " << dataframe[a].y << " , " << dataframe[a].z << std::endl;

					if (recordData)  //only write out if we need to
						writer->Record(a, dataframe[a], Target);
					else
						Target.starttime = dataframe[a].time;
				}
			}

		} //end if inputs updated

		if (flagoffsetkey == 0) //set the offsets, if requested
		{
			if (trackstatus > 0)
			{
				sysconfig.PosOffset[0] = dataframe[HAND].x;
				sysconfig.PosOffset[1] = dataframe[HAND].y;
				sysconfig.PosOffset[2] = dataframe[HAND].z;
				std::cerr << "Data Pos Offsets set: " << sysconfig.PosOffset[0] << " , " << sysconfig.PosOffset[1] << " , "<< sysconfig.PosOffset[2] << std::endl;
			}
			else
				std::cerr << "Data Pos Requested but mouse is being used, request is ignored." << std::endl;
			flagoffsetkey = -1;
			Target.key = ' ';

		}
		else if (flagoffsetkey == 1) //set the startCircle position
		{	
			if (trackstatus > 0)
			{
				//startCircle->SetPos(curs[HAND]->GetMeanX(),curs[HAND]->GetMeanY(),curs[HAND]->GetMeanZ());
				startCircle->SetPos(curs[HAND]->GetX(),curs[HAND]->GetY(),curs[HAND]->GetZ());
				//std::cerr << "Curs " << HAND << ": " << curs[HAND]->GetX() << " , " << curs[HAND]->GetY() << " , " << curs[HAND]->GetZ() << std::endl;
			}
			else //mouse mode
			{
				//startCircle->SetPos(curs[0]->GetMeanX(),curs[0]->GetMeanY(),0.0f);
				startCircle->SetPos(curs[0]->GetX(),curs[0]->GetY(),0.0f);
				//std::cerr << "Curs0: " << curs[0]->GetX() << " , " << curs[0]->GetY() << " , " << curs[0]->GetZ() << std::endl;
			}

			//std::cerr << "Player: " << player->GetX() << " , " << player->GetY() << " , " << player->GetZ() << std::endl;
			std::cerr << "StartPos Offsets set: " << startCircle->GetX() << " , " << startCircle->GetY() << " , "<< startCircle->GetZ() << std::endl;

			flagoffsetkey = -1;
			Target.key = ' ';
		}

		if (!quit)
		{

			game_update(); // Run the game loop (state machine update)

			//if (updatedisplay)  //reduce number of calls to draw_screen -- does this speed up display/update?
			draw_screen();
		}

	}

	std::cerr << "Exiting..." << std::endl;

	clean_up();
	return 0;
}



//function to read in the name of the trial table file, and then load that trial table
int LoadTrFile(char *fname)
{

	//std::cerr << "LoadTrFile begin." << std::endl;

	char tmpline[100] = ""; 
	int ntrials = 0;

	//read in the trial file name
	std::ifstream trfile(fname);

	if (!trfile)
	{
		std::cerr << "Cannot open input file." << std::endl;
		return(-1);
	}
	else
		std::cerr << "Opened TrialFile " << TRIALFILE << std::endl;

	trfile.getline(tmpline,sizeof(tmpline),'\n');  //get the first line of the file, which is the name of the trial-table file

	while(!trfile.eof())
	{
		sscanf(tmpline, "%d %d %d %d %d %d %d %d", 
			&trtbl[ntrials].trialType,
			&trtbl[ntrials].item,
			&trtbl[ntrials].showcontext,
			&trtbl[ntrials].context,
			&trtbl[ntrials].contextdur,
			&trtbl[ntrials].srdur,
			&trtbl[ntrials].trdur,
			&trtbl[ntrials].practice);

		ntrials++;
		trfile.getline(tmpline,sizeof(tmpline),'\n');
	}

	trfile.close();
	if(ntrials == 0)
	{
		std::cerr << "Empty input file." << std::endl;
		//exit(1);
		return(-1);
	}
	return ntrials;
}


//function to read in the text strings and transform them into text images for display
int LoadContextFile(char *fname,Image* cText[])
{

	char tmpline[100] = ""; 
	int nlines = 0;

	//read in the trial file name
	std::ifstream trfile(fname);

	if (!trfile)
	{
		std::cerr << "Cannot open input file." << std::endl;
		return(-1);
	}
	else
		std::cerr << "Opened ContextFile " << fname << std::endl;

	trfile.getline(tmpline,sizeof(tmpline),'\n');  //get the first line of the file, which is the name of the trial-table file

	std::string textline;

	while(!trfile.eof())
	{
		textline.assign("");
		textline.assign(tmpline);
		cText[nlines] = Image::ImageText(cText[nlines],textline.c_str(),"arial.ttf", 48,textColor);
		cText[nlines]->Off();
		nlines++;
		trfile.getline(tmpline,sizeof(tmpline),'\n');
		std::cerr << "  Loaded context text: " << textline.c_str() << std::endl;
	}

	trfile.close();
	if(nlines == 0)
	{
		std::cerr << "Empty context file." << std::endl;
		return(-1);
	}
	return nlines;
}

//initialization function - set up the experimental environment and load all relevant parameters/files
bool init()
{

	int a;
	char tmpstr[80];
	char fname[50] = TRIALFILE;
	

	//std::cerr << "Start init." << std::endl;

	std::cerr << std::endl;
	std::cout << "Initializing the tracker... " ;

	Target.starttime = 0;
	trackstatus = TrackBird::InitializeBird(&sysconfig);
	if (trackstatus <= 0)
	{
		std::cerr << "Tracker failed to initialize. Mouse Mode." << std::endl;
		std::cout << "failed. Switching to mouse mode." << std::endl;
	}
	else
		std::cout << "completed" << std::endl;

	std::cerr << std::endl;

	std::cout << "Initializing SDL and loading files... ";

	// Initialize SDL, OpenGL, SDL_mixer, and SDL_ttf
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cerr << "SDL failed to intialize."  << std::endl;
		return false;
	}
	else

		std::cerr << "SDL initialized." << std::endl;
	

	//screen = SDL_CreateWindow("Code Base SDL2",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | (WINDOWED ? 0 : SDL_WINDOW_FULLSCREEN)); //SCREEN_BPP,
	screen = SDL_CreateWindow("Code Base SDL2",
		//SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
		(WINDOWED ? SDL_WINDOWPOS_UNDEFINED : SDL_WINDOWPOS_CENTERED),(WINDOWED ? SDL_WINDOWPOS_UNDEFINED : SDL_WINDOWPOS_CENTERED),
		SCREEN_WIDTH, SCREEN_HEIGHT, 
		SDL_WINDOW_OPENGL | (WINDOWED ? 0 : (SDL_WINDOW_MAXIMIZED) ) ); //SCREEN_BPP, //
	//note, the fullscreen option doesn't play nicely with the video window, so we will make a "fake" fullscreen which is simply a maximized window. 
	//Also, "borderless" doesn't play nicely either so we can't use that option, we just have to make the window large enough that the border falls outside the window.
	//To get full screen (overlaying the taskbar), we need to set the taskbar to auto-hide so that it stays offscreen. Otherwise it has priority and will be shown on top of the window.
	if (screen == NULL)
	{
		std::cerr << "Screen failed to build." << std::endl;
		return false;
	}
	else
	{
		if (!WINDOWED)
		{
			//SDL_SetWindowBordered(screen, SDL_FALSE);
			SDL_Rect usable_bounds;
			int display_index = SDL_GetWindowDisplayIndex(screen);
			if (0 != SDL_GetDisplayUsableBounds(display_index, &usable_bounds)) 
				std::cerr << "SDL error getting usable screen bounds." << std::endl;
		
			SDL_SetWindowPosition(screen, usable_bounds.x, usable_bounds.y);
			SDL_SetWindowSize(screen, usable_bounds.w, usable_bounds.h);
		}
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

		//create OpenGL context
		glcontext = SDL_GL_CreateContext(screen);
		std::cerr << "Screen built." << std::endl;
	}


	SDL_GL_SetSwapInterval(0); //ask for immediate updates rather than syncing to vertical retrace

	setup_opengl();

	//a = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 512);  //initialize SDL_mixer
	a = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 2048);  //initialize SDL_mixer, may have to play with the chunksize parameter to tune this a bit
	if (a != 0)
	{
		std::cerr << "Audio failed to initialize." << std::endl;
		return false;
	}
	else
		std::cerr << "Audio initialized." << std::endl;


	//initialize SDL_TTF (text handling)
	if (TTF_Init() == -1)
	{
		std::cerr << "SDL_TTF failed to initialize." << std::endl;
		return false;
	}
	else
		std::cerr << "SDL_TTF initialized." << std::endl;

	//turn off the computer cursor
	//SDL_ShowCursor(0);

	std::cerr << std::endl;


	//load trial table from file
	NTRIALS = LoadTrFile(fname);
	//std::cerr << "Filename: " << fname << std::endl;

	if(NTRIALS == -1)
	{
		std::cerr << "Trial File did not load." << std::endl;
		return false;
	}
	else
		std::cerr << "Trial File loaded: " << NTRIALS << " trials found." << std::endl;

	//assign the data-output file name based on the trial-table name 
	std::string savfile;
	savfile.assign(fname);
	savfile.insert(savfile.rfind("."),"_data");

	std::strcpy(fname,savfile.c_str());

	std::cerr << "SavFileName: " << fname << std::endl;

	//writer = new DataWriter(&sysconfig,Target,"InstructFile");  //set up the data-output file
	//recordData = true;
	didrecord = false;
	recordData = false;




	// Load files and initialize pointers


	Image* instructim[2][NINSTRUCT];

	for (a = 0; a < NINSTRUCT; a++)
	{
		//load main instructions
		sprintf(tmpstr,"%s/Instruct%d.png",INSTRUCTPATH,a);
		instructim[0][a] = Image::LoadFromFile(tmpstr);
		if (instructim[0][a] == NULL)
		{
			std::cerr << "Instruction " << a << " did not load." << std::endl;
			instructim[0][a]->Off();
		}
		else
		{
			instructimages[0][a] = new Object2D(instructim[0][a]);
			std::cerr << "   Instruction " << a << " loaded." << std::endl;
			instructimages[0][a]->SetPos(PHYSICAL_WIDTH / 2, PHYSICAL_HEIGHT / 2);
		}

		//load practice instructions
		sprintf(tmpstr,"%s/InstructP%d.png",INSTRUCTPATH,a);
		instructim[1][a] = Image::LoadFromFile(tmpstr);
		if (instructim[1][a] == NULL)
		{
			std::cerr << "Instruction P" << a << " did not load." << std::endl;
			instructim[1][a]->Off();
		}
		else
		{
			instructimages[1][a] = new Object2D(instructim[0][a]);
			std::cerr << "   Instruction P" << a << " loaded." << std::endl;
			instructimages[1][a]->SetPos(PHYSICAL_WIDTH / 2, PHYSICAL_HEIGHT / 2);
		}
	}
	
	//initialize the video player and the video file
	// Trial type (Mless: 1 = Mless, 2 = MlessAwkward, 3 = MlessStatic; 
	//              Mful: 4 = Mful,  5 = MfulAwkward,  6 = MfulStatic,   7 = Named Mful (same path as 4))
	//               (note, Mful=4 with context set to 1 is Named Mful but the instructions will be wrong! Use Mful=7 for named Mful)
	std::string vpathbase;
	switch(trtbl[0].trialType) //assume this is blocked; we have to call trial 0 since CurTrial is initialized to -1
		{
			case 1:
				vpathbase.assign(VPATH0);
				NVids = NVIDEOS0;
				break;
			case 2:
				vpathbase.assign(VPATH1);
				NVids = NVIDEOS1;
				break;
			case 3:
				vpathbase.assign(VPATH2);
				NVids = NVIDEOS2;
				break;
			case 4:
			case 7:
				vpathbase.assign(VPATH3);
				NVids = NVIDEOS3;
				break;
			case 5: 
				vpathbase.assign(VPATH4);
				NVids = NVIDEOS4;
				break;
			case 6: 
				vpathbase.assign(VPATH5);
				NVids = NVIDEOS5;
				break;
			default:
				vpathbase.assign("");
				NVids = 0;
		}


	std::stringstream vidfile;
	int errcode;
	for (a = 0; a < NVids; a++)
	{
		vidfile.str(std::string());
		vidfile << vpathbase.c_str() <<  "Video" << a << ".divx";
		Vid[a] = new Video(vidfile.str().c_str(),SCREEN_WIDTH/2,SCREEN_HEIGHT/2,VIDEO_WIDTH,VIDEO_HEIGHT,&errcode);
		if (errcode != 0)
		{
			std::cerr << "Video " << a << " did not load." << std::endl;
			Vid[a]->SetValidStatus(0);
		}		
		else
		{
			std::cerr << "   Video " << a << " loaded." << std::endl;
			std::cerr << "      Vid_size: " << VIDEO_WIDTH << "x" << VIDEO_HEIGHT << std::endl;
			//Vid[a]->SetValidStatus(1);
			//Vid[a]->SetPos(SCREEN_WIDTH/2,SCREEN_HEIGHT/2);

		}
			
	}



	textcontext = trtbl[0].showcontext;  //if this is -1, do nothing; if this is 0, read a number; if this is >0, show context of specified type
	/*
	Image* tgtimages[NCONTEXTS];

	//load image contexts
	for (a = 0; a < NCONTEXTS; a++)
	{
		if (textcontext) //load meaningful context images
		{
			sprintf(tmpstr,"%s/%d.jpg",IMAGEPATH,a+1);
			tgtimages[a] = Image::LoadFromFile(tmpstr);
			if (tgtimages[a] == NULL)
				std::cerr << "Image Trace" << a << " did not load." << std::endl;
			else
			{
				items[a] = new Object2D(tgtimages[a]);
				std::cerr << "   Image " << a+1 << " loaded." << std::endl;
				items[a]->SetPos(PHYSICAL_WIDTH / 2, PHYSICAL_HEIGHT / 2);
			}
		}
	}
	std::cerr << "Images loaded." << std::endl;
	*/

	//load audio contexts
	if (textcontext == 2) //load the descriptive audio files, static
		for (a = 0; a < NCONTEXTS; a++)  //we can just let the extra ones fail to load
		{
			sprintf(tmpstr,"%s/Context%d.wav",AUDIOPATHC_S,a);
			itemsounds[a] = new Sound(tmpstr);
			if (itemsounds[a] == NULL)
				std::cerr << "   Audiofile_S " << a+1 << " did not load." << std::endl;
			else
				std::cerr << "   Audiofile_S " << a+1 << " loaded." << std::endl;
		}
	else if (textcontext == 1) //load the descriptive audio files, dynamic
		for (a = 0; a < NCONTEXTS; a++)
		{
			sprintf(tmpstr,"%s/Context%d.wav",AUDIOPATHC_D,a);
			itemsounds[a] = new Sound(tmpstr);
			if (itemsounds[a] == NULL)
				std::cerr << "   Audiofile_D " << a+1 << " did not load." << std::endl;
			else
				std::cerr << "   Audiofile_D " << a+1 << " loaded." << std::endl;
		}
	else if (textcontext == 0)  //load the number audio files
		for (a = 0; a < NCONTEXTS; a++)
		{
			sprintf(tmpstr,"%s/%d.wav",AUDIOPATH,a+1);
			itemsounds[a] = new Sound(tmpstr);
			if (itemsounds[a] == NULL)
				std::cerr << "   Audiofile_N " << a+1 << " did not load." << std::endl;
			else
				std::cerr << "   Audiofile_N " << a+1 << " loaded." << std::endl;
		}
	std::cerr << "Audio loaded: " << a-1 << "." << std::endl;

	//create text contexts
	int ntexts;
	if (textcontext == 2) //load the descriptive audio files, static
	{
		sprintf(tmpstr,"%s/texts.txt",AUDIOPATHC_S);
		ntexts = LoadContextFile(tmpstr,contextText);
		NcontextTexts = ntexts;  //keep track of how many contexts are valid!
		if (ntexts <= 0)
			std::cerr << "No context text images loaded." << std::endl;
		else
			std::cerr << "Context text images loaded: " << ntexts << "." << std::endl;
	}
	else if (textcontext == 1) //load the descriptive audio files, dynamic
	{
		sprintf(tmpstr,"%s/texts.txt",AUDIOPATHC_D);
		ntexts = LoadContextFile(tmpstr,contextText);
		NcontextTexts = ntexts;  //keep track of how many contexts are valid!
		if (ntexts <= 0)
			std::cerr << "No context text images loaded." << std::endl;
		else
			std::cerr << "Context text images loaded: " << ntexts << "." << std::endl;

	}
	else if (textcontext == 0)  //load the number audio files
	{	
		//do nothing, no display goes with the number audio files
		std::cerr << "No context text images requested to be loaded (number audio / no audio only)." << std::endl;
		NcontextTexts = 0;  //keep track of how many contexts are valid!
	}


	//set up the start position
	startCircle = new Circle(PHYSICAL_WIDTH/2.0f, PHYSICAL_HEIGHT/2.0f, 0.0f, START_RADIUS*2, blkColor);
	startCircle->SetBorderWidth(0.001f);
	startCircle->SetBorderColor(blkColor);
	startCircle->BorderOff();
	startCircle->Off();
	//startCircle->On();
	std::cerr << "Start Circle: " << startCircle->GetX() << " , " << startCircle->GetY() << " : " << startCircle->drawState() << std::endl;
	
	std::cerr << std::endl;




	// set up the cursors
	if (trackstatus > 0)
	{
		/* Assign birds to the same indices of controller and cursor that they use
		* for the Flock of Birds
		*/
		for (a = 1; a <= BIRDCOUNT; a++)
		{
			curs[a] = new HandCursor(0.0f, 0.0f, CURSOR_RADIUS*2, cursColor);
			curs[a]->BorderOff();
			curs[a]->SetOrigin(0.0f, 0.0f);
		}

		player = curs[HAND];  //this is the cursor that represents the hand
		std::cerr << "Player = " << HAND << std::endl;
	}
	else
	{
		// Use mouse control
		curs[0] = new HandCursor(0.0f, 0.0f, CURSOR_RADIUS*2, cursColor);
		curs[0]->SetOrigin(0.0f, 0.0f);
		player = curs[0];
		std::cerr << "Player = 0"  << std::endl;
	}

	//player->On();
	player->Off();


	//load sound files
	startbeep = new Sound("Resources/IM_Sounds/beep.wav");
	


	//set up placeholder text
	endtext = Image::ImageText(endtext, "Block ended.","arial.ttf", 28, textColor);
	endtext->Off();

	readytext = Image::ImageText(readytext, "Get ready...","arial.ttf", 28, textColor);
	readytext->Off();
	stoptext = Image::ImageText(stoptext, "STOP!","arial.ttf", 32, textColor);
	stoptext->Off();
	holdtext = Image::ImageText(holdtext, "Wait until the go signal!","arial.ttf", 28, textColor);
	holdtext->Off();
	proceedtext = Image::ImageText(proceedtext, "Zero the tracker and proceed when ready.","arial.ttf", 28, textColor);
	proceedtext->Off();
	returntext = Image::ImageText(returntext, "Please return to start position.","arial.ttf", 28, textColor);
	returntext->Off();

	trialinstructtext = Image::ImageText(trialinstructtext, "Press (space) to advance or (r) to repeat trial.","arial.ttf", 12, textColor);
	trialinstructtext->Off();
	redotext = Image::ImageText(redotext, "Press (r) to repeat trial.","arial.ttf", 12, textColor);
	redotext->Off();
	recordtext = Image::ImageText(recordtext, "Recording...","arial.ttf", 12, textColor);
	recordtext->Off();
	mousetext = Image::ImageText(mousetext, "Trackers not found! Mouse-emulation mode.","arial.ttf", 12, textColor);
	if (trackstatus > 0)
		mousetext->Off();
	else
		mousetext->On();

	//set up trial number text image
	trialnum = Image::ImageText(trialnum,"0_0","arial.ttf", 12,textColor);
	trialnum->On();

	hoverTimer = new Timer();
	trialTimer = new Timer();
	movTimer = new Timer();

	// Set the initial game state
	state = Idle; 

	std::cerr << "Initialization complete." << std::endl;
	std::cout << "completed." << std::endl;

	return true;
}


static void setup_opengl()
{
	glClearColor(1, 1, 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* The default coordinate system has (0, 0) at the bottom left. Width and
	* height are in meters, defined by PHYSICAL_WIDTH and PHYSICAL_HEIGHT
	* (config.h). If MIRRORED (config.h) is set to true, everything is flipped
	* horizontally.
	*/
	glOrtho(MIRRORED ? PHYSICAL_WIDTH : 0, MIRRORED ? 0 : PHYSICAL_WIDTH,
		0, PHYSICAL_HEIGHT, -1.0f, 1.0f);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

}


//end the program; clean up everything neatly.
void clean_up()
{
	std::cout << "Shutting down." << std::endl;

	delete startbeep;

	for (int a = 0; a < NCONTEXTS; a++)
	{
		//delete items[a];
		delete itemsounds[a];
	}

	for (int a = 0; a < NcontextTexts; a++)
		delete contextText[a];


	for (int b = 0; b < 2; b++)
		for (int a = 0; a < NINSTRUCT; a++)
			delete instructimages[b][a];
	
	delete endtext;
	delete readytext;
	delete trialinstructtext;
	delete stoptext;
	delete holdtext;
	delete redotext;
	delete trialnum;
	delete recordtext;
	delete proceedtext;
	delete returntext;
	delete mousetext;

	for (int a = 0; a < NVids; a++)
	{
		Vid[a]->CleanUp();
		delete Vid[a];
	}

	//std::cerr << "Deleted all objects." << std::endl;

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(screen);
	Mix_CloseAudio();
	TTF_Quit();
	SDL_Quit();

	std::cerr << "Shut down SDL." << std::endl;

	if (trackstatus > 0)
	{
		TrackBird::ShutDownBird(&sysconfig);
		std::cerr << "Shut down tracker." << std::endl;
	}

	freopen( "CON", "w", stderr );

}

//control what is drawn to the screen
static void draw_screen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//draw the image specified
	/*
	Target.trace = -1;
	for (int a = 0; a < NCONTEXTS; a++)
	{
		items[a]->Draw();
		if (items[a]->DrawState())
			Target.trace = a;
	}
	*/

	//draw the instructions specified
	for (int b = 0; b < 2; b++)
		for (int a = 0; a < NINSTRUCT; a++)
		{
			instructimages[0][a]->Draw(PHYSICAL_WIDTH,PHYSICAL_HEIGHT);
			if (instructimages[0][a]->DrawState())
				Target.instruct = b*NINSTRUCT + a;
		}


	for (int a = 0; a < NcontextTexts; a++)
		contextText[a]->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);


	startCircle->Draw();
	//if (startCircle->drawState())
		//std::cerr << " Start circle drawn at " << startCircle->GetX() << " , " << startCircle->GetY() << std::endl;

	player->Draw();


	// Draw text
	endtext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);

	proceedtext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);
	readytext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);
	stoptext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);
	holdtext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);
	returntext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*1.0f/2.0f);

	trialinstructtext->DrawAlign(PHYSICAL_WIDTH*0.5f/24.0f,PHYSICAL_HEIGHT*0.5f/24.0f,3);
	redotext->DrawAlign(PHYSICAL_WIDTH*0.5f/24.0f,PHYSICAL_HEIGHT*0.5f/24.0f,3);
	recordtext->DrawAlign(PHYSICAL_WIDTH*0.5f/24.0f,PHYSICAL_HEIGHT*0.5f/24.0f,3);
	mousetext->DrawAlign(PHYSICAL_WIDTH*23.5f/24.0f,PHYSICAL_HEIGHT*23.5f/24.0f,1);
	//write the trial number
	trialnum->Draw(PHYSICAL_WIDTH*23.0f/24.0f, PHYSICAL_HEIGHT*0.5f/24.0f);

	SDL_GL_SwapWindow(screen);
	glFlush();

}


//game update loop - state machine controlling the status of the experiment
bool mvtStarted = false;
bool falsestart = false;
bool vidEnded = false;
bool donePlayingContext = false;

bool reachedvelmin = false;
bool reachedvelmax = false;

bool mvmtEnded = false;
bool hitTarget = false;
bool hitRegion = false;
bool hitPath = false;

float LastPeakVel = 0;
bool returntostart = true;

bool writefinalscore;

void game_update()
{

	switch (state)
	{
	case Idle:
		// This state only happens at the start of the experiment, and is just a null pause until the experimenter is ready to begin.

		Target.trial = -1;
		Target.trace = -1;
		Target.instruct = -1;
		Target.lat = -999;
		Target.redo = -1;
		Target.trial = -1;
		Target.context = -1;
		Target.vidstatus = -1;
		Target.TrType = trtbl[0].trialType;

		endtext->Off();
		readytext->Off();
		stoptext->Off();
		trialinstructtext->Off();
		holdtext->Off();
		recordtext->Off();
		proceedtext->On();

		
		if (!returntostart)
		{
			//if we haven't yet gotten back to the start target yet
			if (player->Distance(startCircle) < START_RADIUS)
				returntostart = true;
		}

		/*
		//shut off all images
		for (int a = 0; a < NCONTEXTS; a++)
			items[a]->Off();
		*/

		for (int b = 0; b < 2; b++)
			for (int a = 0; a < NINSTRUCT; a++)
				instructimages[b][a]->Off();

		for (int a = 0; a < NVids; a++)
		{
			Vid[a]->Invisible();
		}

		if( returntostart && nextstateflag)  //hand is in the home position and the experimenter asked to advance the experiment
		{
			hoverTimer->Reset();
			trialTimer->Reset();

			proceedtext->Off();

			nextstateflag = false;

			Target.key = ' ';

			std::cerr << "Leaving IDLE state." << std::endl;

			state = Instruct;
		}
		break;

	case Instruct: 
		//this state is just for displaying instructions, and only happens at the start of the experiment.

		//display instructions
		//std::cerr << " Instruction " << trtbl[0].practice*6+trtbl[0].trialType << std::endl;
		instructimages[trtbl[0].practice][trtbl[0].trialType-1]->On();
		Target.instruct = trtbl[0].practice*NINSTRUCT + trtbl[0].trialType;

		// If experimenter hits advance button, move on (after delay to make sure not triggered from previous keypress)
		if (hoverTimer->Elapsed() > 1000 && nextstateflag)
		{
			nextstateflag = false;

			redotrialflag = false;

			instructimages[trtbl[0].practice][trtbl[0].trialType-1]->Off();

			//make sure that all the videos have stopped playing and are invisible
			for (int a = 0; a < NVids; a++)
			{
				Vid[a]->Stop(); //stop also renders the video invisible
				Vid[a]->ResetVid();
			}

			falsestart = false;
			mvtStarted = false;

			//Target.trial = CurTrial+1;

			Target.key = ' ';

			hoverTimer->Reset();
			returntext->Off();

			state = WaitStim;
			std::cerr << "Leaving INSTRUCT state." << std::endl;

		}
		break;

	case WaitStim:
		//this state is just a "pause" state between trials.
		
		trialnum->Off();  //is it confusing that we show the "last" trial here, since we don't know if we want to redo or advance yet?

		returntext->Off();
		Target.item = -1;
		Target.vidstatus = -1;

		vidEnded = false;
		donePlayingContext = false;
		
		/*
		//make sure all the images are hidden
		for (int a = 0; a < NCONTEXTS; a++)
			items[a]->Off();
		*/

		//show "pause" text, and instructions to experimenter at the bottom of the screen
		readytext->On();
		holdtext->Off();
		if (falsestart && hoverTimer->Elapsed() < 1000)
		{
			readytext->Off();
			holdtext->On();
		}

		//recordtext->Off();

		if (!falsestart)
			trialinstructtext->On();
		else
			redotext->On();
		

		if (hoverTimer->Elapsed() > 1000 && (nextstateflag || redotrialflag) )
		{
			
			if (!falsestart && (nextstateflag || (redotrialflag && CurTrial < 0) )) //if accidentally hit "r" before the first trial, ignore this error!
			{
				nextstateflag = false;
				redotrialflag = false;
				numredos = 0;
				CurTrial++;  //we started the experiment with CurTrial = -1, so now we are on the "next" trial (or first trial)
				Target.trial = CurTrial+1;
				Target.redo = 0;
				Target.context = curtr.context;
				Target.item = curtr.item;
				Target.key = ' ';
			}
			else if ( (!falsestart && (redotrialflag && CurTrial >= 0)) ||  (falsestart && (nextstateflag || redotrialflag)) )
			{
				redotrialflag = false;
				falsestart = false;
				numredos++;
				Target.trial = CurTrial+1; //we do not update the trial number
				Target.item = curtr.item;
				Target.redo = numredos; //count the number of redos
				Target.key = ' ';
			}
			
			readytext->Off();
			redotext->Off();
			holdtext->Off();
			trialinstructtext->Off();

			//if we have reached the end of the trial table, quit
			if (CurTrial >= NTRIALS)
			{
				std::cerr << "Going to FINISHED state." << std::endl;
				trialTimer->Reset();
				writer->Close();
				state = Finished;
			}
			else
			{
				std::stringstream texttn;
				texttn << CurTrial+1 << "_" << numredos+1;  //CurTrial starts from 0, so we add 1 for convention.
				trialnum = Image::ImageText(trialnum,texttn.str().c_str(),"arial.ttf", 12,textColor);
				std::cerr << "Trial " << CurTrial+1 << " Redo " << numredos+1 << " started at " << SDL_GetTicks() << std::endl;

				//set up the new data file and start recording
				recordData = true;
				recordtext->On();
				//Target.starttime = SDL_GetTicks();
				if (!didrecord)  //detect if a file has ever been opened; if so shut it. if not, skip this.
					didrecord = true;
				else
					writer->Close();  //this will this cause an error if no writer is open
				std::string savfname;
				savfname.assign(TRIALFILE);
				savfname = savfname.substr(savfname.rfind("/")+1);  //cut off the file extension
				savfname = savfname.replace(savfname.rfind(".txt"),4,"");  //cut off the file extension
				std::stringstream datafname;
				//datafname << savfname.c_str() << "_" << SUBJECT_ID << "_Item" << curtr.item << "-" << numredos+1 << "_" << (curtr.vision == 1 ? "VF" : "NVF");
				datafname << DATAPATH << savfname.c_str() << "_" << SUBJECT_ID << "_Item" << curtr.item << "-" << numredos+1;

				writer = new DataWriter(&sysconfig,Target,datafname.str().c_str());  //create the data-output file

				//reset the trial timers
				hoverTimer->Reset();
				trialTimer->Reset();


				mvtStarted = false;
				mvmtEnded = false;

				Target.trace = curtr.item;

				//show the stimulus and play the audio, if desired
				if (curtr.trialType == 0)
				{
					//free stream condition
					//just cue movement onset and go to the Active state

					//play start tone
					startbeep->Play();
					mvtStarted = false;
					mvmtEnded = false;
					movTimer->Reset();
					hoverTimer->Reset();
					trialTimer->Reset();

					readytext->Off();
					holdtext->Off();

					nextstateflag = false;

					state = Active;

				}
				else if (curtr.showcontext >= 0)
				{
					//video trial type, some kind of context is requested (number or audio)

					if (curtr.showcontext > 0)
					{
						//a text-based context has been selected, so we need to turn it on
						contextText[curtr.context-1]->On();
						//items[curtr.context-1]->On();
					}
					//play the audio context
					itemsounds[curtr.context-1]->Play();

					Target.vidstatus = 0;
					Target.showcontext = curtr.showcontext;
					Target.context = curtr.context;

					state = ShowContext; //move to the next state
				}
				else 
				{
					//no context requested, skip directly to show video state
					Vid[curtr.item-1]->Play();
					Target.vidstatus = 1;
					
				}
				
				std::cerr << "Leaving WAITSTATE state." << std::endl;

			}

		}
		break;
	
	case ShowContext:

		//check if the hand moves too early; if so, reset the trial
		if (player->Distance(startCircle) > START_RADIUS)
		{	//detected movement too early; 
			//std::cerr << "Player: " << player->GetX() << " , " << player->GetY() << " , " << player->GetZ() << std::endl;
			//std::cerr << "Circle: " << startCircle->GetX() << " , " << startCircle->GetY() << " , " << startCircle->GetZ() << std::endl;
			//std::cerr << "Distance: " << player->Distance(startCircle) << std::endl;
			mvtStarted = true;
			//items[curtr.context-1]->Off();
			contextText[curtr.context-1]->Off();
			holdtext->On();
			hoverTimer->Reset();
		}

		if (mvtStarted && !falsestart && (hoverTimer->Elapsed() > 1000) )  //moved too soon!
		{
			//state = WaitStim;

			contextText[curtr.context-1]->Off();
			returntext->On();
			nextstateflag = false;
			redotrialflag = false;
			falsestart = true;
			Target.key = ' ';
			trialTimer->Reset();
		}

		if (falsestart && (player->Distance(startCircle) < START_RADIUS)) //(falsestart && (trialTimer->Elapsed() > 1000))
		{
			nextstateflag = false;
			redotrialflag = false;
			Target.key = ' ';
			recordtext->Off();
			hoverTimer->Reset();
			state = WaitStim;
			std::cerr << "False start; returning to WAITSTIM state." << std::endl;
		}

		
		if (!donePlayingContext && (itemsounds[curtr.context-1]->IsPlaying() == 0))
		{
			hoverTimer->Reset();
			donePlayingContext = true;
		}

		//the hand remained still and the context duration has expired, move on
		//    3 conditions: flag to do simultaneous context and video, move on right away
		//					flag to move on after done playing audio (with a brief pause)
		//                  wait until the requested duration (or the audio finishes playing), then move on
		if (!mvtStarted && ( (curtr.contextdur == -2) || (donePlayingContext && (curtr.contextdur == -1) && (hoverTimer->Elapsed() > 500)) || (donePlayingContext && (hoverTimer->Elapsed() > curtr.contextdur) && (curtr.contextdur>=0)) ) )
		{

			if (curtr.showcontext > 0)
			{
				//items[curtr.context-1]->Off();
				contextText[curtr.context-1]->Off();
				Target.context = -1;
				Target.showcontext = 0;
			}

			Vid[curtr.item-1]->Play();
			Target.vidstatus = 1;
			state = ShowStim;
			hoverTimer->Reset();

			std::cerr << "Leaving SHOWCONTEXT state." << std::endl;
		}


		break;

	case ShowStim:
		//show the stimulus and wait

		//std::cerr << "Item: " << curtr.item-1 << " drawn." << std::endl;
		//items[curtr.item-1]->On();

		if (player->Distance(startCircle) > START_RADIUS)
		{	//detected movement too early; 
			//std::cerr << "Player: " << player->GetX() << " , " << player->GetY() << " , " << player->GetZ() << std::endl;
			//std::cerr << "Circle: " << startCircle->GetX() << " , " << startCircle->GetY() << " , " << startCircle->GetZ() << std::endl;
			//std::cerr << "Distance: " << player->Distance(startCircle) << std::endl;
			mvtStarted = true;
			contextText[curtr.context-1]->Off();
			Vid[curtr.item-1]->Stop();
			Vid[curtr.item-1]->Invisible();
			Target.vidstatus = 0;
			//items[curtr.context-1]->Off();
			Target.context = -1;
			holdtext->On();
			hoverTimer->Reset();
		}

		if (mvtStarted && !falsestart && (hoverTimer->Elapsed() > 1000) )  //moved too soon!
		{
			//state = WaitStim;
			contextText[curtr.context-1]->Off();
			Vid[curtr.item-1]->Stop();
			Vid[curtr.item-1]->Invisible();
			Target.vidstatus = 0;
			returntext->On();
			nextstateflag = false;
			redotrialflag = false;
			falsestart = true;
			Target.key = ' ';
			trialTimer->Reset();
		}

		if (falsestart && (player->Distance(startCircle) < START_RADIUS)) //(falsestart && (trialTimer->Elapsed() > 1000))
		{
			nextstateflag = false;
			redotrialflag = false;
			Target.key = ' ';
			recordtext->Off();
			hoverTimer->Reset();
			state = WaitStim;
			std::cerr << "False start; returning to WAITSTIM state." << std::endl;
		}

		//video has finished playing, we can shut it off and flag ready to proceed
		if (!falsestart && Vid[curtr.item-1]->HasEnded())
		{
			vidEnded = true;
			trialTimer->Reset();
			Target.vidstatus = 2;
			Vid[curtr.item-1]->Stop();
			Vid[curtr.item-1]->ResetVid();
			//items[curtr.context-1]->Off();
			//contextText[curtr.context-1]->Off();
			Target.context = -1;
		}

		if (!mvtStarted && vidEnded && (trialTimer->Elapsed() > curtr.srdur) )  //prompt start signal
		{
			
			//play start tone
			startbeep->Play();
			mvtStarted = false;
			mvmtEnded = false;
			movTimer->Reset();
			hoverTimer->Reset();
			trialTimer->Reset();

			//recordData = true;
			//recordtext->On();

			//we will replay the video now so people can imitate during the second video play.
			Vid[curtr.item-1]->Play();

			readytext->Off();
			holdtext->Off();

			std::cerr << "Leaving SHOWSTIM state." << std::endl;

			nextstateflag = false;

			state = Active;
		}

		break;

	case Active:

		//detect the onset of hand movement, for calculating latency
		if (!mvtStarted && (player->Distance(startCircle) > START_RADIUS))
		{
			mvtStarted = true;
			Target.lat = movTimer->Elapsed();
			movTimer->Reset();
		}

		//detect movement offset
		if (!mvmtEnded && mvtStarted && (player->GetVel3D() < VEL_MVT_TH) && (movTimer->Elapsed()>200))
		{
			mvmtEnded = true;
			//Target.dur = movTimer->Elapsed();
			hoverTimer->Reset();
			//std::cerr << "Mvmt Ended: " << float(SDL_GetTicks()) << std::endl;
		}

		if (mvmtEnded && (hoverTimer->Elapsed() >= VEL_END_TIME))
		{
			Target.dur = movTimer->Elapsed()-VEL_END_TIME;  //if the hand was still long enough, call this the "end" of the movement!
		}
		else if (mvmtEnded && (hoverTimer->Elapsed() < VEL_END_TIME) && (player->GetVel3D() > VEL_MVT_TH))
			mvmtEnded = false;  //just kidding, the movement is still going...

		//if trial duration is exceeded, display a "stop" signal but do not change state until experimenter prompts
		if (!nextstateflag && ((curtr.trdur > 0 && trialTimer->Elapsed() > curtr.trdur) || (trialTimer->Elapsed() > MAX_TRIAL_DURATION) ))
		{
			Vid[curtr.item-1]->Stop();
			Vid[curtr.item-1]->Invisible();
			Target.vidstatus = 0;
			stoptext->On();
		}

		//if the experimenter ends the trial
		if (nextstateflag)
		{
			nextstateflag = false;
			Target.key = ' ';

			if (!mvmtEnded)
			{
				//if the movement hasn't ended yet, we will call this the "duration"
				Target.dur = movTimer->Elapsed();
			}

			contextText[curtr.context-1]->Off();

			Vid[curtr.item-1]->Stop();
			Vid[curtr.item-1]->ResetVid();
			Vid[curtr.item-1]->Invisible();

			returntostart = false;

			stoptext->Off();

			//stop recording data
			recordtext->Off();
			recordData = false;

			//go to next state
			trialTimer->Reset();// = SDL_GetTicks();
			state = EndTrial;

		}

		break;

	case EndTrial:

		returntext->On();

		if (player->Distance(startCircle) > START_RADIUS)
			returntostart = false;
		else
			returntostart = true;

		//wait for participant to return to start before allowing entry into next trial
		if (returntostart)
		{
			returntext->Off();
				
			//make sure that all the videos have stopped playing and are invisible
			for (int a = 0; a < NVids; a++)
			{
				Vid[a]->Stop(); //stop also renders the video invisible
				Vid[a]->ResetVid();
			}


			nextstateflag = false;
			std::cerr << "Ending Trial." << std::endl;
			state = WaitStim;

		}

		break;

	case Finished:
		// Trial table ended, wait for program to quit

		endtext->On();

		if (trialTimer->Elapsed() > 5000)
			quit = true;


		break;

	}
}

