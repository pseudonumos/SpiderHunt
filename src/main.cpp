#include "main.h"
	
vec4 lightPosition(0, 0, -1, 1);

using namespace std;
void applyMat4(mat4 &mat);
void renderBitmapString(float x, float y, void *font, char *string);
//****************************************************
// Some Classes
//****************************************************
bool drawSphere = true;
GLuint sphereDL;
bool firstPersonMode = true;
bool hover = false;
class Viewport {
public:
    Viewport(): mousePos(0.0,0.0) { orientation = identity3D(); };
	int w, h; // width and height
	vec2 mousePos;
    mat4 orientation;
};
Viewport viewport;
int bound = 4;
face * currFace;

GLubyte data[32*4];
double height = 0;
double verticalVelocity = 0;
bool showHealth = false;

bullet * gunShot;
class slider {
public:
	int percent;
	slider(){
	}
	void render(){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, '<');
		for (int i=0; i< 30; i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, '-');
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, '>');
	}
};

bool ape(double a, double b){
	return (abs(a-b) < 0.05 );
}

class tank {
public:
	vec3 pos;
	vec3 veloc;
	vec3 forward;
	bool s1, s2, s3, s4;
	bool t1, t2, t3, t4;
	bool shoot;
	double angle;
	tank(vec3 p){
		pos = p;
		veloc = vec3(0.0, 0.0, 0.0);
		angle =  0;
		forward = vec3(0, 1, 0);
	}
	void fire(int type){
		shoot = true;
		delete gunShot;
		gunShot = new bullet(type);
		gunShot->start = pos + vec3(0, 0, 0.5);
		gunShot->current = pos;
		//gunShot->dir = 	-1 * (rotation3D(vec3(0, 0, 1), 270) * vec3(cos(angle / 180 * 3.1415), sin(angle / 180 * 3.1415), 0));
		GLfloat	matrix[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
		vec4 v_1(matrix[0], matrix[1], matrix[2], matrix[3]);
		vec4 v_2(matrix[4], matrix[5], matrix[6], matrix[7]);
		vec4 v_3(matrix[8], matrix[9], matrix[10], matrix[11]);
		vec4 v_4(matrix[12], matrix[13], matrix[14], matrix[15]);
		mat4 orientMatrix = mat4(v_1, v_2, v_3, v_4);
		vec3 forwardVector = vec3(orientMatrix * vec4(0, 0, 1, 1));
		gunShot->dir = forwardVector;
	}
	void render(ccMap * cube, face * f, double h=0, vec3 goal=vec3(0, 0, 0)) {
		// check if within wireframe
		if (pos.length() < 6 && drawSphere) {
			veloc = pos / 25;
		// check with intersection of flag
		}
		vec3 diff = pos - goal;
		if (abs(diff.length()) < 1) {
			cout << "YOU'VE WON THE GAME!" << endl;
			system("PAUSE");
			exit(0);
		}
		
		pos += veloc;
		//if (!drawSphere && pos.length() < 6 && pos.length() > 2 && verticalVelocity > 0)
			//if (ape(pos[2], 4.3) || ape(pos[2], -1.7)) verticalVelocity = 0;
		veloc /=1.1;
		if (gunShot != NULL){
			glPushMatrix();	
			gunShot->render();
			glPopMatrix();
		}
		mat4 o = viewport.orientation;
				
        glPushMatrix();
        glTranslatef(pos[0], pos[1], pos[2]);
		glRotatef(90, 1, 0, 0);
		applyMat4(viewport.orientation.inverse());
				
		if (firstPersonMode){
			if (verticalVelocity != 0.0){
				if (!hover) verticalVelocity += 0.00020; //gravity
				//if (!hover) verticalVelocity += 0.005; //gravity
				else if (verticalVelocity < 0.0000000000000000000000000000000000000000000001) hover = false;
				else verticalVelocity += -1 * verticalVelocity / 10;
			}
				
			if (hover){
			//glEnable(GL_POLYGON_STIPPLE);
			
			//glPolygonStipple(data);
			glPushMatrix();
		    glUseProgramObjectARB(0);	
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(0.9, 0.5, 0.1, 0.5);
			glRotatef(90, -1, 0, 0);
			//glutSolidCone(0.5, 0.3, 9, 9);
			glutSolidTorus(0.3, 0.5, 5, 5);
			glDisable(GL_BLEND);
			//glDisable(GL_POLYGON_STIPPLE);
			glPopMatrix();
		}
			glColor4f(1, 1, 1, 1); //crossHairs
			glBegin(GL_LINES);
				glVertex3f(0, -0.05, 0);
				glVertex3f(0,  0.05, 0);
				glVertex3f(0, 0, -0.05);
				glVertex3f(0, 0, 0.05);
			glEnd();
		}
		
		glCallList(sphereDL);
        glPopMatrix();	

	}
	void boost(vec3 d){
		veloc += d/10;
	}
};

//****************************************************
// Global Variables
//****************************************************
double delta = 0.4;
int    aniFrame;
int	   frameCount = 0;
int    faceNum;
bool   bump = false;
bool   animate = false;
mat4 defaultOrientation = mat4(vec4(0, 0, 1, 0), vec4(0, 1, 0, 0), vec4(-1, 0, 0, 0), vec4(0, 0, 0, 1));
mat4 oldOrientation = identity3D();

// Z values will be rendered to this texture when using fboId framebuffer
GLuint depthTextureId;
// Hold id of the framebuffer for light POV rendering
GLuint fboId;

UCB::ImageSaver * imgSaver;
face  * currentFace;
ccMap * cube;
tank  * firstPerson;
Cloth * flag;

GLuint coastDL;
vector<Spider> creatures;

void makeList();

void initShadows();

// A simple helper function to load a mat4 into opengl
void applyMat4(mat4 &mat) {
    double glmat[16];
    int k = 0;
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            glmat[k++] = mat[j][i];
        }
    }
    glMultMatrixd(glmat);
}
//-------------------------------------------------------------------------------
/// You will be calling all of your drawing-related code from this function.
/// Nowhere else in your code should you use glBegin(...) and glEnd() except code
/// called from this method.
///
/// To force a redraw of the screen (eg. after mouse events or the like) simply call
/// glutPostRedisplay();

