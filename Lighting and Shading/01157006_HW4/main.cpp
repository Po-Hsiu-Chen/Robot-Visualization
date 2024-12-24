#include <windows.h>
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <array>
#include <chrono>
using namespace std;
using namespace std::chrono;
#include "light.hpp"
#define PI   acos(-1)
#define M_PI 3.14159265358979323846
#define SIZE  100
#define FLOOR_NUMBER_OF_GRID 20
#define NUM_OBSTACLES 4
#define NORMAL_MODE 0   
#define ROBOT_MODE 1    
#define WALK_MODE 0     
#define RUN_MODE 1      
#define RWAVE_MODE 0    
#define LWAVE_MODE 1    
#define WAVE_MODE 2 
#define NUM_OBSTACLES 4
GLUquadricObj* sphere = NULL;
GLUquadricObj* disk;
/*----------------------------�����H--------------------------*/
// �����H��m�P����
float position[3] = { 0.0, 0.0, 0.0 };  // �����H��m
float self_ang = 0.0;                   // �����H���ਤ��
float glob_ang = 0.0;                   // �����H��������

// �����H�ؤo�P�ʧ@
float robot_width = 3.0, robot_length = 2.0;  // �����H�e�׻P����
float step = 0.3;                             // �C�B�j�p
float swing = 4.0;                            // ���u�t��
float swing_angle = 40.0;                     // �̤j���u����

// ���`����
float rhand_joint_angle[4] = { 0, 1, 0, 0 };
float lhand_joint_angle[4] = { 0, 1, 0, 0 };
float relbow_joint_angle[4] = { 0, 1, 0, 0 };
float lelbow_joint_angle[4] = { 0, 1, 0, 0 };
float rfoot_joint_angle = 0, lfoot_joint_angle = 0;
float rfinger_joint_angle = 45, lfinger_joint_angle = -45;
float rknee_joint_angle = 0, lknee_joint_angle = 0;
float head_angle = 0;

// �ʧ@���A
int dir = 1;                // �\�ʤ�V
int finger_dir = 1;         // ����\�ʤ�V
bool is_exploding = false;  // �O�_�z�����A
bool isHoldingLollipop = false;  // �O�_���δο}
bool is_enlarged = false;   // �O�_��j


/*---------------------------�۾��P����-------------------------*/
float eyeDx = 0.0, eyeDy = 0.0, eyeDz = 0.0;  // �۾����ʰ����q
float eyeAngx = 0.0, eyeAngy = 0.0, eyeAngz = 0.0; // �۾����ਤ��
float eye[3] = { 0.0, 10.0, 30.0 };          // �۾���m
float u[3][3] = { {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0} };
float   cv, sv; /* cos(5.0) and sin(5.0) */

#define SPLIT_VIEW 5
float zoomFactor = 1.5;                      // �Y����
float fovy = 30.0, nearDist = 5.0, farDist = 100.0f; // �z����v�Ѽ�
bool mode_viewVolume = false;                // �O�_��ܵ�����
int mode_orient = NORMAL_MODE;               // ���ϼҦ�
double eqn[] = { 0.0, 1.0, 0.0, 10.0 };      // �ŵ�����
int viewMode = SPLIT_VIEW;
int style = 0; // 0: �|����, 1~3: �����v, 4: �z����v

/*----------------------------�����j�p---------------------------------*/
int width = 800, height = 800;

/*----------------------------��ê��-------------------------------*/
struct Obstacle {
    float x, y, z; // ��ê����m
    float length, width, height; // ��ê���j�p
};
Obstacle obstacles[NUM_OBSTACLES] = {
    {-30.0, 10, -20.0, 4.0, 3.0, 20},
    {-20.0, 10, -20.0, 5.0, 3.0, 20},
    {-10.0, 10, -20.0, 6.0, 2.0, 20},
    {0.0, 10, -20.0, 7.0, 4.0, 20}
};
bool isWithinRange(double robotX, double robotZ, double lollipopX, double lollipopZ) { //�ˬd�����H�O�_�b�δο}�d��
    double dist = std::sqrt(
        (robotX - lollipopX) * (robotX - lollipopX) +
        (robotZ - lollipopZ) * (robotZ - lollipopZ)
    );
    return dist <= 8;
}
/*----------------------------�l�u-------------------------------*/
const float bullet_speed = 1.0;   // �l�u�t��
struct Bullet {
    float x, y, z;
    float dx, dz;
};
std::vector<Bullet> bullets;

/*----------------------------�δο}------------------------------*/
struct Lollipop {
    double x, z;
    bool isPickedUp = false;
};
Lollipop lollipop = { 5.0, -5.0 }; // �δο}��l��m


/*-------------------------------�C��]�w-------------------------*/
float robot_colors[3] = { 0.4, 0.5, 0.8 };
float obstacle_colors[3] = { 0.678, 0.847, 0.902 };
float  points[][3] = { {-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5},
                      {0.5, 0.5, -0.5}, {-0.5, 0.5, -0.5},
                      {-0.5, -0.5, 0.5}, {0.5, -0.5, 0.5},
                      {0.5, 0.5, 0.5}, {-0.5, 0.5, 0.5} };



float norm2(float v[]);

int face[][4] = { {0, 3, 2, 1}, {0, 1, 5, 4}, {1, 2, 6, 5},
                  {4, 5, 6, 7}, {2, 3, 7, 6}, {0, 4, 7, 3} };
int cube[6] = { 0, 1, 2, 3, 4, 5 };


struct rotate_node
{
    GLfloat x, y, z;
    rotate_node() {} // �w�]�c�y�禡
    rotate_node(GLfloat _x, GLfloat _y, GLfloat _z) {
        x = _x; y = _y; z = _z;
    }
};




/*---------------------------���������Ѽ�-------------------------*/
// �������
float global_ambient[] = { 0.2f, 0.2f, 0.2f, 0.0f }; // ���ҥ�

// ��V��
float sun_pos[] = { 0.0, 1.0, 0.0, 0.0 };         // �Ӷ���m
float sun_light_dir[] = { -1.0f, -0.5f, -1.0f, 0.0f };  // ���u��V
float sun_light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float sun_light_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
int sun_on = 0;

float dir_light_dir[] = { -1.0f, -0.5f, -1.0f, 0.0f };  // ���u��V
float dir_light_diffuse[] = { 1.0, 0.9, 0.7, 1.0 }; 
float dir_light_ambient[] = { 0.3, 0.25, 0.2, 1.0 }; 
float dir_light_specular[] = { 1.0, 0.9, 0.7, 1.0 }; 

// �I����
float point_light_pos[] = { -10.0, 20.0, -20.0, 1.0 };
float point_light_diffuse[] = { 0.5, 0.7, 1.0, 1.0 };
float point_light_specular[] = { 1.0, 0.8, 0.8, 1.0 };
int current_color_mode = 0;

