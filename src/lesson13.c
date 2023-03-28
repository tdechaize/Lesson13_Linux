/*
=====================================================================
OpenGL Lesson 13 :  Creating Another OpenGL Window with glut on Linux
=====================================================================

  Authors Name: Jeff Molofee ( NeHe )

  This code was created by Jeff Molofee '99 (ported to Linux/GLUT by Richard Campbell '99)

  If you've found this code useful, please let me know.

  Visit me at www.demonews.com/hosted/nehe
 (email Richard Campbell at ulmont@bellsouth.net)

  Disclaimer:

  This program may crash your system or run poorly depending on your
  hardware.  The program and code contained in this archive was scanned
  for virii and has passed all test before it was put online.  If you
  use this code in project of your own, send a shout out to the author!

=====================================================================
                NeHe Productions 1999-2004
=====================================================================
*/

 /*******************************************************************
 *  Project: $(project)
 *  Function : Main program
 ***************************************************************
 *  $Author: Jeff Molofee 2000 ( NeHe )
 *  $Name:  $
 ***************************************************************
 *
 *  Copyright NeHe Production
 *
 *******************************************************************/

/**         Comments manageable by Doxygen
*
*  Modified smoothly by Thierry DECHAIZE
*
*  Paradigm : obtain one source (only one !) compatible for multiple free C Compilers
*    and provide for all users an development environment on Linux (64 bits compatible),
*    the great Code::Blocks manager (version 20.03), and don't use glaux.lib or glaux.dll.
*
*	a) gcc 11.3.0 (32 and 64 bits) version officielle : commande pour l'installation sur Linux Mint
*       -> sudo apt-get install freeglut3 libglew-dev gcc g++ mesa-common-dev build-essential libglew2.2 libglm-dev binutils libc6 libc6-dev gcc-multilib g++-multilib; option -m32 to 32 bits ; no option to 64 bits
*	b) CLANG version 14.0.6 (32 et 64 bits), adossé aux environnements gcc : commande pour l'installation sur Linux Mint
*       -> sudo apt-get install llvm clang ; options pour la compilation et l'édition de liens -> --target=i686-pc-linux-gnu (32 bits) --target=x86_64-pc-linux-gnu (64 bits)
*
*  Date : 2023/03/28
*
* \file            lesson13.c
* \author          Jeff Molofee ( NeHe ) originely, ported to Linux/glut by Richard Campbell, and after by Thierry Dechaize on Linux Mint
* \version         2.0.1.0
* \date            28 mars 2023
* \brief           Ouvre une simple fenêtre Wayland on Linux et affiche un texte coloré qui se déplace sur l'écran à l'aide d'une fonte spécifique avec OpenGL + glut.
* \details         Ce programme gére plusieurs événements : le clic sur le bouton "Fermé" de la fenêtre, la sortie par la touche ESC ou par les touches "q" ou "Q" [Quit]
* \details                                                  les touches "t" ou "T" qui active ou non le "twinkle", les touches "f" ou "F" qui active ou non le "Full Screen".
*
*
*/

#include <GL/glut.h>    // Header File For The GLUT Library
#include <GL/gl.h>	    // Header File For The OpenGL32 Library
#include <GL/glu.h>	    // Header File For The GLu32 Library
#include <GL/glx.h>     // Header file fot the glx libraries.

#define _XOPEN_SOURCE   600  // Needed because use of function usleep depend of these two define ...!!! However function usleep appear like "... warning: implicit declaration of ..."
#include <unistd.h>     // Header file for sleeping (function usleep)
#include <stdio.h>      // Header file needed by use of printf in this code
#include <string.h>     // Header file needed by use of strcmp in this code
#include <stdlib.h>     // Header file needed by use of malloc/free function in this code
#include <math.h>       // Header file for sin/cos functions.

#include "logger.h"     //  added by Thierry DECHAIZE : logger for trace and debug if needed ... only in mode Debug !!!

/* ascii code for the escape key */
#define ESCAPE 27

char *level_debug;    // added by Thierry DECHAIZE, needed in test of logging activity (only with DEBUG mode)

