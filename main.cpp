/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
| 3D Camera control
|
| Description: This program loads and draws a textured 3D model and allows for camera control with mouse and keyboard.
|   It also allows the user to quit the program using the 'q' or 'Esc' keys.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//#define DEBUG_CODE			// Define when debugging

// This is needed to use fopen()
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <GL/glut.h>      // Window and event handling
#include <iostream>				// Input/Output for console
#include <string>					// String handling
#include <stdio.h>	  		// C Standard Library
#include <stdlib.h>
#include <math.h>
#include "math3d.h"
#include "ReadOBJFile.h"
using namespace std;

// Function prototypes
GLenum errorCheck(string function);
void keyboard(unsigned char,int,int);
void keyboard_up(unsigned char, int,int);
void keyboardSpecial(int,int,int);
void mouseMove(int x,int y);
void init();
void loadModels();
void cleanup();
void render();
void update();
void model3D_draw(Object3D *o);
void model3D_drawFast(Object3D *o);
void modelTex3D_drawFast(Object3D *o, GLuint texture_id, unsigned char *texture_data);

bool loadBMPfile(char *filename,int *width,int *height,unsigned char **data);

// List the static OpenGL libraries to link into this application
#pragma comment (lib, "glut32.lib")
#pragma comment (lib, "glew32.lib")

#ifndef DEBUG_CODE
// Hide the console text window - optional
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")		
#endif

// The size of the view window
#define VIEW_WIDTH  700
#define VIEW_HEIGHT 700

// Global variables (can be used by all functions in this file)
int wireframe = 0;    // 0=off, 1=on
int polygonshade = 1; // 0=flat shading, 1=smooth shading
int lighton = 1;      // 0=off, 1=on

// 3D models
Object3D *obj_teapot = 0;
GLuint texture_id = -1;           // -1 means not loaded
unsigned char *texture_data = 0;  // 0 means not loaded

//overlay
Object3D *obj_overlay = 0;
GLuint texture_overlay_id = -1;           // -1 means not loaded
unsigned char *texture_overlay_data = 0;  // 0 means not loaded

// Current mouse position
int mouse_x,mouse_y;

// Movement commands (to move the camera)
bool move_forward = false;
bool move_back = false;
bool move_left = false;
bool move_right = false;

// Camera rotation
float current_xrotate = 0;    // current rotation about x-axis
float current_yrotate = 0;    // current rotation about y-axis

// Camera position
Vector3D camera_position;
// Camera heading (which way the camera is pointing, in world space)
Vector3D start_heading = {0, 0, -1}; // starting pointing down the negative z axis
Vector3D camera_heading;
// Camera up vector (which way is up for the camera)
Vector3D start_up = {0, 1, 0};  // starts with looking up positive y axis
Vector3D camera_up;

/*************************************************************************************
| Function: main
|
| Description: This function will consist mostly of calls to GLUT functions to set up
| the window. It initializes the window, draws geometry, and runs the program's main
| loop. It also monitors keyboard events.
*************************************************************************************/
int main(int argc, char **argv) {

	glutInit(&argc, argv);										                  // Initialize GLUT  
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_SINGLE);	  // Set up display buffer (single buffer and z-buffer (depth buffer) with RGB color mode)  
	glutInitWindowSize(VIEW_WIDTH,VIEW_WIDTH);								  // Set the width and height of the window  
	glutInitWindowPosition(700, 400);							              // Set the position of the window on the screen 
	glutCreateWindow("Hunter Jones");							        // Set title and create the window  

	init();														                          // Call init() to initialize display 
	glutKeyboardFunc(keyboard);									                // Tell GLUT to use the keyboard() function to handle ASCII key presses
  glutKeyboardUpFunc(keyboard_up);            
  glutSpecialFunc(keyboardSpecial);							              // Tell GLUT to use the keyboardSpecial() function to handle special key presses
  glutPassiveMotionFunc(mouseMove);                           // Tell GLUT a function to call if the mouse moves
  glutDisplayFunc(render);									                  // Tell GLUT to use the render() function for rendering 
	errorCheck("main");

	glutMainLoop();												                      // Enter GLUT's main loop 

  cleanup();
}