// �E���O
float spot_light_pos[] = { 0.0f, 2.0f, 0.0f, 1.0f };
float spot_light_dir[] = { 0.0f, -1.0f, 0.0f };
float spot_light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float spot_light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
float spot_light_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
float spot_light_cutoff = 40.0f;        // �E���O�I������
float spot_exponent = 8.0f;       // ���������{��
float spot_attenuation = 0.05f;   // �E���O�I��
int dir_light_on = 1;             // ��V���}��
int point_light_on = 1;           // �I�����}��
int spot_light_on = 1;            // �E���O�}��

// ����
float mat_shininess = 64.0; // ������A��

/*---------------------------�ɶ������Ѽ�-------------------------*/
float time1 = 0.0;               // �����ɶ��A�d�� 0 ~ 1
float clockTime = 15000;              // ���������ɶ�
int lastSysTime;                      // �t�ήɶ�
float clockVarying = 3600;            // �����t��

rotate_node rotate(GLfloat x, GLfloat y, GLfloat z) {
    // y
    GLfloat x1 = x * cos(eyeAngy * PI / 180.0) + z * sin(eyeAngy * PI / 180.0);
    GLfloat y1 = y;
    GLfloat z1 = -x * sin(eyeAngy * PI / 180.0) + z * cos(eyeAngy * PI / 180.0);

    // x
    GLfloat x2 = x1;
    GLfloat y2 = y1 * cos(eyeAngx * PI / 180.0) - z1 * sin(eyeAngx * PI / 180.0);
    GLfloat z2 = y1 * sin(eyeAngx * PI / 180.0) + z1 * cos(eyeAngx * PI / 180.0);

    // z
    GLfloat x3 = x2 * cos(eyeAngz * PI / 180.0) - y2 * sin(eyeAngz * PI / 180.0);
    GLfloat y3 = x2 * sin(eyeAngz * PI / 180.0) + y2 * cos(eyeAngz * PI / 180.0);
    GLfloat z3 = z2;

    // result
    return rotate_node(x3, y3, z3);
}

void draw_light_sphere(float* position, float* color) {
    glPushMatrix();
    glTranslatef(position[0], position[1], position[2]); 

    // �]�m���誺�o���ݩ�
    float emission[] = { color[0], color[1], color[2], 1.0 }; 
    float default_emission[] = { 0.0, 0.0, 0.0, 1.0 }; 

    glMaterialfv(GL_FRONT, GL_EMISSION, emission); 
    glColor3fv(color);
    glutSolidSphere(0.5, 20, 20); 

    glMaterialfv(GL_FRONT, GL_EMISSION, default_emission);
    glPopMatrix();
}
void draw_light_spheres() {
    if (point_light_on) {
        if (current_color_mode == 1) {
            float point_light_color[] = { 1.0, 0.5, 0.5 }; // ��
            draw_light_sphere(point_light_pos, point_light_color);
        }
        else {
            float point_light_color[] = { 0.5, 0.5, 1.0 }; // ��
            draw_light_sphere(point_light_pos, point_light_color);
        }
    }
    if (spot_light_on) {
        float spot_light_color[] = { 0.5, 0.5, 1.0 }; 
        float spot_light_pos[] = { position[0], position[1] + 9.0, position[2] ,1.0 }; // ��۾����H
        glLightfv(GL_LIGHT2, GL_POSITION, spot_light_pos);
        draw_light_sphere(spot_light_pos, spot_light_color);
    }
}
void set_material(float* ambient, float* diffuse, float* specular, float shininess) {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}
void draw_goldMat() {
    float ambient[] = { 0.8, 0.8, 0.8, 1.0 };  
    float diffuse[] = { 0.6, 0.6, 0.6, 1.0 };  
    float specular[] = { 1.0, 1.0, 1.0, 1.0 }; 
    float shininess = 64.0;   
    set_material(ambient, diffuse, specular, shininess);

}
static void set_atmosphereMat() {
    float ambient[] = { 1.5f, 1.5f, 1.7f, 1.0f };
    float diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    float specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float shininess = 15;

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, eqn);

    switch (style) {
    case 1: // X��V�������
    {
        double eqn1[4] = { -1, 0, 0, 30 };  
        glClipPlane(GL_CLIP_PLANE1, eqn1);
        glEnable(GL_CLIP_PLANE1);
    }
    break;

    case 2: // Y��V�������
    {
        double eqn1[4] = { 0, -1, 0, 30 }; 
        glClipPlane(GL_CLIP_PLANE1, eqn1);
        glEnable(GL_CLIP_PLANE1);
    }
    break;

    case 3: // Z��V�������
    {
        double eqn1[4] = { 0, 0, -1, 30 };  
        glClipPlane(GL_CLIP_PLANE1, eqn1);
        glEnable(GL_CLIP_PLANE1);
    }
    break;

    default: // ������

        break;
    }
    GLUquadricObj* sphere = gluNewQuadric();
    gluQuadricDrawStyle(sphere, GLU_FILL);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere, 70, 32, 32);

    glDisable(GL_CLIP_PLANE0); // �T�ΰŵ�����
}

void normalize(float vec[3]) {
    float length = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    vec[0] /= length;
    vec[1] /= length;
    vec[2] /= length;
}

void idle_func() {
    int curSysTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() % 100000000;
    float dt = (curSysTime - lastSysTime) / 1000.0;
    lastSysTime = curSysTime;
    clockTime += clockVarying * dt;
    if (clockTime > 86400) clockTime -= 86400;
}
static void setSUNLights() {
    float sunElevation = (clockTime - 21600) / 240;
    float narrowElevation = (clockTime > 43200 ? 270 - clockTime / 240 : clockTime / 240 - 90);
    std::cout << "narrowElevation: " << narrowElevation << std::endl;
    if (narrowElevation < -5.5) {
        global_ambient[0] = 0.03;
        global_ambient[1] = 0.03;
        global_ambient[2] = 0.05;
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    }
    else {
        float sunElevation_cos = cos(sunElevation * PI / 180);
        float sunElevation_sin = sin(sunElevation * PI / 180);
        sun_pos[0] = sunElevation_cos;
        sun_pos[1] = sunElevation_sin;
        sun_light_diffuse[0] = 0.8 + sunElevation_sin * 0.1;
        sun_light_diffuse[1] = 0.4 + sunElevation_sin * 0.6;
        sun_light_diffuse[2] = 0.15 + sunElevation_sin * 0.85;
        if (narrowElevation < 0.5) {
            if (narrowElevation > -0.5)
                for (int i = 0; i < 3; i++)
                    sun_light_diffuse[i] *= (narrowElevation * 0.6 + 0.7);
            else
                for (int i = 0; i < 3; i++)
                    sun_light_diffuse[i] *= (narrowElevation + 5.5) * 0.06;
        }
        glEnable(GL_LIGHT4);
        glLightfv(GL_LIGHT4, GL_POSITION, sun_pos);
        glLightfv(GL_LIGHT4, GL_DIFFUSE, sun_light_diffuse);
        glLightfv(GL_LIGHT4, GL_SPECULAR, sun_light_specular);

        global_ambient[0] = sun_light_diffuse[0] * 0.3 + 0.05;
        global_ambient[1] = sun_light_diffuse[1] * 0.3 + 0.05;
        global_ambient[2] = sun_light_diffuse[2] * 0.32 + 0.08;
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    }
}

