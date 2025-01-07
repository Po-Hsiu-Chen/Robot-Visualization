#include <windows.h>
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <array>
#define PI   acos(-1)
#define GL_BGR 0x80E0
#define M_PI 3.14159265358979323846
#define SIZE  100
#define FLOOR_NUMBER_OF_GRID 20
#define NUM_OBSTACLES 4  
#define WALK_MODE 0     
#define RUN_MODE 1     
#define WAVE_MODE 2 
#define NUM_OBSTACLES 4
#define MAX_BIRD 10
#define SPLIT_VIEW 5
#define TSIZE 64 /* define texture dimension */
#define WAVE_WIDTH 64
#define WAVE_HEIGHT 64
GLUquadricObj* sphere = NULL;

/*----------------------------機器人--------------------------*/
// 機器人位置與旋轉
float position[3] = { 0.0, 0.0, 0.0 };  // 機器人位置
float self_ang = 0.0;                   // 機器人自轉角度
float glob_ang = 0.0;                   // 機器人全局角度

// 機器人尺寸與動作
float robot_width = 3.0, robot_length = 2.0;  // 機器人寬度與長度
float step = 0.3;                             // 每步大小
float swing = 4.0;                            // 揮臂速度
float swing_angle = 40.0;                     // 最大揮臂角度

// 關節角度
float rhand_joint_angle[4] = { 0, 1, 0, 0 };
float lhand_joint_angle[4] = { 0, 1, 0, 0 };
float relbow_joint_angle[4] = { 0, 1, 0, 0 };
float lelbow_joint_angle[4] = { 0, 1, 0, 0 };
float rfoot_joint_angle = 0, lfoot_joint_angle = 0;
float rfinger_joint_angle = 45, lfinger_joint_angle = -45;
float rknee_joint_angle = 0, lknee_joint_angle = 0;
float head_angle = 0;

// 動作狀態
int dir = 1;                // 擺動方向
int finger_dir = 1;         // 手指擺動方向
bool is_exploding = false;  // 是否爆炸狀態
bool isHoldingLollipop = false;  // 是否抓住棒棒糖
bool is_enlarged = false;   // 是否放大

/*---------------------------相機與視角-------------------------*/
float eyeDx = 0.0, eyeDy = 0.0, eyeDz = 0.0;  // 相機移動偏移量
float eyeAngx = 0.0, eyeAngy = 0.0, eyeAngz = 0.0; // 相機旋轉角度
float eye[3] = { 20.0, 20.0, 30.0 };          // 相機位置
float u[3][3] = { {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0} };
float cv, sv; /* cos(5.0) and sin(5.0) */

float zoomFactor = 1.5;                      // 縮放比例
float fovy = 30.0, nearDist = 5.0, farDist = 300.0f; // 透視投影參數
bool mode_viewVolume = false;                // 是否顯示視圖體
double eqn[] = { 0.0, 1.0, 0.0, 10.0 };      // 剪裁平面
int viewMode = SPLIT_VIEW;
int style = 4; // 0: 四分割, 1~3: 平行投影, 4: 透視投影

/*----------------------------視窗大小---------------------------------*/
int width = 800, height = 800;

/*----------------------------障礙物-------------------------------*/
struct Obstacle {
    float x, y, z; // 障礙物位置
    float length, width, height; // 障礙物大小
};
Obstacle obstacles[NUM_OBSTACLES] = {
    {-30.0, 7.5, -20.0, 6.0, 3.0, 15},
    {-20.0, 7.5, -20.0, 6.0, 3.0, 15},
    {-10.0, 7.5, -20.0, 6.0, 3.0, 15},
    {0.0, 7.5, -20.0, 6.0, 3.0, 15}
};

/*--------------------------------子彈-------------------------------*/
const float bullet_speed = 1.0;   // 子彈速度
struct Bullet {
    float x, y, z;
    float dx, dz;
};
std::vector<Bullet> bullets;

/*----------------------------棒棒糖------------------------------*/
struct Lollipop {
    double x, z;
    bool isPickedUp = false;
};
Lollipop lollipop = { 5.0, -5.0 }; // 棒棒糖初始位置


/*-------------------------------顏色設定-------------------------*/
float robot_colors[3] = { 0.4, 0.5, 0.8 };
float obstacle_colors[3] = { 0.678, 0.847, 0.902 };


/*-------------------------------其他-------------------------*/
float  points[][3] = { {-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5},
                      {0.5, 0.5, -0.5}, {-0.5, 0.5, -0.5},
                      {-0.5, -0.5, 0.5}, {0.5, -0.5, 0.5},
                      {0.5, 0.5, 0.5}, {-0.5, 0.5, 0.5} };
int face[][4] = { {0, 3, 2, 1}, {0, 1, 5, 4}, {1, 2, 6, 5},
                  {4, 5, 6, 7}, {2, 3, 7, 6}, {0, 4, 7, 3} };
int cube[6] = { 0, 1, 2, 3, 4, 5 };


