#include <windows.h>
#include <GL/glut.h>
#include <cmath>
#include <vector>
#define PI   acos(-1)
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

// �����H
float position[3] = { 0.0, 0.0, 0.0 };   // �����H��m
float self_ang = 0.0;                    // ���ਤ��
float glob_ang = 0.0;
float robot_width = 3.0, robot_length = 2.0;   // �����H���e
float step = 0.3;                              // �C�B�j�p
float swing = 4.0;                             // ���u�t��
float swing_angle = 40.0;                      // �̤j���u����
float relbow_joint_angle[4] = { 0, 1, 0, 0 };
float lelbow_joint_angle[4] = { 0, 1, 0, 0 };
float rhand_joint_angle[4] = { 0, 1, 0, 0 };
float lhand_joint_angle[4] = { 0, 1, 0, 0 };
float rfoot_joint_angle = 0, lfoot_joint_angle = 0;
float rfinger_joint_angle = 45, lfinger_joint_angle = -45;
float rknee_joint_angle = 0, lknee_joint_angle = 0;
float head_angle = 0;
int dir = 1;                // �\�ʤ�V��
int finger_dir = 1;         // ����\�ʤ�V
bool follow_robot = false;  // �O�_���H�����H����
bool is_exploding = false;  // �O�_�z�����A
double robotX = 0.0, robotY = 0.0;
bool isHoldingLollipop = false;   // �O�_���δο}
bool is_enlarged = false;

// ����
int width = 800, height = 600;

// �l�u
const float bullet_speed = 1.0;   // �l�u�t��

GLUquadricObj* sphere = NULL;
GLUquadricObj* disk;


struct Bullet {
    float x, y, z;
    float dx, dz;
};

struct Obstacle {
    float x, y, z; // ��ê����m
    float length, width, height; // ��ê���j�p
};
struct Lollipop {
    double x, z;
    bool isPickedUp = false;
};


std::vector<Bullet> bullets;
Lollipop lollipop = { 5.0, -5.0 }; // �δο}��l��m
Obstacle obstacles[NUM_OBSTACLES] = {
    {5.0, 3.0, -20.0, 4.0, 3.0, 6.0},
    {15.0, 3.0, -10.0, 5.0, 3.0, 5.0},
    {15.0, 3.0, -30.0, 6.0, 2.0, 4.0},
    {30.0, 3.0, -30.0, 7.0, 4.0, 6.0}
};

// �ˬd�����H�O�_�b�δο}�d��
bool isWithinRange(double robotX, double robotZ, double lollipopX, double lollipopZ) {
    double dist = std::sqrt(
        (robotX - lollipopX) * (robotX - lollipopX) +
        (robotZ - lollipopZ) * (robotZ - lollipopZ)
    );
    return dist <= 8;
}

// �C��]�w
float robot_colors[7][3] = { {0.4, 0.5, 0.8}, {0.2, 0.3, 0.6}, {0.3, 0.4, 0.7},
                             {0.1, 0.2, 0.5}, {0.5, 0.6, 0.9}, {0.6, 0.7, 1.0}, {0.7, 0.8, 1.0} };
float obstacle_colors[6][3] = { {0.3, 0.3, 0.3}, {0.4, 0.4, 0.4}, {0.5, 0.5, 0.6},
                                {0.35, 0.35, 0.45}, {0.6, 0.6, 0.7}, {0.45, 0.45, 0.45} };


float  points[][3] = { {-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5},
                      {0.5, 0.5, -0.5}, {-0.5, 0.5, -0.5},
                      {-0.5, -0.5, 0.5}, {0.5, -0.5, 0.5},
                      {0.5, 0.5, 0.5}, {-0.5, 0.5, 0.5} };

int face[][4] = { {0, 3, 2, 1}, {0, 1, 5, 4}, {1, 2, 6, 5},
                  {4, 5, 6, 7}, {2, 3, 7, 6}, {0, 4, 7, 3} };
int cube[6] = { 0, 1, 2, 3, 4, 5 };

void  myinit()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);

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

}

