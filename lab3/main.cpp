#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdio>

// Переменные камеры
float angle = 0.0f;    // Угол для вращения камеры
float radius = 10.0f;   // Радиус окружности, по которой движется камера
float height = 1.0f;   // Высота камеры

// Текстура для заднего фона

GLuint backgroundTexture;
GLuint cubeTexture;

// Флаги движения камеры
bool movingLeft = false;
bool movingRight = false;
bool movingUp = false;
bool movingDown = false;

// Флаг автоматического вращения
bool autoRotate = true;

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


// // Функция для отрисовки пирамиды с цветовыми преобразованиями
// void drawPyramid() {
//     glBegin(GL_TRIANGLES);

//     // Вершина пирамиды (нос)
//     glColor3f(1.0f, 0.0f, 0.0f); // Красный цвет
//     glVertex3f(0.0f, 1.5f, 0.0f); // Вершина (нос)

//     // Боковые грани
//     glColor3f(0.0f, 1.0f, 0.0f); // Зелёный цвет
//     glVertex3f(-0.5f, 0.0f, -0.5f); // Левый задний угол основания
//     glVertex3f(0.5f, 0.0f, -0.5f);  // Правый задний угол основания

//     glColor3f(0.0f, 0.0f, 1.0f); // Синий цвет
//     glVertex3f(0.0f, 1.5f, 0.0f); // Вершина (нос)
//     glVertex3f(0.5f, 0.0f, -0.5f);  // Правый задний угол основания
//     glVertex3f(0.5f, 0.0f, 0.5f);   // Правый передний угол основания

//     glColor3f(1.0f, 1.0f, 0.0f); // Жёлтый цвет
//     glVertex3f(0.0f, 1.5f, 0.0f); // Вершина (нос)
//     glVertex3f(0.5f, 0.0f, 0.5f);   // Правый передний угол основания
//     glVertex3f(-0.5f, 0.0f, 0.5f);  // Левый передний угол основания

//     glColor3f(1.0f, 0.0f, 1.0f); // Фиолетовый цвет
//     glVertex3f(0.0f, 1.5f, 0.0f); // Вершина (нос)
//     glVertex3f(-0.5f, 0.0f, 0.5f);  // Левый передний угол основания
//     glVertex3f(-0.5f, 0.0f, -0.5f); // Левый задний угол основания

//     glEnd();

//     // Основание пирамиды
//     glBegin(GL_QUADS);
//     glColor3f(1.0f, 1.0f, 0.0f); // Жёлтый цвет
//     glVertex3f(-0.5f, 0.0f, -0.5f); // Левый задний угол
//     glVertex3f(0.5f, 0.0f, -0.5f);  // Правый задний угол
//     glVertex3f(0.5f, 0.0f, 0.5f);   // Правый передний угол
//     glVertex3f(-0.5f, 0.0f, 0.5f);  // Левый передний угол
//     glEnd();
// }

void drawCube() {
    glBindTexture(GL_TEXTURE_2D, cubeTexture);
    glEnable(GL_TEXTURE_2D);

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

// Отрисовка сцены
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawBackground(); // Отрисовка фона

    glLoadIdentity();
    float camX = radius * cos(angle);
    float camY = height;
    float camZ = radius * sin(angle);
    gluLookAt(camX, camY, camZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    drawCube();

    // glPushMatrix();
    // glRotatef(0.0f, 0.0f, 0.0f, 1.0f); // Повернуть пирамиду на бок
    // drawCheese(); // Отрисовка пирамиды вместо куба
    // glPopMatrix();

    glutSwapBuffers();
}


// Установка перспективы
void setupProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// Настройка OpenGL
void setup() {
    glEnable(GL_DEPTH_TEST); // Включаем тест глубины
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Серый фон
    loadTexture("/home/snowwy/Desktop/code/_0alg/git/comp_graphics/comp_graphics/lab3/assets/background.jpg", backgroundTexture); // Загрузка текстуры для фона
    loadTexture("/home/snowwy/Desktop/code/_0alg/git/comp_graphics/comp_graphics/lab3/assets/cube.jpg", cubeTexture);             // Загрузка текстуры для куба

    setupProjection(); // Устанавливаем перспективу
}

// Обновление состояния
void update() {
    if (autoRotate) {
        angle += 0.01f; // Автоматическое вращение
    }
    if (movingLeft) {
        angle += 0.05f;
    }
    if (movingRight) {
        angle -= 0.05f;
    }
    if (movingUp) {
        height += 0.1f;
    }
    if (movingDown) {
        height -= 0.1f;
    }

    glutPostRedisplay();
}

// Обработчик нажатия специальных клавиш
void keyboardPress(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_DOWN:
            movingDown = true;
            break;
        case GLUT_KEY_UP:
            movingUp = true;
            break;
        case GLUT_KEY_LEFT:
            movingLeft = true;
            break;
        case GLUT_KEY_RIGHT:
            movingRight = true;
            break;
    }
}

// Обработчик отпускания специальных клавиш
void keyboardRelease(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_DOWN:
            movingDown = false;
            break;
        case GLUT_KEY_UP:
            movingUp = false;
            break;
        case GLUT_KEY_LEFT:
            movingLeft = false;
            break;
        case GLUT_KEY_RIGHT:
            movingRight = false;
            break;
    }
}

// Обработчик клавиш клавиатуры
void normalKeyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'e': // Остановка или запуск автоматического вращения
        case 'E':
            autoRotate = !autoRotate;
            break;
    }
}


// Главная функция
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Cube with Background");

    setup();        // Загрузка текстуры для куба

    glutDisplayFunc(display);
    glutSpecialFunc(keyboardPress);   // Регистрация нажатия специальных клавиш
    glutSpecialUpFunc(keyboardRelease); // Регистрация отпускания специальных клавиш
    glutKeyboardFunc(normalKeyboard); // Регистрация обычных клавиш
    glutIdleFunc(update); // Регистрация функции для обновления покадрово

    glutMainLoop();
    return 0;
}