/* The number of our GLUT window */
int window;

/* Indicator of Full Screen */
int nFullScreen=0;

GLuint base;            // base display list for the font set.
GLfloat cnt1;           // 1st counter used to move text & for coloring.
GLfloat cnt2;           // 2nd counter used to move text & for coloring.

/**	            This function construct font of character with instructions Wayland to use in window OpenGL
*
* \brief      Fonction BuildFont : fonction chargeant une fonte de caractères à l'aide d'instruction Wayland pour un affichage attendu dans la fenêtre OpenGL
* \details    Aucun paramètre dans ce cas de figure car tout est géré directement à l'intérieur de cette fonction de construction / chargement.
* \return     GLvoid              aucun retour.
*
*/

GLvoid BuildFont(GLvoid)
{
    Display *dpy;
    XFontStruct *fontInfo;  // storage for our font.

    base = glGenLists(96);                      // storage for 96 characters.

    // load the font.  what fonts any of you have is going
    // to be system dependent, but on my system they are
    // in /usr/X11R6/lib/X11/fonts/*, with fonts.alias and
    // fonts.dir explaining what fonts the .pcf.gz files
    // are.  in any case, one of these 2 fonts should be
    // on your system...or you won't see any text.

    // get the current display.  This opens a second
    // connection to the display in the DISPLAY environment
    // value, and will be around only long enough to load
    // the font.
    dpy = XOpenDisplay(NULL); // default to DISPLAY env.

    fontInfo = XLoadQueryFont(dpy, "-adobe-helvetica-medium-r-normal--18-*-*-*-p-*-iso8859-1");
    if (fontInfo == NULL) {
	fontInfo = XLoadQueryFont(dpy, "fixed");
	if (fontInfo == NULL) {
	    printf("no X font available?\n");
	}
    }

    // after loading this font info, this would probably be the time
    // to rotate, scale, or otherwise twink your fonts.

    // start at character 32 (space), get 96 characters (a few characters past z), and
    // store them starting at base.
    glXUseXFont(fontInfo->fid, 32, 96, base);

    // free that font's info now that we've got the
    // display lists.
    XFreeFont(dpy, fontInfo);

    // close down the 2nd display connection.
    XCloseDisplay(dpy);
}

/**	            This function construct font of character with instructions Wayland to use in window OpenGL
*
* \brief      Fonction KillFont : fonction détruisant la zone mémoire dédiée à la fonte de cacactère
* \details    Aucun paramètre dans ce cas de figure car tout est géré directement à l'intérieur de cette fonction de construction / chargement.
* \return     GLvoid              aucun retour.
*
*/

GLvoid KillFont(GLvoid)                         // delete the font.
{
    glDeleteLists(base, 96);                    // delete all 96 characters.
}

/**	            This function print text on OpenGL'window
*
* \brief      Fonction glPrint: fonction affichant le texte transmis en paramètre sur la fenêtre OpenGL
* \details    Un seul paramètre : le texte devant être affiché à l'écran.
* \param	  *text		        un pointeur dur la chaîne de caractères à afficher à l'écran					*
* \return     GLvoid              aucun retour.
*
*/

GLvoid glPrint(char *text)                      // custom gl print routine.
{
    if (text == NULL) {                         // if there's no text, do nothing.
	return;
    }

    glPushAttrib(GL_LIST_BIT);                  // alert that we're about to offset the display lists with glListBase
    glListBase(base - 32);                      // sets the base character to 32.

    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text); // draws the display list text.
    glPopAttrib();                              // undoes the glPushAttrib(GL_LIST_BIT);
}

/* A general OpenGL initialization function.  Sets all of the initial parameters. */

/**	            This function initialize the basics characteristics of the scene with instructions OpenGL (background, depth, type of depth, reset of the projection matrix, ...)
*
* \brief      Fonction InitGL : fonction gerant les caractéristiques de base de la scéne lorsque avec des instructions OpenGL (arrière plan, profondeur, type de profondeur, ...)
* \details    En retour les deux paramètres des nouvelles valeurs de largeur et de hauteur de la fenêtre redimensionnée.
* \param	  Width			    la largeur de la fenêtre lors de l'initialisation					*
* \param	  Height			la hauteur de la fenêtre lors de l'initialisation					*
*
*/

