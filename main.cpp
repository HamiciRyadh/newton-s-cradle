#include <windows.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


//Declaration of the functions used with glut.
void reshape(int width, int height);
void display(void);
void congifureLight1(void);
void idle(void);
void specialKey(int key, int x, int y);
void keyPressed(unsigned char touche, int x, int y);

//Declaration of the custom functions used.
void init(void);
void writeText(float posX, float posY, float posZ, const char str[]);
void displayTexts(void);
void setBallColor(int idx);
void drawWireAndBall(bool animate, int ballNumber);
void handleFriction(int coef);
void reset(void);


//Global variables.
float rDx = 0.0f, rDy = 0.0f, rDz = 0.0f;
float maxBallDeg = 65.0, usedMaxBallDeg = maxBallDeg;
float ballDeg = -maxBallDeg, ballStep = -0.3, frictionEnergyReduction = -3,
        gravityEnergyAmplification = 1.01, gravityEnergyReduction = 0.98;
int counts[5] = {0, 0, 0, 0, 0};
bool states[5] = {false, false, false, false, false}, doIdle = true;
int numberOfBalls = 5, usedBalls = 1;
bool applyFriction = false, applyGravity = true, showEnergyPosition = true, transmittingEnergy = false;


//Animation of Newton's Cradle (Pendule de Newton).
int main(int argc, char** argv) {
    glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH | GLUT_RGBA);
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(600, 0);
    glutCreateWindow("Newton's Cradle Project");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutSpecialFunc(specialKey);
    glutKeyboardFunc(keyPressed);

    glutMainLoop();
}

void init(void) {
    glClearColor(0.0, 0.5, 0.5, 0.5);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(25, 1, 0.1, 100);
    gluLookAt(0, 10, 26, 0, 0, 0, 0, 1, 0);
    glColorMask(true, true, true, true);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    congifureLight1();

    glEnable(GL_COLOR_MATERIAL);
}

void congifureLight1(void) {
    GLfloat light_couleur_ambient[] = {0, 0, 0, 1};
    GLfloat light_couleur_diffuse[] = {0.9, 0.9, 0.9, 0};
    GLfloat light_couleur_specular[] = {0, 0, 0, 0};
    GLfloat light_position[] = {0, 16, 4, 4};
    GLfloat obj_shine[] = {50.0};

    glLightfv(GL_LIGHT1, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_couleur_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_couleur_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_couleur_specular);

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, light_couleur_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, light_couleur_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, obj_shine);

    glEnable(GL_LIGHT1);
}


void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

//Writes the given string at the given position.
void writeText(float posX, float posY, float posZ, const char str[]) {
    glColor3f(0, 0, 0);
    glRasterPos3f(posX, posY, posZ);
    for (unsigned int i = 0; i < strlen(str); i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
    }
}

//Handles all the text to be displayed.
void displayTexts(void) {
    writeText(-3, 5, 1, "Newton's Cradle (");
    writeText(0, 5, 1, applyFriction ? "Friction : On)" : "Friction : Off)");
    writeText(-4, -3, 1, "Press '+'/'-' to add/remove intermediate balls.");
    writeText(-4, -3.5, 1, "Use the number pad to select the number of moving balls.");
    writeText(-4, -4.5, 1, "Press 'f' to control the friction.");
    writeText(-4, -4, 1, "Press 'c' to display/hide the energy's position.");
    writeText(-4, -5, 1, "Press 'n' to reset.");
    writeText(-4, -5.5, 1, "Press space bar to pause/start.");
}