void setupLights() {
    float length = sqrt(spot_light_dir[0] * spot_light_dir[0] +
    spot_light_dir[1] * spot_light_dir[1] +
    spot_light_dir[2] * spot_light_dir[2]);

    spot_light_dir[0] /= length;
    spot_light_dir[1] /= length;
    spot_light_dir[2] /= length;
    
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST); 
    glEnable(GL_NORMALIZE);  
    glEnable(GL_LIGHTING);  
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);           /* local viewer */
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient); /*global ambient*/

    // ��V��
    glLightfv(GL_LIGHT0, GL_POSITION, dir_light_dir);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dir_light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, dir_light_specular);


    // �I����
    glLightfv(GL_LIGHT1, GL_POSITION, point_light_pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, point_light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, point_light_specular);

    // �E���O
    normalize(spot_light_dir);
    glLightfv(GL_LIGHT2, GL_POSITION, spot_light_pos);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, spot_light_diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, spot_light_specular);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_light_dir);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, spot_light_cutoff); 
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, spot_exponent);

    set_atmosphereMat();

}
void switch_light_color() {
    if (current_color_mode == 0) {
        // ����
        point_light_diffuse[0] = 1.0; point_light_diffuse[1] = 0.8; point_light_diffuse[2] = 0.8;
        point_light_specular[0] = 1.0; point_light_specular[1] = 0.8; point_light_specular[2] = 0.8;
    }
    else {
        // �ť�
        point_light_diffuse[0] = 0.5; point_light_diffuse[1] = 0.7; point_light_diffuse[2] = 1.0;
        point_light_specular[0] = 0.5; point_light_specular[1] = 0.7; point_light_specular[2] = 1.0;
    }

    // �����C��
    current_color_mode = 1 - current_color_mode;
    glLightfv(GL_LIGHT1, GL_DIFFUSE, point_light_diffuse);
    glutPostRedisplay(); 
}


void  myinit()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    cv = cos(5.0 * PI / 180.0);
    sv = sin(5.0 * PI / 180.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    if (sphere == NULL) {
        sphere = gluNewQuadric();
        gluQuadricDrawStyle(sphere, GLU_FILL);
        gluQuadricNormals(sphere, GLU_SMOOTH);
    }

    if (disk == NULL) {
        disk = gluNewQuadric();
        gluQuadricDrawStyle(disk, GLU_FILL);
        gluQuadricNormals(disk, GLU_SMOOTH);
    }

    glEnable(GL_LIGHTING);  
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    setupLights();  
}

void draw_cube(const float color[3]) {
    float ambient[] = { color[0] * 0.2f, color[1] * 0.2f, color[2] * 0.2f, 1.0f };
    float diffuse[] = { color[0] * 0.8f, color[1] * 0.8f, color[2] * 0.8f, 1.0f };
    float specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float shininess = 80.0f;
    set_material(ambient, diffuse, specular, shininess);

    // ø�s�ߤ��骺���ӭ�
    for (int i = 0; i < 6; i++) {
        glBegin(GL_POLYGON);
        glVertex3fv(points[face[i][0]]);
        glVertex3fv(points[face[i][1]]);
        glVertex3fv(points[face[i][2]]);
        glVertex3fv(points[face[i][3]]);
        glEnd();
    }
}


void draw_floor() {
    int i, j;
    for (i = -25; i < 25; i++) {
        for (j = -25; j < 25; j++) {
            float ambient[] = { 0.2, 0.2, 0.2, 1.0 };
            float diffuse[] = {
                (i + j) % 2 == 0 ? 0.7 : 0.5, // �ǥչ��
                (i + j) % 2 == 0 ? 0.7 : 0.5,
                (i + j) % 2 == 0 ? 0.7 : 0.5,
                1.0
            };
            float specular[] = { 0.0, 0.0, 0.0, 1.0 }; //���ϥ�
            float shininess = 0.0;

            set_material(ambient, diffuse, specular, shininess);

            glColor4f((i + j) % 2 == 0 ? 0.8 : 0.6, 
                (i + j) % 2 == 0 ? 0.8 : 0.6,
                (i + j) % 2 == 0 ? 0.8 : 0.6,
                1.0);

            glNormal3f(0.0, 1.0, 0.0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBegin(GL_POLYGON);
            glVertex3f(i * 2.0 + 0.0, 0, j * 2.0 + 0.0);
            glVertex3f(i * 2.0 + 0.0, 0, j * 2.0 + 2.0);
            glVertex3f(i * 2.0 + 2.0, 0, j * 2.0 + 2.0);
            glVertex3f(i * 2.0 + 2.0, 0, j * 2.0 + 0.0);
            glEnd();
        }
    }
}

void draw_lollipop(double x, double z, double angle = 0) {
    GLUquadricObj* cylinder = gluNewQuadric();
    GLUquadricObj* sphere = gluNewQuadric();

    glPushMatrix();
    // �δο}����m
    glTranslated(x, 0.85, z);
    glRotated(angle, 0, 1, 0);

    // �e�Τl
    glColor3f(1, 1, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gluCylinder(cylinder, 0.2, 0.2, 10, 20, 20);

    // �e�}
    glTranslated(0, 0, 10);
    glColor3f(1, 0, 0);
    gluSphere(sphere, 1.5, 20, 20); // �D�n
    glColor3f(1, 0.3, 0.3);
    glScaled(1, 1, 0.5);
    gluSphere(sphere, 1.7, 20, 20); // ��������

    glPopMatrix();

    gluDeleteQuadric(cylinder);
    gluDeleteQuadric(sphere);
}
void draw_obstacle() {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        glPushMatrix();
        glTranslatef(obstacles[i].x, obstacles[i].y , obstacles[i].z);
        glScalef(obstacles[i].length, obstacles[i].height, obstacles[i].width);
        draw_cube(obstacle_colors);  
        glPopMatrix();
    }
}

void draw_world_axes() {
    glLineWidth(2.0); // �]�m�b�u���e��

    // ø�s X �b�]����^
    glColor3f(1.0, 0.0, 0.0); // ����
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0); // �_�I�b�@�ɭ��I
    glVertex3f(20.0, 0.0, 0.0); // X �b����V
    glEnd();

    // ø�s Y �b�]���^
    glColor3f(0.0, 1.0, 0.0); // ���
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0); // �_�I�b�@�ɭ��I
    glVertex3f(0.0, 20.0, 0.0); // Y �b����V
    glEnd();

    // ø�s Z �b�]�Ŧ�^
    glColor3f(0.0, 0.0, 1.0); // �Ŧ�
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0); // �_�I�b�@�ɭ��I
    glVertex3f(0.0, 0.0, 20.0); // Z �b����V
    glEnd();
}


void draw_rectangle(double x, double y, double z) {
    glPushMatrix();
    draw_goldMat();
    glTranslatef(0, -y / 2, 0);
    glScalef(x, y, z);
    draw_cube(robot_colors);
    glPopMatrix();
}
void draw_head() {
    glPushMatrix();
    draw_goldMat();
    glRotatef(head_angle, 0, 1, 0);
    glColor3fv(robot_colors);
    draw_rectangle(1.3, 1.3, 1.3);
    glPopMatrix();
}