/*-----------------------------光源相關參數---------------------------*/
// 全域光源
float global_ambient[] = { 0.8f, 0.8f, 0.8f, 0.0f }; // 環境光

// 方向光
float dir_light_dir[] = { 1.0, -1.0, 1.0, 0.0 };  // 光線方向
float dir_light_diffuse[] = { 1.2, 1.2, 1.2, 1.0 };
float dir_light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
float dir_light_specular[] = { 0.9, 0.9, 0.9, 1.0 };

// 點光源
float point_light_pos[] = { -10.0, 20.0, -20.0, 1.0 };
float point_light_diffuse[] = { 0.5, 0.7, 1.0, 1.0 };
float point_light_specular[] = { 0.8, 0.8, 0.8, 1.0 };
int current_color_mode = 0;

// 聚光燈
float spot_light_pos[] = { 0.0f, 2.0f, 0.0f, 1.0f };
float spot_light_dir[] = { 0.0f, -1.0f, 0.0f };
float spot_light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float spot_light_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float spot_light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float spot_light_cutoff = 40.0f;        // 聚光燈截角角度
float spot_exponent = 8.0f;       // 光束集中程度
float spot_attenuation = 0.05f;   // 聚光燈衰減
int dir_light_on = 1;             // 方向光開關
int point_light_on = 1;           // 點光源開關
int spot_light_on = 1;            // 聚光燈開關


/*---------------------------材質-----------------------------*/
float mat_ambient[] = { 0.8, 0.8, 0.8, 1.0 };
float mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
float flr_diffuse[] = { 0.1, 0.1, 0.7, 1.0 };
float mat_specular[] = { 0.4, 0.4, 0.4, 1.0 };
float mat_shininess = 16.0;

/*---------------------------霧-----------------------------*/
float fogColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // 初始霧顏色
int fogMode = GL_EXP;                         // 霧模式，預設為指數霧
float fogDensity = 0.0f;                      // 初始霧的密度

/*---------------------------材質貼圖-----------------------------*/
unsigned char waterwave[WAVE_WIDTH][WAVE_HEIGHT][4];
unsigned char checkboard[TSIZE][TSIZE][4]; /* checkboard textures */
unsigned int textName[2]; /* declare two texture maps*/
float mtx[16];
float a[3], b[3];
GLuint floorTexture;      // 用於地板的紋理ID
GLuint robotTexture;      // 用於機器人的紋理ID
GLuint wallTexture;       // 用於障礙物的紋理ID
GLuint skySphereTexture;
/*---------------------------會飛的鳥-----------------------------*/
typedef struct
{
    float x, y, z;   // 鳥位置
    float angle;     // 目前旋轉角度
    float speed;     // 飛的速度
    float wingState; // 翅膀狀態
} Birdfly;

Birdfly birdflies[10];
GLuint birdTextures[2];
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

// 加載 BMP 圖片
GLuint loadBMP(const char* filename) {
    FILE* file;
    errno_t err = fopen_s(&file, filename, "rb");
    if (err != 0) {
        printf("Error: Cannot open BMP file %s\n", filename);
        exit(1);
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file); // BMP header
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int imageSize = *(int*)&header[34];

    unsigned char* data = (unsigned char*)malloc(imageSize);
    fread(data, sizeof(unsigned char), imageSize, file); // BMP data
    fclose(file);

    // 翻轉圖像（BMP 像素是上下顛倒的）
    for (int i = 0; i < height / 2; ++i) {
        for (int j = 0; j < width * 3; ++j) {
            unsigned char temp = data[i * width * 3 + j];
            data[i * width * 3 + j] = data[(height - i - 1) * width * 3 + j];
            data[(height - i - 1) * width * 3 + j] = temp;
        }
    }

    // 生成 OpenGL 紋理
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 設置紋理參數
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 加載紋理數據
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);


    free(data);
    return textureID;
}
void make_check() {
    int i, j, c;

    for (i = 0; i < TSIZE; i++)
        for (j = 0; j < TSIZE; j++) {
            if (j > i / 2 && j <= 64 - i / 2)
                c = 255;
            else
                c = 0;
            checkboard[i][j][0] = c / 8;
            checkboard[i][j][1] = c / 2;
            checkboard[i][j][2] = c / 4;
            if (c == 255)
                checkboard[i][j][3] = 255;
            else
                checkboard[i][j][3] = 0;
        }
    // Generate trunk
    for (i = 0; i < TSIZE / 3; i++) {
        for (j = 0; j < TSIZE / 2 - 4; j++)
            checkboard[i][j][3] = 0;
        for (j = TSIZE / 2 + 4; j < TSIZE; j++)
            checkboard[i][j][3] = 0;
    }
}

void compute_ab_axes(void) {
    float w0, w2;
    double len;

    /*----Get w0 and w2 from the modelview matrix mtx[] ---*/
    w0 = mtx[2];
    w2 = mtx[10];

    len = sqrt(w0 * w0 + w2 * w2);
    /*---- Define the a and b axes for billboards ----*/
    b[0] = 0.0;
    b[1] = 1.0;
    b[2] = 0.0;
    a[0] = w2 / len;
    a[1] = 0.0;
    a[2] = -w0 / len;
}