/*************************************************************************************
| Function: errorCheck
|
| Description: Checks for internally stored error codes and informs the user of any
| errors by printing a message to the screen. Note, however, that the only errors this
| function can detect are those that occur in calls to base library routines or GLU
| functions.
| Input: The name of the function that calls errorCheck.
| Output: The GLenum containing the error code.
*************************************************************************************/
GLenum errorCheck(string function) {

  GLenum code;												        // Stores the error code
  const GLubyte *errorString;									// Stores error string

  code = glGetError();										    // Check for stored error codes

#ifdef DEBUG_CODE											
  if(code != GL_NO_ERROR) {									  // Error detected
    errorString = gluErrorString(code);				// Store error string and print it
    cout << "OpenGL error in function '" << function
      << "' : " << errorString << endl;

    if(errorString == NULL)								    // If error is not a valid error, gluErrorString returns null
      cout << "Unidentified/invalid "
      << "GL or GLU error code " << endl;
  }
  else
    cout << "Function '" << function << "' execution successful." << endl;
#endif

  return code;
}

/*************************************************************************************
| Function: keyboard
|
| Description: This is a keyboard callback function that controls what happens when
| an ASCII key is pressed (a key with a corresponding ASCII code, such as a letter key).
| Input: The key (ASCII character) and the location of the mouse at the time the key
| was pressed. The coordinates of the mouse are in window coordinates.
*************************************************************************************/
void keyboard(unsigned char key,int x,int y) {

  if(key == 'q' || key == 'Q' || key == 27)		// Exit program if 'Esc', 'q', or 'Q' is pressed
    exit(0);
  else if(key == 'w' || key == 'W')
    move_forward = true;
  else if(key == 's' || key == 'S')
    move_back = true;
  else if(key == 'a' || key == 'A')
    move_left = true;
  else if(key == 'd' || key == 'D')
    move_right = true;

  errorCheck("keyboard");
}

/*************************************************************************************
| Function: keyboard_up
*************************************************************************************/
void keyboard_up(unsigned char key,int x,int y) {

  if(key == 'w' || key == 'W')
    move_forward = false;
  else if(key == 's' || key == 'S')
    move_back = false;
  else if(key == 'a' || key == 'A')
    move_left = false;
  else if(key == 'd' || key == 'D')
    move_right = false;

  errorCheck("keyboard_up");
}


/*************************************************************************************
| Function: keyboardSpecial
|
| Description: This is a keyboard callback function that controls what happens when
| a special key is pressed (one other than those keys with corresponding ASCII codes,
| e.g. the function or arrow keys).
| Input: The key (GLUT_KEY_* constant) and the location of the mouse at the time the
| key was pressed. The coordinates of the mouse are in window coordinates.
*************************************************************************************/
void keyboardSpecial (int key,int x,int y) {

  // Are any arrow keys being pressed?
  if (key == GLUT_KEY_RIGHT || key == GLUT_KEY_LEFT || key == GLUT_KEY_UP || key == GLUT_KEY_DOWN) {		

    // ADD CODE HERE: Change something in the program based on this key press
    // glutPostRedisplay();				// This function sets a flag in GLUT's main loop which redraws the screen
  }															

  errorCheck("keyboardSpecial");
}

/*************************************************************************************
| Function: mouseMove
|
| Description: This is a keyboard callback function that is called if the mouse is moved.
*************************************************************************************/
void mouseMove(int x,int y)
{
  // Store the new mouse position
  mouse_x = x;
  mouse_y = y;
}