void draw_finger(float finger_joint_angle) {
    glPushMatrix();
    draw_goldMat();
    glRotatef(finger_joint_angle, 0, 0, 1);
    glColor3fv(robot_colors);
    draw_rectangle(0.3, 1, 0.3);
    glPopMatrix();
}

void draw_hand(float* hand_joint_angle, float* elbow_joint_angle) {
    glColor3fv(robot_colors);

    glPushMatrix();
    draw_goldMat();
    glRotatef(hand_joint_angle[0], hand_joint_angle[1], hand_joint_angle[2], hand_joint_angle[3]);
    glutSolidSphere(0.5, 20.0, 20.0); //�ӻH
    draw_rectangle(0.5, 1.5, 0.5); //�W�u

    glTranslatef(0, -1.5, 0);
    glPushMatrix();
    glColor3fv(robot_colors);
    glRotatef(45, 1, 0, 0); // ��y�s��
    glRotatef(elbow_joint_angle[0], elbow_joint_angle[1], elbow_joint_angle[2], elbow_joint_angle[3]);
    glutSolidSphere(0.5, 20.0, 20.0); //��y
    draw_rectangle(0.5, 1.5, 0.5); //�U�u

    glTranslatef(0, -1.5, 0);
    glColor3fv(robot_colors);
    glutSolidSphere(0.4, 20.0, 20.0); //���
    if (lollipop.isPickedUp) { // ���۴δο}
        glPushMatrix();
        glTranslatef(0.0, 0.0, 2.0);
        glRotatef(180, 1, 0, 0);
        draw_lollipop(0, 0, 0);
        glPopMatrix();
        draw_finger(25);// �����
        draw_finger(-25);// �k���
    }
    else {
        draw_finger(lfinger_joint_angle);// �����
        draw_finger(rfinger_joint_angle);// �k���
    }

    glPopMatrix();
    glPopMatrix();
}

void draw_foot(float foot_joint_angle, float knee_joint_angle) {
    glPushMatrix();
    glRotatef(foot_joint_angle, 1, 0, 0);
    draw_rectangle(0.7, 1.5, 0.7); //�j�L
    glTranslatef(0, -1.5, 0);
    glColor3fv(robot_colors);
    glutSolidSphere(0.5, 20.0, 20.0); //���\
    glRotatef(knee_joint_angle, 1, 0, 0);
    draw_rectangle(0.7, 1.5, 0.7); //�p�L
    glPopMatrix();
}

bool check_collision(float next_x, float next_z, float robot_width, float robot_length) {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        // ��ê�����
        float obstacle_min_x = obstacles[i].x - (obstacles[i].length / 2.0);
        float obstacle_max_x = obstacles[i].x + (obstacles[i].length / 2.0);
        float obstacle_min_z = obstacles[i].z - (obstacles[i].width / 2.0);
        float obstacle_max_z = obstacles[i].z + (obstacles[i].width / 2.0);

        // �����H���
        float robot_min_x = next_x - (robot_width / 2.0);
        float robot_max_x = next_x + (robot_width / 2.0);
        float robot_min_z = next_z - (robot_length / 2.0);
        float robot_max_z = next_z + (robot_length / 2.0);

        // �ˬd�O�_�����|�A�|���ˬd�O�_���|
        bool overlap_x = (robot_min_x <= obstacle_max_x) && (robot_max_x >= obstacle_min_x);
        bool overlap_z = (robot_min_z <= obstacle_max_z) && (robot_max_z >= obstacle_min_z);

        if (overlap_x && overlap_z) {
            return true; // �o�͸I��
        }
    }
    return false; // �L�I��
}
void make_view(int x)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    switch (x) {
    case 4: /* Perspective */
        gluLookAt(eye[0], eye[1], eye[2],
            eye[0] - u[2][0], eye[1] - u[2][1], eye[2] - u[2][2],
            u[1][0], u[1][1], u[1][2]);
        break;

    case 1: /* X direction parallel viewing */
        gluLookAt(50, 15.0, 0.0, 0.0, 15.0, 0.0, 0.0, 1.0, 0.0);
        break;

    case 2: /* Y direction parallel viewing */
        gluLookAt(0.0, 50, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
        break;

    case 3: /* Z direction parallel viewing */
        gluLookAt(0.0, 15.0, 50, 0.0, 15.0, 0.0, 0.0, 1.0, 0.0);
        break;
    }

}

// �o�g�l�u
void shoot_bullet() {
    Bullet new_bullet;
    new_bullet.x = position[0];
    new_bullet.y = position[1] + 4.0; // �l�u�_�l��m����������H
    new_bullet.z = position[2];
    new_bullet.dx = bullet_speed * sin(self_ang * PI / 180.0); // �l�u��V
    new_bullet.dz = bullet_speed * cos(self_ang * PI / 180.0);
    bullets.push_back(new_bullet);
}