void draw_billboard(float x, float z, float w, float h) {
    float v0[3], v1[3], v2[3], v3[3];

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    /*----Compute the 4 vertices of the billboard ----*/
    v0[0] = x - (w / 2) * a[0];
    v0[1] = 0.0;
    v0[2] = z - (w / 2) * a[2];
    v1[0] = x + (w / 2) * a[0];
    v1[1] = 0.0;
    v1[2] = z + (w / 2) * a[2];
    v2[0] = x + (w / 2) * a[0];
    v2[1] = h;
    v2[2] = z + (w / 2) * a[2];
    v3[0] = x - (w / 2) * a[0];
    v3[1] = h;
    v3[2] = z - (w / 2) * a[2];

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3fv(v0);
    glTexCoord2f(1.0, 0.0);
    glVertex3fv(v1);
    glTexCoord2f(1.0, 1.0);
    glVertex3fv(v2);
    glTexCoord2f(0.0, 1.0);
    glVertex3fv(v3);
    glEnd();
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
}

void Create_Texture_Bird() {
    unsigned char texture1[64][64][4]; // 第一種紋理
    unsigned char texture2[64][64][4]; // 第二種紋理

    // 填充第一種紋理（橙色翅膀）
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            if ((x < 5 || x > 58) || (y < 5 || y > 58)) {
                texture1[y][x][0] = 0;   
                texture1[y][x][1] = 0;
                texture1[y][x][2] = 0;
                texture1[y][x][3] = 255; 
            }
            else {
                texture1[y][x][0] = 255; 
                texture1[y][x][1] = 165;
                texture1[y][x][2] = 0;
                texture1[y][x][3] = 255; 
            }
        }
    }

    // 填充第二種紋理（藍色翅膀）
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            if ((x < 5 || x > 58) || (y < 5 || y > 58)) {
                texture2[y][x][0] = 0;  
                texture2[y][x][1] = 0;
                texture2[y][x][2] = 0;
                texture2[y][x][3] = 255;
            }
            else {
                texture2[y][x][0] = 0;  
                texture2[y][x][1] = 0;
                texture2[y][x][2] = 255;
                texture2[y][x][3] = 255; 
            }
        }
    }

    // 加載紋理到 OpenGL
    glGenTextures(2, birdTextures);

    // 紋理 1
    glBindTexture(GL_TEXTURE_2D, birdTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 紋理 2
    glBindTexture(GL_TEXTURE_2D, birdTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
bool isWithinRange(double robotX, double robotZ, double lollipopX, double lollipopZ) { //檢查機器人是否在棒棒糖範圍內
    double dist = std::sqrt(
        (robotX - lollipopX) * (robotX - lollipopX) +
        (robotZ - lollipopZ) * (robotZ - lollipopZ)
    );
    return dist <= 8;
}
void adjustLight()
{
    if (dir_light_on)
    {
        // 設置光源 0 (方向光)
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        GLfloat light0_diffuse[] = { (2.0f), 2.0f, 2.0f, 1.0f };   // 白光，強度適中
        GLfloat light0_direction[] = { 1.0f, -1.0f, -0.5f, 0.0f }; // 方向光: w=0

        glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
        glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
    }
    else if (!dir_light_on)
    {
        glDisable(GL_LIGHT0);
    }

    // ---------------------------------------------------------------------------------------
    if (point_light_on)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT1);
        // 定義光源的屬性

        GLfloat light1_position[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 點光源: w=1 (齊次坐標)

        // 設置光源屬性
        glLightfv(GL_LIGHT1, GL_DIFFUSE, point_light_diffuse);
        glLightfv(GL_LIGHT1, GL_POSITION, point_light_pos);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 180);
    }
    else if (!point_light_on)
    {
        glDisable(GL_LIGHT1);
    }

    if (spot_light_on) {
        glEnable(GL_LIGHT2);
    }
    else {
        glDisable(GL_LIGHT2);
    }
}

void Create_Texture_Waterwave() {
    int x, y;
    float waveFrequency = 3.0f; // 波浪頻率，越大波紋越密集
    float waveAmplitude = 50.0f; // 波浪振幅

    for (y = 0; y < WAVE_HEIGHT; y++) {
        for (x = 0; x < WAVE_WIDTH; x++) {
            // 生成斜向波紋公式，使用 x 和 y 的線性組合
            float u = (float)x / WAVE_WIDTH;  // x 坐標歸一化
            float v = (float)y / WAVE_HEIGHT; // y 坐標歸一化

            // 斜向波紋公式，45 度方向
            float wave = sinf(2.0f * M_PI * waveFrequency * (u + v));

            // 設置紋理的 RGBA 值
            waterwave[x][y][0] = (unsigned char)(50 + waveAmplitude * wave); // R
            waterwave[x][y][1] = (unsigned char)(100 + waveAmplitude * wave); // G
            waterwave[x][y][2] = (unsigned char)(200 + waveAmplitude * wave); // B
            waterwave[x][y][3] = 255; // A
        }
    }
}