void InitGL(int Width, int Height)	        // We call this right after our OpenGL window is created.
{
#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Begin function InitGL.");
#endif // defined DEBUG

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		// This Will Clear The Background Color To Black
  glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
  glDepthFunc(GL_LESS);				// The Type Of Depth Test To Do
  glEnable(GL_DEPTH_TEST);			// Enables Depth Testing
  glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();				// Reset The Projection Matrix

  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window

  glMatrixMode(GL_MODELVIEW);

  BuildFont();

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"End function InitGL.");
#endif // defined DEBUG
}

/* The function called when our window is resized  */

/**	            This function manage the new dimension of the scene when resize of windows with instructions OpenGL
*
* \brief      Fonction ReSizeGLScene : fonction gerant les nouvelles dimensions de la scéne lorsque l'utilisateur agit sur un redimensionnement de la fenêtre
* \details    En retour les deux paramètres des nouvelles valeurs de largeur et de hauteur de la fenêtre redimensionnée.
* \param	  Width			    la  nouvelle largeur de la fenêtre redimensionnée					*
* \param	  Height			la  nouvelle hauteur de la fenêtre redimensionnée					*
*
*/

void ReSizeGLScene(int Width, int Height)
{
#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Begin function ResizeGLScene.");
#endif // defined DEBUG

  if (Height==0)				// Prevent A Divide By Zero If The Window Is Too Small
    Height=1;

  glViewport(0, 0, Width, Height);		// Reset The Current Viewport And Perspective Transformation

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);
  glMatrixMode(GL_MODELVIEW);

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"End function ResizeGLScene.");
#endif // defined DEBUG
}

/* The main drawing function. */

/**	            This function generate the scene with instructions OpenGL
*
* \brief      Fonction DrawGLScene : fonction generant l'affichage attendu avec des instructions OpenGL
* \details    Aucun paramètre dans ce cas de figure car tout est géré directement à l'intérieur de cette fonction d'affichage.
*
*/

void DrawGLScene()
{
#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Begin function DrawGLScene.");
#endif // defined DEBUG

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
  glLoadIdentity();				// Reset The View
  glTranslatef(0.0f, 0.0f, -1.0f);              // move 1 unit into the screen.

  // Pulsing Colors Based On Text Position
  glColor3f(1.0f*((float)(cos(cnt1))),1.0f*((float)(sin(cnt2))),1.0f-0.5f*((float)(cos(cnt1+cnt2))));

  // Position The Text On The Screen
  glRasterPos2f(-0.2f+0.35f*((float)(cos(cnt1))), 0.35f*((float)(sin(cnt2))));

  glPrint("OpenGL With NeHe");                  // print gl text to the screen.

  cnt1 += 0.01f;
  cnt2 += 0.0081f;

  // since this is double buffered, swap the buffers to display what just got drawn.
  glutSwapBuffers();

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"End function DrawGLScene.");
#endif // defined DEBUG
}

/* The function called whenever a key is pressed. */

/**	            This function inform caller with key pressed.
*
* \brief      Fonction keyPressed : fonction permettant de connaître quelle touche a été tapée au clavier
* \details    Le premier paarmètre en retour de cette fonction correspond à la touche précédemment appuyée.
* \param	  key			    la touche appuyée 					*
* \param	  x             	une position x au niveau du clavier *
* \param	  y             	une position y au niveau du clavier *
* \return     void              aucun retour.
*
*/