// ��s�l�u��m
void update_bullets() {
    glColor3f(1.0, 0.0, 0.0);
    for (auto it = bullets.begin(); it != bullets.end();) {
        // ��s�l�u��m
        it->x -= it->dx;
        it->z -= it->dz;

        // �e�l�u
        glPushMatrix();
        glTranslatef(it->x, it->y, it->z);
        glutSolidSphere(0.3, 10, 10);
        glPopMatrix();

        // �ˬd�l�u�O�_�W�X�d��
        if (it->x > 100 || it->x < -100 || it->z > 100 || it->z < -100) {
            it = bullets.erase(it);
        }
        else {
            ++it;
        }
    }
}
void parallelProject() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10.0f * zoomFactor, 10.0f * zoomFactor, -10.0f * zoomFactor, 10.0f * zoomFactor, -1000.0f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void perspectiveProject() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovy * zoomFactor, (float)width / height, nearDist, farDist);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void draw_ViewVolume() {
    if (mode_viewVolume) {
        float aspectRatio = (float)(width) / (float)(height);  // �T�O���T�p��B�I�Ȫ����e��
        float nearHeight = 2.0f * nearDist * tanf(fovy * zoomFactor * M_PI / 360.0f); // �񥭭�����
        float nearWidth = nearHeight * aspectRatio;  // �񥭭��e��
        float farHeight = 2.0f * farDist * tanf(fovy * zoomFactor * M_PI / 360.0f);  // ����������
        float farWidth = farHeight * aspectRatio;  // �������e��

        glPushMatrix();

        // �N�[��̪���m�]���@�骺�_�I
        glTranslatef(eye[0], eye[1], eye[2]);
        glRotatef(eyeAngy, 0.0f, 1.0f, 0.0f);  // �u Y �b����]Yaw�^
        glRotatef(eyeAngx, 1.0f, 0.0f, 0.0f);  // �u X �b����]Pitch�^
        glRotatef(eyeAngz, 0.0f, 0.0f, 1.0f);  // �u Z �b����]Roll�^

        glColor3f(1.0f, 1.0f, 1.0f); // �զ�u��

        // ø�s�񥭭����x�ή�
        glBegin(GL_LINE_LOOP);
        glVertex3f(-nearWidth / 2, nearHeight / 2, -nearDist); // ���W
        glVertex3f(nearWidth / 2, nearHeight / 2, -nearDist);  // �k�W
        glVertex3f(nearWidth / 2, -nearHeight / 2, -nearDist); // �k�U
        glVertex3f(-nearWidth / 2, -nearHeight / 2, -nearDist);// ���U
        glEnd();

        // ø�s���������x�ή�
        glBegin(GL_LINE_LOOP);
        glVertex3f(-farWidth / 2, farHeight / 2, -farDist); // ���W
        glVertex3f(farWidth / 2, farHeight / 2, -farDist);  // �k�W
        glVertex3f(farWidth / 2, -farHeight / 2, -farDist); // �k�U
        glVertex3f(-farWidth / 2, -farHeight / 2, -farDist); // ���U
        glEnd();

        // ø�s�q�[��̨�񥭭��|�ӳ��I���s���u
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);  // �[��̦�m
        glVertex3f(-nearWidth / 2, nearHeight / 2, -nearDist); // �񥭭����W
        glVertex3f(0.0f, 0.0f, 0.0f);  // �[��̦�m
        glVertex3f(nearWidth / 2, nearHeight / 2, -nearDist);  // �񥭭��k�W
        glVertex3f(0.0f, 0.0f, 0.0f);  // �[��̦�m
        glVertex3f(nearWidth / 2, -nearHeight / 2, -nearDist); // �񥭭��k�U
        glVertex3f(0.0f, 0.0f, 0.0f);  // �[��̦�m
        glVertex3f(-nearWidth / 2, -nearHeight / 2, -nearDist); // �񥭭����U
        glEnd();

        // ø�s�q�[��̨컷�����|�ӳ��I���s���u
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);  // �[��̦�m
        glVertex3f(-farWidth / 2, farHeight / 2, -farDist); // ���������W
        glVertex3f(0.0f, 0.0f, 0.0f);  // �[��̦�m
        glVertex3f(farWidth / 2, farHeight / 2, -farDist);  // �������k�W
        glVertex3f(0.0f, 0.0f, 0.0f);  // �[��̦�m
        glVertex3f(farWidth / 2, -farHeight / 2, -farDist); // �������k�U
        glVertex3f(0.0f, 0.0f, 0.0f);  // �[��̦�m
        glVertex3f(-farWidth / 2, -farHeight / 2, -farDist); // ���������U
        glEnd();

        // ø�s���������|�ӳ��I�������s�u�]�Φ��x�ήء^
        glBegin(GL_LINES);
        glVertex3f(-farWidth / 2, farHeight / 2, -farDist); // ���W
        glVertex3f(farWidth / 2, farHeight / 2, -farDist);  // �k�W
        glVertex3f(farWidth / 2, farHeight / 2, -farDist);  // �k�W
        glVertex3f(farWidth / 2, -farHeight / 2, -farDist); // �k�U
        glVertex3f(farWidth / 2, -farHeight / 2, -farDist); // �k�U
        glVertex3f(-farWidth / 2, -farHeight / 2, -farDist);// ���U
        glVertex3f(-farWidth / 2, -farHeight / 2, -farDist);// ���U
        glVertex3f(-farWidth / 2, farHeight / 2, -farDist); // ���W
        glEnd();
        glPopMatrix();
    }

}


void draw_view()
{
    glMatrixMode(GL_MODELVIEW);

    /*----Draw Eye position-----*/
    glPushMatrix();
    glTranslatef(eye[0], eye[1], eye[2]);
    glColor3f(0.0, 1.0, 0.0);
    glutWireSphere(1.0, 10, 10);
    glPopMatrix();

    /*----Draw eye coord. axes -----*/
    glColor3f(1.0, 1.0, 0.0); // Draw Xe
    glBegin(GL_LINES);
    glVertex3f(eye[0], eye[1], eye[2]);
    glVertex3f(eye[0] + 20.0 * u[0][0], eye[1] + 20.0 * u[0][1], eye[2] + 20.0 * u[0][2]);
    glEnd();

    glColor3f(1.0, 0.0, 1.0); // Draw Ye
    glBegin(GL_LINES);
    glVertex3f(eye[0], eye[1], eye[2]);
    glVertex3f(eye[0] + 20.0 * u[1][0], eye[1] + 20.0 * u[1][1], eye[2] + 20.0 * u[1][2]);
    glEnd();

    glColor3f(0.0, 1.0, 1.0); // Draw Ze
    glBegin(GL_LINES);
    glVertex3f(eye[0], eye[1], eye[2]);
    glVertex3f(eye[0] + 20.0 * u[2][0], eye[1] + 20.0 * u[2][1], eye[2] + 20.0 * u[2][2]);
    glEnd();


}

void timer(int value) {
    float deltaTime = 0.016f; // �� 60 FPS ���ɶ����j (16ms)
    //update(deltaTime);       // ��s�Ӷ������A
    glutPostRedisplay();     // Ĳ�o�e����ø
    glutTimerFunc(16, timer, 0); // �]�m�U�@���w�ɾ��^��
}

void draw_robot() {
    
    // ���ʱ����Ӿ����H
    glTranslatef(position[0], position[1], position[2]);
    glRotatef(self_ang, 0.0, 1.0, 0.0);

    // ����
    glPushMatrix();
    glTranslatef(0.0, 5.0, 0.0);
    glScalef(3.0, 4.0, 2.0);
    draw_cube(robot_colors);  // �ϥ� robot_colors[0] �C��
    glPopMatrix();

    // �Y��
    glPushMatrix();
    glTranslatef(0.0, 8.5, 0.0);
    draw_head();
    glPopMatrix();

    // ����
    glPushMatrix();
    glTranslatef(-1.8, 6.5, 0.0);
    draw_hand(lhand_joint_angle, lelbow_joint_angle);
    glPopMatrix();

    // �k��
    glPushMatrix();
    glTranslatef(1.8, 6.5, 0.0);
    draw_hand(rhand_joint_angle, relbow_joint_angle);
    glPopMatrix();

    // ���}
    glPushMatrix();
    glTranslatef(-0.75, 3.0, 0.0);
    draw_foot(lfoot_joint_angle, lknee_joint_angle);
    glPopMatrix();

    // �k�}
    glPushMatrix();
    glTranslatef(0.75, 3.0, 0.0);
    draw_foot(rfoot_joint_angle, rknee_joint_angle);
    glPopMatrix();
}