/*************************************************************************************
| Function: init
|
| Description: In this function, we set state variables concerned with attributes and
| viewing, which are things that we want to set once. The function initializes the
| display by setting background color and display mode.
*************************************************************************************/
void init() {

  glClearColor(0.0,0.0,0.0,1.0);  // Assign the background color of our window (with color black)  

  glMatrixMode(GL_PROJECTION);		// Set the display mode as projection
  glLoadIdentity();							  // Load the identity matrix								  
  gluPerspective(100,1,0.1,1000); // Set perspective projection (with 100 field of view in Y dimension, near plane at 0.1, far plane at 1000)
  glEnable(GL_DEPTH_TEST);				// Enable the z-buffer algorithm (a visible-surface algorithm)                           
  glEnable(GL_CULL_FACE);         // Enable backface culling (dont draw backfaces)
  glCullFace(GL_BACK);
  //glFrontFace(GL_CCW);          // shouldn't be necessary to set since CCW is the default

  // Load 3D models
  loadModels ();             

  errorCheck("init");
}

/*************************************************************************************
| Function: loadModels
|
| Description: Load any 3D models needed.
*************************************************************************************/
void loadModels() {

  // Load a model
  bool load_texcoords = true;
  bool smooth_discontinuous_vertices = true;
  ReadOBJFile ("romanshield.obj",&obj_teapot,load_texcoords,smooth_discontinuous_vertices);

  // Load a texture
  int width,height;
  if (loadBMPfile("romantexture.bmp",&width,&height,&texture_data)) {
    // Create an OpenGL texture
    glGenTextures(1,&texture_id);
    // Bind the newly created texture - all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D,texture_id);
    // Pass the image data to OpenGL
    //glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_BGR,GL_UNSIGNED_BYTE,data);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,texture_data);
    // Define how the texture will be sampled
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  }

  // Load a model
  ReadOBJFile("overlay.obj", &obj_overlay, load_texcoords, false);
  // Load a texture
  if (loadBMPfile("overlay.bmp", &width, &height, &texture_overlay_data)) {
	  // Create an OpenGL texture
	  glGenTextures(1, &texture_overlay_id);
	  // Bind the newly created texture - all future texture functions will modify this texture
	  glBindTexture(GL_TEXTURE_2D, texture_overlay_id);
	  // Pass the image data to OpenGL
	  //glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_BGR,GL_UNSIGNED_BYTE,data);
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_overlay_data);
	  // Define how the texture will be sampled
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

}

/*************************************************************************************
| Function: cleanup
|
| Description: Frees any memory allocated by this program.
*************************************************************************************/
void cleanup() {

  FreeObject (obj_teapot);
  obj_teapot = 0;
  // Free the data buffer created by loadBMPfile()
  free(texture_data);

  FreeObject(obj_overlay);
  obj_overlay = 0;
  free(texture_overlay_data);
}

