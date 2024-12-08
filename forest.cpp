#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>

#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time() 

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif



#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glut.h"



//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Autumn Arcand

// title of these windows:

const char *WINDOWTITLE = "OpenGL / GLUT Sample -- Autumn Arcand -- Final Project";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// which light type:

enum lightType
{
	POINT,
	SPOT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { 0.584f, 0.576f, 0.510f, 0.75f };  // Darker version of the original fog color
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.10f;
const GLfloat FOGSTART    = 5.0f;
const GLfloat FOGEND      = 90.f;

// for lighting:

const float	WHITE[ ] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 40000;		// 10000 milliseconds = 10 seconds

// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;			// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
void			HsvRgb( float[3], float [3] );
void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);
float			Unit(float [3]);

bool Frozen; 			// Freeze the animation

float timeOffset;		// For offsetting cat running animations

// Grid variables
#define XSIDE	50					// length of the x side of the grid
#define X0      (-XSIDE/2.)			// where one side starts
#define NX		500					// how many points in x
#define DX		( XSIDE/(float)NX )	// change in x between the points

#define YGRID	0.f					// y-height of the grid

#define ZSIDE	50					// length of the z side of the grid
#define Z0      (-ZSIDE/2.)			// where one side starts
#define NZ		500					// how many points in z
#define DZ		( ZSIDE/(float)NZ )	// change in z between the points

GLuint GridDL; 						// Declare display list for grid

// Forest object display lists
GLuint BushDL, RockDL, DeerDL, BearDL, OrangeCatDL, BlackCatDL;

// Forest textures
GLuint TreeTexture, FloorTexture, BushDiffuseTexture, BushSpecularTexture, RockTexture, DeerTexture, PanelTexture, BearTexture, OrangeCatTexture, BlackCatTexture;

// Tree vertex buffer
GLuint treeVBO, treeEBO;
std::vector<float> treeVertices;
std::vector<unsigned int> treeIndices;

// Bush vertex buffer
GLuint bushVBO, bushEBO;
std::vector<float> bushVertices;
std::vector<unsigned int> bushIndices;

// Rock vertex buffer
GLuint rockVBO, rockEBO;
std::vector<float> rockVertices;
std::vector<unsigned int> rockIndices;

// Structure to hold tree positions
struct TreePosition {
    float x, z;
    // Constructor
    TreePosition(float xVal, float zVal) : x(xVal), z(zVal) {}
};

// Structure to hold bush positions
struct BushPosition {
	float x, z;
	// Constructor
	BushPosition(float xVal, float zVal) : x(xVal), z(zVal) {}
};

// Structure to hold rock positions
struct RockPosition {
	float x, z;
	// Constructor
	RockPosition(float xVal, float zVal) : x(xVal), z(zVal) {}
};

// Structure to hold deer positions and rotations
struct DeerPosition {
    float x, z;
    float rotationY;
    // Constructor
    DeerPosition(float xVal, float zVal, float rotationVal) : x(xVal), z(zVal), rotationY(rotationVal) {}
};

// Structure to hold orange cat positions
struct OrangeCatPos {
    float x, z;
	float rotationY;              

	OrangeCatPos(float xVal, float zVal, float rotationVal) : x(xVal), z(zVal), rotationY(rotationVal) {}
};

// Structure to hold orange cat positions
struct BlackCatPos {
    float x, z;
	float rotationY;              

	BlackCatPos(float xVal, float zVal, float rotationVal) : x(xVal), z(zVal), rotationY(rotationVal) {}
};

// Structure to hold side panel (wall) positions
struct PanelPosition {
    float x, y, z;
    float width, height, depth;
	// Constructor
    PanelPosition(float xVal, float yVal, float zVal, float w, float h, float d)
        : x(xVal), y(yVal), z(zVal), width(w), height(h), depth(d) {}
};

std::vector<TreePosition> treePositions;
std::vector<BushPosition> bushPositions;
std::vector<RockPosition> rockPositions;
std::vector<DeerPosition> deerPositions;
std::vector<OrangeCatPos> orangeCats; 
std::vector<BlackCatPos> blackCats; 
std::vector<PanelPosition> panelPositions;

// Various scaling for bushes
float bushScaleFactors[] = { 0.37f, 0.3f, 0.27f, 0.24f, 0.2f };

// Various scaling for rocks
float rockScaleFactors[] = { 1.75f, 1.5f, 1.2f, 1.0f, 0.9f };

// Define colors for light
float lightColors[6][3] = {
    {1.0f, 1.0f, 1.0f},  	// White
    {1.0f, 0.0f, 0.0f},  	// Red
    {0.0f, 1.0f, 0.0f},  	// Green
    {0.0f, 0.0f, 1.0f},  	// Blue
    {0.0f, 1.0f, 1.0f},  	// Cyan
    {1.0f, 0.0f, 1.0f}   	// Magenta
};


// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}

// these are here for when you need them -- just uncomment the ones you need:

#include "setmaterial.cpp"
#include "setlight.cpp"
//#include "osusphere.cpp"
//#include "osucone.cpp"
//#include "osutorus.cpp"
#include "bmptotexture.cpp"
#include "loadobjfile.cpp"
#include "keytime.cpp"
#include "glslprogram.cpp"

// Shaders
GLSLProgram Deer, Bear, OrangeCat, BlackCat;

// Keytime variables
Keytimes CameraX, CameraZ, OrangeCatX, BlackCat1X, BlackCat1Z, BlackCat2X, BlackCat2Z, BearScale, DeerScale, CatScale;

// main program:
int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display lists that **will not change**:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it
void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