void draw_scene() {
    draw_world_axes();  // �e�@�ɧ��Шt
    draw_floor();
    draw_obstacle();
    draw_lollipop(lollipop.x, lollipop.z);
    draw_robot();
}
void display() {
    // �M���C��M�`�׽w�İ�
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // �]�m�ҫ����ϯx�}
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // ��s����
    update_bullets();

    if (sun_on) {
        setSUNLights();
    }
    

    // �ھڷ�e�˦����ø�s�Ҧ�
    switch (style) {
    case 0: // �|���f�Ҧ�
        // �k�U�����]�z����v�^
        perspectiveProject();
        glViewport(width / 2, 0, width / 2, height / 2);
        make_view(4);
        draw_view();
        setupLights();
        draw_light_spheres();
        draw_scene();


        // ���W�����]X��V��������^
        parallelProject();
        glViewport(0, height / 2, width / 2, height / 2);
        make_view(1);
        draw_view();
        draw_ViewVolume();
        setupLights();
        draw_light_spheres();
		draw_scene();

        // �k�W�����]Y��V��������^
        parallelProject();
        glViewport(width / 2, height / 2, width / 2, height / 2);
        make_view(2);
        draw_view();
        draw_ViewVolume();
        setupLights();
        draw_light_spheres();
        draw_scene();

        // ���U�����]Z��V��������^
        parallelProject();
        glViewport(0, 0, width / 2, height / 2);
        make_view(3);
        draw_view();
        draw_ViewVolume();
        setupLights();
        draw_light_spheres();
        draw_scene();
        break;

    case 1: // X��V�����������
        parallelProject();
        glViewport(0, 0, width, height);
        make_view(1);
        draw_view();
        draw_ViewVolume();
        setupLights();
        draw_light_spheres();
        draw_scene();
        break;

    case 2: // Y��V�����������
        parallelProject();
        glViewport(0, 0, width, height);
        make_view(2);
        draw_view();
        draw_ViewVolume();
        setupLights();
        draw_light_spheres();
        draw_scene();
        break;

    case 3: // Z��V�����������
        parallelProject();
        glViewport(0, 0, width, height);
        make_view(3);
        draw_view();
        draw_ViewVolume();
        setupLights();
        draw_light_spheres();
        draw_scene();
        break;

    case 4: // �z����v����
        perspectiveProject();
        glViewport(0, 0, width, height);
        make_view(4);
        draw_view();
        setupLights();
        draw_light_spheres();
        draw_scene();
        break;
    }

    // �洫�e��w�İ�
    glutSwapBuffers();
}

void reset() {

    //motion
    dir = 1, finger_dir = 1;

    // hand
    rhand_joint_angle[0] = 0, rhand_joint_angle[1] = 1, rhand_joint_angle[2] = 0, rhand_joint_angle[3] = 0;
    lhand_joint_angle[0] = 0, lhand_joint_angle[1] = 1, lhand_joint_angle[2] = 0, lhand_joint_angle[3] = 0;
    relbow_joint_angle[0] = 0, relbow_joint_angle[1] = 1, relbow_joint_angle[2] = 0, relbow_joint_angle[3] = 0;
    lelbow_joint_angle[0] = 0, lelbow_joint_angle[1] = 1, lelbow_joint_angle[2] = 0, lelbow_joint_angle[3] = 0;
    rfinger_joint_angle = 45, lfinger_joint_angle = -45;

    //foot
    rfoot_joint_angle = 0, lfoot_joint_angle = 0;
    rknee_joint_angle = 0, lknee_joint_angle = 0;

    // y_axis
    position[1] = 0;
}
void reset_dance() {


    // hand
    rhand_joint_angle[0] = 0, rhand_joint_angle[1] = 0, rhand_joint_angle[2] = 0, rhand_joint_angle[3] = 1;
    lhand_joint_angle[0] = 0, lhand_joint_angle[1] = 0, lhand_joint_angle[2] = 0, lhand_joint_angle[3] = 1;
    relbow_joint_angle[0] = 0, relbow_joint_angle[1] = 0, relbow_joint_angle[2] = 0, relbow_joint_angle[3] = 1;
    lelbow_joint_angle[0] = 0, lelbow_joint_angle[1] = 0, lelbow_joint_angle[2] = 0, lelbow_joint_angle[3] = 1;

    //foot
    rfoot_joint_angle = 0, lfoot_joint_angle = 0;
    rknee_joint_angle = 0, lknee_joint_angle = 0;

    // y_axis
    position[1] = 0;
}

void squat() {
    reset();
    rknee_joint_angle = -70, lknee_joint_angle = -70;
    rfoot_joint_angle = 40, lfoot_joint_angle = 40;
    position[1] -= 1.5 * sin(40);
    display();
    Sleep(200);
    reset();
}

void jump() {
    for (int i = 0; i < 5; i++) {
        position[1] += 1;
        display();
        Sleep(30);
    }
    for (int i = 0; i < 5; i++) {
        position[1] -= 1;
        display();
        Sleep(30);
    }
}

void set_moving(int mode_move) {
    if (mode_move == WALK_MODE) {
        step = 0.2; // �樫�t��
        swing = 3; // ���u�t��
        swing_angle = 40; // ���u�T��
    }
    else if (mode_move == RUN_MODE) {
        step = 0.5;
        swing = 10;
        swing_angle = 70;
    }

    // �T�O�}���\�ʨ��פ��|�W�L�ثe�Ҧ������u�T��
    if (rfoot_joint_angle > swing_angle || lfoot_joint_angle < -swing_angle) {
        rfoot_joint_angle = swing_angle;
        lfoot_joint_angle = -swing_angle;
        rhand_joint_angle[0] = -swing_angle;
        lhand_joint_angle[0] = swing_angle;
    }
    if (rfoot_joint_angle < -swing_angle || lfoot_joint_angle > swing_angle) {
        rfoot_joint_angle = -swing_angle;
        lfoot_joint_angle = swing_angle;
        rhand_joint_angle[0] = swing_angle;
        lhand_joint_angle[0] = -swing_angle;
    }

}

void move_forward(int mode_move) {
    set_moving(mode_move);

    // �w���U�@�B����m
    float next_step_sin = step * sin(self_ang * PI / 180.0);
    float next_step_cos = step * cos(self_ang * PI / 180.0);
    float next_x = position[0] - next_step_sin;
    float next_z = position[2] - next_step_cos;

    // �ˬd�I��
    if (!check_collision(next_x, next_z, robot_width, robot_length)) {
        // �S���I���A��s��m
        position[0] = next_x;
        position[2] = next_z;
    }

    // ��}�\�ʤ�V
    if (rfoot_joint_angle >= swing_angle || rfoot_joint_angle <= -swing_angle)
        dir *= -1;

    // �\�ʨ���
    rfoot_joint_angle += (swing * dir);
    lfoot_joint_angle += (-swing * dir);
    rhand_joint_angle[0] += (-swing * dir);
    lhand_joint_angle[0] += (swing * dir);
}
void move_back(int mode_move) {
    set_moving(mode_move);

    // �U�@�B��m
    float next_step_sin = step * sin(self_ang * PI / 180.0);
    float next_step_cos = step * cos(self_ang * PI / 180.0);
    float next_x = position[0] + next_step_sin;
    float next_z = position[2] + next_step_cos;

    // �ˬd�I��
    if (!check_collision(next_x, next_z, robot_width, robot_length)) {
        // �S���I���A��s��m
        position[0] = next_x;
        position[2] = next_z;
    }

    // ��}�\�ʤ�V
    if (rfoot_joint_angle >= swing_angle || rfoot_joint_angle <= -swing_angle)
        dir *= -1;

    // �\�ʨ���
    rfoot_joint_angle -= (swing * dir);
    lfoot_joint_angle -= (swing * -1 * dir);
    rhand_joint_angle[0] -= (swing * -1 * dir);
    lhand_joint_angle[0] -= (swing * dir);
}

