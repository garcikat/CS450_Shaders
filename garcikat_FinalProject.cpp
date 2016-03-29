#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include "glew.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"


// title of these windows:
const char *WINDOWTITLE = { "Flower in the Wind -- Katelynn Garcia" };
const char *GLUITITLE   = { "User Interface Window" };


// what the glui package defines as true and false:
const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };

// the escape key:
#define ESCAPE		0x1b

// initial window size:
const int INIT_WINDOW_SIZE = { 600 };

// multiplication factors for input interaction:
//  (these are known from previous experience)
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

// minimum allowable scale factor:
const float MINSCALE = { 0.05f };

// active mouse buttons (or them together):
const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };

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


// window background color (rgba):
const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:
const GLfloat AXES_WIDTH   = { .5 };

struct Point
{
        float x0, y0, z0;       // initial coordinates
        float x,  y,  z;        // animated coordinates
};

struct Curve
{
        float r, g, b;
        Point p0, p1, p2, p3;
};

const int MS_PER_CYCLE = 20000.; // 20 second wind animation cycle

// non-constant global variables:
int		petalsize = 1.;
float	windArray[3][5];
int		gustEndTime, gustNum;
float	Time;
Curve	c1;
Point	p0, p1, p2, p3;
float	theta1, theta2, theta3;
int		bendLeft;
int		ActiveButton;			// current button that is down
GLuint	AxesList;
GLuint	PetalList;
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:
void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
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

void	Axes( float );
void	HsvRgb( float[3], float [3] );
void	DrawCurve(Curve c);
void	DrawCurveVerts(Curve c);
void	RotateVerts( Curve c, float deg);
float	Randfloat( float, float );
int		Randint(int, int);


