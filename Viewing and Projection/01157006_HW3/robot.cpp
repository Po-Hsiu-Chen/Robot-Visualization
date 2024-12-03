#include <windows.h>
#include <GL/glut.h>
#include <cmath>
#include <vector>
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
/*--------------機器人------------*/
float position[3] = { 0.0, 0.0, 0.0 };   // 機器人位置
float self_ang = 0.0;                    // 旋轉角度
float glob_ang = 0.0;
float robot_width = 3.0, robot_length = 2.0;   // 機器人長寬
float step = 0.3;                              // 每步大小
float swing = 4.0;                             // 揮臂速度
float swing_angle = 40.0;                      // 最大揮臂角度
float relbow_joint_angle[4] = { 0, 1, 0, 0 };
float lelbow_joint_angle[4] = { 0, 1, 0, 0 };
float rhand_joint_angle[4] = { 0, 1, 0, 0 };
float lhand_joint_angle[4] = { 0, 1, 0, 0 };
float rfoot_joint_angle = 0, lfoot_joint_angle = 0;
float rfinger_joint_angle = 45, lfinger_joint_angle = -45;
float rknee_joint_angle = 0, lknee_joint_angle = 0;
float head_angle = 0;
int dir = 1;                // 擺動方向
int finger_dir = 1;         // 手指擺動方向
bool is_exploding = false;  // 是否爆炸狀態
double robotX = 0.0, robotY = 0.0;
bool isHoldingLollipop = false;   // 是否抓住棒棒糖
bool is_enlarged = false;

/*--------------eye coordinate system------------*/
float   eyeDx = 0.0, eyeDy = 0.0, eyeDz = 0.0;
float   eyeAngx = 0.0, eyeAngy = 0.0, eyeAngz = 0.0;
float   u[3][3] = { {1.0,0.0,0.0}, {0.0,1.0,0.0}, {0.0,0.0,1.0} };
float   eye[3] = { 0.0, 10.0, 30.0 };
float   cv, sv; /* cos(5.0) and sin(5.0) */
bool mode_viewVolume = false;
int mode_orient = NORMAL_MODE; 

/*------------視窗大小--------------------*/
int width = 800, height = 800;

/*---------------障礙物------------------*/
struct Obstacle {
    float x, y, z; // 障礙物位置
    float length, width, height; // 障礙物大小
    bool exploded;          // 是否已爆炸
};
Obstacle obstacles[NUM_OBSTACLES] = {
    {-10.0, 3.0, -25.0, 4.0, 3.0, 6.0, false},
    {0.0, 3.0, -15.0, 5.0, 3.0, 5.0, false},
    {0.0, 3.0, -30.0, 6.0, 2.0, 4.0, false},
    {15.0, 3.0, -30.0, 7.0, 4.0, 6.0, false}
};
bool isWithinRange(double robotX, double robotZ, double lollipopX, double lollipopZ) { //檢查機器人是否在棒棒糖範圍內
    double dist = std::sqrt(
        (robotX - lollipopX) * (robotX - lollipopX) +
        (robotZ - lollipopZ) * (robotZ - lollipopZ)
    );
    return dist <= 8;
}
/*---------------子彈------------------*/
const float bullet_speed = 1.0;   // 子彈速度
struct Bullet {
    float x, y, z;
    float dx, dz;
};
std::vector<Bullet> bullets;

/*---------------棒棒糖------------------*/
struct Lollipop {
    double x, z;
    bool isPickedUp = false;
};
Lollipop lollipop = { 5.0, -5.0 }; // 棒棒糖初始位置

/*---------------爆炸------------------*/
int explosionCount = 0;      // 目前爆炸次數
bool isExploding = false;    // 是否正在爆炸
float explosionTime = 2.0f; // 正方體存活時間
int explosionIndex = 0; // 用於追蹤爆炸的次數
int cubeCount = 20; // 爆炸時生成的小正方體數量
struct Cube { // 報炸的小塊
    float position[3];
    float velocity[3];
    float lifeTime; // 正方體存在的時間
};
std::vector<Cube> cubes;


/*-----顏色設定-----*/
float robot_colors[7][3] = { {0.4, 0.5, 0.8}, {0.2, 0.3, 0.6}, {0.3, 0.4, 0.7},
                             {0.1, 0.2, 0.5}, {0.5, 0.6, 0.9}, {0.6, 0.7, 1.0}, {0.7, 0.8, 1.0} };