// Create hard coded tree positions
void InitializeTreePositions() {
	// x, z
    treePositions.push_back(TreePosition((-30.0f / 1.5f) - 4, (20.0f / 1.5f)));
    treePositions.push_back(TreePosition((-27.0f / 1.5f) - 4, (7.0f / 1.5f)));
    treePositions.push_back(TreePosition((-24.0f / 1.5f) - 4, (-10.0f / 1.5f)));
    treePositions.push_back(TreePosition((-20.0f / 1.5f) - 4, (15.0f / 1.5f)));
    treePositions.push_back(TreePosition((-15.0f / 1.5f) - 4, (-5.0f / 1.5f)));
    treePositions.push_back(TreePosition((-13.0f / 1.5f) - 4, (19.0f / 1.5f)));
    treePositions.push_back(TreePosition((-10.0f / 1.5f) - 4, (0.0f / 1.5f)));
    treePositions.push_back(TreePosition((-8.0f / 1.5f) - 4, (12.0f / 1.5f)));
    treePositions.push_back(TreePosition((-6.0f / 1.5f) - 4, (-22.0f / 1.5f)));
    treePositions.push_back(TreePosition((-2.0f / 1.5f) - 4, (-15.0f / 1.5f)));
    treePositions.push_back(TreePosition((0.0f / 1.5f) - 4, (7.0f / 1.5f)));
    treePositions.push_back(TreePosition((3.0f / 1.5f) - 4, (-18.0f / 1.5f)));
    treePositions.push_back(TreePosition((5.0f / 1.5f) - 4, (12.0f / 1.5f)));
    treePositions.push_back(TreePosition((8.0f / 1.5f) - 4, (-8.0f / 1.5f)));
    treePositions.push_back(TreePosition((12.0f / 1.5f) - 4, (22.0f / 1.5f)));
    treePositions.push_back(TreePosition((-32.0f / 1.5f) - 4, (-23.0f / 1.5f)));
    treePositions.push_back(TreePosition((-18.0f / 1.5f) - 4, (22.0f / 1.5f)));
    treePositions.push_back(TreePosition((12.0f / 1.5f) - 4, (5.0f / 1.5f)));
    treePositions.push_back(TreePosition((-28.0f / 1.5f) - 4, (-2.0f / 1.5f)));
    treePositions.push_back(TreePosition((-25.0f / 1.5f) - 4, (-18.0f / 1.5f)));
    treePositions.push_back(TreePosition((-12.0f / 1.5f) - 4, (-10.0f / 1.5f)));
    treePositions.push_back(TreePosition((-5.0f / 1.5f) - 4, (15.0f / 1.5f)));
    treePositions.push_back(TreePosition((7.0f / 1.5f) - 4, (20.0f / 1.5f)));
    treePositions.push_back(TreePosition((-30.0f / 1.5f) - 4, (-8.0f / 1.5f)));
    treePositions.push_back(TreePosition((-22.0f / 1.5f) - 4, (10.0f / 1.5f)));
    treePositions.push_back(TreePosition((-9.0f / 1.5f) - 4, (-4.0f / 1.5f)));
    treePositions.push_back(TreePosition((2.0f / 1.5f) - 4, (-20.0f / 1.5f)));
    treePositions.push_back(TreePosition((9.0f / 1.5f) - 4, (18.0f / 1.5f)));
    treePositions.push_back(TreePosition((12.0f / 1.5f) - 4, (-7.0f / 1.5f)));
    treePositions.push_back(TreePosition((-14.0f / 1.5f) - 4, (5.0f / 1.5f)));
    treePositions.push_back(TreePosition((-32.0f / 1.5f) - 4, (18.0f / 1.5f)));
    treePositions.push_back(TreePosition((-32.0f / 1.5f) - 4, (8.0f / 1.5f)));
    treePositions.push_back(TreePosition((-30.0f / 1.5f) - 4, (-20.0f / 1.5f)));
    treePositions.push_back(TreePosition((-25.0f / 1.5f) - 4, (22.0f / 1.5f)));
    treePositions.push_back(TreePosition((-20.0f / 1.5f) - 4, (-22.0f / 1.5f)));
    treePositions.push_back(TreePosition((-12.0f / 1.5f) - 4, (-22.0f / 1.5f)));
    treePositions.push_back(TreePosition((12.0f / 1.5f) - 4, (-15.0f / 1.5f)));
    treePositions.push_back(TreePosition((12.0f / 1.5f) - 4, (12.0f / 1.5f)));
    treePositions.push_back(TreePosition((10.0f / 1.5f) - 4, (-18.0f / 1.5f)));
    treePositions.push_back(TreePosition((8.0f / 1.5f) - 4, (20.0f / 1.5f)));
    treePositions.push_back(TreePosition((-18.0f / 1.5f) - 4, (-7.0f / 1.5f)));
    treePositions.push_back(TreePosition((-10.0f / 1.5f) - 4, (-12.0f / 1.5f)));
    treePositions.push_back(TreePosition((-8.0f / 1.5f) - 4, (-2.0f / 1.5f)));
    treePositions.push_back(TreePosition((-4.0f / 1.5f) - 4, (8.0f / 1.5f)));
    treePositions.push_back(TreePosition((1.0f / 1.5f) - 4, (-10.0f / 1.5f)));
    treePositions.push_back(TreePosition((3.0f / 1.5f) - 4, (3.0f / 1.5f)));
    treePositions.push_back(TreePosition((5.0f / 1.5f) - 4, (16.0f / 1.5f)));
}

// Create hard coded and random bush positions 
void InitializeBushPositions() {
    srand(static_cast<unsigned int>(time(0)));

    // Number of bushes to generate
    int numBushes = 130;

    // Generate random bush positions
    for (int i = 0; i < numBushes; i++) {
        // Generate x and z coordinates within the range [-60, 60]
        float x = static_cast<float>(rand() % 121 - 60);
        float z = static_cast<float>(rand() % 121 - 60);

        // Add the new random position to the bushPositions vector
        bushPositions.push_back(BushPosition(x, z));
    }

	// x, z
    bushPositions.push_back(BushPosition(-50.0f, 50.0f));
    bushPositions.push_back(BushPosition(-45.0f, -40.0f));
    bushPositions.push_back(BushPosition(25.0f, -10.0f));
    bushPositions.push_back(BushPosition(40.0f, 30.0f));
    bushPositions.push_back(BushPosition(-30.0f, 20.0f));
	bushPositions.push_back(BushPosition(60.0f, 60.0f));
	bushPositions.push_back(BushPosition(-60.0f, 60.0f));
	bushPositions.push_back(BushPosition(-60.0f, -60.0f));
	bushPositions.push_back(BushPosition(60.0f, -60.0f));
	bushPositions.push_back(BushPosition(40.0f, 60.0f));
	bushPositions.push_back(BushPosition(55.0f, 57.0f));
	bushPositions.push_back(BushPosition(30.0f, 59.0f));
    bushPositions.push_back(BushPosition(10.0f, -5.0f));
    bushPositions.push_back(BushPosition(-15.0f, -15.0f));
}

// Create hard coded rock positions
void InitializeRockPositions() {
	// x, z
    rockPositions.push_back(RockPosition(-22.0f, 28.0f));
    rockPositions.push_back(RockPosition(-18.0f, -5.0f));
    rockPositions.push_back(RockPosition(-10.0f, 15.0f));
    rockPositions.push_back(RockPosition(-5.0f, -10.0f));
    rockPositions.push_back(RockPosition(0.0f, 22.0f));
    rockPositions.push_back(RockPosition(7.0f, 2.0f));
    rockPositions.push_back(RockPosition(12.0f, 25.0f));
    rockPositions.push_back(RockPosition(18.0f, -8.0f));
    rockPositions.push_back(RockPosition(22.0f, 13.0f));
    rockPositions.push_back(RockPosition(-21.0f, 5.0f));
    rockPositions.push_back(RockPosition(-10.0f, 0.0f));
    rockPositions.push_back(RockPosition(-15.0f, 30.0f));
    rockPositions.push_back(RockPosition(5.0f, 9.0f));
    rockPositions.push_back(RockPosition(10.0f, -12.0f));
	rockPositions.push_back(RockPosition(6.0f, -7.0f));
    rockPositions.push_back(RockPosition(23.0f, 33.0f));
    rockPositions.push_back(RockPosition(2.0f, 31.0f));
	rockPositions.push_back(RockPosition(-20.0f, 18.0f));
	rockPositions.push_back(RockPosition(-8.0f, 20.0f));
	rockPositions.push_back(RockPosition(1.0f, -3.0f));
	rockPositions.push_back(RockPosition(3.0f, 17.0f));
	rockPositions.push_back(RockPosition(-12.0f, -8.0f));
	rockPositions.push_back(RockPosition(14.0f, 5.0f));
	rockPositions.push_back(RockPosition(-3.0f, 10.0f));
	rockPositions.push_back(RockPosition(17.0f, -4.0f));
	rockPositions.push_back(RockPosition(19.0f, -13.0f));
	rockPositions.push_back(RockPosition(8.0f, 12.0f));
	rockPositions.push_back(RockPosition(-18.0f, 7.0f));
	rockPositions.push_back(RockPosition(13.0f, 8.0f));
	rockPositions.push_back(RockPosition(-18.0f, -2.0f));
	rockPositions.push_back(RockPosition(5.0f, 25.0f));
	rockPositions.push_back(RockPosition(-6.0f, 14.0f));
}