void initShadows() {
	GLenum FBOstatus;

	// Try to use a texture depth component
	glGenTextures(1, &depthTextureId);
	glBindTexture(GL_TEXTURE_2D, depthTextureId);
	
	// GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// Remove artefact on the edges of the shadowmap
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	
	//glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );
	
	// No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available 
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 600, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// create a framebuffer object
	glGenFramebuffersEXT(1, &fboId);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
	
	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	// attach the texture to FBO depth attachment point
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, depthTextureId, 0);
	
	// check FBO status
	FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
		printf("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO\n");
	
	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

}

void display() {

//	BEGIN SHADOWS

	//First step: Render from the light POV to a FBO, story depth values only
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);	//Rendering offscreen
	
	//Using the fixed pipeline to render to the depthbuffer
	glUseProgramObjectARB(0);
		
	// Clear previous frame values
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//Calculate & save matrices

    glLoadIdentity();
	//    gluPerspective(45.0f, 1.0f, 2.0f, 8.0f);
//	gluPerspective(45.0, ((double)viewport.w / MAX(viewport.h, 1)), 1.0, 100.0);

    glLoadIdentity();
    gluLookAt( lightPosition[0], lightPosition[1], lightPosition[2],
		       0.0f, 1.0f, 1.0f,
               1.0f, 1.0f, 0.0f);

	//Disable color rendering, we only want to write to the Z-Buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		
	// Culling switching, rendering only backface, this is done to avoid self-shadowing
//	glCullFace(GL_FRONT);

	// RENDER OBJECTS HERE!!!

	if (firstPersonMode){
		glutSolidSphere(100, 100, 100);
	} 

	glCallList(coastDL);

	drawSphere = true;
	for (unsigned int i=0; i< creatures.size(); i++){
		glPushMatrix();
		drawSphere = drawSphere && !(creatures[i].alive);
		bool ret;
		if (gunShot != NULL) ret = creatures[i].render(showHealth, gunShot, cube, i);
		else ret = creatures[i].render(showHealth, NULL, cube, i);
		if (ret){
			delete gunShot;
			gunShot = NULL;
		}
		glPopMatrix();
	}
	drawSphere = !drawSphere;
	
	if (drawSphere) {
		glutWireSphere(6, 55, 55);
	}