// main program:
int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)
	glutInit( &argc, argv );

	// setup all the graphics stuff:
	InitGraphics( );

	// create the display structures that will not change:
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

	// this is here to make the compiler happy:
	return 0;
}

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:
	// force a call to Display( ) next time it is convenient:

	//int ms = glutGet( GLUT_ELAPSED_TIME );
	//ms %= MS_PER_CYCLE; 
	//Time = (float)ms / (float)( MS_PER_CYCLE - 1 );

	/*if( Time == windArray[gustNum][0] ) //check if current cycle iteration is equal to a gust start time
	{
		gustEndTime = glutGet( GLUT_ELAPSED_TIME ) + windArray[gustNum][1]*1000;
		gustNum++;
		bendLeft = 1;
		//set intensity
	}

	if( glutGet( GLUT_ELAPSED_TIME ) == gustEndTime)
	{
		gustEndTime == -1;
		bendLeft = 0;
	}
	*/

	int randNum = Randint(0, 100);
	if( randNum == 0 )
	{
		bendLeft = !bendLeft;
	}

	if( c1.p3.y <= 0.)
		bendLeft = !bendLeft;
	
	if(bendLeft)
	//if(gustEndTime == -1)
	{
		theta1 += .25/10;
		theta2 += .5/10;
		theta3 += 1./10;
	}
	if(!bendLeft && theta3 >= 90.)
	{
		theta1 -= .25/10;
		theta2 -= .5/10;
		theta3 -= 1./10;
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );


	// specify shading to be flat:
	glShadeModel( GL_FLAT );

	// set the viewport to a square centered in the window:
	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );

	// place the objects into the scene:
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:
	gluLookAt( 0., 2.5, 3.,     0., 0., 0.,     0., 1., 0. );

	// rotate the scene:
	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );

	// uniformly scale the scene:
	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// possibly draw the axes:
	if( AxesOn != 0 )
	{
		//glColor3fv( &Colors[WhichColor][0] );
		glColor3f(1., 1., 1.);
		glCallList( AxesList );
	}
	
	// since we are using glScalef( ), be sure normals get unitized:
	glEnable( GL_NORMALIZE );

	// draw the current object:

	c1.p1.x = .5*cos(theta1*(M_PI/180.));
	c1.p1.y = .5*sin(theta1*(M_PI/180.));

	c1.p2.x = 1.*cos((theta2)*(M_PI/180.));
	c1.p2.y = 1.*sin((theta2)*(M_PI/180.));

	c1.p3.x = 1.5*cos((theta3)*(M_PI/180.));
	c1.p3.y = 1.5*sin((theta3)*(M_PI/180.));

	glPushMatrix();
		glColor3f(.86, .78, .19);
		glTranslatef( c1.p3.x, c1.p3.y, c1.p3.z);
		glutSolidSphere(0.05, 20, 20);
	glPopMatrix();

	DrawCurve( c1 );
	DrawCurveVerts( c1 );
	int petalheight = petalsize*sin(70.*(M_PI/180));
	glPushMatrix();
	glTranslatef( c1.p3.x, c1.p3.y, c1.p3.z );
	glRotatef( theta3-90., 0., 0., 1. );
	glBegin( GL_TRIANGLES );
			glColor3f(.86, .50, .66);

			glNormal3f( 0., 1., 0. );
			//back
			glVertex3f( 0., -0.05, 0.);
			glVertex3f( -petalsize*cos(70.*(M_PI/180)), 0.25, petalsize*sin(70.*(M_PI/180)) );
			glVertex3f( petalsize*cos(70.*(M_PI/180)), 0.25, petalsize*sin(70.*(M_PI/180)) );
			//front
			glVertex3f( 0.,-0.05, 0. );
			glVertex3f( -petalsize*cos(70.*(M_PI/180)), 0.25, -petalsize*sin(70.*(M_PI/180)) );
			glVertex3f( petalsize*cos(70.*(M_PI/180)), 0.25, -petalsize*sin(70.*(M_PI/180)) );
			//left
			glVertex3f( 0., -0.05, 0. );
			glVertex3f( -petalsize*sin(70.*(M_PI/180)), 0.25, petalsize*cos(70.*(M_PI/180)) );
			glVertex3f( -petalsize*sin(70.*(M_PI/180)), 0.25, -petalsize*cos(70.*(M_PI/180)) );
			//right
			glVertex3f( 0., -0.05, 0. );
			glVertex3f( petalsize*sin(70.*(M_PI/180)), 0.25, petalsize*cos(70.*(M_PI/180)) );
			glVertex3f(	petalsize*sin(70.*(M_PI/180)), 0.25, -petalsize*cos(70.*(M_PI/180)) );

	glEnd();
	glPopMatrix();

	//glCallList( PetalList );

	
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
DoDebugMenu( int id )
{
	DebugOn = id;

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
	WhichProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
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
	glutSetWindow( MainWindow );

	int axesmenu = glutCreateMenu( DoAxesMenu );
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
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:
	glutAttachMenu( GLUT_RIGHT_BUTTON );
}

// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
void
InitGraphics( )
{
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
	glutPassiveMotionFunc( NULL );
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
	glutIdleFunc( Animate );

	// init glew (a window must be open to do this):

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

	Point p0 = { 0., 0., 0., 	0., 0., 0.};
	Point p1 = { 0., .5, 0.,	0., 5., 0.};
	Point p2 = { 0., 1., 0.,	0., 1., 0.};
	Point p3 = { 0., 1.5,0.,	0., 1.5, 0.};

	c1.p0 = p0;
	c1.p1 = p1;
	c1.p2 = p2;
	c1.p3 = p3;
	c1.r = 0.;
	c1.g = 0.78;
	c1.b = 0.42;

	/*int windIntervals = (MS_PER_CYCLE/1000)/5;
	float prevtime = 0;
	for ( int i = 0; i < 5; i++)
	{
		//gust time
		windArray[i][0] = Randfloat(0., 5.) + prevtime;
		prevtime = windArray[i][0];
		windArray[i][0] /= (MS_PER_CYCLE/1000); //decimal percentage of cycle progression

		//gust duration
		windArray[i][1] = Randfloat(.25, 1.5);

		//gust intensity
		windArray[i][2] = Randfloat( 1.5, 5.);
	}*/


}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	glutSetWindow( MainWindow );

	// create the object:
	PetalList = glGenLists( 1 );
	glNewList( PetalList, GL_COMPILE );
		glBegin( GL_TRIANGLES );
			glColor3f(.86, .50, .66);

			glNormal3f( 0., 1., 0. );
			//back
			glVertex3f( 0., -0.05, 0.);
			glVertex3f( -petalsize*cos(70.*(M_PI/180)), 0.25, petalsize*sin(70.*(M_PI/180)) );
			glVertex3f( petalsize*cos(70.*(M_PI/180)), 0.25, petalsize*sin(70.*(M_PI/180)) );
			//front
			glVertex3f( 0.,-0.05, 0. );
			glVertex3f( -petalsize*cos(70.*(M_PI/180)), 0.25, -petalsize*sin(70.*(M_PI/180)) );
			glVertex3f( petalsize*cos(70.*(M_PI/180)), 0.25, -petalsize*sin(70.*(M_PI/180)) );
			//left
			glVertex3f( 0., -0.05, 0. );
			glVertex3f( -petalsize*sin(70.*(M_PI/180)), 0.25, petalsize*cos(70.*(M_PI/180)) );
			glVertex3f( -petalsize*sin(70.*(M_PI/180)), 0.25, -petalsize*cos(70.*(M_PI/180)) );
			//right
			glVertex3f( 0., -0.05, 0. );
			glVertex3f( petalsize*sin(70.*(M_PI/180)), 1.5*theta3, petalsize*cos(70.*(M_PI/180)) );
			glVertex3f(	petalsize*sin(70.*(M_PI/180)), 1.5*theta3, -petalsize*cos(70.*(M_PI/180)) );
		
	glEnd();

		glEnd();
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
			WhichProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
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
}