// Create hard coded deer positions and rotations
void InitializeDeerPositions() {
	// x, z, y rotation
    deerPositions.push_back(DeerPosition(-1.0f, 2.0f, 150.0f));
    deerPositions.push_back(DeerPosition(-2.0f, 4.0f, 30.0f));
    deerPositions.push_back(DeerPosition(11.0f, 3.0f, -40.0f));
    deerPositions.push_back(DeerPosition(12.0f, 10.0f, 100.0f));
    deerPositions.push_back(DeerPosition(15.0f, 10.0f, 190.0f));
    deerPositions.push_back(DeerPosition(18.0f, 9.0f, -70.0f));
    deerPositions.push_back(DeerPosition(-4.0f, -10.0f, 150.0f));
    deerPositions.push_back(DeerPosition(-7.0f, -18.0f, -110.0f));
	deerPositions.push_back(DeerPosition(-11.0f, 10.0f, -40.0f)); 
}

// Create hard coded orange cat positions
void InitializeOrangeCats() {
	// x, z, y rotation
    orangeCats.push_back(OrangeCatPos(8.0f, 8.0f, 20.0f));
    orangeCats.push_back(OrangeCatPos(9.0f, 5.0f, -110.0f));
    orangeCats.push_back(OrangeCatPos(-4.0f, -15.0f, 0.0f));
	orangeCats.push_back(OrangeCatPos(-10.0f, 10.0f, 10.0f));
	orangeCats.push_back(OrangeCatPos(3.0f, -18.0f, 60.0f));
}

// Create hard coded black cat positions
void InitializeBlackCats() {
	// x, z, y rotation
    blackCats.push_back(BlackCatPos(10.0f, 7.0f, 25.0f));
    blackCats.push_back(BlackCatPos(0.0f, 8.0f, -110.0f));
    blackCats.push_back(BlackCatPos(-6.0f, -7.0f, -60.0f));
	blackCats.push_back(BlackCatPos(6.0f, 14.0f, -10.0f));
	blackCats.push_back(BlackCatPos(17.0f, 0.0f, 130.0f));
}

// Create side panel (wall) positions
void InitializePanelPositions() {
    panelPositions.push_back(PanelPosition(-XSIDE / 2, 0.0f, -ZSIDE / 2, XSIDE, 0.1f, ZSIDE));  // Left panel
	panelPositions.push_back(PanelPosition(XSIDE / 2, 0.0f, -ZSIDE / 2, XSIDE, 0.1f, ZSIDE));   // Right panel
	panelPositions.push_back(PanelPosition(-XSIDE / 2, 0.0f, ZSIDE / 2, XSIDE, 0.1f, ZSIDE));   // Bottom panel
	panelPositions.push_back(PanelPosition(-XSIDE / 2, 0.0f, -ZSIDE / 2, XSIDE, 0.1f, ZSIDE));  // Top panel
}

// Draw the trees using vertex buffer
void DrawTrees() {
    // Apply material properties
	SetMaterial(0.5f, 0.45f, 0.4f, 30.0f);

	// Bind tree texture
    glBindTexture(GL_TEXTURE_2D, TreeTexture);

    // Bind VBO and EBO
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, treeEBO);

    // Set up vertex attribute pointers
    GLint positionAttribLocation = 0;
    GLint normalAttribLocation = 1;
    GLint texCoordAttribLocation = 2;

    // Position (3 floats)
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 8 * sizeof(float), (void*)0);

    // Normal (3 floats)
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // Texture Coordinates (2 floats)
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // Draw trees
	for (int i = 0; i < treePositions.size(); i++) {
		const TreePosition& pos = treePositions[i];
		glPushMatrix();
			glTranslatef(pos.x, 0.18f, pos.z);
			glScalef(1.5f, 1.6f, 1.5f);
			glDrawElements(GL_TRIANGLES, treeIndices.size(), GL_UNSIGNED_INT, 0); // Indexed drawing
		glPopMatrix();
	}

    // Disable client state
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %d\n", err);
    }
}

// Draw bushes using vertex buffer
void DrawBushes() {
    // Apply material properties
	SetMaterial(0.85f, 0.8f, 0.55f, 50.0f);

	// Activate and bind the bush diffuse texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, BushDiffuseTexture);

	// Activate and bind the bush specular texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, BushSpecularTexture);

    // Bind VBO and EBO
    glBindBuffer(GL_ARRAY_BUFFER, bushVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bushEBO);

    // Set up vertex attribute pointers
    GLint positionAttribLocation = 0;
    GLint normalAttribLocation = 1;
    GLint texCoordAttribLocation = 2;

    // Position (3 floats)
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 8 * sizeof(float), (void*)0);

    // Normal (3 floats)
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // Texture Coordinates (2 floats)
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	for (int i = 0; i < bushPositions.size(); i++) {
		const BushPosition &pos = bushPositions[i];

		// Choose scale from scale factors array
		float scaleFactor = bushScaleFactors[i % 5];

		glPushMatrix();
			glScalef(scaleFactor, scaleFactor, scaleFactor); // Apply scale factor
			glTranslatef(pos.x, 0.0f, pos.z);
			glDrawElements(GL_TRIANGLES, bushIndices.size(), GL_UNSIGNED_INT, 0); // Indexed drawing
		glPopMatrix();
	}

	// Reset active texture to default
	glActiveTexture(GL_TEXTURE0);

    // Disable client state
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %d\n", err);
    }
}

// Draw rocks using vertex buffer
void DrawRocks() {
    // Apply material properties
	SetMaterial(0.4f, 0.35f, 0.3f, 10.0f);

	// Bind rock texture
	glBindTexture(GL_TEXTURE_2D, RockTexture);

    // Bind VBO and EBO
    glBindBuffer(GL_ARRAY_BUFFER, rockVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rockEBO);

    // Set up vertex attribute pointers
    GLint positionAttribLocation = 0;
    GLint normalAttribLocation = 1;
    GLint texCoordAttribLocation = 2;

    // Position (3 floats)
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 8 * sizeof(float), (void*)0);

    // Normal (3 floats)
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // Texture Coordinates (2 floats)
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	for (int i = 0; i < rockPositions.size(); i++) {
		const RockPosition& pos = rockPositions[i];
		// Choose scale from scale factors array
		float scaleFactor = rockScaleFactors[i % 5];

		glPushMatrix();
			glTranslatef(pos.x, 0.4f, pos.z);
			glScalef(scaleFactor, scaleFactor, scaleFactor); // Apply scale factor
			glDrawElements(GL_TRIANGLES, rockIndices.size(), GL_UNSIGNED_INT, 0); // Indexed drawing
		glPopMatrix();
	}

    // Disable client state
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %d\n", err);
    }
}