//	firstPerson->render(cube, currentFace,0 ,flag->top);
	glColor3f(0, 0, 0);
	flag->renderCloth(depthTextureId);

	// END RENDER OBJECTS HERE

	// create Z buffer from light's point of view
	static double modelView[16];
	static double projection[16];
	
	// This is matrix transform every coordinate x,y,z
	// x = x* 0.5 + 0.5 
	// y = y* 0.5 + 0.5 
	// z = z* 0.5 + 0.5 
	// Moving from unit cube [-1,1] to [0,1]  
	const GLdouble bias[16] = {	
		0.5, 0.0, 0.0, 0.0, 
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0};
	
	// Grab modelview and transformation matrices
	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	
	glMatrixMode(GL_TEXTURE);
	glActiveTextureARB(GL_TEXTURE7);
	
	glLoadIdentity();	
	glLoadMatrixd(bias);
	
	// concatating all matrice into one.
	glMultMatrixd (projection);
	glMultMatrixd (modelView);
	
	// Go back to normal matrix mode
	glMatrixMode(GL_MODELVIEW);

	// Now rendering from the camera POV, using the FBO to generate shadows
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

	//Enabling color write (previously disabled for light POV z-buffer rendering)
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
	
	// Clear previous frame values
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//*/

	// END SHADOWS

	//Clear Buffers
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);					// indicate we are specifying camera transformations
	glLoadIdentity();							// make sure transformation is "zero'd"
	
	if (firstPersonMode){
		glUseProgramObjectARB(0);
		glColor3f(.25, .65, 1.0);
		glutSolidSphere(100, 100, 100);

		glutWarpPointer(viewport.h * 0.5, viewport.w * 0.5);
		viewport.mousePos = vec2(0.5,0.5);
		if (abs(firstPerson->pos[2]) > 25){
			firstPerson->pos = vec3(0, -10, 0);
			verticalVelocity = 0.0;
		}
		glTranslatef(0.0,0.0,-1.0);
		applyMat4(viewport.orientation);
		
		glRotatef(-90, 1, 0, 0);
		glRotatef(90, 0, 0, 1);
		glTranslatef(-firstPerson->pos[0],-firstPerson->pos[1], -firstPerson->pos[2]);
		
	} else{
		glTranslatef(0,0,-20);
		applyMat4(viewport.orientation);
	}

	glCallList(coastDL);
	
	if (animate) {
		vec3 axis = vec3(0, 1, 0);
		//viewport.orientation = viewport.orientation * rotation3D(axis, 10);
		cout << aniFrame << endl;
		aniFrame++;
		imgSaver->saveFrame();
		if (aniFrame > 33) animate=false;
	}
	drawSphere = true;
	for (unsigned int i=0; i< creatures.size(); i++){
		glPushMatrix();
		drawSphere = drawSphere && !(creatures[i].alive);
		bool ret;
		if (gunShot != NULL) ret = creatures[i].render(showHealth, gunShot, cube, i);
		else ret = creatures[i].render(showHealth, NULL, cube, i);
		if (ret){
			delete gunShot;
			gunShot = NULL;
		}
		glPopMatrix();
	}
	drawSphere = !drawSphere;
	
	if (drawSphere) {
		glutWireSphere(6, 55, 55);
	}
	firstPerson->render(cube, currentFace,0 ,flag->top);
	firstPerson->pos -= vec3(0.0, 0.0, verticalVelocity);	
	height -= verticalVelocity;
	
	flag->renderCloth(depthTextureId);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutSwapBuffers();

	if (viewport.orientation != oldOrientation) {
		/*
		cout << matrix[0] << "\t" << matrix[1] << "\t" << matrix[2] << "\t" << matrix[3] << endl;
		cout << matrix[4] << "\t" << matrix[5] << "\t" << matrix[6] << "\t" << matrix[7] << endl;
		cout << matrix[8] << "\t" << matrix[9] << "\t" << matrix[10] << "\t" << matrix[11] << endl;
		cout << matrix[12] << "\t" << matrix[13] << "\t" << matrix[14] << "\t" << matrix[15] << endl;
		*/
//		cout << viewport.orientation << endl << endl;
		oldOrientation = viewport.orientation;
	}
	if (cube->subdivLevel < 3){
			cube->subdivide();
			makeList();
		}

}