void draw_cube(float colors[6][3])
{
    for (int i = 0; i < 6; i++) {
        glColor3fv(colors[i]);
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
    float gridSize = SIZE / FLOOR_NUMBER_OF_GRID; // �p��C�Ӯ�l���j�p
    // �p��a�O���_�l�I 
    float offset = FLOOR_NUMBER_OF_GRID / 2.0;

    for (i = 0; i < FLOOR_NUMBER_OF_GRID; i++) {
        for (j = 0; j < FLOOR_NUMBER_OF_GRID; j++) {
            if ((i + j) % 2 == 0)
                glColor3f(0.8, 0.8, 0.8);
            else
                glColor3f(0.5, 0.5, 0.5);

            glBegin(GL_POLYGON);
            glVertex3f((i - offset) * gridSize, 0, (j - offset) * gridSize);
            glVertex3f((i - offset) * gridSize, 0, (j - offset + 1) * gridSize);
            glVertex3f((i - offset + 1) * gridSize, 0, (j - offset + 1) * gridSize);
            glVertex3f((i - offset + 1) * gridSize, 0, (j - offset) * gridSize);
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
        glTranslatef(obstacles[i].x, obstacles[i].y, obstacles[i].z);
        glScalef(obstacles[i].length, obstacles[i].height, obstacles[i].width);
        draw_cube(obstacle_colors);
        glPopMatrix();
    }
}

void draw_rectangle(double x, double y, double z) {
    glPushMatrix();
    glTranslatef(0, -y / 2, 0);
    glScalef(x, y, z);
    draw_cube(robot_colors);
    glPopMatrix();
}
void draw_head() {
    glPushMatrix();
    glRotatef(head_angle, 0, 1, 0);
    glColor3fv(robot_colors[6]);
    draw_rectangle(1.3, 1.3, 1.3);
    glPopMatrix();
}

void draw_finger(float finger_joint_angle) {
    glPushMatrix();
    glRotatef(finger_joint_angle, 0, 0, 1);
    glColor3fv(robot_colors[6]);
    draw_rectangle(0.3, 1, 0.3);
    glPopMatrix();
}

void draw_hand(float* hand_joint_angle, float* elbow_joint_angle) {
    glColor3fv(robot_colors[6]);

    glPushMatrix();
    glRotatef(hand_joint_angle[0], hand_joint_angle[1], hand_joint_angle[2], hand_joint_angle[3]);
    glutSolidSphere(0.5, 20.0, 20.0); //�ӻH
    draw_rectangle(0.5, 1.5, 0.5); //�W�u

    glTranslatef(0, -1.5, 0);
    glPushMatrix();
    glColor3fv(robot_colors[6]);
    glRotatef(45, 1, 0, 0); // ��y�s��
    glRotatef(elbow_joint_angle[0], elbow_joint_angle[1], elbow_joint_angle[2], elbow_joint_angle[3]);
    glutSolidSphere(0.5, 20.0, 20.0); //��y
    draw_rectangle(0.5, 1.5, 0.5); //�U�u

    glTranslatef(0, -1.5, 0);
    glColor3fv(robot_colors[6]);
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
    glColor3fv(robot_colors[6]);
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
void updateView() {
    if (follow_robot) {
        gluLookAt(
            position[0] + 10.0 * cos(glob_ang),   // �۾�X�y��
            position[1] + 10.0 * sin(glob_ang) + 4.0,   // �۾�Y�y��
            position[2],                  // �۾�Z�y�С]�����۾����ס^
            position[0],                         // �ؼ�X�y��
            position[1],                         // �ؼ�Y�y��
            position[2],                         // �ؼ�Z�y��
            0.0, 1.0, 0.0);                      // �W��V
    }
    else {
        // ��_�쥻����
        gluLookAt(10.0, 10.0, 20.0,   // �۾���m
            0.0, 0.0, 0.0,     // �ؼЦ�m
            0.0, 1.0, 0.0);    // �W��V
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

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}
void draw_robot() {
    // ���ʱ����Ӿ����H
    glTranslatef(position[0], position[1], position[2]);
    glRotatef(self_ang, 0.0, 1.0, 0.0);

    // ����
    glPushMatrix();
    glTranslatef(0.0, 5.0, 0.0);
    glScalef(3.0, 4.0, 2.0);
    draw_cube(robot_colors);
    glPopMatrix();

    // �Y
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

void display()
{
    static float  ang_self = 0.0;
    static float  angle = 0.0;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    updateView();

    // �e�δο}
    draw_lollipop(lollipop.x, lollipop.z, 0.0);

    // �e�a�O
    draw_floor();

    // �e��ê��
    draw_obstacle();

    // �e�l�u
    update_bullets();

    if (is_exploding) {
        //update_and_draw_explosion();
    }
    else {
        // ���`��ܾ����H
        draw_robot();
    }


    glutSwapBuffers();
    return;
}

void my_reshape(int w, int h)
{
    width = w, height = h;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w > h)
        glOrtho(-40.0, 40.0, -40.0 * (float)h / (float)w, 40.0 * (float)h / (float)w, 0.0, 120);
    else
        glOrtho(-40.0 * (float)w / (float)h, 40.0 * (float)w / (float)h, -40.0, 40.0, 0.0, 120);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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


int prev_key; // �W�@������
void my_quit(unsigned char key, int x, int y)
{
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

    else if (key == 'g') { // g ��
        if (rfinger_joint_angle == 0 || rfinger_joint_angle == 45) finger_dir *= -1;
        rfinger_joint_angle += (5 * finger_dir);
        lfinger_joint_angle += (5 * -1 * finger_dir);
    }


    else if (key == 'j') {
        squat();
        jump();
    }
    else if (key == 'k') {
        squat();
    }
    else if (key == 'l') {
        wave();
    }
    else if (key == 'z') {
        follow_robot = !follow_robot;
    }
    else if (key == 'x') {
        dance();
    }
    else if (key == ' ') {
        shoot_bullet();
    }
    else if (key == 'c') {
        //isBaseballMode = true;
        //initialize_game();
    }
    else if (key == 'p') {
        //swing_bat();  // ����
    }
    else if (key == 'q') {
        if (head_angle >= 40 || head_angle <= -40) {
            dir *= -1;
        }
        head_angle += dir * 5;
    }
    else if (key == 'v') {
        if (isWithinRange(position[0], position[2], lollipop.x, lollipop.z)) {
            lollipop.isPickedUp = true;
        }
        else {
            lollipop.isPickedUp = false;
        }
    }
    display();
    prev_key = int(key);
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
    glutMainLoop();
}

