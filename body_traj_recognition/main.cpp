/* This code runs a same/different test for the trajectory/body-config project.
   
   In this task, people will observe two videos played sequentially, and must report
   if the movements shown in the videos are the same or different.

   Responses will be made by pressing keys on a keyboard.

*/


#include <cmath>
#include <sstream>
#include <iostream>
#include <fstream>
#include <istream>
#include <windows.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"

#include "Circle.h"
#include "DataWriter.h"
#include "Object2D.h"
#include "Sound.h"
#include "Timer.h"
#include "Image.h"
#include "vlcVideoPlayerSM.h"

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
	ShowStim1 = 0x04, //00100
	ShowStim2 = 0x05,   //00101
	Active = 0x06,     //00110
	EndTrial = 0x08, //01000
	Finished = 0x10    //10000
};



SDL_Event event;
SDL_Window *screen = NULL;
SDL_GLContext glcontext = NULL;

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
Image* respondtext = NULL;
Image* trialnum = NULL;
Sound* startbeep = NULL;
Sound* correctbeep = NULL;
Sound* errorbeep = NULL;
SDL_Color textColor = {0, 0, 0, 1};
DataWriter* writer = NULL;
GameState state;
Timer* trialTimer;
Timer* hoverTimer;
Timer* movTimer;

//videos
Video *Vid1 = NULL;
Video *Vid2 = NULL;
int NVid1;
int NVid2;
std::string vpathbase;
int textcontext; //flag to also create/show text along with audio context
int NcontextTexts;

bool recordData = false;
bool didrecord = false;