void keyPressed(unsigned char key, int x, int y)
{
    /* avoid thrashing this procedure */
    usleep(100);

    switch (key) {

	//Mode plein écran : il suffit de taper au clavier sur la touche F majuscule ou f minuscule en mode flip-flop
		case 'f' :
		case 'F' :
			if (nFullScreen==0)
			{
				glutFullScreen();
				nFullScreen=1;
				break;
			}
			if (nFullScreen==1)
			{
		        glutReshapeWindow(640, 480);
				glutPositionWindow (0, 0);
				nFullScreen=0;
				break;
			}
	//Quitter
		case 'q' :
		case 'Q' :
		case ESCAPE  : // Touche ESC : il s'agit de sortir proprement de ce programme
            glutDestroyWindow(window);
			exit (0);
			break;

        default:
            printf ("Key %d pressed. No action there yet.\n", key);
            break;
    }
}

/* Main function : GLUT runs as a console application starting with program call main()  */

/**         Comments manageable by Doxygen
*
* \brief      Programme Main obligatoire pour les programmes sous Linux (OpenGL en mode console).
* \details    Programme principal de lancement de l'application qui appelle ensuite toutes les fonctions utiles OpenGL et surtout glut.
* \param      argc         le nombre de paramètres de la ligne de commande.
* \param      argv         un pointeur sur le tableau des différents paramètres de la ligne de commande.
* \return     int          un entier permettant de connaître la statut du lancement du programme.
*
*/