// draw the complete scene:
void
Display( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif

	// Set the total cycle time to 40 seconds (40,000 milliseconds)
	int cycleTime = 40000;

	// Get the elapsed time in milliseconds and wrap it within the cycle time
	int msec = glutGet(GLUT_ELAPSED_TIME) % cycleTime;

	// Convert the time into a value between 0 and 40 seconds
	float nowTime = (float)msec / 1000.0f;

	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );

	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( NowProjection == ORTHO )
		glOrtho( -2.f, 2.f,     -2.f, 2.f,     0.1f, 1000.f );
	else
		gluPerspective( 70.f, 1.f,	0.1f, 1000.f );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// Get eye positions
	float eyePosX = CameraX.GetValue(nowTime);
	float eyePosZ = CameraZ.GetValue(nowTime);

    // Set the eye
    gluLookAt(eyePosX, 5.0f, eyePosZ, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f); // Animated eye
	//gluLookAt(24.0f, 5.0f, 0.0f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f); // Static eye

    // Rotate the scene:
    glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
    glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);
        
    // Uniformly scale the scene:
    if (Scale < MINSCALE)
        Scale = MINSCALE;
    glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
    

	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[NowColor][0] );
		glCallList( AxesList );
	}

	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable( GL_NORMALIZE );

	SetPointLight(GL_LIGHT0, 20.0f, 60.0f, 25.0f, 0.6f, 0.5f, 0.2f);  // A warm golden light

	// Enable textures and lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Draw the grid
	SetMaterial(0.2f, 0.2f, 0.2f, 5.0f);
	glBindTexture(GL_TEXTURE_2D, FloorTexture); 
	glCallList(GridDL);
	
	// Draw the trees
	DrawTrees();

	// Draw the bushes
	DrawBushes();

	// Draw the rocks
	DrawRocks();

	// Turn on deer shader
	Deer.Use();

	// Pass uniform variables to control the animation
	float deerEnableTurn;    					// Enable turning animation
	float deerEnableGrazing;         			// Enable grazing animation
	float deerTurnDuration = 0.15f;  		// Time spent turning
	float deerPauseDuration = 0.05;			// Time for pause in turning
	float deerTurnIntensity = 0.018f; 			// Degree of turning
	float deerGrazingIntensity = 0.45f; 	// Intensity of turning (adjust for more/less turning)
	float deerGrazingCycleTime = 0.2f;  	// Time for one complete grazing cycle

	Deer.SetUniformVariable("uTime", Time);
	Deer.SetUniformVariable("uTurnIntensity", deerTurnIntensity);
	Deer.SetUniformVariable("uGrazingCycleTime", deerGrazingCycleTime);
	Deer.SetUniformVariable("uGrazingIntensity", deerGrazingIntensity);
	Deer.SetUniformVariable("uTurnDuration", deerTurnDuration);
	Deer.SetUniformVariable("uPauseDuration", deerPauseDuration);

	// Bind the deer texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DeerTexture);

	// Pass texture to shader
	Deer.SetUniformVariable("uTexture", 0);

	// Loop through and draw each deer at its position
    for (int i = 0; i < deerPositions.size(); i++) {
        const DeerPosition& pos = deerPositions[i];

		// Enable turning animation for half the deer
		// and enable grazing animtion for other half
		if (i % 2 == 0) {
			deerEnableTurn = 1.0f;
			deerEnableGrazing = 0.0f;
		} else {
			deerEnableTurn = 0.0f;
			deerEnableGrazing = 1.0f;
		}

		// Pass enable animation variables to shader
		Deer.SetUniformVariable("uEnableTurn", deerEnableTurn);
		Deer.SetUniformVariable("uEnableGrazing", deerEnableGrazing);

        glPushMatrix();
			glRotatef(pos.rotationY, 0.0f, 1.0f, 0.0f);
			glTranslatef(pos.x, 0.0f, pos.z);
			glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
			glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

			// Apply keytimed scaling
			float deerScale = DeerScale.GetValue(nowTime);
			glScalef(0.1f, 0.1f, deerScale);

			glCallList(DeerDL);
        glPopMatrix();
    }

	// Turn off deer shader
	Deer.UnUse();

	// Turn on bear shader
	Bear.Use();

	// Pass uniform variables to control the animation
	float bearTurnIntensity = 0.007f; 	// Amount of turning
	float bearEnableTurn = 1.0f;    	// Enable turning
	float bearTurnDuration = 0.15f;		// Time spent turning
	float bearPauseDuration = 0.005f;	// Time of pause in turning

	Bear.SetUniformVariable("uTime", Time);
	Bear.SetUniformVariable("uTurnIntensity", bearTurnIntensity);
	Bear.SetUniformVariable("uEnableTurn", bearEnableTurn);
	Bear.SetUniformVariable("uTurnDuration", bearTurnDuration);
	Bear.SetUniformVariable("uPauseDuration", bearPauseDuration);

	// Bind the bear texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, BearTexture);

	// Pass texture to shader
	Bear.SetUniformVariable("uTexture", 0);
	
	// Draw bear
	glPushMatrix();
		glTranslatef(1.0f, .0f, -5.0f); 
		glRotatef(200.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

		// Apply keytimed scaling
		float bearScale = BearScale.GetValue(nowTime);  // Get the position based on the current time for X
		glScalef(0.1f, 0.1f, bearScale);

		glCallList(BearDL);            
	glPopMatrix();

	// Turn off bear shader
	Bear.UnUse();

	// Turn on orange cat shader
	OrangeCat.Use();
	
	// Pass uniform variables to control the animation
	float orangeCatRunCycleTime = 0.05f;  	// Time for one complete cycle (narrow to wide to narrow)
	float orangeCatMaxBend = 0.03f;  		// Maximum bend for running
	float orangeCatTurnIntensity = 0.01f;	// Turning amount
	float orangeCatEnableTurn = 0.0f;		// Enable turning
	float orangeCatEnableRun = 1.0f;		// Enable running
	float orangeCatTurnDuration = 0.4f;		// Time for turning
	float orangeCatPauseDuration = 0.05f;	// Time for pause in turning

	OrangeCat.SetUniformVariable("uTime", Time);
	OrangeCat.SetUniformVariable("uRunCycleTime", orangeCatRunCycleTime);
	OrangeCat.SetUniformVariable("uMaxBend", orangeCatMaxBend);
	OrangeCat.SetUniformVariable("uTurnIntensity", orangeCatTurnIntensity);
	OrangeCat.SetUniformVariable("uEnableTurn", orangeCatEnableTurn);
	OrangeCat.SetUniformVariable("uEnableRun", orangeCatEnableRun);
	OrangeCat.SetUniformVariable("uTurnDuration", orangeCatTurnDuration);
	OrangeCat.SetUniformVariable("uPauseDuration", orangeCatPauseDuration);

	// Bind the cat texture 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, OrangeCatTexture);

	// Pass texture to shader
	OrangeCat.SetUniformVariable("uTexture", 0);

	float catScale = CatScale.GetValue(nowTime);

	// Orange cat running along x axis
	glPushMatrix();
		// Apply keytimed positioning
		float orangeCatPosX = OrangeCatX.GetValue(nowTime);
		glTranslatef(orangeCatPosX, 0.0f, 0.0f);

		// Orientation animation
		float orangeCatRot;
		if (nowTime < 10.0) {
			orangeCatRot = -90.0;
		} else if (nowTime < 20.0) {
			orangeCatRot = 90.0;
		} else if (nowTime < 30.0) {
			orangeCatRot = -90.0;
		} else if (nowTime < 40.0) {
			orangeCatRot = 90.0;
		}
		glRotatef(orangeCatRot, 0.0f, 1.0f, 0.0f);

		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

		// Apply keytimed scaling
		glScalef(catScale, catScale, catScale);

		// Draw cat
		glCallList(OrangeCatDL);            
	glPopMatrix();

	// Static orange cats
	for (int i = 0; i < orangeCats.size(); i++) {
		const OrangeCatPos& cat = orangeCats[i];

		// Enable turning animation
		orangeCatEnableTurn = 1.0f;
		orangeCatEnableRun = 0.0f;

		// Pass new enable animation variables to shader
		OrangeCat.SetUniformVariable("uEnableTurn", orangeCatEnableTurn);
		OrangeCat.SetUniformVariable("uEnableRun", orangeCatEnableRun);

		glPushMatrix();
			glRotatef(cat.rotationY, 0.0f, 1.0f, 0.0f);
			glTranslatef(cat.x, 0.0f, cat.z);
			glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

			// Apply keytimed scaling
			glScalef(0.1f, 0.1f, catScale);

			// Draw cat
			glCallList(OrangeCatDL);
		glPopMatrix();
	}

	// Turn off orange cat shader
	OrangeCat.UnUse();

	// Turn on black cat shader
	BlackCat.Use();

	// Pass uniform variables to control the animation
	float blackCatRunCycleTime = 0.05f;  	// Time for one complete cycle (narrow to wide to narrow)
	float blackCatMaxBend = 0.03f;  		// Maximum bend for running
	float blackCatTurnIntensity = 0.01f;	// Turning amount
	float blackCatEnableTurn = 1.0f;		// Enable turning
	float blackCatEnableRun = 0.0f;			// Enable running
	float blackCatTurnDuration = 0.5f;		// Time for turning
	float blackCatPauseDuration = 0.07f;	// Time for pause in turning

	BlackCat.SetUniformVariable("uTime", Time);
	BlackCat.SetUniformVariable("uRunCycleTime", blackCatRunCycleTime);
	BlackCat.SetUniformVariable("uMaxBend", blackCatMaxBend);
	BlackCat.SetUniformVariable("uTurnIntensity", blackCatTurnIntensity);
	BlackCat.SetUniformVariable("uEnableTurn", blackCatEnableTurn);
	BlackCat.SetUniformVariable("uEnableRun", blackCatEnableRun);
	BlackCat.SetUniformVariable("uTurnDuration", blackCatTurnDuration);
	BlackCat.SetUniformVariable("uPauseDuration", blackCatPauseDuration);

	// Bind the cat texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, BlackCatTexture);

	// Pass texture to shader
	BlackCat.SetUniformVariable("uTexture", 0);

	// Static black cats
	for (int i = 0; i < blackCats.size(); i++) {
		const BlackCatPos& cat = blackCats[i];

		glPushMatrix();
			glRotatef(cat.rotationY, 0.0f, 1.0f, 0.0f);
			glTranslatef(cat.x, 0.0f, cat.z);
			glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

			// Apply keytimed scaling
			glScalef(0.1f, 0.1f, catScale);

			// Draw cat
			glCallList(BlackCatDL);
		glPopMatrix();
	}

	// Enable running animation
	blackCatEnableTurn = 0.0f;
	blackCatEnableRun = 1.0f;

	// Pass new enable animation variables to shader
	BlackCat.SetUniformVariable("uEnableRun", blackCatEnableRun);
	BlackCat.SetUniformVariable("uEnableTurn", blackCatEnableTurn);

	// Set time offset for first running black cat
	timeOffset = 4.0f;

	// Calculate offsetted time
	float blackCat1Time = nowTime - timeOffset;
	if (blackCat1Time < 0.0f) {
		blackCat1Time += 40.0f;
	}

	// Black cat running diagnoally (pos x pos z to neg x neg z)
	glPushMatrix();
		// Apply keytimed positioning
		float blackCat1PosX = BlackCat1X.GetValue(nowTime);
		float blackCat1PosZ = BlackCat1Z.GetValue(nowTime);
		glTranslatef(blackCat1PosX, 0.0f, blackCat1PosZ);

		// Orientation animation
		float blackCat1Rot;
		if (blackCat1Time < 10.0) {
			blackCat1Rot = -135.0;
		} else if (blackCat1Time < 20.0) {
			blackCat1Rot = 45.0;
		} else if (blackCat1Time < 30.0) {
			blackCat1Rot = -135.0;
		} else if (blackCat1Time < 40.0) {
			blackCat1Rot = 45.0;
		}
		glRotatef(blackCat1Rot, 0.0f, 1.0f, 0.0f);

		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

		// Apply keytimed scaling
		glScalef(catScale, catScale, catScale);

		// Draw cat
		glCallList(BlackCatDL);            
	glPopMatrix();

	// Set time offset for second running black cat
	timeOffset = 7.0f;

	// Calculate offsetted time
	float blackCat2Time = nowTime - timeOffset;
	if (blackCat2Time < 0.0f) {
		blackCat2Time += 40.0f;
	}

	// Black cat running diagnoally (neg x pos z to pos x neg z)
	glPushMatrix();
		// Apply keytimed positioning
		float blackCat2PosX = BlackCat2X.GetValue(nowTime);
		float blackCat2PosZ = BlackCat2Z.GetValue(nowTime);
		glTranslatef(blackCat2PosX, 0.0f, blackCat2PosZ);

		// Orientation animation
		float blackCat2Rot;
		if (blackCat2Time < 10.0) {
			blackCat2Rot = -45.0;
		} else if (blackCat2Time < 20.0) {
			blackCat2Rot = 135.0;
		} else if (blackCat2Time < 30.0) {
			blackCat2Rot = -45.0;
		} else if (blackCat2Time < 40.0) {
			blackCat2Rot = 135.0;
		}
		glRotatef(blackCat2Rot, 0.0f, 1.0f, 0.0f);

		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

		// Apply keytimed scaling
		glScalef(catScale, catScale, catScale);

		// Draw cat
		glCallList(BlackCatDL);            
	glPopMatrix();

	// Turn off black cat shader
	BlackCat.UnUse();

	// Panels (walls around the edge of the grid)
	SetMaterial(1.0f, 1.0f, 1.0f, 10.0f);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Bind the wall texture
	glBindTexture(GL_TEXTURE_2D, PanelTexture); 

	// Back panel: (-25, 0, -25), (-25, 25, -25), (25, 25, -25), (25, 0, -25)
    glPushMatrix();
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-25.0f, 0.0f, -25.0f); // Bottom-left
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-25.0f, 25.0f, -25.0f); // Top-left
			glTexCoord2f(1.0f, 1.0f); glVertex3f(25.0f, 25.0f, -25.0f);  // Top-right
			glTexCoord2f(1.0f, 0.0f); glVertex3f(25.0f, 0.0f, -25.0f);  // Bottom-right
		glEnd();
    glPopMatrix();

    // Left panel: (-25, 0, 25), (-25, 25, 25), (-25, 25, -25), (-25, 0, -25)
    glPushMatrix();
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-25.0f, 0.0f, 25.0f); // Bottom-left
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-25.0f, 25.0f, 25.0f); // Top-left
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-25.0f, 25.0f, -25.0f); // Top-right
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-25.0f, 0.0f, -25.0f); // Bottom-right
		glEnd();
    glPopMatrix();

    // Right panel: (25, 0, -25), (25, 25, -25), (25, 25, 25), (25, 0, 25)
    glPushMatrix();
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(25.0f, 0.0f, -25.0f); // Bottom-left
			glTexCoord2f(0.0f, 1.0f); glVertex3f(25.0f, 25.0f, -25.0f); // Top-left
			glTexCoord2f(1.0f, 1.0f); glVertex3f(25.0f, 25.0f, 25.0f);  // Top-right
			glTexCoord2f(1.0f, 0.0f); glVertex3f(25.0f, 0.0f, 25.0f);  // Bottom-right
		glEnd();
    glPopMatrix();

    // Front panel: (-25, 0, 25), (-25, 25, 25), (25, 25, -25), (25, 0, -25)
    glPushMatrix();
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-25.0f, 0.0f, 25.0f); // Bottom-left
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-25.0f, 25.0f, 25.0f); // Top-left
			glTexCoord2f(1.0f, 1.0f); glVertex3f(25.0f, 25.0f, 25.0f); // Top-right
			glTexCoord2f(1.0f, 0.0f); glVertex3f(25.0f, 0.0f, 25.0f); // Bottom-right
		glEnd();
    glPopMatrix();

	// Disable textures and lighting
	glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
	

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( HelicopterList );
		glPopMatrix( );
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}

