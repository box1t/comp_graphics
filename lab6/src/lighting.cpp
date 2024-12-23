#include <GL/glut.h>
#include "lighting.h"


// Функция для настройки освещения
void setupLighting(float camX, float camY, float camZ, float forwardX, float forwardY, float forwardZ) {
    glEnable(GL_LIGHTING);

    // Направленный свет (Directional Light)
    glEnable(GL_LIGHT0);
    float sunDirection[] = {0.0f, -1.0f, -0.5f, 0.0f}; // Свет направлен вниз и вперёд
    float sunAmbient[] = {0.1f, 0.1f, 0.1f, 1.0f}; // Слабая окружающая подсветка
    float sunDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f}; // Мягкий белый свет
    glLightfv(GL_LIGHT0, GL_POSITION, sunDirection);
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);

    // Прожектор (Spotlight)
    glEnable(GL_LIGHT1);
    float spotlightPos[] = {0.0f, 5.0f, 0.0f, 1.0f}; // Позиция прожектора над сценой
    float spotlightDir[] = {0.0f, -1.0f, 0.0f}; // Направление вниз
    float spotlightAmbient[] = {0.1f, 0.1f, 0.1f, 1.0f}; // Слабый окружающий свет
    float spotlightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Яркий белый свет
    glLightfv(GL_LIGHT1, GL_POSITION, spotlightPos);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotlightDir);
    glLightfv(GL_LIGHT1, GL_AMBIENT, spotlightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, spotlightDiffuse);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0f); // Угол конуса света
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 20.0f); // Экспоненциальное затухание
}