int response = -1;
int score = 0;

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
	int trialType;		// Flag for trial type ( 1 = Same/Different test )
	int vid1;			//First video file
	int vid2;			//Second video file
	int vidtype;		// 0 = same; 1 = same traj/different body config; 2 = same body config/different trajectory; 3 = different both
	int cresp;			//correct response (0 = same, 1 = different)
	int srdur;			//duration to wait after video playback before go cue (stim-response delay duration)
	int trdur;			//trial duration (max response time)
	int practice;		//flag if the block is a practice block or not (changes the instructions disiplayed at the start of the block)
	int pausetrial;		//flag to pause between trials (for patient experiments) or to run continuously (for controls)
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
bool skiptrialflag = false;
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
// Update loop (state machine)
void game_update();
int VidLoad(const char* fname, int vidnum);


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

	//DataStartTime = SDL_GetTicks();

	while (!quit)
	{
		int inputs_updated = 0;

		// Retrieve Flock of Birds data
		// Handle SDL events
		while (SDL_PollEvent(&event))
		{
			// See http://www.libsdl.org/docs/html/sdlevent.html for list of event types
			//if (event.type == SDL_MOUSEMOTION)
			//{
			//	MouseInput::ProcessEvent(event);
			//	inputs_updated = MouseInput::GetFrame(dataframe);
			//}
			if (event.type == SDL_KEYDOWN)
			{
				// See http://www.libsdl.org/docs/html/sdlkey.html for Keysym definitions
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
				else if (event.key.keysym.sym == SDLK_r)
				{
					redotrialflag = true;
					Target.key = 'r';

					//std::cerr << "Redo requested" << std::endl;
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					skiptrialflag = true;
					Target.key = 's';

					//std::cerr << "Skip Trial requested" << std::endl;
				}
				else if (event.key.keysym.sym == SDLK_SPACE)
				{
					nextstateflag = true;
					Target.key = 's';
					inputs_updated = 1;
					//std::cerr << "Advance requested" << std::endl;
				}
				else if ((event.key.keysym.sym == SDLK_z) || (event.key.keysym.sym == SDLK_f))
				{
					response = 0;
					Target.key = 'z';
					inputs_updated = 1;
					//std::cerr << "Respond 0" << std::endl;
				}
				else if ((event.key.keysym.sym == SDLK_x) || (event.key.keysym.sym == SDLK_j))
				{
					response = 1;
					Target.key = 'x';
					inputs_updated = 1;
					//std::cerr << "Respond 1" << std::endl;
				}
				else //if( event.key.keysym.unicode < 0x80 && event.key.keysym.unicode > 0 )
				{
					Target.key = *SDL_GetKeyName(event.key.keysym.sym);  //(char)event.key.keysym.unicode;
					inputs_updated = 1;
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
		if (inputs_updated > 0 || recordData) // if there is a new frame of data
		{
			//std::cerr << "InputsUpdated" << std::endl;

			Target.time = SDL_GetTicks();

			//updatedisplay = true;
			if (recordData)  //only write out if we need to
			{
				writer->Record(a, Target);
				//std::cerr << "Data Frame written." << std::endl;

				recordData = false;
			}
			else
				Target.starttime = SDL_GetTicks();

		} //end if inputs updated

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
		sscanf(tmpline, "%d %d %d %d %d %d %d %d %d", 
			&trtbl[ntrials].trialType,
			&trtbl[ntrials].vid1,
			&trtbl[ntrials].vid2,
			&trtbl[ntrials].vidtype,
			&trtbl[ntrials].cresp,
			&trtbl[ntrials].srdur,
			&trtbl[ntrials].trdur,
			&trtbl[ntrials].practice,
			&trtbl[ntrials].pausetrial);
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



int VidLoad(const char* fname, int vidnum)
{

	//call to (re)load a new video

	//if (Vid->IsValid())
		//Vid->CleanUp();

	int errcode;
	int NVid;

	if (vidnum == 1)
	{

		//std::cerr << "VIdPath: " << vidfile.str().c_str();
		//Vid = new Video(vidfile.str().c_str(),SCREEN_WIDTH/2,SCREEN_HEIGHT/2,VIDEO_WIDTH,VIDEO_HEIGHT,&errcode);
		if (Vid1 == NULL)
			Vid1 = new Video(fname,SCREEN_WIDTH/2,SCREEN_HEIGHT/2,VIDEO_WIDTH,VIDEO_HEIGHT,&errcode);
		else
			errcode = Vid1->LoadNewVid(fname);

		if (errcode != 0)
		{
			std::cerr << "Video " << NVid1 << " did not load." << std::endl;
			Vid1->SetValidStatus(0);
			NVid1 = -1;
		}		
		else
		{
			std::cerr << "   Video " << NVid1 << " loaded." << std::endl;
			std::cerr << "      Vid_size: " << VIDEO_WIDTH << "x" << VIDEO_HEIGHT << std::endl;
		}

		NVid = NVid1;

	}

	else if (vidnum == 2)
	{

		//std::cerr << "VIdPath: " << vidfile.str().c_str();
		//Vid = new Video(vidfile.str().c_str(),SCREEN_WIDTH/2,SCREEN_HEIGHT/2,VIDEO_WIDTH,VIDEO_HEIGHT,&errcode);
		if (Vid2 == NULL)
			Vid2 = new Video(fname,SCREEN_WIDTH/2,SCREEN_HEIGHT/2,VIDEO_WIDTH,VIDEO_HEIGHT,&errcode);
		else
			errcode = Vid2->LoadNewVid(fname);

		if (errcode != 0)
		{
			std::cerr << "Video " << NVid2 << " did not load." << std::endl;
			Vid2->SetValidStatus(0);
			NVid2 = -1;
		}		
		else
		{
			std::cerr << "   Video " << NVid2 << " loaded." << std::endl;
			std::cerr << "      Vid_size: " << VIDEO_WIDTH << "x" << VIDEO_HEIGHT << std::endl;
		}

		NVid = NVid2;
	}

	return(NVid);
}


//initialization function - set up the experimental environment and load all relevant parameters/files
bool init()
{

	int a;
	char tmpstr[80];
	char fname[50] = TRIALFILE;
	

	std::cerr << "Start init." << std::endl;

	Target.starttime = 0;

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

	a = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 512);  //initialize SDL_mixer
	//a = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 2048);  //initialize SDL_mixer, may have to play with the chunksize parameter to tune this a bit
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
	std::string savfname;
	savfname.assign(TRIALFILE);
	savfname = savfname.substr(savfname.rfind("/")+1);  //cut off the file extension
	savfname = savfname.replace(savfname.rfind(".txt"),4,"");  //cut off the file extension
	std::stringstream datafname;
				
	datafname << DATAPATH << savfname.c_str() << "_" << SUBJECT_ID;
	std::cerr << "SavFileName: " << datafname.str() << std::endl;

	writer = new DataWriter(Target,datafname.str().c_str());



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
			instructimages[1][a] = new Object2D(instructim[1][a]);
			std::cerr << "   Instruction P" << a << " loaded." << std::endl;
			instructimages[1][a]->SetPos(PHYSICAL_WIDTH / 2, PHYSICAL_HEIGHT / 2);
		}
	}
	
	//initialize the video player and the video file
	vpathbase.assign(VPATH);

	std::stringstream vidfile;
	vidfile.str(std::string());
	NVid1 = trtbl[0].vid1;
	vidfile << vpathbase.c_str() <<  "Video" << NVid1 << ".mp4";
	VidLoad(vidfile.str().c_str(),1);

	std::stringstream vidfile2;
	vidfile2.str(std::string());
	NVid2 = trtbl[0].vid2;
	vidfile2 << vpathbase.c_str() <<  "Video" << NVid2 << ".mp4";
	VidLoad(vidfile2.str().c_str(),2);

	/*
	Vid = new Video(vidfile.str().c_str(),SCREEN_WIDTH/2,SCREEN_HEIGHT/2,VIDEO_WIDTH,VIDEO_HEIGHT,&errcode);
	if (errcode != 0)
	{
		std::cerr << "Video " << NVid << " did not load." << std::endl;
		Vid->SetValidStatus(0);
		NVid = -1;
	}		
	else
	{
		std::cerr << "   Video " << NVid << " loaded." << std::endl;
		std::cerr << "      Vid_size: " << VIDEO_WIDTH << "x" << VIDEO_HEIGHT << std::endl;
		//Vid[a]->SetValidStatus(1);
		//Vid[a]->SetPos(SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
	}
	*/

	startbeep = new Sound("./Resources/beep.wav");
	correctbeep = new Sound("./Resources/coin.wav");
	errorbeep = new Sound("./Resources/errorbeep.wav");
	std::cerr << "Audio loaded." << std::endl;


	//set up placeholder text
	endtext = Image::ImageText(endtext, "Block ended.","arial.ttf", 28, textColor);
	endtext->Off();

	readytext = Image::ImageText(readytext, "Get ready...","arial.ttf", 28, textColor);
	readytext->Off();
	stoptext = Image::ImageText(stoptext, "STOP!","arial.ttf", 32, textColor);
	stoptext->Off();
	holdtext = Image::ImageText(holdtext, "Wait until the go signal!","arial.ttf", 28, textColor);
	holdtext->Off();

	respondtext = Image::ImageText(respondtext, "Same or Different?","arial.ttf", 28, textColor);
	respondtext->Off();

	trialinstructtext = Image::ImageText(trialinstructtext, "Press (space) to advance, (r) to repeat, or (s) to skip trial.","arial.ttf", 12, textColor);
	trialinstructtext->Off();
	redotext = Image::ImageText(redotext, "Press (r) to repeat or (s) to skip trial.","arial.ttf", 12, textColor);
	redotext->Off();

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
	delete correctbeep;
	delete errorbeep;

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

	Vid1->CleanUp();
	delete Vid1;

	Vid2->CleanUp();
	delete Vid2;
	//std::cerr << "Deleted all objects." << std::endl;

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(screen);
	Mix_CloseAudio();
	TTF_Quit();
	SDL_Quit();

	std::cerr << "Shut down SDL." << std::endl;

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
			instructimages[b][a]->Draw(PHYSICAL_WIDTH,PHYSICAL_HEIGHT);
			if (instructimages[b][a]->DrawState())
				Target.instruct = b*NINSTRUCT + a;
		}


	Vid1->Update();
	Vid2->Update();

	// Draw text
	endtext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);

	readytext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);
	stoptext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);
	holdtext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);

	respondtext->Draw(float(PHYSICAL_WIDTH)/2.0f,float(PHYSICAL_HEIGHT)*2.0f/3.0f);

	trialinstructtext->DrawAlign(PHYSICAL_WIDTH*0.5f/24.0f,PHYSICAL_HEIGHT*0.5f/24.0f,3);
	redotext->DrawAlign(PHYSICAL_WIDTH*0.5f/24.0f,PHYSICAL_HEIGHT*0.5f/24.0f,3);
	//write the trial number
	trialnum->Draw(PHYSICAL_WIDTH*23.0f/24.0f, PHYSICAL_HEIGHT*0.5f/24.0f);

	SDL_GL_SwapWindow(screen);
	glFlush();

}