float obstacle_colors[6][3] = { {0.3, 0.3, 0.3}, {0.4, 0.4, 0.4}, {0.5, 0.5, 0.6},
                                {0.35, 0.35, 0.45}, {0.6, 0.6, 0.7}, {0.45, 0.45, 0.45} };
float  points[][3] = { {-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5},
                      {0.5, 0.5, -0.5}, {-0.5, 0.5, -0.5},
                      {-0.5, -0.5, 0.5}, {0.5, -0.5, 0.5},
                      {0.5, 0.5, 0.5}, {-0.5, 0.5, 0.5} };

/*-----Drawing stye
 0:4-windows, 1:x direction, 2:y direction, 3:z-dirtection, 4:perspective
 */
int style = 0;
float norm2(float v[]);

int face[][4] = { {0, 3, 2, 1}, {0, 1, 5, 4}, {1, 2, 6, 5},
                  {4, 5, 6, 7}, {2, 3, 7, 6}, {0, 4, 7, 3} };
int cube[6] = { 0, 1, 2, 3, 4, 5 };


struct rotate_node
{
    GLfloat x, y, z;
    rotate_node() {} // 預設構造函式
    rotate_node(GLfloat _x, GLfloat _y, GLfloat _z) {
        x = _x; y = _y; z = _z;
    }
};

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


float zoomFactor = 1.5;
#define SPLIT_VIEW 5
int viewMode = SPLIT_VIEW;
float fovy = 30.0, nearDist = 1.0, farDist = 100000.0f;
 

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
    float gridSize = SIZE / FLOOR_NUMBER_OF_GRID; // 計算每個格子的大小
    // 計算地板的起始點 
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
    // 棒棒糖的位置
    glTranslated(x, 0.85, z);
    glRotated(angle, 0, 1, 0);

    // 畫棒子
    glColor3f(1, 1, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gluCylinder(cylinder, 0.2, 0.2, 10, 20, 20);

    // 畫糖
    glTranslated(0, 0, 10);
    glColor3f(1, 0, 0);
    gluSphere(sphere, 1.5, 20, 20); // 主要
    glColor3f(1, 0.3, 0.3);
    glScaled(1, 1, 0.5);
    gluSphere(sphere, 1.7, 20, 20); // 中間那圈

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

void initializeExplosion() {
    if (explosionIndex >= NUM_OBSTACLES) return; // 如果超過最大次數，則不再爆炸

    cubes.clear(); // 清空之前的正方體
    const float speedMultiplier = 150.0f; // 爆炸速度

    // 獲取當前爆炸的障礙物位置
    float startX = obstacles[explosionIndex].x;
    float startY = obstacles[explosionIndex].y;
    float startZ = obstacles[explosionIndex].z;

    for (int i = 0; i < cubeCount; ++i) {
        Cube cube;

        // 設置初始位置（當前障礙物的位置）
        cube.position[0] = startX;
        cube.position[1] = startY;
        cube.position[2] = startZ;

        // 設置隨機速度方向和大小
        cube.velocity[0] = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * speedMultiplier;
        cube.velocity[1] = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * speedMultiplier;
        cube.velocity[2] = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * speedMultiplier;

        cube.lifeTime = explosionTime; // 設置生存時間

        cubes.push_back(cube);
    }

    // 標記當前障礙物已被爆炸，並移動到下一個
    obstacles[explosionIndex].exploded = true;
    explosionIndex++;
    isExploding = true;
}

void draw_world_axes() {
    glLineWidth(2.0); // 設置軸線的寬度

    // 繪製 X 軸（紅色）
    glColor3f(1.0, 0.0, 0.0); // 紅色
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0); // 起點在世界原點
    glVertex3f(20.0, 0.0, 0.0); // X 軸正方向
    glEnd();

    // 繪製 Y 軸（綠色）
    glColor3f(0.0, 1.0, 0.0); // 綠色
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0); // 起點在世界原點
    glVertex3f(0.0, 20.0, 0.0); // Y 軸正方向
    glEnd();

    // 繪製 Z 軸（藍色）
    glColor3f(0.0, 0.0, 1.0); // 藍色
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0); // 起點在世界原點
    glVertex3f(0.0, 0.0, 20.0); // Z 軸正方向
    glEnd();
}