int main(int argc, char **argv)
{

   if (getenv("LEVEL")) {                 // LEVEL is set
       level_debug=getenv("LEVEL");           // Added by Thierry DECHAIZE : récupérer la valeur de la variable d'environnement LEVEL si elle existe
       }
    else {
       snprintf(level_debug,2,"%s"," ");
    }

#ifdef DEBUG
    printf("Niveau de trace : %s.\n",level_debug);

    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Enter within main, before call of glutInit.");
#endif // defined DEBUG

  /* Initialize GLUT state - glut will take any command line arguments that pertain to it or
     X Windows - look at its documentation at http://reality.sgi.com/mjk/spec3/spec3.html */

/**	            This Code initialize the context of windows on Wayland with glut.
*
* \brief      Appel de la fonction glutInit : fonction Glut d'initialisation
* \details    En entrée de cette fonction, les paramètres de la ligne de commande.
* \param	  argc			    le nombre de paramètres mis à disposition			*
* \param	  argv              Hle tableau des différents paramètres mis à disposition *
*
*/

  glutInit(&argc, argv);

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Next step within main, before call of glutInitDisplayMode.");
#endif // defined DEBUG

  /* Select type of Display mode:
     Double buffer
     RGBA color
     Alpha components supported
     Depth buffer */

/**	            This Code initialize le mode d'affichage défini avec une fonction glut.
*
* \brief      Appel de la fonction glutInitDisplayMode(: fonction Glut initialisant le mode d'affichage.
* \details    En entrée de cette fonction, des paramètres séparés par des "ou logique".
* \param	  const         une succession de paramètres séparés par des "ou logique".
*
*/

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Next step within main, before call of glutInitWindowSize");
#endif // defined DEBUG

  /* get a 640 x 480 window */

/**	            This Code initialize the dimensions (width & height) of the window into screen.
*
* \brief      Appel de la fonction glutInitWindowSize : fonction Glut initialisant la position de la fenêtre dans l'écran.
* \details    En entrée de cette fonction, les deux paramètres X et Y correspondant à la taille de la fenêtre dans l'écran (deux dimensions)
* \param	  X			    la largeur de la fenêtre en x
*
*/

  glutInitWindowSize(640, 480);

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Next step within main, before call of glutInitWindowPosition");
#endif // defined DEBUG

  /* the window starts at the upper left corner of the screen */

/**	            This Code initialize the position of the window into screen.
*
* \brief      Appel de la fonction glutInitWindowPosition : fonction Glut initialisant la position de la fenêtre dans l'écran.
* \details    En entrée de cette fonction, les deux paramètres X et Y de positionnement de la fenêtre dans l'écran (deux dimensions)
* \param	  X			    le positionnement de la fenêtre en x
* \param	  Y			    le positionnement de la fenêtre en y
*
*/

  glutInitWindowPosition(0, 0);

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Next step within main, before call of glutCreateWindow");
#endif // defined DEBUG

  /* Open a window */

/**	            This Code create windows on Wayland with glut.
*
* \brief      Appel de la fonction glutCreateWindow : fonction Glut créant une fenêtre Wayland avec glut.
* \details    En entrée de cette fonction, l'identification de la fenêtre (.id. son nom).
* \param	  tittle			le nom de la fenêtre
* \return     int               l'identifiant entier de la fenêtre créee.
*
*/

  window = glutCreateWindow("Jeff Molofee's GL Tutorial : draw mooving colored text with special font - NeHe '99");

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Next step within main, before call of glutDisplayFunc");
#endif // defined DEBUG

  /* Register the function to do all our OpenGL drawing. */

/**	            This Code rely the internal function DrawGLScene at the display function of glut.
*
* \brief      Appel de la fonction glutDisplayFunc : fonction Glut permettant d'activer la fonction interne d'affichage.
* \details    En entrée de cette fonction, l'adresse de la fonction interne appellée .
* \param	  &function			l'adresse de la fonction interne d'affichage.
*
*/

  glutDisplayFunc(&DrawGLScene);

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Next step within main, before call of glutIdleFunc");
#endif // defined DEBUG

  /* Even if there are no events, redraw our gl scene. */

/**	            This Code rely the internal function DrawGLScene at the Idle Function of glut.
*
* \brief      Appel de la fonction glutIdleFunc : fonction d'attente de Glut permettant d'activer la fonction interne d'affichage.
* \details    En entrée de cette fonction, l'adresse de la fonction interne appellée .
* \param	  &function			l'adresse de la fonction interne d'affichage.
*
*/

  glutIdleFunc(&DrawGLScene);

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Next step within main, before call of glutReshapeFunc");
#endif // defined DEBUG

  /* Register the function called when our window is resized. */

/**	            This Code rely the internal function ResizeGLScene at the reshape function of glut.
*
* \brief      Appel de la fonction glutReshapeFunc : fonction Glut permettant d'activer la fonction interne de changement des dimensions d'affichage.
* \details    En entrée de cette fonction, l'adresse de la fonction interne appellée.
* \param	  &ResizeGLScene	l'adresse de la fonction interne traitant des changements de dimension de l'affichage.
*
*/

  glutReshapeFunc(&ReSizeGLScene);

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Next step within main, before call of glutKeyboardFunc");
#endif // defined DEBUG

  /* Register the function called when the keyboard is pressed. */

/**	            This Code rely the internal function keyPressed at the keyboard function of glut (normal touchs).
*
* \brief      Appel de la fonction glutKeyboardFunc : fonction Glut permettant de recupérer la touche appuyée sur le clavier (touches normales).
* \details    En entrée de cette fonction, l'adresse de la fonction interne gérant les appuis sur le clavier.
* \param	  &keyPressed			l'adresse de la fonction interne gérant les appuis sur le clavier (touches normales).
*
*/

  glutKeyboardFunc(&keyPressed);

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Next step within main, before call of InitGL");
#endif // defined DEBUG

  /* Initialize our window. */

/**	            This Code initialize les paramètres d'affichage OpenGL.
*
* \brief      Appel de la fonction InitGL : fonction d'initialisation de la taille de la fenêtre d'affichage OpenGL (la même que celle de glut).
* \details    En entrée de cette fonction, les deux paramètres X et Y correspondant à la taille de la fenêtre OpenGL dans l'écran (deux dimensions)
* \param	  X			    la largeur de la fenêtre en x
* \param	  Y			    la hauteur de la fenêtre en y
*
*/

  InitGL(640, 480);

#ifdef DEBUG
    if (strcmp(level_debug,"BASE") == 0 || strcmp(level_debug,"FULL") == 0)
        log_print(__FILE__, __LINE__,"Next step within main, before call of glutMainLoop");
#endif // defined DEBUG

  /* Start Event Processing Engine */

/**	            This Code run the permanently wait loop of events.
*
* \brief      Appel de la fonction glutMainLoop : fonction lancant la boucle d'attente des événements sous glut.
* \details    Aucun paramètre en entrée ni en sortie.
*
*/

  glutMainLoop();

  return 1;
}