// called when the mouse moves while a button is down:
void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


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
	AxesOn = 1;
	DebugOn = 0;
	Scale  = 1.0;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	
	theta1 = 90.;
	theta2 = 90.;
	theta3 = 90.;
	bendLeft = 1.;
	
	//gustNum = 0;
	//gustEndTime = -1;
}


// called when user resizes the window:
void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display( )
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
static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

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
	
	float i = floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r, g, b;			// red, green, blue
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
DrawCurve( Curve c)
{
	int NUMPOINTS = 25;
	glLineWidth( 3. );
	glColor3f( c.r, c.g, c.b );
	glBegin( GL_LINE_STRIP );
		for( int it = 0; it <= NUMPOINTS; it++ )
		{
			float t = (float)it / (float)NUMPOINTS;
			float omt = 1.f - t;
			float x = omt*omt*omt*c.p0.x + 3.f*t*omt*omt*c.p1.x + 3.f*t*t*omt*c.p2.x + t*t*t*c.p3.x;
			float y = omt*omt*omt*c.p0.y + 3.f*t*omt*omt*c.p1.y + 3.f*t*t*omt*c.p2.y + t*t*t*c.p3.y;
			float z = omt*omt*omt*c.p0.z + 3.f*t*omt*omt*c.p1.z + 3.f*t*t*omt*c.p2.z + t*t*t*c.p3.z;
			glVertex3f( x, y, z );
		}
	glEnd( );
	glLineWidth( 1. );
}

void 
DrawCurveVerts( Curve c )
{
	glPointSize( 4. );
	glColor3f( 1., 1., 1. );
	glBegin( GL_POINTS );
		glVertex3f( c.p0.x, c.p0.y, c.p0.z );
		glVertex3f( c.p1.x, c.p1.y, c.p1.z );
		glVertex3f( c.p2.x, c.p2.y, c.p2.z );
		glVertex3f( c.p3.x, c.p3.y, c.p3.z );
	glEnd();
	glPointSize(1. );

}

void
RotateVerts( Curve c, float deg ) // deg measured from the vertical
{
	c.p1.x += cos(90+deg)*c.p1.y;
	c.p1.y += sin(90+deg)*c.p1.y;

	c.p2.x += cos(90+deg)*c.p2.y;
	c.p2.y += sin(90+deg)*c.p2.y;

	c.p3.x += cos(90+deg)*c.p3.y;
	c.p3.y += sin(90+deg)*c.p3.y;
}

float
Randfloat( float low, float high )
{
	float r = (float) rand();
	float t = r/ RAND_MAX;

	return	low + t*(high - low);
}

int
Randint( int ilow, int ihigh)
{	
	float low = (float) ilow;
	float high = ceil((float) ihigh);

	return (int) Randfloat(low, high);
}