void updateAndDrawFragments() {
    if (isExploding) {
        bool allFragmentsGone = true;

        for (auto& cube : cubes) {
            if (cube.lifeTime > 0) {
                cube.position[0] += cube.velocity[0] * 0.02f; // 調整速度
                cube.position[1] += cube.velocity[1] * 0.02f;
                cube.position[2] += cube.velocity[2] * 0.02f;

                cube.lifeTime -= 0.05f;

                // 繪製爆炸方塊
                glPushMatrix();
                glTranslatef(cube.position[0], cube.position[1], cube.position[2]);
                draw_cube(obstacle_colors); // 使用預設顏色
                glPopMatrix();

                allFragmentsGone = false;
            }
        }

        // 所有碎片消失後停止爆炸狀態
        if (allFragmentsGone) {
            isExploding = false;
        }
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
    glutSolidSphere(0.5, 20.0, 20.0); //肩膀
    draw_rectangle(0.5, 1.5, 0.5); //上臂

    glTranslatef(0, -1.5, 0);
    glPushMatrix();
    glColor3fv(robot_colors[6]);
    glRotatef(45, 1, 0, 0); // 手肘彎曲
    glRotatef(elbow_joint_angle[0], elbow_joint_angle[1], elbow_joint_angle[2], elbow_joint_angle[3]);
    glutSolidSphere(0.5, 20.0, 20.0); //手肘
    draw_rectangle(0.5, 1.5, 0.5); //下臂

    glTranslatef(0, -1.5, 0);
    glColor3fv(robot_colors[6]);
    glutSolidSphere(0.4, 20.0, 20.0); //手腕
    if (lollipop.isPickedUp) { // 拿著棒棒糖
        glPushMatrix();
        glTranslatef(0.0, 0.0, 2.0);
        glRotatef(180, 1, 0, 0);
        draw_lollipop(0, 0, 0);
        glPopMatrix();
        draw_finger(25);// 左手指
        draw_finger(-25);// 右手指
    }
    else {
        draw_finger(lfinger_joint_angle);// 左手指
        draw_finger(rfinger_joint_angle);// 右手指
    }

    glPopMatrix();
    glPopMatrix();
}

void draw_foot(float foot_joint_angle, float knee_joint_angle) {
    glPushMatrix();
    glRotatef(foot_joint_angle, 1, 0, 0);
    draw_rectangle(0.7, 1.5, 0.7); //大腿
    glTranslatef(0, -1.5, 0);
    glColor3fv(robot_colors[6]);
    glutSolidSphere(0.5, 20.0, 20.0); //膝蓋
    glRotatef(knee_joint_angle, 1, 0, 0);
    draw_rectangle(0.7, 1.5, 0.7); //小腿
    glPopMatrix();
}

bool check_collision(float next_x, float next_z, float robot_width, float robot_length) {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        // 障礙物邊界
        float obstacle_min_x = obstacles[i].x - (obstacles[i].length / 2.0);
        float obstacle_max_x = obstacles[i].x + (obstacles[i].length / 2.0);
        float obstacle_min_z = obstacles[i].z - (obstacles[i].width / 2.0);
        float obstacle_max_z = obstacles[i].z + (obstacles[i].width / 2.0);

        // 機器人邊界
        float robot_min_x = next_x - (robot_width / 2.0);
        float robot_max_x = next_x + (robot_width / 2.0);
        float robot_min_z = next_z - (robot_length / 2.0);
        float robot_max_z = next_z + (robot_length / 2.0);

        // 檢查是否有重疊，四邊檢查是否重疊
        bool overlap_x = (robot_min_x <= obstacle_max_x) && (robot_max_x >= obstacle_min_x);
        bool overlap_z = (robot_min_z <= obstacle_max_z) && (robot_max_z >= obstacle_min_z);

        if (overlap_x && overlap_z) {
            return true; // 發生碰撞
        }
    }
    return false; // 無碰撞
}
void make_view(int x)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    switch (x) {
    case 4:       /* Perspective */
        gluLookAt(eye[0], eye[1], eye[2],
            eye[0] - u[2][0], eye[1] - u[2][1], eye[2] - u[2][2],
            u[1][0], u[1][1], u[1][2]);
        break;

    case 1:       /* X direction parallel viewing */
        gluLookAt(100, 5.0, 0.0, 0.0, 5.0, 0.0, 0.0, 1.0, 0.0);
        break;

    case 2:       /* Y direction parallel viewing */
        gluLookAt(0.0, 10, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
        break;

    case 3:       /* Z direction parallel viewing */
        gluLookAt(0.0, 5.0, 100, 0.0, 5.0, 0.0, 0.0, 1.0, 0.0);
        break;
    }
}