void draw_water_surface(float elapsedTime) {
    // 啟用紋理
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textName[1]);

    // 動態紋理變換
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(elapsedTime * 0.05f, elapsedTime * 0.05f, 0.0f); // 紋理移動模擬水流
    glMatrixMode(GL_MODELVIEW);

    // 繪製一小條河流（例如，10x50的區域）
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-5.0, 0.0, -25.0); // 左下

    glTexCoord2f(1.0, 0.0);
    glVertex3f(5.0, 0.0, -25.0); // 右下

    glTexCoord2f(1.0, 5.0);
    glVertex3f(5.0, 0.0, 25.0);  // 右上

    glTexCoord2f(0.0, 5.0);
    glVertex3f(-5.0, 0.0, 25.0); // 左上
    glEnd();

    // 還原紋理矩陣
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // 禁用紋理
    glDisable(GL_TEXTURE_2D);
}

void initTextures() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // 樹紋理
    make_check();
    glGenTextures(1, &textName[0]);
    glBindTexture(GL_TEXTURE_2D, textName[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TSIZE, TSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkboard);

    // 水波紋理
    Create_Texture_Waterwave();
    glGenTextures(1, &textName[1]);
    glBindTexture(GL_TEXTURE_2D, textName[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WAVE_WIDTH, WAVE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, waterwave);

    // 加載地板紋理
    floorTexture = loadBMP("PavingStones.bmp"); // 地板紋理

    // 加載機器人紋理
    robotTexture = loadBMP("Blue.bmp"); // 機器人紋理

    // 加載牆壁紋理
    wallTexture = loadBMP("Bricks.bmp"); // 牆壁紋理

    // 初始化鳥紋理
    Create_Texture_Bird();
}

void initFog()
{
    glEnable(GL_FOG);                   // 開啟霧效果
    glFogi(GL_FOG_MODE, fogMode);       // 設定霧模式
    glFogfv(GL_FOG_COLOR, fogColor);    // 設定霧顏色
    glFogf(GL_FOG_DENSITY, fogDensity); // 設定霧密度
    glHint(GL_FOG_HINT, GL_NICEST);     // 設定霧的品質
    glFogf(GL_FOG_START, 5.0f);         // 線性霧的起始點
    glFogf(GL_FOG_END, 50.0f);          // 線性霧的終止點
}
void InitBirds()
{
    srand(time(NULL)); // 随机种子
    for (int i = 0; i < MAX_BIRD; i++)
    {
        birdflies[i].x = (rand() % 20 - 10) * 0.5f; // -10 到 10 范围内的随机值
        birdflies[i].y = 5.0f + rand() % 5;         // 高度在 5 到 10 之间
        birdflies[i].z = (rand() % 20 - 10) * 0.5f;
        birdflies[i].angle = rand() % 360;
        birdflies[i].speed = 0.02f + (rand() % 10) * 0.01f;
        birdflies[i].wingState = 0.0f;
    }
}
void DrawBirdfly(Birdfly* b) {
    glPushMatrix();

    // 更新位置
    b->angle += b->speed;
    if (b->angle > 360.0f)
        b->angle -= 360.0f;

    b->x += sin(b->angle * M_PI / 180.0f) * 0.05f;
    b->z += cos(b->angle * M_PI / 180.0f) * 0.05f;
    b->y += sin(glutGet(GLUT_ELAPSED_TIME) / 1000.0f) * 0.01f;

    // 翅膀動畫狀態
    b->wingState += 0.1f;
    if (b->wingState > 1.0f)
        b->wingState = 0.0f;

    // 平移到鳥的位置
    glTranslatef(b->x, b->y, b->z);

    // 縮放鳥的大小
    glScalef(2.0f, 2.0f, 2.0f); // 放大鳥

    // 使鳥面向攝影機
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    glRotatef(atan2(-modelview[2], modelview[10]) * 180 / M_PI, 0.0f, 1.0f, 0.0f);

    // 繪製鳥的翅膀
    float wingAngle = sin(b->wingState * M_PI * 2) * 30.0f; // 翅膀揮動角度
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, birdTextures[(int)(b->wingState * 2) % 2]); // 切換紋理

    // 左翅膀
    glPushMatrix();
    glRotatef(wingAngle, 0.0f, 0.0f, 1.0f); // 動態旋轉
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);   // 頂點
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.2f, 0.0f);  // 左下
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -0.2f, 0.0f); // 右下
    glEnd();
    glPopMatrix();

    // 右翅膀
    glPushMatrix();
    glRotatef(-wingAngle, 0.0f, 0.0f, 1.0f); // 動態旋轉
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);   // 頂點
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.2f, 0.0f);   // 左下
    glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -0.2f, 0.0f);  // 右下
    glEnd();
    glPopMatrix();

    // 繪製鳥的身體
    glDisable(GL_TEXTURE_2D); // 不使用紋理
    glColor3f(0.0f, 0.0f, 0.0f); // 黑色
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.1f, 0.0f);  // 頭部
    glVertex3f(0.0f, -0.1f, 0.0f); // 身體
    glEnd();

    glPopMatrix();
}