//-------------------------------------------------------------------------------
/// \brief	Called when the screen gets resized.
/// This gives you the opportunity to set up all the relevant transforms.
///
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
	viewport.w = w;
	viewport.h = h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, ((double)w / MAX(h, 1)), 1.0, 100.0);
	//glOrtho(-10,10,-10,10,1,100);
	
	viewport.mousePos = vec2(.5, .5);

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


//-------------------------------------------------------------------------------
/// Called to handle keyboard events.
void myKeyboardFunc (unsigned char key, int x, int y) {
	mat4 o = viewport.orientation;
	// This computes the 'RIGHT' movement vector
	vec3 deltaVector = delta * vec3(cos(firstPerson->angle / 180 * 3.1415), sin(firstPerson->angle / 180 * 3.1415), 0);
	GLfloat	matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	vec4 v_1(matrix[0], matrix[1], matrix[2], matrix[3]);
	vec4 v_2(matrix[4], matrix[5], matrix[6], matrix[7]);
	vec4 v_3(matrix[8], matrix[9], matrix[10], matrix[11]);
	vec4 v_4(matrix[12], matrix[13], matrix[14], matrix[15]);
	mat4 orientMatrix = mat4(v_1, v_2, v_3, v_4);
	vec3 forwardVector = delta * vec3(orientMatrix * vec4(0, 0, 1, 1));
	vec3 leftVector = delta * vec3(orientMatrix * vec4(-1, 0, 0, 1));
	vec3 rightVector = delta * vec3(orientMatrix * vec4(1, 0, 0, 1));
	vec3 backVector = -1 * delta * vec3(orientMatrix * vec4(0, 0, 1, 1));

	switch (key) {
		// ESC key
        case 27:
                exit(0);
                break;
		// UP
		case 'W':
		case 'w':
			if (firstPerson->pos.length() > 7) {				
				firstPerson->boost(-1 * (rotation3D(vec3(0, 0, 1), 270) * deltaVector));
			} else if (drawSphere) {
				firstPerson->boost((rotation3D(vec3(0, 0, 1), 270) * deltaVector));
			} else {
				firstPerson->boost(-1 * (rotation3D(vec3(0, 0, 1), 270) * deltaVector));
			}		
        	break;
        // LEFT
		case 'A':
		case 'a':
			firstPerson->boost(-1 * (deltaVector));
            break;
        // DOWN
		case 'S':
		case 's':
			firstPerson->boost(rotation3D(vec3(0, 0, 1), 270) * deltaVector);
		    break;
        // RIGHT
		case 'D':
		case 'd':
			firstPerson->boost(deltaVector);
            break;
		// PAUSE
        case 'C' :
        case 'c' :
            cout << "subdivide" << endl;
            cube->subdivide();
            makeList();
            break;
        case 'O' :
        case 'o':
			cube->shadersOn = !cube->shadersOn;
            makeList();
            break;
		case 'H' :
		case 'h':
			showHealth = !showHealth;
        case 'V':
        case 'v':
            imgSaver->saveFrame();
            break;
		case 'R':
			delete flag;
			flag = new Cloth("track2.trk", "sweepShader.vert", "sweepShader.frag");
			break;
		case 'r':
			delete flag;
			flag = new Cloth("track2.trk", "toonShader.vert", "toonShader.frag");
			break;
		case 'P':
        case 'p':
			system("PAUSE");
			break;
        case 'F':
        case 'f':
            firstPersonMode = !firstPersonMode;
			viewport.orientation = defaultOrientation;
			firstPerson->angle = 0;
            break;
		case 'x':
			hover = !hover;
			break;
        case 'N':
        case 'n':
           animate = !animate;
            aniFrame = 0;
            break;
		case ' ':
			verticalVelocity += -0.025;
			break;
		case 'Q':
			flag->wind = vec3(.2, .2, 4.2);
			break;
		case 'q':
			flag->wind = vec3(0, 0, 0);
			break;
		case 'u':
			cout << viewport.orientation[2] << endl;
			vec4 v = viewport.orientation[2];
			firstPerson->boost(vec3(v[0], v[1], v[2]));
			break;
		
	}
}

//-------------------------------------------------------------------------------
/// Called whenever the mouse moves while a button is pressed



void makeList(){

	if (!animate) glNewList(coastDL, GL_COMPILE);
	cube->renderCube(depthTextureId);
	if (!animate) glEndList();
	
}