/*************************************************************************************
| Function: render
|
| Description: This is a display callback function that is called whenever the display
| window needs to be redisplayed. It clears the window and does any drawing.
*************************************************************************************/
void render() {
  GLfloat light0_position[] = {100, 150, -100,1.0};       // Position vector for light0 (first light)
  GLfloat light0_ambient[]  = {0.0, 0.0, 0.0, 1.0};       // Array with ambient values for light0 - can be black (0) or very dim 10% lighting (0.1), etc.
  GLfloat light0_diffuse[]  = {1.0, 1.0, 1.0, 1.0};		    // Array with diffuse values for light0
  GLfloat light0_specular[] = {0.0, 0.0, 0.0, 0.0};		    // Array with specular values for light0

  update();   // Process user input

  if (lighton) {
    // Enable lighting
    glEnable(GL_LIGHTING);										            // Enable lighting for the scene
                                                          // Enable one light (light0) as a point light source
    glEnable(GL_LIGHT0);										              // Enable point light source, light0
    glLightfv(GL_LIGHT0,GL_POSITION,light0_position);			// Set position values for light0
    glLightfv(GL_LIGHT0,GL_AMBIENT,light0_ambient);			  // Set ambient values for light0
    glLightfv(GL_LIGHT0,GL_DIFFUSE,light0_diffuse);			  // Set diffuse values for light0
    glLightfv(GL_LIGHT0,GL_SPECULAR,light0_specular);			// Set specular values for light0

    glEnable(GL_COLOR_MATERIAL);								          // Enable colors for the pyramid
  }
  else
    glDisable(GL_LIGHTING);

  static float rotate = 0;							  // Defines the degree of rotation for the pyramid
  static float rotate_incr = 0.005;
  rotate += rotate_incr;									// Increment rotate every time through display
  while(rotate >= 360)										// Reset rotate once it passes 360 degrees
    rotate -= 360;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // Clear display window with set color and clear depth buffer

  // Enable wireframe rendering?
  if(wireframe)
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);					  // Switches to wireframe mode (usually not desirable but can be good for debugging)
  else
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  // Set flat or smooth shading
  if(polygonshade == 0)
    glShadeModel(GL_FLAT);
  else
    glShadeModel(GL_SMOOTH);      // Smooth discontinuous vertices when loading model for this to appear correctly

  glMatrixMode(GL_MODELVIEW);     // Switch matrix mode back to model-view (usually done when about to draw object geometry)
  glLoadIdentity();							  // Load the identity matrix		
  glEnable(GL_NORMALIZE);         // Only needed if any of the vertex normals are scaled

  // Compute a point the camera is looking at
  Vector3D offset, camera_to;
  MultiplyScalarVector(1,&camera_heading,&offset);
  AddVector(&camera_position,&offset,&camera_to);

  glPushMatrix();
  
  // Add to the modelview matrix the necessary tranforms to move the camera
  gluLookAt(camera_position.x,camera_position.y,camera_position.z, 
            camera_to.x, camera_to.y, camera_to.z, 
            camera_up.x, camera_up.y, camera_up.z);

  // Draw a model
  glColor3f(1,1,1); 
  glPushMatrix();
  glTranslatef(0,-5,-10);
  glRotatef(rotate,10,1,0);
  glScalef(40,40,40);
  modelTex3D_drawFast(obj_teapot,texture_id,texture_data);
  glPopMatrix();

    // Draw a model
  glColor3f(1,1,1); 
  glPushMatrix();
  glTranslatef(10,-5,-25);
  glRotatef(rotate,10,1,0);
  glScalef(40,40,40);
  modelTex3D_drawFast(obj_teapot, texture_id, texture_data);
  glPopMatrix();

  // Draw a model
  glColor3f(1, 1, 1);
  glPushMatrix();
  glTranslatef(20, -5, -35);
  glRotatef(rotate, 10, 1, 0);
  glScalef(40, 40, 40);
  modelTex3D_drawFast(obj_teapot, texture_id, texture_data);
  glPopMatrix();

  glPopMatrix();

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glColor3f(1, 1, 1);
  glPushMatrix();
  glTranslatef(-3, 3, -0.88);
  modelTex3D_drawFast(obj_overlay, texture_overlay_id, texture_overlay_data);
  glPopMatrix();
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  glFlush();																	// Flush the OpenGL buffers to the window 
  glutPostRedisplay();												// This function sets a flag in GLUT's main loop which
                                              // indicates that the display needs to be redrawn
  errorCheck("render");
}

/*************************************************************************************
| Function: udpate
|
| Description: Update any variables based on user input, etc.
*************************************************************************************/
#define MOVE_AMOUNT 0.005f    // the amount of movement per frame
#define ROTATE_AMOUNT 0.25f   // the amount of rotation per frame, in degrees