void DrawBirds()
{
    for (int i = 0; i < MAX_BIRD; i++)
    {
        DrawBirdfly(&birdflies[i]);
    }
}

void initSkyTexture() {
    unsigned char skyData[64][64][4]; // RGBA 格式

    // 創建藍色漸層背景
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            float gradient = (float)y / 64.0f;                         // 模擬由深藍到淺藍的漸層
            skyData[y][x][0] = (unsigned char)(135 + gradient * 120); // 藍色漸層
            skyData[y][x][1] = (unsigned char)(206 + gradient * 50);
            skyData[y][x][2] = (unsigned char)(235 + gradient * 20);
            skyData[y][x][3] = 255; // 不透明
        }
    }

    // 加入更多、更小的模糊白色雲
    for (int i = 0; i < 10; i++) { // 增加雲的數量
        int centerX = rand() % 64;
        int centerY = rand() % 64;
        for (int y = -3; y <= 3; y++) { // 減小雲的範圍
            for (int x = -3; x <= 3; x++) {
                int nx = centerX + x;
                int ny = centerY + y;
                if (nx >= 0 && nx < 64 && ny >= 0 && ny < 64) {
                    float dist = sqrtf(x * x + y * y);
                    if (dist < 3) { // 半徑縮小到 3
                        float alpha = 1.0f - dist / 3.0f;
                        skyData[ny][nx][0] = (unsigned char)((1.0f - alpha) * skyData[ny][nx][0] + alpha * 255);
                        skyData[ny][nx][1] = (unsigned char)((1.0f - alpha) * skyData[ny][nx][1] + alpha * 255);
                        skyData[ny][nx][2] = (unsigned char)((1.0f - alpha) * skyData[ny][nx][2] + alpha * 255);
                    }
                }
            }
        }
    }

    // 生成紋理
    glGenTextures(1, &skySphereTexture);
    glBindTexture(GL_TEXTURE_2D, skySphereTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, skyData);
}

void drawDynamicSkySphere(float elapsedTime) {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    // 動態設置紋理環境為 REPLACE，避免受光影影響
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // 動態旋轉天空球，模擬雲層或星空移動
    float rotationAngle = fmod(elapsedTime * 10.0f, 360.0f); // 每秒旋轉 10 度
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

    // 綁定天空紋理
    glBindTexture(GL_TEXTURE_2D, skySphereTexture);

    // 繪製天空球
    GLUquadric* skySphere = gluNewQuadric();
    gluQuadricTexture(skySphere, GL_TRUE);
    gluSphere(skySphere, 100.0, 64, 64); // 半徑 100，分段 64
    gluDeleteQuadric(skySphere);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void draw_light_sphere(float* position, float* color) {
    glPushMatrix();
    glTranslatef(position[0], position[1], position[2]);

    // 設置材質的發光屬性
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
            float point_light_color[] = { 1.0, 0.5, 0.5 }; // 藍
            draw_light_sphere(point_light_pos, point_light_color);
        }
        else {
            float point_light_color[] = { 0.5, 0.5, 1.0 }; // 紅
            draw_light_sphere(point_light_pos, point_light_color);
        }
    }
    if (spot_light_on) {
        float spot_light_color[] = { 0.5, 0.5, 1.0 };
        float spot_light_pos[] = { position[0], position[1] + 9.0, position[2] ,1.0 }; // 跟著機器人
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
//void set_metallicMat() {
//    float ambient[] = { robot_colors[0], robot_colors[1], robot_colors[2], 1.0 };
//    float diffuse[] = { 0.6, 0.6, 0.6, 1.0 };
//    float specular[] = { 0.9, 0.9, 0.9, 1.0 };
//    float shininess = 64.0;
//    set_material(ambient, diffuse, specular, shininess);
//}

//void set_glossyMat() {
//    float ambient[] = { obstacle_colors[0] * 0.2f, obstacle_colors[1] * 0.2f, obstacle_colors[2] * 0.2f, 1.0f };
//    float diffuse[] = { obstacle_colors[0] * 0.8f, obstacle_colors[1] * 0.8f, obstacle_colors[2] * 0.8f, 1.0f };
//    float specular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
//    float shininess = 80.0f;
//    set_material(ambient, diffuse, specular, shininess);
//}

void normalize(float vec[3]) {
    float length = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    vec[0] /= length;
    vec[1] /= length;
    vec[2] /= length;
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

    // 方向光
    glLightfv(GL_LIGHT0, GL_POSITION, dir_light_dir);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dir_light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, dir_light_specular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, dir_light_ambient);


    // 點光源
    glLightfv(GL_LIGHT1, GL_POSITION, point_light_pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, point_light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, point_light_specular);

    // 聚光燈
    normalize(spot_light_dir);
    glLightfv(GL_LIGHT2, GL_POSITION, spot_light_pos);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, spot_light_diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, spot_light_specular);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_light_dir);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, spot_light_cutoff);
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, spot_exponent);

}
void switch_light_color() {
    if (current_color_mode == 0) {
        // 紅光
        point_light_diffuse[0] = 1.0; point_light_diffuse[1] = 0.8; point_light_diffuse[2] = 0.8;
        point_light_specular[0] = 1.0; point_light_specular[1] = 0.8; point_light_specular[2] = 0.8;
    }
    else {
        // 藍光
        point_light_diffuse[0] = 0.5; point_light_diffuse[1] = 0.7; point_light_diffuse[2] = 1.0;
        point_light_specular[0] = 0.5; point_light_specular[1] = 0.7; point_light_specular[2] = 1.0;
    }

    // 切換顏色
    current_color_mode = 1 - current_color_mode;
    glLightfv(GL_LIGHT1, GL_DIFFUSE, point_light_diffuse);
    glutPostRedisplay();
}