//Draws all the objects.
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glRotatef(rDx, 1, 0, 0);
    glRotatef(rDy, 0, 1, 0);
    glRotatef(rDz, 0, 0, 1);

    displayTexts();

    //Draw the scene.
    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
        glVertex3f(-6, 0, -6);
        glVertex3f(-6, 0, 6);
        glVertex3f(6, 0, 6);
        glVertex3f(6, 0, -6);
    glEnd();

    //Draw the left and right supports.
    for (int i = -1; i <= 1; i += 2) {
        glPushMatrix();
            glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
            glTranslatef(i*4, 2, 2);
            glScalef(0.2, 4, 0.2);
            glutSolidCube(1);
            glTranslatef(0, 0, -20);
            glutSolidCube(1);
            glScalef(5, 0.25, 5);
            glTranslatef(0, 2, 2);
            glScalef(0.2, 0.2, 4);
            glutSolidCube(1.05);
        glPopMatrix();
    }

    //Draw the top support.
    glPushMatrix();
        glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
        glTranslatef(0, 4, 0);
        glScalef(8, 0.2, 0.2);
        glutSolidCube(1.02);
    glPopMatrix();

    int step = 50;
    float offset = step/2 + step/2*(numberOfBalls-2);
    int idx = 0;

    //Draw the wires and the balls
    glPushMatrix();
        glScalef(0.02, 1, 0.02);
        glTranslatef(0, 4, 0);

        //Draw the left moving balls if they exist.
        for (int i = 0; i < usedBalls && ballDeg <= 0; i++) {
            idx = 1;
            glPushMatrix();
                glTranslatef(-offset+i*step, 0, 0);
                drawWireAndBall(ballDeg < 0, 0);
            glPopMatrix();
        }

        int borneInf = 0, borneSup = 0;
        if (ballDeg > 0) {
            borneInf = -offset;
            borneSup = offset-usedBalls*step;
        } else {
            borneInf = -offset+usedBalls*step;
            borneSup = offset;
        }

        //Draw the non moving balls.
        for (int i = borneInf; i <= borneSup; i+= step) {
            glPushMatrix();
                glTranslatef(i, 0, 0);
                drawWireAndBall(false, idx);
                idx++;
            glPopMatrix();
        }

        //Draw the right moving balls if they exist.
        for (int i = 0; i < usedBalls && ballDeg > 0; i++) {
            glPushMatrix();
                glTranslatef(offset-i*step, 0, 0);
                drawWireAndBall(ballDeg > 0, numberOfBalls-1);
            glPopMatrix();
        }
    glPopMatrix();

    glFlush();
}

//Handles the drawing of the wire and ball and their corresponding animation.
void drawWireAndBall(bool animate, int ballNumber) {
    if (animate) {
        glScalef(50, 1, 50);
        glRotatef(ballDeg, 0, 0, 1);
        glScalef(0.02, 1, 0.02);
    }
    glTranslatef(0, -0.5, 0);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glutSolidCube(1.10);
    glTranslatef(0, -1, 0);
    glScalef(50, 1, 50);
    setBallColor(ballNumber);
    glutSolidSphere(0.5, 100, 100);
    if (animate) glTranslatef(0, -0.5, 0);
}

//Sets the color corresponding to the given ball index.
void setBallColor(int idx) {
    if (!showEnergyPosition) glColor4f(0.5, 0.5, 0.5, 1.0f);
    else if (idx == 0) {
        if (ballDeg < 0 && !transmittingEnergy) {
            glColor4f(1.0f, 0.1f, 0.1f, 1.0f);
        } else {
            glColor4f(0.5, 0.5, 0.5, 1.0f);
        }
    } else if (idx == numberOfBalls-1) {
        if (ballDeg > 0 && !transmittingEnergy) {
            glColor4f(1.0f, 0.1f, 0.1f, 1.0f);
        } else {
            glColor4f(0.5, 0.5, 0.5, 1.0f);
        }
    } else {
        if (states[idx-1] || (idx >= 2 && states[idx-2]) || (idx < 5 && states[idx])) {
            glColor4f(1.0f, 0.1f, 0.1f, 1.0f);
        } else {
            glColor4f(0.5, 0.5, 0.5, 1.0f);
        }
    }
}

//Resets the counters for the color animation.
void resetCounters(void) {
    for (int i = 0; i < numberOfBalls-2; i++) {
        counts[i] = 0;
        states[i] = false;
    }
}


void handleFriction(int coef) {
    if (applyFriction) {
        usedMaxBallDeg += frictionEnergyReduction;
        ballDeg = coef*usedMaxBallDeg;
        if (usedMaxBallDeg <= 0) {
            doIdle = false;
            resetCounters();
            ballDeg = 0;
        }
    }
}