void update() {

  // If this is the first time in this function, query where the mouse is and save the coordinates
  static bool first_time = true;
  static int mouse_x_last,mouse_y_last;
  
  // If first time using this function, intialize some variables
  if (first_time) {
    mouse_x_last = mouse_x;
    mouse_y_last = mouse_y;
    camera_heading = start_heading;
    camera_up = start_up;
    first_time = false;
  }

  /*____________________________________________________________________
  |
  | Rotate heading?
  |___________________________________________________________________*/

  bool changed = false;

  int mouse_dx = (int)(fabsf(mouse_x-mouse_x_last));
  int mouse_dy = (int)(fabsf(mouse_y-mouse_y_last));

  // Has the mouse moved left?
  if (mouse_x < mouse_x_last)  {
    current_yrotate += ROTATE_AMOUNT * mouse_dx;
    changed = true;
  }
  // Has the mouse moved right?
  else if(mouse_x > mouse_x_last) {
    current_yrotate -= ROTATE_AMOUNT * mouse_dx;
    changed = true;
  }
  // Has the mouse moved up?
  if(mouse_y < mouse_y_last) {
    current_xrotate -= ROTATE_AMOUNT * mouse_dy;
    changed = true;
  }
  // Has the mouse moved down?
  else if(mouse_y > mouse_y_last) {
    current_xrotate += ROTATE_AMOUNT * mouse_dy;
    changed = true;
  }
  // Rotate heading
  if (changed) {
    Matrix3D mx, my, m;
    GetRotateYMatrix(&my,current_yrotate);
    GetRotateXMatrix(&mx,current_xrotate);
    MultiplyMatrix(&my,&mx,&m);
    MultiplyVectorMatrix(&start_heading,&m,&camera_heading);
    MultiplyVectorMatrix(&start_up,&m,&camera_up);
    // Make sure heading is normalized
    NormalizeVector(&camera_heading,&camera_heading);
  }

  /*____________________________________________________________________
  |
  | Move camera position?
  |___________________________________________________________________*/

  if(move_forward || move_back || move_left || move_right) {
    Vector3D offset, v_left;
    if(move_forward && !move_back) {
      MultiplyScalarVector(MOVE_AMOUNT,&camera_heading,&offset);
      AddVector(&camera_position,&offset,&camera_position);
    }
    if(move_back && !move_forward) {
      MultiplyScalarVector(-MOVE_AMOUNT,&camera_heading,&offset);
      AddVector(&camera_position,&offset,&camera_position);
    }
    if(move_left && !move_right) {
      VectorCrossProduct(&camera_up,&camera_heading,&v_left);
      NormalizeVector(&v_left,&v_left);
      MultiplyScalarVector(MOVE_AMOUNT,&v_left,&offset);
      AddVector(&camera_position,&offset,&camera_position);
    }
	if (move_right && !move_left) {
		VectorCrossProduct(&camera_up, &camera_heading, &v_left);
		NormalizeVector(&v_left, &v_left);
		MultiplyScalarVector(-MOVE_AMOUNT, &v_left, &offset);
		AddVector(&camera_position, &offset, &camera_position);
	}
    // ADD CODE TO MOVE RIGHT
  }


  glutWarpPointer(VIEW_WIDTH/2,VIEW_HEIGHT/2);
  mouse_x_last = VIEW_WIDTH/2;
  mouse_y_last = VIEW_HEIGHT/2;
}

/*************************************************************************************
| Function: model3D_draw
|
| Description: Renders a 3D model.
*************************************************************************************/
void model3D_draw(Object3D *o) {

  for(int i = 0; i<o->num_polygons; i++) {
    glBegin(GL_TRIANGLES);
      glNormal3f(o->vertex_normal[o->polygon[i].index[0]].x,
                 o->vertex_normal[o->polygon[i].index[0]].y,
                 o->vertex_normal[o->polygon[i].index[0]].z);
      glVertex3f(o->vertex[o->polygon[i].index[0]].x,
                 o->vertex[o->polygon[i].index[0]].y,
                 o->vertex[o->polygon[i].index[0]].z);

      glNormal3f(o->vertex_normal[o->polygon[i].index[1]].x,
                 o->vertex_normal[o->polygon[i].index[1]].y,
                 o->vertex_normal[o->polygon[i].index[1]].z);
      glVertex3f(o->vertex[o->polygon[i].index[1]].x,
                 o->vertex[o->polygon[i].index[1]].y,
                 o->vertex[o->polygon[i].index[1]].z);

      glNormal3f(o->vertex_normal[o->polygon[i].index[2]].x,
                 o->vertex_normal[o->polygon[i].index[2]].y,
                 o->vertex_normal[o->polygon[i].index[2]].z);
      glVertex3f(o->vertex[o->polygon[i].index[2]].x,
                 o->vertex[o->polygon[i].index[2]].y,
                 o->vertex[o->polygon[i].index[2]].z);
    glEnd();
  }

  errorCheck("model3D");
}