//game update loop - state machine controlling the status of the experiment
bool falsestart = false;
bool vidEnded = false;
bool vidEndedAck = false;
bool donePlayingContext = false;

bool responded = false;

bool writefinalscore;

bool loadVid;

void game_update()
{

	switch (state)
	{
	case Idle:
		// This state only happens at the start of the experiment, and is just a null pause until the experimenter is ready to begin.

		Target.trial = -1;
		Target.instruct = -1;
		Target.redo = -1;

		Target.vid1 = -1;
		Target.vid2 = -1;
		Target.vidtype = -1;
		Target.vidstatus = -1;

		Target.cresp = -1;			//correct response (0 = same, 1 = different)
		Target.resp = -1;
		Target.correct = -1;

		Target.practice = trtbl[0].practice;	

		Target.key = ' ';

		Target.lat = -999;
		Target.dur = -1;
		
		Target.TrType = trtbl[0].trialType;

		endtext->Off();
		readytext->Off();
		stoptext->Off();
		trialinstructtext->Off();
		holdtext->Off();
		respondtext->Off();

		loadVid = false;
		
		recordData = false;

		for (int b = 0; b < 2; b++)
			for (int a = 0; a < NINSTRUCT; a++)
				instructimages[b][a]->Off();
		
		if (Vid1->VisibleState() != 0)
			Vid1->Invisible();

		if (Vid2->VisibleState() != 0)
			Vid2->Invisible();

		hoverTimer->Reset();
		trialTimer->Reset();

		nextstateflag = false;

		Target.key = ' ';

		std::cerr << "Leaving IDLE state." << std::endl;

		state = Instruct;

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
			skiptrialflag = false;

			instructimages[trtbl[0].practice][trtbl[0].trialType-1]->Off();

			//make sure that all the videos have stopped playing and are invisible
			Vid1->Stop();
			Vid1->ResetVid();
			
			Vid2->Stop();
			Vid2->ResetVid();


			falsestart = false;

			//Target.trial = CurTrial+1;

			Target.key = ' ';

			hoverTimer->Reset();

			state = WaitStim;
			std::cerr << "Leaving INSTRUCT state." << std::endl;

		}
		break;

	case WaitStim:
		//this state is just a "pause" state between trials.
		
		trialnum->Off();  //is it confusing that we show the "last" trial here, since we don't know if we want to redo or advance yet?

		Target.vid1 = -1;
		Target.vid2 = -1;
		Target.vidtype = -1;

		Target.vidstatus = -1;

		Target.cresp = -1;			//correct response (0 = same, 1 = different);
		Target.resp = -1;
		Target.correct = -1;

		Target.key = ' ';
		response = -1;

		Target.lat = -999;
		Target.dur = -1;

		vidEnded = false;
		vidEndedAck = false;
		donePlayingContext = false;
		
		responded = false;
		response = -1;

		//show "pause" text, and instructions to experimenter at the bottom of the screen
		readytext->On();
		holdtext->Off();
		respondtext->Off();
		if (falsestart && hoverTimer->Elapsed() < 1000)
		{
			readytext->Off();
			holdtext->On();
		}

		//recordtext->Off();

		if (!falsestart)
		{
			trialinstructtext->On();
			if ((CurTrial >= 0 && curtr.pausetrial == 0) || (CurTrial < 0 && trtbl[0].pausetrial == 0))
				nextstateflag = true;  //if we request to automatically move on, we force the nextstateflag 
		}
		else
		{
			redotext->On();
			if ((CurTrial >= 0 && curtr.pausetrial == 0) || (CurTrial < 0 && trtbl[0].pausetrial == 0))
				redotrialflag = true;  //if we request to automatically move on, we force the nextstateflag 
		}
		


		if (hoverTimer->Elapsed() > 1200 && (nextstateflag || redotrialflag || skiptrialflag) )
		{
			
			if (skiptrialflag || (!falsestart && (nextstateflag || (redotrialflag && CurTrial < 0) ))) //if accidentally hit "r" before the first trial, ignore this error!  //if false start but want to just move on, can use skip trial option
			{
				nextstateflag = false;
				redotrialflag = false;
				skiptrialflag = false;
				falsestart = false;
				numredos = 0;
				CurTrial++;  //we started the experiment with CurTrial = -1, so now we are on the "next" trial (or first trial)
				Target.trial = CurTrial+1;
				Target.redo = 0;
				Target.vid1 = curtr.vid1;
				Target.vid2 = curtr.vid2;
				Target.vidtype = curtr.vidtype;
				Target.cresp = curtr.cresp;
				Target.key = ' ';
			}
			else if ( (!falsestart && (redotrialflag && CurTrial >= 0)) ||  (falsestart && (nextstateflag || redotrialflag)) )
			{
				redotrialflag = false;
				falsestart = false;
				numredos++;
				Target.trial = CurTrial+1; //we do not update the trial number
				Target.vid1 = curtr.vid1;
				Target.vid2 = curtr.vid2;
				Target.vidtype = curtr.vidtype;
				Target.redo = numredos; //count the number of redos
				Target.key = ' ';
				Target.cresp = curtr.cresp;
			}
			
			if (NVid1 == curtr.vid1 && Vid1->IsValid())
			{
				//video1 is already loaded successfully, do nothing
				std::cerr << "Video " << NVid1 << " already loaded." << std::endl;
			}
			else
			{
				std::stringstream vidfile;
				vidfile.str(std::string());
				NVid1 = curtr.vid1;
				vidfile << vpathbase.c_str() <<  "Video" << NVid1 << ".mp4";
				VidLoad(vidfile.str().c_str(),1);
			}

			if (NVid2 == curtr.vid2 && Vid2->IsValid())
			{
				//video2 is already loaded successfully, do nothing
				std::cerr << "Video " << NVid2 << " already loaded." << std::endl;
			}
			else
			{
				std::stringstream vidfile;
				vidfile.str(std::string());
				NVid2 = curtr.vid2;
				vidfile << vpathbase.c_str() <<  "Video" << NVid2 << ".mp4";
				VidLoad(vidfile.str().c_str(),2);
			}

			Target.vidstatus = 0;
			vidEnded = false;
			vidEndedAck = false;

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

				//Target.starttime = SDL_GetTicks();

				//reset the trial timers
				hoverTimer->Reset();
				trialTimer->Reset();

				//go to show the first video state
				Vid1->Play();
				Target.vidstatus = 1;
					
				std::cerr << "Leaving WAITSTATE state." << std::endl;
				state = ShowStim1;

			}

		}
		break;
	

	case ShowStim1:
		//show the first video and wait

		//std::cerr << "Item: " << curtr.item-1 << " drawn." << std::endl;
		//items[curtr.item-1]->On();

		if (response >= 0 && !falsestart)
		{	//detected response too early; 
			Vid1->Stop();
			Vid1->Invisible();
			Target.vidstatus = 0;
			holdtext->On();
			hoverTimer->Reset();

			falsestart = true;
		}

		if (falsestart && (hoverTimer->Elapsed() > 1000) )  //moved too soon!
		{
			Vid1->ResetVid();
			Target.vidstatus = 0;
			nextstateflag = false;
			redotrialflag = false;
			falsestart = true;
			Target.key = ' ';
			response = -1;
			trialTimer->Reset();

			state = WaitStim;
			std::cerr << "False start; returning to WAITSTIM state." << std::endl;
		}

		if (!vidEnded && (Vid1->HasEnded() == 1))
		{
			vidEnded = true;
			hoverTimer->Reset();
		}


		//video has finished playing, we can shut it off and flag ready to proceed
		if (!falsestart && vidEnded && !vidEndedAck && (hoverTimer->Elapsed() > 300) )
		{
			vidEndedAck = true;
			std::cerr << "Vid1 finished playing. " << std::endl;
			trialTimer->Reset();

			Vid1->Stop();
			Vid1->ResetVid();
			
			//Target.vidstatus = 2;
		}

		
		//if video finished playing and we are ready to move on...
		if (vidEndedAck && (trialTimer->Elapsed() > curtr.srdur) )  //prompt start signal
		{
			
			movTimer->Reset();
			hoverTimer->Reset();
			trialTimer->Reset();

			//play video 2
			Vid1->Invisible();
			Vid2->Play();
			Target.vidstatus = 2;

			readytext->Off();
			holdtext->Off();

			vidEnded = false;
			vidEndedAck = false;

			std::cerr << "Leaving SHOWSTIM1 state." << std::endl;

			nextstateflag = false;

			state = ShowStim2;
		}

		break;

	case ShowStim2:
		//show the second video and wait

		//std::cerr << "Item: " << curtr.item-1 << " drawn." << std::endl;
		//items[curtr.item-1]->On();

		if (response >= 0 && !falsestart)
		{	//detected response too early; 
			Vid2->Stop();
			Vid2->Invisible();
			Target.vidstatus = 0;
			holdtext->On();
			hoverTimer->Reset();

			falsestart = true;
		}

		if (falsestart && (hoverTimer->Elapsed() > 1000) )  //moved too soon!
		{
			Vid2->ResetVid();
			Target.vidstatus = 0;
			nextstateflag = false;
			redotrialflag = false;
			falsestart = true;
			Target.key = ' ';
			response = -1;
			trialTimer->Reset();

			state = WaitStim;
			std::cerr << "False start; returning to WAITSTIM state." << std::endl;
		}

		if (!vidEnded && (Vid2->HasEnded() == 1))
		{
			vidEnded = true;
			hoverTimer->Reset();
		}


		//video has finished playing, we can shut it off and flag ready to proceed
		if (!falsestart && vidEnded && !vidEndedAck && (hoverTimer->Elapsed() > 300) )
		{
			vidEndedAck = true;
			std::cerr << "Vid1 finished playing. " << std::endl;
			trialTimer->Reset();

			Vid2->Stop();
			Vid2->ResetVid();
			
			//Target.vidstatus = 2;
		}

		
		//if video finished playing and we are ready to move on...
		if (vidEndedAck && (trialTimer->Elapsed() > curtr.srdur) )  //prompt start signal
		{
			//recordData = true;

			//play start tone
			startbeep->Play();
			respondtext->On();
			movTimer->Reset();
			hoverTimer->Reset();
			trialTimer->Reset();

			//we will replay the video now so people can imitate during the second video play.
			Vid1->Invisible();
			Vid2->Invisible();
			Target.vidstatus = 0;

			readytext->Off();
			holdtext->Off();

			std::cerr << "Leaving SHOWSTIM2 state." << std::endl;

			nextstateflag = false;

			state = Active;
		}

		break;

	case Active:

		//detect the first response, record and calculate latency
		if (response >= 0 && !responded)
		{
			Target.lat = movTimer->Elapsed();
			Target.resp = response;
			responded = true;
			hoverTimer->Reset();

			recordData = true;

			//record if response is correct
			if (response == curtr.cresp) //curtr.cresp codes same/different; vidtype codes how they are different
				Target.correct = 1;
			else
				Target.correct = 0;

			score += Target.correct;
			Target.score = score;

			//give feedback if in a practice block
			if (curtr.practice && (Target.correct == 1) && (Target.lat > 100))
				correctbeep->Play();
			else if (curtr.practice && (Target.correct == 0) && (Target.lat > 100))
				errorbeep->Play();
			//if no feedback tone, the response latency was too short

		}


		//if trial duration is exceeded, display a "stop" signal but do not change state until experimenter prompts
		if (!nextstateflag && !responded && ((curtr.trdur > 0 && trialTimer->Elapsed() > curtr.trdur) || (trialTimer->Elapsed() > MAX_TRIAL_DURATION) ))
			stoptext->On();

		//if the experimenter ends the trial or we ask the trial to end automatically
		if (nextstateflag || (responded && curtr.pausetrial == 0 && hoverTimer->Elapsed() > 500))
		{
			nextstateflag = false;
			Target.key = ' ';

			//clear all responses
			response = -1;
			respondtext->Off();

			Target.vidstatus = 0;
			stoptext->Off();

			//go to next state
			trialTimer->Reset();// = SDL_GetTicks();
			hoverTimer->Reset();
			state = EndTrial;

			std::cerr << "Leaving ACTIVE state." << std::endl;

		}

		break;

	case EndTrial:

		//wait for participant to return to start before allowing entry into next trial
		if (response >= 0)
		{

			recordData = false;

			response = -1;
				
		}

		if (hoverTimer->Elapsed() > ITI)
		{

			//make sure that all the videos have stopped playing and are reset (reset also makes them invisible)
			Vid1->Stop();
			Vid1->ResetVid();

			Vid2->Stop();
			Vid2->ResetVid();

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