// 發射子彈
void shoot_bullet() {
    Bullet new_bullet;
    new_bullet.x = position[0];
    new_bullet.y = position[1] + 4.0; // 子彈起始位置略高於機器人
    new_bullet.z = position[2];
    new_bullet.dx = bullet_speed * sin(self_ang * PI / 180.0); // 子彈方向
    new_bullet.dz = bullet_speed * cos(self_ang * PI / 180.0);
    bullets.push_back(new_bullet);
}


// 更新子彈位置
void update_bullets() {
    glColor3f(1.0, 0.0, 0.0);
    for (auto it = bullets.begin(); it != bullets.end();) {
        // 更新子彈位置
        it->x -= it->dx;
        it->z -= it->dz;

        // 畫子彈
        glPushMatrix();
        glTranslatef(it->x, it->y, it->z);
        glutSolidSphere(0.3, 10, 10);
        glPopMatrix();

        // 檢查子彈是否超出範圍
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
    glOrtho(-10.0f * zoomFactor, 10.0f * zoomFactor, -10.0f * zoomFactor, 10.0f * zoomFactor, 1.0f, 100.0f);
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

void drawViewVolume() {
    float aspectRatio = (float)(width) / (float)(height);  // 確保正確計算浮點值的長寬比
    float nearHeight = 2.0f * nearDist * tanf(fovy * zoomFactor * M_PI / 360.0f); // 近平面高度
    float nearWidth = nearHeight * aspectRatio;  // 近平面寬度
    float farHeight = 2.0f * farDist * tanf(fovy * zoomFactor * M_PI / 360.0f);  // 遠平面高度
    float farWidth = farHeight * aspectRatio;  // 遠平面寬度

    glPushMatrix();

    // 將觀察者的位置設為錐體的起點
    glTranslatef(eye[0], eye[1], eye[2]);
    glRotatef(eyeAngy, 0.0f, 1.0f, 0.0f);  // 沿 Y 軸旋轉（Yaw）
    glRotatef(eyeAngx, 1.0f, 0.0f, 0.0f);  // 沿 X 軸旋轉（Pitch）
    glRotatef(eyeAngz, 0.0f, 0.0f, 1.0f);  // 沿 Z 軸旋轉（Roll）

    glColor3f(1.0f, 1.0f, 1.0f); // 白色線條

    // 繪製近平面的矩形框
    glBegin(GL_LINE_LOOP);
    glVertex3f(-nearWidth / 2, nearHeight / 2, -nearDist); // 左上
    glVertex3f(nearWidth / 2, nearHeight / 2, -nearDist);  // 右上
    glVertex3f(nearWidth / 2, -nearHeight / 2, -nearDist); // 右下
    glVertex3f(-nearWidth / 2, -nearHeight / 2, -nearDist);// 左下
    glEnd();

    // 繪製遠平面的矩形框
    glBegin(GL_LINE_LOOP);
    glVertex3f(-farWidth / 2, farHeight / 2, -farDist); // 左上
    glVertex3f(farWidth / 2, farHeight / 2, -farDist);  // 右上
    glVertex3f(farWidth / 2, -farHeight / 2, -farDist); // 右下
    glVertex3f(-farWidth / 2, -farHeight / 2, -farDist); // 左下
    glEnd();

    // 繪製從觀察者到近平面四個頂點的連接線
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);  // 觀察者位置
    glVertex3f(-nearWidth / 2, nearHeight / 2, -nearDist); // 近平面左上
    glVertex3f(0.0f, 0.0f, 0.0f);  // 觀察者位置
    glVertex3f(nearWidth / 2, nearHeight / 2, -nearDist);  // 近平面右上
    glVertex3f(0.0f, 0.0f, 0.0f);  // 觀察者位置
    glVertex3f(nearWidth / 2, -nearHeight / 2, -nearDist); // 近平面右下
    glVertex3f(0.0f, 0.0f, 0.0f);  // 觀察者位置
    glVertex3f(-nearWidth / 2, -nearHeight / 2, -nearDist); // 近平面左下
    glEnd();

    // 繪製從觀察者到遠平面四個頂點的連接線
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);  // 觀察者位置
    glVertex3f(-farWidth / 2, farHeight / 2, -farDist); // 遠平面左上
    glVertex3f(0.0f, 0.0f, 0.0f);  // 觀察者位置
    glVertex3f(farWidth / 2, farHeight / 2, -farDist);  // 遠平面右上
    glVertex3f(0.0f, 0.0f, 0.0f);  // 觀察者位置
    glVertex3f(farWidth / 2, -farHeight / 2, -farDist); // 遠平面右下
    glVertex3f(0.0f, 0.0f, 0.0f);  // 觀察者位置
    glVertex3f(-farWidth / 2, -farHeight / 2, -farDist); // 遠平面左下
    glEnd();

    glPopMatrix();
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

    drawViewVolume();

}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}
void draw_robot() {
    // 移動旋轉整個機器人
    glTranslatef(position[0], position[1], position[2]);
    glRotatef(self_ang, 0.0, 1.0, 0.0);

    // 身體
    glPushMatrix();
    glTranslatef(0.0, 5.0, 0.0);
    glScalef(3.0, 4.0, 2.0);
    draw_cube(robot_colors);
    glPopMatrix();

    // 頭
    glPushMatrix();
    glTranslatef(0.0, 8.5, 0.0);
    draw_head();
    glPopMatrix();

    // 左手
    glPushMatrix();
    glTranslatef(-1.8, 6.5, 0.0);
    draw_hand(lhand_joint_angle, lelbow_joint_angle);
    glPopMatrix();

    // 右手
    glPushMatrix();
    glTranslatef(1.8, 6.5, 0.0);
    draw_hand(rhand_joint_angle, relbow_joint_angle);
    glPopMatrix();

    // 左腳
    glPushMatrix();
    glTranslatef(-0.75, 3.0, 0.0);
    draw_foot(lfoot_joint_angle, lknee_joint_angle);
    glPopMatrix();

    // 右腳
    glPushMatrix();
    glTranslatef(0.75, 3.0, 0.0);
    draw_foot(rfoot_joint_angle, rknee_joint_angle);
    glPopMatrix();
}