void myinit() {
    // 初始化所有紋理
    initTextures();

    // 初始化鳥
    InitBirds();

    // 初始化霧效果
    initFog();

    // 初始化天空紋理
    initSkyTexture();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    cv = cos(5.0 * PI / 180.0);
    sv = sin(5.0 * PI / 180.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // 初始化球體
    if (sphere == NULL) {
        sphere = gluNewQuadric();
        gluQuadricDrawStyle(sphere, GLU_FILL);
        gluQuadricNormals(sphere, GLU_SMOOTH);
    }

    // 啟用光照
    glEnable(GL_LIGHTING);
    setupLights();
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
}

void draw_cube() {
    // 定義每個頂點對應的紋理坐標
    float texCoords[4][2] = {
        {0.0, 0.0}, // 左下
        {1.0, 0.0}, // 右下
        {1.0, 1.0}, // 右上
        {0.0, 1.0}  // 左上
    };

    // 繪製立方體的六個面
    for (int i = 0; i < 6; i++) {
        glBegin(GL_QUADS); // 使用 GL_QUADS 繪製四邊形面
        for (int j = 0; j < 4; j++) {
            glTexCoord2fv(texCoords[j]);  // 設定每個頂點的紋理坐標
            glVertex3fv(points[face[i][j]]); // 設定每個頂點的位置
        }
        glEnd();
    }
}

void draw_floor() {
    glPushMatrix();
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // 確保紋理與顏色調制

    glEnable(GL_TEXTURE_2D);          // 啟用紋理貼圖
    glBindTexture(GL_TEXTURE_2D, floorTexture); // 綁定地板紋理

    glBegin(GL_QUADS); // 繪製地板的四邊形
    glTexCoord2f(0.0, 0.0); glVertex3f(-50.0, 0.0, -50.0); // 左下角
    glTexCoord2f(1.0, 0.0); glVertex3f(50.0, 0.0, -50.0);  // 右下角
    glTexCoord2f(1.0, 1.0); glVertex3f(50.0, 0.0, 50.0);   // 右上角
    glTexCoord2f(0.0, 1.0); glVertex3f(-50.0, 0.0, 50.0);  // 左上角
    glEnd();

    glDisable(GL_TEXTURE_2D);         // 繪製完成後禁用紋理貼圖
    glPopMatrix();
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
    glEnable(GL_TEXTURE_2D);          // 啟用紋理貼圖
    glBindTexture(GL_TEXTURE_2D, wallTexture); // 綁定建築物紋理

    for (int i = 0; i < NUM_OBSTACLES; i++) {
        glPushMatrix();
        glTranslatef(obstacles[i].x, obstacles[i].y, obstacles[i].z);
        glScalef(obstacles[i].length, obstacles[i].height, obstacles[i].width);
        draw_cube(); // 使用改進版的 draw_cube()
        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);         // 禁用紋理貼圖
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

void draw_rectangle(double x, double y, double z) {
    glPushMatrix();
    glTranslatef(0, -y / 2, 0);
    glScalef(x, y, z);
    draw_cube();
    glPopMatrix();
}
void draw_head() {
    glPushMatrix();
    glRotatef(head_angle, 0, 1, 0);
    glColor3fv(robot_colors);
    draw_rectangle(1.3, 1.3, 1.3);
    glPopMatrix();
}

void draw_finger(float finger_joint_angle) {
    glPushMatrix();
    glRotatef(finger_joint_angle, 0, 0, 1);
    glColor3fv(robot_colors);
    draw_rectangle(0.3, 1, 0.3);
    glPopMatrix();
}

void draw_hand(float* hand_joint_angle, float* elbow_joint_angle) {
    glColor3fv(robot_colors);
    glPushMatrix();
    glRotatef(hand_joint_angle[0], hand_joint_angle[1], hand_joint_angle[2], hand_joint_angle[3]);
    glutSolidSphere(0.5, 20.0, 20.0); //肩膀
    draw_rectangle(0.5, 1.5, 0.5); //上臂

    glTranslatef(0, -1.5, 0);
    glPushMatrix();
    glColor3fv(robot_colors);
    glRotatef(45, 1, 0, 0); // 手肘彎曲
    glRotatef(elbow_joint_angle[0], elbow_joint_angle[1], elbow_joint_angle[2], elbow_joint_angle[3]);
    glutSolidSphere(0.5, 20.0, 20.0); //手肘
    draw_rectangle(0.5, 1.5, 0.5); //下臂

    glTranslatef(0, -1.5, 0);
    glColor3fv(robot_colors);
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
    glColor3fv(robot_colors);
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
    case 4: /* Perspective */
        gluLookAt(eye[0], eye[1], eye[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

        break;

    case 1: /* X direction parallel viewing */
        gluLookAt(10, 15.0, 0.0, 0.0, 15.0, 0.0, 0.0, 1.0, 0.0);
        break;

    case 2: /* Y direction parallel viewing */
        gluLookAt(0.0, 50, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
        break;

    case 3: /* Z direction parallel viewing */
        gluLookAt(0.0, 15.0, 50, 0.0, 15.0, 0.0, 0.0, 1.0, 0.0);
        break;
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

        // 繪製遠平面的四個頂點之間的連線（形成矩形框）
        glBegin(GL_LINES);
        glVertex3f(-farWidth / 2, farHeight / 2, -farDist); // 左上
        glVertex3f(farWidth / 2, farHeight / 2, -farDist);  // 右上
        glVertex3f(farWidth / 2, farHeight / 2, -farDist);  // 右上
        glVertex3f(farWidth / 2, -farHeight / 2, -farDist); // 右下
        glVertex3f(farWidth / 2, -farHeight / 2, -farDist); // 右下
        glVertex3f(-farWidth / 2, -farHeight / 2, -farDist);// 左下
        glVertex3f(-farWidth / 2, -farHeight / 2, -farDist);// 左下
        glVertex3f(-farWidth / 2, farHeight / 2, -farDist); // 左上
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

void draw_robot() {
    glEnable(GL_TEXTURE_2D);                  // 啟用紋理
    glBindTexture(GL_TEXTURE_2D, robotTexture);  // 綁定機器人紋理
    // 移動旋轉整個機器人
    glTranslatef(position[0], position[1], position[2]);
    glRotatef(self_ang, 0.0, 1.0, 0.0);

    //set_metallicMat();
    // 身體
    glPushMatrix();
    glTranslatef(0.0, 5.0, 0.0);
    glScalef(3.0, 4.0, 2.0);
    draw_cube();
    glPopMatrix();

    // 頭部
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
    glPushMatrix();
    DrawBirds();
    draw_world_axes();  // 畫世界坐標系
    draw_floor();
    draw_obstacle();
    draw_lollipop(lollipop.x, lollipop.z);
    draw_robot();
    glPopMatrix();

}
void set_billboard(float elapsedTime) {
    // 計算 billboard 軸
    glGetFloatv(GL_MODELVIEW_MATRIX, mtx);
    compute_ab_axes();

    // 繪製 billboard
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textName[0]);
    draw_billboard(5.0 * 4.0, 3.0 * 4.0, 5.0, 8.0);
    draw_billboard(6.0 * 4.0, 5.0 * 4.0, 5.0, 8.0);
    draw_billboard(3.0 * 4.0, 6.0 * 4.0, 5.0, 8.0);
    draw_billboard(2.0 * 4.0, 7.0 * 4.0, 5.0, 8.0);
    draw_billboard(7.0 * 4.0, 2.0 * 4.0, 5.0, 8.0);
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(40, 0.01, 0); // 紋理移動模擬水流
    glScalef(1.0f, 1.0f, 2.0f);
    draw_water_surface(elapsedTime);
    glPopMatrix();
}
void display() {
    // 清空顏色和深度緩衝區
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 設置模型視圖矩陣
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 繪製動態天空球
    static float elapsedTime = 0.0f;
    elapsedTime += 0.005f;
    

    // 根據當前樣式選擇繪製模式
    switch (style) {
    case 0: // 四窗口模式
        // 右下視角（透視投影）
        perspectiveProject();
        glViewport(width / 2, 0, width / 2, height / 2);
        make_view(4);
        drawDynamicSkySphere(elapsedTime);
        set_billboard(elapsedTime);
        draw_view();
        setupLights();
        adjustLight();
        draw_light_spheres();
        draw_scene();

        // 左上視角（X方向平行視角）
        parallelProject();
        glViewport(0, height / 2, width / 2, height / 2);
        make_view(1);
        set_billboard(elapsedTime);
        draw_view();
        setupLights();
        adjustLight();
        draw_light_spheres();
        draw_scene();

        // 右上視角（Y方向平行視角）
        parallelProject();
        glViewport(width / 2, height / 2, width / 2, height / 2);
        make_view(2);
        set_billboard(elapsedTime);
        draw_view();
        setupLights();
        adjustLight();
        draw_light_spheres();
        draw_scene();

        // 左下視角（Z方向平行視角）
        parallelProject();
        glViewport(0, 0, width / 2, height / 2);
        make_view(3);
        set_billboard(elapsedTime);
        draw_view();
        setupLights();
        adjustLight();
        draw_light_spheres();
        draw_scene();
        break;

    case 1: // X方向平行視角全屏
        perspectiveProject();
        glViewport(0, 0, width, height);
        make_view(1); // 使用 make_view 設置相機
        drawDynamicSkySphere(elapsedTime);
        set_billboard(elapsedTime);
        draw_view();
        setupLights();
        adjustLight();
        draw_light_spheres();
        draw_scene();
        break;

    case 2: // Y方向平行視角全屏
        perspectiveProject();
        glViewport(0, 0, width, height);
        make_view(2); // 使用 make_view 設置相機
        drawDynamicSkySphere(elapsedTime);
        set_billboard(elapsedTime);
        draw_view();
        setupLights();
        adjustLight();
        draw_light_spheres();
        draw_scene();
        break;

    case 3: // Z方向平行視角全屏
        perspectiveProject();
        glViewport(0, 0, width, height);
        make_view(3); // 使用 make_view 設置相機
        drawDynamicSkySphere(elapsedTime);
        set_billboard(elapsedTime);
        draw_view();
        setupLights();
        adjustLight();
        draw_light_spheres();
        draw_scene();
        break;

    case 4: // 透視投影全屏
        perspectiveProject();
        glViewport(0, 0, width, height);
        make_view(4); // 使用 make_view 設置相機
        drawDynamicSkySphere(elapsedTime);
        set_billboard(elapsedTime);
        draw_view();
        setupLights();
        adjustLight();
        draw_light_spheres();
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
    float step = 0.1f;
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
    else if (key == '7') {
        dir_light_on = !dir_light_on;

    }
    else if (key == '8') {
        point_light_on = !point_light_on;

    }
    else if (key == '9') {// 切換聚光燈開關
        spot_light_on = !spot_light_on;
    }
    else if (key == 'E') {                   // 增加點光源強度
        for (int i = 0; i < 3; i++) point_light_diffuse[i] += 0.1;
        glLightfv(GL_LIGHT1, GL_DIFFUSE, point_light_diffuse);
    }
    else if (key == 'e') {                     // 減少點光源強度
        for (int i = 0; i < 3; i++) {
            point_light_diffuse[i] -= 0.1;
            if (point_light_diffuse[i] < 0.0) point_light_diffuse[i] = 0.0; // 限制最低值
        }
        glLightfv(GL_LIGHT1, GL_DIFFUSE, point_light_diffuse);
    }
    else if (key == 'R') {                      // 增加聚光燈截角角度
        spot_light_cutoff += 5.0;
        if (spot_light_cutoff > 90.0) spot_light_cutoff = 90.0;
        glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, spot_light_cutoff);
        printf("%f", spot_light_cutoff);
    }
    else if (key == 'r') {                     // 減少聚光燈截角角度
        spot_light_cutoff -= 5.0;
        if (spot_light_cutoff < 10.0) spot_light_cutoff = 10.0;
        glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, spot_light_cutoff);
    }
    else if (key == 'T') {                      // 增加聚光燈強度
        for (int i = 0; i < 3; i++) spot_light_diffuse[i] += 1.1;
        glLightfv(GL_LIGHT2, GL_DIFFUSE, spot_light_diffuse);
    }
    else if (key == 't') {                     // 減少聚光燈強度
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
    else if (key == '+') {
        fogDensity += 0.01f;
        if (fogDensity > 1.0f)
            fogDensity = 1.0f;
        glFogf(GL_FOG_DENSITY, fogDensity);
    }
    else if (key == '-') {
        fogDensity -= 0.01f;
        if (fogDensity < 0.0f)
            fogDensity = 0.0f;
        glFogf(GL_FOG_DENSITY, fogDensity);
    }
    else if (key == '5') {
        if (fogMode == GL_EXP)
            fogMode = GL_EXP2;
        else if (fogMode == GL_EXP2)
            fogMode = GL_LINEAR;
        else
            fogMode = GL_EXP;
        glFogi(GL_FOG_MODE, fogMode);
    }
    else if (key == 'o') {
        fogColor[0] += 0.01f; // 改變增量為 0.01
        fogColor[2] += 0.01f; // 改變增量為 0.01

        // 確保 fogColor 不超過範圍
        if (fogColor[0] > 1.0f) fogColor[0] = 1.0f;
        if (fogColor[2] > 1.0f) fogColor[2] = 1.0f;

        glFogfv(GL_FOG_COLOR, fogColor);
    }
    else if (key == 'O') {
            fogColor[0] -= 0.01f; // 改變減量為 0.01
            fogColor[2] -= 0.01f; // 改變減量為 0.01

            // 確保 fogColor 不低於範圍
            if (fogColor[0] < 0.0f) fogColor[0] = 0.0f;
            if (fogColor[2] < 0.0f) fogColor[2] = 0.0f;

            glFogfv(GL_FOG_COLOR, fogColor);
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
    glutIdleFunc(display);
    glutReshapeFunc(my_reshape);
    glutKeyboardFunc(my_quit);
    glutMainLoop();
}