/*************************************************************************************
| Function: model3D_drawFast
|
| Description: Renders a 3D model using a faster method.
*************************************************************************************/
void model3D_drawFast(Object3D *o) {

  // Use the vertex buffer for rendering
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3,GL_FLOAT,0,o->vertex);

  // Use the vertex normal buffer for rendering
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT,0,o->vertex_normal);

  // Draw the model
  glDrawElements(GL_TRIANGLES,o->num_polygons * 3,GL_UNSIGNED_SHORT,o->polygon);

  // Disable the buffers
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}

/*************************************************************************************
| Function: modelTex3D_drawFast
|
| Description: Renders a textured 3D model using the fast method.
*************************************************************************************/
void modelTex3D_drawFast(Object3D *o,  GLuint texture_id, unsigned char *texture_data) {

  // Use the vertex buffer for rendering
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3,GL_FLOAT,0,o->vertex);

  // Use the vertex normal buffer for rendering
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT,0,o->vertex_normal);

  if (texture_id != -1 && texture_data != 0) {
    glEnable(GL_TEXTURE_2D);
    // Enable the texture state
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    // Point to our buffer
    glTexCoordPointer(2,GL_FLOAT,0,o->tex_coords);
    glBindTexture(GL_TEXTURE_2D,texture_id);
  }

  // Draw the model
  glDrawElements(GL_TRIANGLES,o->num_polygons * 3,GL_UNSIGNED_SHORT,o->polygon);

  // Disable the buffers
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  if (texture_id != -1 && texture_data != 0)
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

/*************************************************************************************
| Function: loadBMPfile
|
| Description: loads the data from a BMP file.  Returns true on success, false on any
|   error.  Caller should free() the buffer when done using it.
*************************************************************************************/
bool loadBMPfile (char *filename, int *width, int *height, unsigned char **data)
{
  // Data read from the header of the BMP file
  unsigned char header[54]; // Each BMP file begins by a 54-bytes header
  unsigned int dataPos;     // Position in the file where the actual data begins
  unsigned int imageSize;   // = width*height*3

  // Init variables
  *data = 0;  // buffer is not created yet

  // Open the file
  FILE *file = fopen (filename,"rb");
  // If file not opened, return with an error
  if (file == 0) {
    cout << "File not opened" << endl;
    return false;
  }
  // Read the 54-byte header
  if (fread(header,1,54,file) != 54) { // If 54 bytes not read, then error
    cout << "Not a correct BMP file" << endl;
    fclose(file);
    return false;
  }
  // Make sure first 2 bytes are BM
  if (header[0] != 'B' || header[1] != 'M') {
    cout << "Not a correct BMP file" << endl;
    return false;
  }
  // Read values from the byte array
  dataPos   = *(int*)&(header[0x0A]);         
  imageSize = *(int*)&(header[0x22]);
  *width    = *(int*)&(header[0x12]);
  *height   = *(int*)&(header[0x16]);
  // Fill in any missing information
  if (imageSize == 0)    
    imageSize = (*width) * (*height) * 3; // 3 since 1 byte for each red, green and blue component
  if (dataPos == 0)      
    dataPos = 54; 
  // Create a buffer (caller should free this buffer when done using it)
  *data = (unsigned char *) malloc(imageSize);
  // Read the image data from the file into the buffer
  fread((void*)*data,1,imageSize,file);
  // Reorder the data from RBG to GBR (which OpenGL likes)
  unsigned char red;
  for (unsigned n=0; n<imageSize; n+=3) {
    red = (*data)[n];           // save red
    (*data)[n] = (*data)[n+2];  // move green into reds old place
    (*data)[n+2] = red;         // move red into greens old place
  }

  // close the file
  fclose(file);
  return true;
}