void draw_scene() {
    draw_world_axes();  // 畫世界坐標系
    draw_floor();
    draw_obstacle();
    draw_lollipop(lollipop.x, lollipop.z);
    draw_robot();
}
void display() {
    // 清空顏色和深度緩衝區
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 設置模型視圖矩陣
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 更新場景（爆炸效果和子彈）
    updateAndDrawFragments();
    update_bullets();

    // 根據當前樣式選擇繪製模式
    switch (style) {
    case 0: // 四窗口模式
        // 右下視角（透視投影）
        perspectiveProject();
        glViewport(width / 2, 0, width / 2, height / 2);
        make_view(4);
        draw_view();    
        draw_scene();

        // 左上視角（X方向平行視角）
        parallelProject();
        glViewport(0, height / 2, width / 2, height / 2);
        make_view(1);
        draw_view();
        draw_scene();

        // 右上視角（Y方向平行視角）
        parallelProject();
        glViewport(width / 2, height / 2, width / 2, height / 2);
        make_view(2);
        draw_view();
        draw_scene();

        // 左下視角（Z方向平行視角）
        parallelProject();
        glViewport(0, 0, width / 2, height / 2);
        make_view(3);
        draw_view();
        draw_scene();
        break;

    case 1: // X方向平行視角全屏
        parallelProject();
        glViewport(0, 0, width, height);
        make_view(1);
        draw_view();
        draw_scene();
        break;

    case 2: // Y方向平行視角全屏
        parallelProject();
        glViewport(0, 0, width, height);
        make_view(2);
        draw_view();
        draw_scene();
        break;

    case 3: // Z方向平行視角全屏
        parallelProject();
        glViewport(0, 0, width, height);
        make_view(3);
        draw_view();
        draw_scene();
        break;

    case 4: // 透視投影全屏
        perspectiveProject();
        glViewport(0, 0, width, height);
        make_view(4);
        draw_view();
        draw_scene();
        break;
    }

    // 交換前後緩衝區
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
        step = 0.2; // 行走速度
        swing = 3; // 揮臂速度
        swing_angle = 40; // 揮臂幅度
    }
    else if (mode_move == RUN_MODE) {
        step = 0.5;
        swing = 10;
        swing_angle = 70;
    }

    // 確保腳的擺動角度不會超過目前模式的揮臂幅度
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

    // 預測下一步的位置
    float next_step_sin = step * sin(self_ang * PI / 180.0);
    float next_step_cos = step * cos(self_ang * PI / 180.0);
    float next_x = position[0] - next_step_sin;
    float next_z = position[2] - next_step_cos;

    // 檢查碰撞
    if (!check_collision(next_x, next_z, robot_width, robot_length)) {
        // 沒有碰撞，更新位置
        position[0] = next_x;
        position[2] = next_z;
    }

    // 手腳擺動方向
    if (rfoot_joint_angle >= swing_angle || rfoot_joint_angle <= -swing_angle)
        dir *= -1;

    // 擺動角度
    rfoot_joint_angle += (swing * dir);
    lfoot_joint_angle += (-swing * dir);
    rhand_joint_angle[0] += (-swing * dir);
    lhand_joint_angle[0] += (swing * dir);
}
void move_back(int mode_move) {
    set_moving(mode_move);

    // 下一步位置
    float next_step_sin = step * sin(self_ang * PI / 180.0);
    float next_step_cos = step * cos(self_ang * PI / 180.0);
    float next_x = position[0] + next_step_sin;
    float next_z = position[2] + next_step_cos;

    // 檢查碰撞
    if (!check_collision(next_x, next_z, robot_width, robot_length)) {
        // 沒有碰撞，更新位置
        position[0] = next_x;
        position[2] = next_z;
    }

    // 手腳擺動方向
    if (rfoot_joint_angle >= swing_angle || rfoot_joint_angle <= -swing_angle)
        dir *= -1;

    // 擺動角度
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

    // 重置所有關節角度
    rhand_joint_angle[0] = rhand_joint_angle[1] = rhand_joint_angle[2] = 0;
    lhand_joint_angle[0] = lhand_joint_angle[1] = lhand_joint_angle[2] = 0;
    relbow_joint_angle[0] = relbow_joint_angle[1] = relbow_joint_angle[2] = 0;
    lelbow_joint_angle[0] = lelbow_joint_angle[1] = lelbow_joint_angle[2] = 0;
    display();
}