void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	NowProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(float) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	InitializeTreePositions();
	InitializeBushPositions();
	InitializeRockPositions();
	InitializeDeerPositions();
	InitializeOrangeCats();
	InitializeBlackCats();
	InitializePanelPositions();

	// Camera/eye keytime animation
	float radius = 24.0f;
	float angleSpeed = 2.0f * 3.14159f / 40.0f; // Full rotation in 40 seconds

	CameraX.Init();
	CameraZ.Init();

	// Calculate the X and Z positions based on time for a continuous circle
	for (float time = 0.0f; time <= 40.0f; time += 0.5f) {
		float angle = angleSpeed * time;

		CameraX.AddTimeValue(time, radius * cos(angle));
		CameraZ.AddTimeValue(time, radius * sin(angle));
	}

	// Orange cat positioning keytime aimation (running along x axis)
	OrangeCatX.Init();
	OrangeCatX.AddTimeValue(0.0, 23.0);           
	OrangeCatX.AddTimeValue(10.0, -23.0);
	OrangeCatX.AddTimeValue(20.0, 23.0);
	OrangeCatX.AddTimeValue(30.0, -23.0);
	OrangeCatX.AddTimeValue(40.0, 23.0);   

	// Black cat positioning keytime animation (running diagnoally (pos x pos z to neg x neg z))
	timeOffset = 4.0f; // Set time offset for cat

	BlackCat1X.Init();
	BlackCat1X.AddTimeValue(0.0 + timeOffset, 20.0);           
	BlackCat1X.AddTimeValue(10.0 + timeOffset, -20.0);
	BlackCat1X.AddTimeValue(20.0 + timeOffset, 20.0);
	BlackCat1X.AddTimeValue(30.0 + timeOffset, -20.0);
	BlackCat1X.AddTimeValue(40.0 + timeOffset, 20.0);    

	BlackCat1Z.Init();
	BlackCat1Z.AddTimeValue(0.0 + timeOffset, 20.0);           
	BlackCat1Z.AddTimeValue(10.0 + timeOffset, -20.0);
	BlackCat1Z.AddTimeValue(20.0 + timeOffset, 20.0);
	BlackCat1Z.AddTimeValue(30.0 + timeOffset, -20.0);
	BlackCat1Z.AddTimeValue(40.0 + timeOffset, 20.0);  

	// Black cat positioning keytime animation (running diagnoally (neg x pos z to pos x neg z))
	timeOffset = 7.0f; // Set time offset for cat

	BlackCat2X.Init();
	BlackCat2X.AddTimeValue(0.0 + timeOffset, 20.0);           
	BlackCat2X.AddTimeValue(10.0 + timeOffset, -20.0);
	BlackCat2X.AddTimeValue(20.0 + timeOffset, 20.0);
	BlackCat2X.AddTimeValue(30.0 + timeOffset, -20.0);
	BlackCat2X.AddTimeValue(40.0 + timeOffset, 20.0);    

	BlackCat2Z.Init();
	BlackCat2Z.AddTimeValue(0.0 + timeOffset, -20.0);           
	BlackCat2Z.AddTimeValue(10.0 + timeOffset, 20.0);
	BlackCat2Z.AddTimeValue(20.0 + timeOffset, -20.0);
	BlackCat2Z.AddTimeValue(30.0 + timeOffset, 20.0);
	BlackCat2Z.AddTimeValue(40.0 + timeOffset, -20.0);  

	// Bear scaling keytime animation
	BearScale.Init();
	BearScale.AddTimeValue(0.0, -0.2); 
	BearScale.AddTimeValue(24.5, -0.2);          
	BearScale.AddTimeValue(25.0, 0.15);
	BearScale.AddTimeValue(26.0, 0.1);
	BearScale.AddTimeValue(36.5, 0.1);
	BearScale.AddTimeValue(37.0, 0.15);
	BearScale.AddTimeValue(38.0, -0.2); 

	// Deer scaling keytime animation
	DeerScale.Init();
	DeerScale.AddTimeValue(0.0, 0.13); 
	DeerScale.AddTimeValue(28.0, 0.13);          
	DeerScale.AddTimeValue(28.5, 0.18);
	DeerScale.AddTimeValue(29.5, -0.2);
	DeerScale.AddTimeValue(38.0, -0.2);
	DeerScale.AddTimeValue(39.0, 0.18);
	DeerScale.AddTimeValue(39.5, 0.13); 

	// Cat scaling keytime animation
	CatScale.Init();
	CatScale.AddTimeValue(0.0, 0.065); 
	CatScale.AddTimeValue(28.0, 0.065);          
	CatScale.AddTimeValue(28.5, 0.115);
	CatScale.AddTimeValue(29.5, -0.005);
	CatScale.AddTimeValue(38.0, -0.005);
	CatScale.AddTimeValue(39.0, 0.115);
	CatScale.AddTimeValue(39.5, 0.065); 

    int width, height;

	// Load the texture for the floor
	unsigned char* floorTexture = BmpToTexture("./obj/ground.bmp", &width, &height);
	if (floorTexture == NULL) {
		fprintf(stderr, "Cannot open texture for floor\n");
		return;
	}

	glGenTextures(1, &FloorTexture);
	glBindTexture(GL_TEXTURE_2D, FloorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, floorTexture);
	free(floorTexture); // Free the texture data after loading

	// Load the texture for the tree
    unsigned char* treeTexture = BmpToTexture("./obj/22-trees_9_obj/Texture/Bark___0_edit.bmp", &width, &height);
    if (treeTexture == NULL) {
        fprintf(stderr, "Cannot open texture for tree\n");
        return;
    }

    glGenTextures(1, &TreeTexture);
    glBindTexture(GL_TEXTURE_2D, TreeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, treeTexture);
	free(treeTexture); // Free the texture data after loading

	// Load the diffuse texture for the bush
	unsigned char* bushDiffuseTexture = BmpToTexture("./obj/Matteuccia_Struthiopteris_OBJ/maps/matteuccia_struthiopteris_leaf_1_01_diffuse.bmp", &width, &height);
	if (bushDiffuseTexture == NULL) {
		fprintf(stderr, "Cannot open diffuse texture for bush\n");
		return;
	}

	glGenTextures(1, &BushDiffuseTexture);
	glBindTexture(GL_TEXTURE_2D, BushDiffuseTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bushDiffuseTexture);
	free(bushDiffuseTexture); // Free the texture data after loading

	// Set texture environment mode for the diffuse texture
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Load the specular texture for the bush
	unsigned char* bushSpecularTexture = BmpToTexture("./obj/Matteuccia_Struthiopteris_OBJ/maps/matteuccia_struthiopteris_leaf_1_02_specular.bmp", &width, &height);
	if (bushSpecularTexture == NULL) {
		fprintf(stderr, "Cannot open specular texture for bush\n");
		return;
	}

	glGenTextures(1, &BushSpecularTexture);
	glBindTexture(GL_TEXTURE_2D, BushSpecularTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bushSpecularTexture);
	free(bushSpecularTexture); // Free the texture data after loading

	// Set texture environment mode for the specular texture
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Load the texture for the rock
	unsigned char* rockTexture = BmpToTexture("./obj/moss rock 13 sketchfab/moss rock 13 (4096).bmp", &width, &height);
	if (rockTexture == NULL) {
		fprintf(stderr, "Cannot open texture for rock\n");
		return;
	}

	glGenTextures(1, &RockTexture);
	glBindTexture(GL_TEXTURE_2D, RockTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rockTexture);
	free(rockTexture); // Free the texture data after loading

	// Create deer shader program
	Deer.Init();
	bool deerValid = Deer.Create("deer.vert", "deer.frag");
	if (!deerValid) {
		fprintf(stderr, "Yuch! The Deer shader did not compile.\n");
	} else {
		fprintf(stderr, "Woo-Hoo! The Deer shader compiled.\n");
	}

	// Set the shader uniform variables
	Deer.SetUniformVariable("uKa", 0.5f);
	Deer.SetUniformVariable("uKd", 0.4f);
	Deer.SetUniformVariable("uKs", 0.1f);
	Deer.SetUniformVariable("uShininess", 20.0f);

	// Load the texture for the deer
	unsigned char* deerTexture = BmpToTexture("./obj/White-TailedDeer_V1_L2.123c4f372813-f2b8-4711-8c23-8d6c4953de32/12961_White-TailedDeer_diffuse.bmp", &width, &height);
	if (deerTexture == NULL) {
		fprintf(stderr, "Cannot open texture for deer\n");
		return;
	}

	glGenTextures(1, &DeerTexture);
	glBindTexture(GL_TEXTURE_2D, DeerTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, deerTexture);
	free(deerTexture); 

	// Create bear shader program
	Bear.Init();
	bool bearValid = Bear.Create("bear.vert", "bear.frag");
	if (!bearValid) {
		fprintf(stderr, "Yuch! The Bear shader did not compile.\n");
	} else {
		fprintf(stderr, "Woo-Hoo! The Bear shader compiled.\n");
	}

	// Set the shader uniform variables
	Bear.SetUniformVariable("uKa", 0.5f);
	Bear.SetUniformVariable("uKd", 0.4f);
	Bear.SetUniformVariable("uKs", 0.1f);
	Bear.SetUniformVariable("uShininess", 20.0f);

	// Load the texture for the bear
	unsigned char* bearTexture = BmpToTexture("./obj/Tibetan_Blue_Bear_v1_L3.123c942e6fa9-d7c1-4f52-ac2a-5aa1f6bc9dce/Tibetan_bear_diffuse.bmp", &width, &height);
	if (bearTexture == NULL) {
		fprintf(stderr, "Cannot open texture for bear\n");
		return;
	}

	glGenTextures(1, &BearTexture);
	glBindTexture(GL_TEXTURE_2D, BearTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bearTexture);
	free(bearTexture); 

	// Create orange cat shader program
	OrangeCat.Init();
	bool orangeCatValid = OrangeCat.Create("orangeCat.vert", "orangeCat.frag");
	if (!orangeCatValid) {
		fprintf(stderr, "Yuch! The Orange Cat shader did not compile.\n");
	} else {
		fprintf(stderr, "Woo-Hoo! The Orange Cat shader compiled.\n");
	}

	// Set the shader uniform variables
	OrangeCat.SetUniformVariable("uKa", 0.5f);
	OrangeCat.SetUniformVariable("uKd", 0.4f);
	OrangeCat.SetUniformVariable("uKs", 0.1f);
	OrangeCat.SetUniformVariable("uShininess", 20.0f);

	// Load the texture for the orange cat
	unsigned char* orangeCatTexture = BmpToTexture("./obj/Cat_v1_L3.123cb1b1943a-2f48-4e44-8f71-6bbe19a3ab64/Cat_diffuse_orange.bmp", &width, &height);
	if (orangeCatTexture == NULL) {
		fprintf(stderr, "Cannot open texture for orange cat\n");
		return;
	}

	glGenTextures(1, &OrangeCatTexture);
	glBindTexture(GL_TEXTURE_2D, OrangeCatTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, orangeCatTexture);
	free(orangeCatTexture); 

	// Create black cat shader program
	BlackCat.Init();
	bool blackCatValid = BlackCat.Create("blackCat.vert", "blackCat.frag");
	if (!blackCatValid) {
		fprintf(stderr, "Yuch! The Black Cat shader did not compile.\n");
	} else {
		fprintf(stderr, "Woo-Hoo! The Black Cat shader compiled.\n");
	}

	// Set the shader uniform variables
	BlackCat.SetUniformVariable("uKa", 0.5f);
	BlackCat.SetUniformVariable("uKd", 0.4f);
	BlackCat.SetUniformVariable("uKs", 0.1f);
	BlackCat.SetUniformVariable("uShininess", 20.0f);

	// Load the texture for the black cat
	unsigned char* blackCatTexture = BmpToTexture("./obj/Cat_v1_L3.123cc81ac858-7d2c-4c7e-bf80-81982996d26d/Cat_diffuse.bmp", &width, &height);
	if (blackCatTexture == NULL) {
		fprintf(stderr, "Cannot open texture for black cat\n");
		return;
	}

	glGenTextures(1, &BlackCatTexture);
	glBindTexture(GL_TEXTURE_2D, BlackCatTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, blackCatTexture);
	free(blackCatTexture); 

	// Load the texture for the floor
	unsigned char* wallTexture = BmpToTexture("./obj/forest_view.bmp", &width, &height);
	if (wallTexture == NULL) {
		fprintf(stderr, "Cannot open texture for floor\n");
		return;
	}

	glGenTextures(1, &PanelTexture);
	glBindTexture(GL_TEXTURE_2D, PanelTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, wallTexture);
	free(wallTexture);  // Free the texture data after loading

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:

}

// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow( MainWindow );

	// create the objects:

	const float TEXTURE_SCALE = 2.0f;

	// Draw grid
	GridDL = glGenLists(1);
	glNewList(GridDL, GL_COMPILE);
		SetMaterial(0.6f, 0.6f, 0.6f, 0.f);
		glNormal3f(0., 1., 0.);

		for (int i = 0; i < NZ; i++) {
			glBegin(GL_QUAD_STRIP);
			for (int j = 0; j < NX; j++) {
				// Scale the texture coordinates
				float s = TEXTURE_SCALE * (float)j / (NX - 1);
				float t1 = TEXTURE_SCALE * (float)i / (NZ - 1);
				float t2 = TEXTURE_SCALE * (float)(i + 1) / (NZ - 1);

				glTexCoord2f(s, t1);
				glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 0));

				glTexCoord2f(s, t2);
				glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 1));
			}
			glEnd();
		}
	glEndList();

	// Load tree obj file for use with vertex buffer
	LoadTreeGeometry("./obj/22-trees_9_obj/trees9.obj", "Bark___0", treeVertices, treeIndices);

    // Generate and bind a Vertex Buffer Object (VBO) for the tree
    glGenBuffers(1, &treeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glBufferData(GL_ARRAY_BUFFER, treeVertices.size() * sizeof(float), treeVertices.data(), GL_STATIC_DRAW);

    // Generate and bind an Element Buffer Object (EBO) for the tree
    glGenBuffers(1, &treeEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, treeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, treeIndices.size() * sizeof(unsigned int), treeIndices.data(), GL_STATIC_DRAW);

	printf("Tree Vertices Count: %zu\n", treeVertices.size());
	printf("Tree Indices Count: %zu\n", treeIndices.size());

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Load bush obj file for use with vertex buffer
	LoadGeometry("./obj/Matteuccia_Struthiopteris_OBJ/matteucia_struthiopteris_2.obj", bushVertices, bushIndices);

	// Generate and bind a Vertex Buffer Object (VBO) for the bush
    glGenBuffers(1, &bushVBO);
    glBindBuffer(GL_ARRAY_BUFFER, bushVBO);
    glBufferData(GL_ARRAY_BUFFER, bushVertices.size() * sizeof(float), bushVertices.data(), GL_STATIC_DRAW);

    // Generate and bind an Element Buffer Object (EBO) for the bush
    glGenBuffers(1, &bushEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bushEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bushIndices.size() * sizeof(unsigned int), bushIndices.data(), GL_STATIC_DRAW);

	printf("Bush Vertices Count: %zu\n", bushVertices.size());
	printf("Bush Indices Count: %zu\n", bushIndices.size());

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Load rock obj file for use with vertex buffer
	LoadGeometry("./obj/moss rock 13 sketchfab/moss rock 13.obj", rockVertices, rockIndices);

	// Generate and bind a Vertex Buffer Object (VBO) for the rock
    glGenBuffers(1, &rockVBO);
    glBindBuffer(GL_ARRAY_BUFFER, rockVBO);
    glBufferData(GL_ARRAY_BUFFER, rockVertices.size() * sizeof(float), rockVertices.data(), GL_STATIC_DRAW);

    // Generate and bind an Element Buffer Object (EBO) for the rock
    glGenBuffers(1, &rockEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rockEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rockIndices.size() * sizeof(unsigned int), rockIndices.data(), GL_STATIC_DRAW);

	printf("Rock Vertices Count: %zu\n", rockVertices.size());
	printf("Rock Indices Count: %zu\n", rockIndices.size());

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create deer display list
	DeerDL = glGenLists(1);
	glNewList(DeerDL, GL_COMPILE);
		LoadObjFile((char*)"./obj/White-TailedDeer_V1_L2.123c4f372813-f2b8-4711-8c23-8d6c4953de32/12961_White-Tailed_Deer_v1_l2.obj");
	glEndList();

	// Create bear display list
	BearDL = glGenLists(1);
	glNewList(BearDL, GL_COMPILE);
		LoadObjFile((char*)"./obj/Tibetan_Blue_Bear_v1_L3.123c942e6fa9-d7c1-4f52-ac2a-5aa1f6bc9dce/13576_Tibetan_Bear_v1_l3.obj"); 
	glEndList();

	// Create orange cat display list
	OrangeCatDL = glGenLists(1);
	glNewList(OrangeCatDL, GL_COMPILE);
		LoadObjFile((char*)"./obj/Cat_v1_L3.123cb1b1943a-2f48-4e44-8f71-6bbe19a3ab64/12221_Cat_v1_l3.obj"); 
	glEndList();

	// Create black cat display list
	BlackCatDL = glGenLists(1);
	glNewList(BlackCatDL, GL_COMPILE);
		LoadObjFile((char*)"./obj/Cat_v1_L3.123cc81ac858-7d2c-4c7e-bf80-81982996d26d/12222_Cat_v1_l3.obj"); 
	glEndList();

	// create the axes:
	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			NowProjection = ORTHO;
			break;
		
		// Handle freezing the scene
		case 'f':
		case 'F':
			Frozen = ! Frozen;
			if ( Frozen )
				glutIdleFunc(NULL);
			else
				glutIdleFunc(Animate);
			break;
		case '+':  // For zooming in (keyboard + key)
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// Keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case '-':  // For zooming out (keyboard - key)
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// Keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 0;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 1;
	Scale  = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
	Frozen = false;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit( float v[3] )
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