void wave()
{
    reset();
    for (int i = 0; i < 5; i++) {
        rhand_joint_angle[0] += 18;
        display();
        Sleep(20);
    }
    relbow_joint_angle[1] = 0, relbow_joint_angle[2] = 0, relbow_joint_angle[3] = 1;
    for (int i = 0; i < 5; i++) {
        relbow_joint_angle[0] += 9;
        display();
        Sleep(20);
    }
    for (int i = 0; i < 10; i++) {
        relbow_joint_angle[0] -= 9;
        display();
        Sleep(20);
    }
    for (int i = 0; i < 10; i++) {
        relbow_joint_angle[0] += 9;
        display();
        Sleep(20);
    }
    for (int i = 0; i < 10; i++) {
        relbow_joint_angle[0] -= 9;
        display();
        Sleep(20);
    }
    for (int i = 0; i < 5; i++) {
        relbow_joint_angle[0] += 9;
        display();
        Sleep(20);
    }
    for (int i = 0; i < 5; i++) {
        rhand_joint_angle[0] -= 18;
        display();
        Sleep(20);
    }
}
void dance() {
    reset_dance();
    for (int i = 0; i < 5; i++) {
        rhand_joint_angle[0] += 18;
        lhand_joint_angle[0] -= 18;
        display();
        Sleep(100);
    }
    squat();
    reset_dance();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 5; j++) {
            rhand_joint_angle[0] += 10 * j;
            relbow_joint_angle[0] -= 10 * j;
            display();
            Sleep(100);
        }
        for (int j = 0; j < 5; j++) {
            rhand_joint_angle[0] += -70 + 10 * j;
            relbow_joint_angle[0] -= 70 - 10 * j;
            display();
            Sleep(100);
        }
        squat();
        reset_dance();
        for (int j = 0; j < 5; j++) {
            lhand_joint_angle[0] -= 10 * j;
            lelbow_joint_angle[0] += 10 * j;
            display();
            Sleep(100);
        }
        for (int j = 0; j < 5; j++) {
            lhand_joint_angle[0] -= -70 + 10 * j;
            lelbow_joint_angle[0] += 70 - 10 * j;
            display();
            Sleep(100);
        }
        squat();
        reset_dance();
    }

    for (int k = 0; k < 3; k++) {
        for (int j = 0; j < 5; j++) {
            rhand_joint_angle[0] -= 10 * j;
            lhand_joint_angle[0] -= 10 * j;
            relbow_joint_angle[0] += 10 * j;
            lelbow_joint_angle[0] += 10 * j;
            display();
            Sleep(50);
        }
        for (int j = 0; j < 5; j++) {
            rhand_joint_angle[0] -= 70 - 10 * j;
            lhand_joint_angle[0] -= 70 - 10 * j;
            relbow_joint_angle[0] += 70 - 10 * j;
            lelbow_joint_angle[0] += 70 - 10 * j;
            display();
            Sleep(50);
        }
        squat();
        reset_dance();
    }

    // ���m�Ҧ����`����
    rhand_joint_angle[0] = rhand_joint_angle[1] = rhand_joint_angle[2] = 0;
    lhand_joint_angle[0] = lhand_joint_angle[1] = lhand_joint_angle[2] = 0;
    relbow_joint_angle[0] = relbow_joint_angle[1] = relbow_joint_angle[2] = 0;
    lelbow_joint_angle[0] = lelbow_joint_angle[1] = lelbow_joint_angle[2] = 0;
    display();
}

void removeObstacle(int i) {
    // �Ni��ê�����j�p�]���s (��ê������)
    obstacles[i].length = 0.0;
    obstacles[i].width = 0.0;
    obstacles[i].height = 0.0;
}