void removeObstacle(int i) {
    // 將i障礙物的大小設為零 (障礙物消失)
    obstacles[i].length = 0.0;
    obstacles[i].width = 0.0;
    obstacles[i].height = 0.0;
}

int prev_key; // 上一次按鍵
void my_quit(unsigned char key, int ix, int iy)
{
    int    i;
    float  x[3], y[3], z[3];
    /*-------------機器人----------------*/
    if (int(key) == 27) { // ESC 離開
        exit(0);
    }
    if (key == 'w') { // w 向前走
        move_forward(WALK_MODE);
    }
    else if (int(key) == 87) { // shift + w 向前跑
        move_forward(RUN_MODE);
    }
    else if (key == 's') { // s 向後走
        move_back(WALK_MODE);
    }
    else if (key == 83) { // shift + s 向後跑
        move_back(RUN_MODE);
    }
    else if (key == 'a') { // a 左轉
        self_ang += 10.0;
    }
    else if (key == 'd') { // d 右轉
        self_ang -= 10.0;
    }
    else if (key == 'z') { // g 抓
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
    else if (key == 'b') {
        if (explosionIndex < NUM_OBSTACLES) {
            wave();
            initializeExplosion();
            removeObstacle(explosionIndex - 1);
        }
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
    display();
    prev_key = int(key);
}

void my_reshape(int w, int h)
{
    glViewport(0, 0, w, h);  // 設置視口
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (w > h) {
        // 寬大於高，保持 X 軸範圍固定，調整 Y 軸
        glOrtho(-10.0, 10.0, -10.0 * (float)h / (float)w, 10.0 * (float)h / (float)w, 0.50, 30.0);
    }
    else {
        // 高大於或等於寬，保持 Y 軸範圍固定，調整 X 軸
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
    glutTimerFunc(0, timer, 0); // 啟動定時器
    glutMainLoop();
}