void myActiveMotionFunc(int x, int y) {
//	if (!firstPersonMode){

    // Rotate viewport orientation proportional to mouse motion
    vec2 newMouse = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));
    vec2 diff = (newMouse - viewport.mousePos);
    double len = diff.length();
    if (len > .001) {
        vec3 axis= vec3(diff[1]/len, diff[0]/len, 0);
		//UNCOMMENT THIS CODE TO RE-ENABLE UP/DOWN LOOKING
		if (!firstPersonMode)
		viewport.orientation = viewport.orientation * rotation3D(axis, -180 * len);
    }
    //Record the mouse location for drawing crosshairs
    viewport.mousePos = newMouse;
    //Force a redraw of the window.
    glutPostRedisplay();
//	}
}


//-------------------------------------------------------------------------------
/// Called whenever the mouse moves without any buttons pressed.
void myPassiveMotionFunc(int x, int y) {
    //Record the mouse location for drawing crosshairs
//    viewport.mousePos = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));
	
        if (firstPersonMode) {
                // Rotate viewport orientation proportional to mouse motion
				vec2 newMouse = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));
                vec2 diff = (newMouse - viewport.mousePos);
				double len = abs(diff[0]);
                if (len > .001) {
                    vec3 axis = vec3(0, diff[0]/len, 0);
                    viewport.orientation = viewport.orientation * rotation3D(axis, -360 * len);
					firstPerson->angle -= diff[0]*360;
					firstPerson->forward =  rotation3D(axis, -360 * len) * vec4(firstPerson->forward);	
                } else {
                    //viewport.mousePos = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));

                }
                //Record the mouse location for drawing crosshairs
                viewport.mousePos = newMouse;
        } else {
                //Force a redraw of the window.
            viewport.mousePos = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));
        }
	
    glutPostRedisplay();
	
}

//-------------------------------------------------------------------------------
/// Called to update the screen at 30 fps.
void frameTimer(int value){

    frameCount++;
    glutPostRedisplay();
    glutTimerFunc(1000/3000, frameTimer, 1);
}

void exitFreeimage(void) {
    FreeImage_DeInitialise();
}
void myMouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) firstPerson->fire(0);
	else firstPerson->fire(1);
}
void specialKeyFunc( int key, int x, int y) {
	mat4 o = viewport.orientation;
	switch( key ) {
		case GLUT_KEY_LEFT:
			firstPerson->boost(rotation3D(vec3(0, 0, 1), 90) * 
			vec3(cos(firstPerson->angle / 180 * 3.1415), sin(firstPerson->angle / 180 * 3.1415), 0));
            break;
        case GLUT_KEY_RIGHT:
            firstPerson->boost(rotation3D(vec3(0, 0, 1), 90) * 
			- vec3(cos(firstPerson->angle / 180 * 3.1415), sin(firstPerson->angle / 180 * 3.1415), 0));
            break;
        case GLUT_KEY_UP:
			if (firstPerson->pos.length() > 7) firstPerson->boost(vec3(cos(firstPerson->angle / 180 * 3.1415), sin(firstPerson->angle / 180 * 3.1415), 0));
			else 
				if (drawSphere) firstPerson->boost(- vec3(cos(firstPerson->angle / 180 * 3.1415), sin(firstPerson->angle / 180 * 3.1415), 0));
				else firstPerson->boost(vec3(cos(firstPerson->angle / 180 * 3.1415), sin(firstPerson->angle / 180 * 3.1415), 0));
            break;
        case GLUT_KEY_DOWN:
            firstPerson->boost(- vec3(cos(firstPerson->angle / 180 * 3.1415), sin(firstPerson->angle / 180 * 3.1415), 0));
            break;	
    }

}


