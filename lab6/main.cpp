#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <unordered_map>


// Переменные камеры
// Переменные камеры
float camX = 0.0f, camY = 0.0f, camZ = 10.0f; // Позиция камеры
float yaw = 0.0f, pitch = 0.0f;              // Углы вращения камеры
float moveSpeed = 0.1f;                       // Скорость движения камеры
float mouseSensitivity = 0.05f;               // Чувствительность мыши
float zoomLevel = 45.0f;


// Состояние клавиш
std::unordered_map<int, bool> keyState;


// Текстура для заднего фона

GLuint backgroundTexture;
GLuint cubeTexture;



// Флаг автоматического вращения
bool autoRotate = true;

float objAngle = 0.0f; // Угол вращения объекта


// Функция для загрузки текстуры
void loadTexture(const char* filename, GLuint& texture) {
    stbi_set_flip_vertically_on_load(true); // Переворот текстуры по вертикали
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        printf("Failed to load texture: %s\n", filename);
        exit(1);
    }

    printf("Loaded texture: %dx%d, Channels: %d\n", width, height, channels);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, 
                 channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

// Функция для отрисовки фона
void drawBackground() {
    glDisable(GL_DEPTH_TEST); // Отключаем тест глубины для фона
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0); // Устанавливаем 2D-вид

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glEnable(GL_TEXTURE_2D);

    // Устанавливаем белый цвет перед отрисовкой текстуры
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST); // Включаем тест глубины обратно
}



void drawCube() {
    glBindTexture(GL_TEXTURE_2D, cubeTexture);
    glEnable(GL_TEXTURE_2D);

        // Подсветка
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float lightPos[] = {2.0f, 4.0f, 2.0f, 1.0f}; // Позиция света
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);


    // Верхняя грань (фиолетовый оттенок)
    glColor3f(0.5f, 0.2f, 0.8f);    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f,  1.0f);
    glEnd();

    // Задняя грань (зеркальный фильтр)
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();

    // Левая грань (зелёный цвет)
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();

    // Правая грань (красный фильтр)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f,  1.0f, -1.0f);
    glEnd();

    // Передняя грань
    glColor3f(0.0f, 0.0f, 1.0f); // Синий цвет    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    glEnd();

    // Нижняя грань (инверсия текстуры)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}



void drawCheese() {
    // Координаты основания (нижний треугольник)
    GLfloat baseVertices[3][3] = {
        {-1.5f, 0.0f, -0.5f},
        { 1.5f, 0.0f, -0.5f},
        { 0.0f, 0.0f,  3.0f}  // Увеличение длины носа
    };

    // Координаты верхней грани (верхний треугольник с наклоном)
    GLfloat topVertices[3][3] = {
        {-1.5f, 1.0f, -0.3f},
        { 1.5f, 1.0f, -0.3f},
        { 0.0f, 1.0f,  3.0f}  // Удлинённый передний угол
    };

    // Нижняя грань (основание треугольника)
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 1.0f, 0.0f); // Жёлтый цвет
    for (int i = 0; i < 3; ++i) {
        glVertex3fv(baseVertices[i]);
    }
    glEnd();

    // Верхняя грань (треугольник сверху)
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.9f, 0.0f); // Светло-жёлтый цвет
    for (int i = 0; i < 3; ++i) {
        glVertex3fv(topVertices[i]);
    }
    glEnd();

    // Боковые грани
    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.75f, 0.0f); // Оранжевый цвет
    for (int i = 0; i < 3; ++i) {
        glVertex3fv(baseVertices[i]);
        glVertex3fv(baseVertices[(i + 1) % 3]);
        glVertex3fv(topVertices[(i + 1) % 3]);
        glVertex3fv(topVertices[i]);
    }
    glEnd();
}


void drawAxes() {
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_LINES);

    // Ось X (красная)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-5.0f, 0.0f, 0.0f);
    glVertex3f(5.0f, 0.0f, 0.0f);

    // Ось Y (зелёная)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -5.0f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);

    // Ось Z (синяя)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -5.0f);
    glVertex3f(0.0f, 0.0f, 5.0f);

    glEnd();
}

// Функция для настройки освещения
void setupLighting(float camX, float camY, float camZ, float forwardX, float forwardY, float forwardZ) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float lightPos[] = {camX, camY, camZ, 1.0f}; // Свет на позиции камеры
    float lightDir[] = {forwardX, forwardY, forwardZ}; // Направление света
    float ambientLight[] = {0.05f, 0.05f, 0.05f, 1.0f};  // Очень слабая подсветка
    float diffuseLight[] = {1.0f, 1.0f, 1.0f, 1.0f};  // Яркость света для куба

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDir);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 20.0f); // Уменьшаем угол конуса
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 50.0f); // Экспоненциальное затухание
}

// Отрисовка сцены
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground(); // Отрисовка фона

    glLoadIdentity();

    // Рассчитываем направление камеры
    float forwardX = cos(yaw) * cos(pitch);
    float forwardY = sin(pitch);
    float forwardZ = sin(yaw) * cos(pitch);

    gluLookAt(camX, camY, camZ, camX + forwardX, camY + forwardY, camZ + forwardZ, 0.0f, 1.0f, 0.0f);
    setupLighting(camX, camY, camZ, forwardX, forwardY, forwardZ); // Настраиваем освещение


    drawAxes(); // Рисуем координатные оси

    glPushMatrix();
    glRotatef(objAngle, 0.0f, 1.0f, 0.0f); // Вращаем объект
    drawCube();
    glPopMatrix();
    glDisable(GL_LIGHTING);

    glutSwapBuffers();
}


