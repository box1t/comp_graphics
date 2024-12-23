#include <GL/glut.h>
#include "objects.h"
#include "render.h"


std::vector<Object> objects;
int removedObjectsCount = 0;

Object::Object(float x, float y, float z, float radius, bool isObstacle)
    : x(x), y(y), z(z), radius(radius), isObstacle(isObstacle) {}

void initializeObjects() {
    objects.emplace_back(0.0f, 0.0f, 0.0f, 1.0f, true); // Пример инициализации
}







void drawCube() {
    glBindTexture(GL_TEXTURE_2D, cubeTexture);
    glEnable(GL_TEXTURE_2D);

        // Подсветка
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    //float lightPos[] = {2.0f, 4.0f, 2.0f, 1.0f}; // Позиция света
    //glLightfv(GL_LIGHT0, GL_POSITION, lightPos);


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