void idle(void) {
    if (!doIdle) return;
    if (ballDeg <= -usedMaxBallDeg) {
        handleFriction(-1);
        ballStep = 0.3;
        resetCounters();
    } else if (ballDeg >= usedMaxBallDeg) {
        handleFriction(1);
        ballStep = -0.3;
        resetCounters();
    }
    if (!doIdle) {
        glutPostRedisplay();
        return;
    }

    float val = 0;
    if (ballStep < 0) {
        if (applyGravity) {
            if (ballDeg > 0) ballStep *= gravityEnergyAmplification;
            else ballStep *= gravityEnergyReduction;
        }
        val = -ballStep;
    } else {
        if (applyGravity) {
            if (ballDeg < 0) ballStep *= gravityEnergyAmplification;
            else ballStep *= gravityEnergyReduction;
        }
        val = ballStep;
    }

    //To avoid the speed dropping considerably and keep having a smooth animation.
    if (ballStep >= -0.3 && ballStep < 0) ballStep = -0.3;
    if (ballStep <= 0.3 && ballStep > 0) ballStep = 0.3;

    //Loops spent on each intermediate ball (the non moving ones) for energy transfer.
    int limit = 5;
    transmittingEnergy = true;
    if (ballDeg >= -val && ballDeg < 0 && usedBalls < numberOfBalls/2) {
        for (int i = 0; i < numberOfBalls-2; i++) {
            if (counts[i] > 0) states[i] = true;
        }

        if (numberOfBalls >= 3 && counts[0] < limit) counts[0]++;
        else if (numberOfBalls >= 4 && counts[1] < limit) counts[1]++;
        else if (numberOfBalls >= 5 && counts[2] < limit) counts[2]++;
        else if (numberOfBalls >= 6 && counts[3] < limit) counts[3]++;
        else if (numberOfBalls >= 7 && counts[4] < limit) counts[4]++;
        else ballDeg = val*1.1;

        for (int i = 0; i < numberOfBalls-2; i++) {
            if (counts[i] == limit) states[i] = false;
        }
    } else if (ballDeg >= 0 && ballDeg < val && usedBalls < numberOfBalls/2) {
        for (int i = 0; i < numberOfBalls-2; i++) {
            if (counts[i] > 0) states[i] = true;
        }

        if (numberOfBalls >= 7 && counts[4] < limit) counts[4]++;
        else if (numberOfBalls >= 6 && counts[3] < limit) counts[3]++;
        else if (numberOfBalls >= 5 && counts[2] < limit) counts[2]++;
        else if (numberOfBalls >= 4 && counts[1] < limit) counts[1]++;
        else if (numberOfBalls >= 3 && counts[0] < limit) counts[0]++;
        else ballDeg = -val*1.1;

        for (int i = 0; i < numberOfBalls-2; i++) {
            if (counts[i] == limit) states[i] = false;
        }
    } else {
        transmittingEnergy = false;
        ballDeg += ballStep;
    }

    glutPostRedisplay();
}

void specialKey(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP : {
            rDx += 10.0f;
            glutPostRedisplay();
            break;
        }
        case GLUT_KEY_DOWN : {
            rDx -= 10.0f;
            glutPostRedisplay();
            break;
        }
        case GLUT_KEY_RIGHT : {
            rDy += 10.0f;
            glutPostRedisplay();
            break;
        }
        case GLUT_KEY_LEFT : {
            rDy -= 10.0f;
            glutPostRedisplay();
            break;
        }
    }
}


void reset(void) {
    doIdle = false;
    transmittingEnergy = false;
    usedMaxBallDeg = maxBallDeg;
    ballDeg = -usedMaxBallDeg;
    ballStep = -0.3;
    resetCounters();
}


void keyPressed(unsigned char touche, int x, int y) {
    switch (touche) {
        case '+' : {
            if (numberOfBalls < 7) numberOfBalls++;
            glutPostRedisplay();
            break;
        }
        case '-' : {
            if (numberOfBalls > 2 && usedBalls < numberOfBalls) numberOfBalls--;
            glutPostRedisplay();
            break;
        }
        case ' ': {
            doIdle = !doIdle;
            break;
        }
        case 'N':
        case 'n': {
            reset();
            break;
        }
        case 'F':
        case 'f': {
            applyFriction = !applyFriction;
            break;
        }
        case 'C':
        case 'c': {
            showEnergyPosition = !showEnergyPosition;
            break;
        }
        case '1': {
            usedBalls = 1;
            reset();
            break;
        }
        case '2': {
            if (numberOfBalls >= 2) usedBalls = 2;
            reset();
            break;
        }
        case '3': {
            if (numberOfBalls >= 3) usedBalls = 3;
            reset();
            break;
        }
        case '4': {
            if (numberOfBalls >= 4) usedBalls = 4;
            reset();
            break;
        }
        case '5': {
            if (numberOfBalls >= 5) usedBalls = 5;
            reset();
            break;
        }
        case '6': {
            if (numberOfBalls >= 6) usedBalls = 6;
            reset();
            break;
        }
        case '7': {
            if (numberOfBalls >= 7) usedBalls = 7;
            reset();
            break;
        }
        case 27: {
            exit(EXIT_SUCCESS);
        }
    }
    glutPostRedisplay();
}