// Установка перспективы
void setupProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(zoomLevel, 800.0f / 600.0f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// Настройка OpenGL
void setup() {
    glEnable(GL_DEPTH_TEST); // Включаем тест глубины
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Чёрный фон
    loadTexture("/home/snowwy/Desktop/code/_0alg/git/comp_graphics/comp_graphics/l3/l_lab3/assets/background.jpg", backgroundTexture); // Загрузка текстуры для фона
    loadTexture("/home/snowwy/Desktop/code/_0alg/git/comp_graphics/comp_graphics/l3/l_lab3/assets/cube.jpg", cubeTexture);             // Загрузка текстуры для куба


    initializeObjects();
    setupProjection(); // Устанавливаем перспективу
}




// Обработка нажатия клавиш
void keyboardPress(unsigned char key, int x, int y) {
    switch (key) {
        case '=':
        case '-':
            keyState[key] = true;
            break;
        default:
            break;
    }
}

// Обработка отпускания клавиш
void keyboardRelease(unsigned char key, int x, int y) {
    switch (key) {
        case '=':
        case '-':
            keyState[key] = false;
            break;
        default:
            break;
    }
}

// Обработка специальных клавиш (стрелки)
void specialKeyPress(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
        case GLUT_KEY_DOWN:
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:
            keyState[key] = true;
            break;
        default:
            break;
    }
}

void specialKeyRelease(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
        case GLUT_KEY_DOWN:
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:
            keyState[key] = false;
            break;
        default:
            break;
    }
}



// Обработка колеса мыши для масштабирования
void mouseWheel(int button, int dir, int x, int y) {
    if (dir > 0) {
        zoomLevel -= 2.0f;
        if (zoomLevel < 10.0f) zoomLevel = 10.0f; // Ограничение минимального масштаба
    } else {
        zoomLevel += 2.0f;
        if (zoomLevel > 90.0f) zoomLevel = 90.0f; // Ограничение максимального масштаба
    }
    setupProjection();
    glutPostRedisplay();
}

// Обновление состояния камеры
void updateCamera() {
    float forwardX = cos(yaw) * cos(pitch);
    float forwardZ = sin(yaw) * cos(pitch);
    float rightX = cos(yaw - M_PI / 2);
    float rightZ = sin(yaw - M_PI / 2);

    if (keyState[GLUT_KEY_UP]) { // Вперёд
        camX += forwardX * moveSpeed;
        camZ += forwardZ * moveSpeed;
    }
    if (keyState[GLUT_KEY_DOWN]) { // Назад
        camX -= forwardX * moveSpeed;
        camZ -= forwardZ * moveSpeed;
    }
    if (keyState[GLUT_KEY_LEFT]) { // Влево
        camX += rightX * moveSpeed;
        camZ += rightZ * moveSpeed;
    }
    if (keyState[GLUT_KEY_RIGHT]) { // Вправо
        camX -= rightX * moveSpeed;
        camZ -= rightZ * moveSpeed;
    }
    if (keyState['=']) { // Увеличить масштаб
        zoomLevel -= 2.0f;
        if (zoomLevel < 10.0f) zoomLevel = 10.0f;
        setupProjection();
    }
    if (keyState['-']) { // Уменьшить масштаб
        zoomLevel += 2.0f;
        if (zoomLevel > 90.0f) zoomLevel = 90.0f;
        setupProjection();
    }
}


// Обновление состояния
void update() {
    updateCamera();
    if (autoRotate) {
        objAngle += 0.5f;
        if (objAngle >= 360.0f) objAngle -= 360.0f;
    }
    glutPostRedisplay();
}





void mouseMotion(int x, int y) {
    static bool firstMouse = true;
    static int lastX, lastY;

    int screenWidth = glutGet(GLUT_WINDOW_WIDTH);
    int screenHeight = glutGet(GLUT_WINDOW_HEIGHT);

    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    int deltaX = x - lastX;
    int deltaY = lastY - y; // Инвертируем, чтобы движение вверх было положительным

    lastX = x;
    lastY = y;

    yaw += deltaX * mouseSensitivity * M_PI / 180.0f;
    pitch += deltaY * mouseSensitivity * M_PI / 180.0f;

    // Привязка центра камеры к положению курсора (без ограничения на движение)
    if (pitch > M_PI) pitch -= 2 * M_PI;
    if (pitch < -M_PI) pitch += 2 * M_PI;

    if (yaw > M_PI) yaw -= 2 * M_PI;
    if (yaw < -M_PI) yaw += 2 * M_PI;
}


// Главная функция
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Cube with Background");

    setup();        // Загрузка текстуры для куба

    glutDisplayFunc(display);


    glutKeyboardFunc(keyboardPress);
    glutKeyboardUpFunc(keyboardRelease);
    glutSpecialFunc(specialKeyPress);
    glutSpecialUpFunc(specialKeyRelease);
    glutIdleFunc(update);
    glutPassiveMotionFunc(mouseMotion);


    glutMainLoop();
    return 0;
}