int prev_key; // �W�@������
void my_quit(unsigned char key, int ix, int iy)
{
    int    i;
    float  x[3], y[3], z[3];
    float step = 0.1f;
    /*-------------�����H----------------*/
    if (int(key) == 27) { // ESC ���}
        exit(0);
    }
    if (key == 'w') { // w �V�e��
        move_forward(WALK_MODE);
    }
    else if (int(key) == 87) { // shift + w �V�e�]
        move_forward(RUN_MODE);
    }
    else if (key == 's') { // s �V�ᨫ
        move_back(WALK_MODE);
    }
    else if (key == 83) { // shift + s �V��]
        move_back(RUN_MODE);
    }
    else if (key == 'a') { // a ����
        self_ang += 10.0;
    }
    else if (key == 'd') { // d �k��
        self_ang -= 10.0;
    }
    else if (key == 'z') { // g ��
        if (rfinger_joint_angle == 0 || rfinger_joint_angle == 45) finger_dir *= -1;
        rfinger_joint_angle += (5 * finger_dir);
        lfinger_joint_angle += (5 * -1 * finger_dir);
    }
    else if (key == 'x') {
        squat();
        jump();
    }
    else if (key == 'c') {
        squat();
    }
    else if (key == 'v') {
        dance();
    }
    else if (key == ' ') {
        shoot_bullet();
    }

    else if (key == 'n') {
        if (head_angle >= 40 || head_angle <= -40) {
            dir *= -1;
        }
        head_angle += dir * 5;
    }
    else if (key == 'm') {
        if (isWithinRange(position[0], position[2], lollipop.x, lollipop.z)) {
            lollipop.isPickedUp = true;
        }
        else {
            lollipop.isPickedUp = false;
        }
    }

    /*------transform the EYE coordinate system ------*/
    else if (key == 'f') {
        eyeDy += 0.5;       /* move up */
        for (i = 0; i < 3; i++) eye[i] -= 0.5 * u[1][i];
    }
    else if (key == 'F') {
        eyeDy += -0.5;       /* move down */
        for (i = 0; i < 3; i++) eye[i] += 0.5 * u[1][i];
    }
    else if (key == 'g') {
        eyeDx += -0.5;       /* move left */
        for (i = 0; i < 3; i++) eye[i] += 0.5 * u[0][i];
    }
    else if (key == 'G') {
        eyeDx += 0.5;        /* move right */
        for (i = 0; i < 3; i++) eye[i] -= 0.5 * u[0][i];
    }
    else if (key == 'h') {
        eyeDz += 0.5;        /* zoom in */
        for (i = 0; i < 3; i++) eye[i] -= 0.5 * u[2][i];
    }
    else if (key == 'H') {
        eyeDz += -0.5;       /* zoom out */
        for (i = 0; i < 3; i++) eye[i] += 0.5 * u[2][i];
    }
    else if (key == 'j') {             /* pitching */
        eyeAngx += 5.0;
        if (eyeAngx > 360.0) eyeAngx -= 360.0;
        y[0] = u[1][0] * cv - u[2][0] * sv;
        y[1] = u[1][1] * cv - u[2][1] * sv;
        y[2] = u[1][2] * cv - u[2][2] * sv;

        z[0] = u[2][0] * cv + u[1][0] * sv;
        z[1] = u[2][1] * cv + u[1][1] * sv;
        z[2] = u[2][2] * cv + u[1][2] * sv;

        for (i = 0; i < 3; i++) {
            u[1][i] = y[i];
            u[2][i] = z[i];
        }
    }
    else if (key == 'J') {
        eyeAngx += -5.0;
        if (eyeAngx < 0.0) eyeAngx += 360.0;
        y[0] = u[1][0] * cv + u[2][0] * sv;
        y[1] = u[1][1] * cv + u[2][1] * sv;
        y[2] = u[1][2] * cv + u[2][2] * sv;

        z[0] = u[2][0] * cv - u[1][0] * sv;
        z[1] = u[2][1] * cv - u[1][1] * sv;
        z[2] = u[2][2] * cv - u[1][2] * sv;

        for (i = 0; i < 3; i++) {
            u[1][i] = y[i];
            u[2][i] = z[i];
        }
    }
    else if (key == 'k') {            /* heading */
        eyeAngy += 5.0;
        if (eyeAngy > 360.0) eyeAngy -= 360.0;
        for (i = 0; i < 3; i++) {
            x[i] = cv * u[0][i] - sv * u[2][i];
            z[i] = sv * u[0][i] + cv * u[2][i];
        }
        for (i = 0; i < 3; i++) {
            u[0][i] = x[i];
            u[2][i] = z[i];
        }
    }
    else if (key == 'K') {
        eyeAngy += -5.0;
        if (eyeAngy < 0.0) eyeAngy += 360.0;
        for (i = 0; i < 3; i++) {
            x[i] = cv * u[0][i] + sv * u[2][i];
            z[i] = -sv * u[0][i] + cv * u[2][i];
        }
        for (i = 0; i < 3; i++) {
            u[0][i] = x[i];
            u[2][i] = z[i];
        }
    }
    else if (key == 'l') {            /* rolling */
        eyeAngz += 5.0;
        if (eyeAngz > 360.0) eyeAngz -= 360.0;
        for (i = 0; i < 3; i++) {
            x[i] = cv * u[0][i] - sv * u[1][i];
            y[i] = sv * u[0][i] + cv * u[1][i];
        }
        for (i = 0; i < 3; i++) {
            u[0][i] = x[i];
            u[1][i] = y[i];
        }
    }
    else if (key == 'L') {
        eyeAngz += -5.0;
        if (eyeAngz < 0.0) eyeAngz += 360.0;
        for (i = 0; i < 3; i++) {
            x[i] = cv * u[0][i] + sv * u[1][i];
            y[i] = -sv * u[0][i] + cv * u[1][i];
        }
        for (i = 0; i < 3; i++) {
            u[0][i] = x[i];
            u[1][i] = y[i];
        }
    }
    else if (key == '4') {
        style = 4;
    }
    else if (key == '0') {
        style = 0;
    }
    else if (key == '1') {
        style = 1;
    }
    else if (key == '2') {
        style = 2;
    }
    else if (key == '3') {
        style = 3;
    }
    else if (int(key) == 9) {
        if (mode_viewVolume == true) {
            mode_viewVolume = false;
        }
        else if (mode_viewVolume == false) {
            mode_viewVolume = true;
        }
    }
    else if (key == 'p') { // Zoom In
        zoomFactor += 0.1;
    }
    else if (key == 'P') { // Zoom Out
        zoomFactor -= 0.1;
    }
    else if (key == '6') {
        sun_on = !sun_on;
        if (sun_on) glEnable(GL_LIGHT4);
        else glDisable(GL_LIGHT4);
    }
    else if (key == '7'){
		dir_light_on = !dir_light_on;
		if (dir_light_on) glEnable(GL_LIGHT0);
		else glDisable(GL_LIGHT0);
	}
	else if (key == '8'){
		point_light_on = !point_light_on;
		if (point_light_on) glEnable(GL_LIGHT1);
		else glDisable(GL_LIGHT1);
	}
	else if (key == '9') {// �����E���O�}��
		spot_light_on = !spot_light_on;
		if (spot_light_on) {
			glEnable(GL_LIGHT2);
		}
		else { 
			glDisable(GL_LIGHT2); 
		}
	}
    else if (key == 'E') {                   // �W�[�I�����j��
		for (int i = 0; i < 3; i++) point_light_diffuse[i] += 0.1;
		glLightfv(GL_LIGHT1, GL_DIFFUSE, point_light_diffuse);
	}
	else if (key == 'e') {                     // ����I�����j��
		for (int i = 0; i < 3; i++) point_light_diffuse[i] -= 0.1;
		glLightfv(GL_LIGHT1, GL_DIFFUSE, point_light_diffuse);
	}
    else if (key == 'R') {                      // �W�[�E���O�I������
		spot_light_cutoff += 5.0;
		if (spot_light_cutoff > 90.0) spot_light_cutoff = 90.0;
		glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, spot_light_cutoff);
		printf("%f", spot_light_cutoff);
	}
	else if (key == 'r') {                     // ��ֻE���O�I������
		spot_light_cutoff -= 5.0;
		if (spot_light_cutoff < 10.0) spot_light_cutoff = 10.0;
		glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, spot_light_cutoff);
	}
	else if (key == 'T') {                      // �W�[�E���O�j��
		for (int i = 0; i < 3; i++) spot_light_diffuse[i] += 1.1;
		glLightfv(GL_LIGHT2, GL_DIFFUSE, spot_light_diffuse);
	}
	else if (key == 't') {                     // ��ֻE���O�j��
		for (int i = 0; i < 3; i++) spot_light_diffuse[i] -= 1.1;
		glLightfv(GL_LIGHT2, GL_DIFFUSE, spot_light_diffuse);
	}
	else if (key == 'q') {
		switch_light_color();
	}
    else if (key == 'Y') {
        spot_light_dir[2] -= step; 
    }
    else if (key == 'y') {
        spot_light_dir[2] += step;
    }
    display();
    prev_key = int(key);
}

void my_reshape(int w, int h)
{
    glViewport(0, 0, w, h);  // �]�m���f
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (w > h) {
        // �e�j�󰪡A�O�� X �b�d��T�w�A�վ� Y �b
        glOrtho(-10.0, 10.0, -10.0 * (float)h / (float)w, 10.0 * (float)h / (float)w, 0.50, 30.0);
    }
    else {
        // ���j��ε���e�A�O�� Y �b�d��T�w�A�վ� X �b
        glOrtho(-10.0 * (float)w / (float)h, 10.0 * (float)w / (float)h, -10.0, 10.0, 0.50, 30.0);
    }

    width = w, height = h;
}


void main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("robot");
    myinit();
    glutDisplayFunc(display);
    glutReshapeFunc(my_reshape);
    glutKeyboardFunc(my_quit);
    glutTimerFunc(0, timer, 0); // �Ұʩw�ɾ�
    glutIdleFunc(idle_func);
    glutMainLoop();
}