//-------------------------------------------------------------------------------
/// Initialize the environment
int main(int argc,char** argv) {
	gunShot = NULL;
	argc=2; argv[1] = "track.trk";
	//Initialize OpenGL
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	
    FreeImage_Initialise();
    atexit(exitFreeimage);

	//Set up global variables
	viewport.w = 600;
	viewport.h = 600;
	
	if (argc < 2) {
	    cout << "USAGE: coaster coaster.trk" << endl;
	    return -1;
    }

	//Initialize the screen capture class to save BMP captures
	//in the current directory, with the prefix "sweep"
	imgSaver = new UCB::ImageSaver("./", "sweep");
	
	//Create OpenGL Window
	glutInitWindowSize(viewport.w,viewport.h);
	glutInitWindowPosition(0,0);
	glutCreateWindow("CS184 Framework");

	//Register event handlers with OpenGL.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(myKeyboardFunc);
	glutSpecialFunc(specialKeyFunc);
	glutMotionFunc(myActiveMotionFunc);
	glutMouseFunc(myMouseFunc);
	glutPassiveMotionFunc(myPassiveMotionFunc);
    frameTimer(0);
	
	/*

	{
		glEnable(GL_LIGHTING);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		
		GLfloat global_ambient[] = { .1f, .1f, .1f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
		
		//define and enable light0.  You have 8 lights in total.

		GLfloat ambient[] = {.05f, .05f, .05f };
		GLfloat diffuse[] = {.9f, .9f, .9f};
		GLfloat specular[] = {0.1, 0.1, 0.1, 1.0};
		GLfloat pos[]	= {0.0, 0.0, 1.0, 0};

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
		glLightfv(GL_LIGHT0, GL_POSITION, pos);

		GLfloat mat_specular [] = {1.0, 0.0, 0.0, 1.0};
		GLfloat mat_diffuse [] = {0.0, 1.0, 0.0, 1.0};
		GLfloat mat_ambient [] = {0.0, 0.1, 0.1, 1.0};
		GLfloat mat_shininess = {10.0};

		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

		glShadeModel(GL_SMOOTH);
		glEnable(GL_NORMALIZE);

	}
	*/

	// set some lights
    {

	   // directional light
	   float ambient[4] = { .0f, .0f, .0f, 1.0f };
       float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	   float specular[4] = { .5f, .2f, .5f, 1.f };
       float pos[4] = { 0, 0, 1, 0 };

       glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
       glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
       glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
       glLightfv(GL_LIGHT0, GL_POSITION, pos);
       glEnable(GL_LIGHT0);

       float ambient2[4] = { .95f, .65f, .65f, 1.f };
       float diffuse2[4] = { 0.5f, 0.5f, 0.5f, 1.f };
	   float specular2[4] = { .7f, .5f, .1f, 1.f };
       float pos2[4] = { lightPosition[0], lightPosition[1], lightPosition[2], lightPosition[3]};
//       float pos2[4] = { -4.5, -7.5, -5.5, .1 };
       glLightfv(GL_LIGHT1, GL_AMBIENT, ambient2);
       glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse2);
       glLightfv(GL_LIGHT1, GL_SPECULAR, specular2);
       glLightfv(GL_LIGHT1, GL_POSITION, pos2);
       glEnable(GL_LIGHT1);		

	   glEnable(GL_NORMALIZE);

    }

    glewInit();

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

	if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
		printf("Ready for GLSL\n");
	else {
		printf("No GLSL support\n");
		system("PAUSE");
		exit(1);
	}

	initShadows();

    // Create the coaster    
	cube = new ccMap(1,argv[1], "sweepShader.vert", "sweepShader.frag");
	firstPerson = new tank(vec3(0, -10, 0));
	flag = new Cloth("track2.trk", "sweepShader.vert", "sweepShader.frag");
	coastDL= glGenLists(1);
	makeList();
	sphereDL = glGenLists(1);
	glNewList(sphereDL, GL_COMPILE);
		glColor3f(0.5, 0.5, 0.5);
		glTranslatef(0, -0.1, 0);
		//glutSolidSphere(0.3,50,50);
		glutSolidTeapot(0.1);
	glEndList();
	viewport.orientation = defaultOrientation;

	creatures.push_back(Spider(0, 9, vec3(0, -2.8, 3.9), "track.trk", "toonShader.vert", "toonShader.frag"));
	//creatures.push_back(Spider(0, 2, vec3(0, 6, 0), "track.trk", "toonShader.vert", "toonShader.frag"));
	creatures.push_back(Spider(0, 3, vec3(0, 2.5, 3.9), "track.trk", "sweepShader.vert", "sweepShader.frag"));
	//creatures.push_back(Spider(0, 3, vec3(0, 0, -3), "track.trk", "sweepShader.vert", "sweepShader.frag"));

	//And Go!
	glutMainLoop();
}
