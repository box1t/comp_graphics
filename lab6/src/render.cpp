#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/glut.h>
#include "render.h"
#include "objects.h"
#include "lighting.h"
#include "camera.h"
#include "hud.h"

GLuint backgroundTexture;
GLuint cubeTexture;


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

    // Яркость при отрисовке фона:
    float backgroundBrightness = 0.5f;
    glColor3f(backgroundBrightness, backgroundBrightness, backgroundBrightness);

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


void drawLightSource(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(1.0f, 1.0f, 0.0f); // Желтый цвет для источника света
    glutSolidSphere(0.2f, 16, 16); // Рисуем маленький шар
    glPopMatrix();
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

    drawLightSource(0.0f, 5.0f, 0.0f); // Позиция прожектора

    glPushMatrix();
    glRotatef(objAngle, 0.0f, 1.0f, 0.0f); // Вращаем объект
    drawCube();
    glPopMatrix();
    glDisable(GL_LIGHTING);

    // Вывод координат камеры
    std::ostringstream coord_ostream;
    //coord_ostream << "Camera Position: (" << camX << ", " << camY << ", " << camZ << ")";
    coord_ostream << "Camera Position: (" << camX << ", " << camY << ", " << camZ << "), "
    << "Yaw: " << yaw << ", Pitch: " << pitch;
    renderText(10, 560, coord_ostream.str()); // Вывод текста на экране


        // Отображаем FPS
    calculateFPS();
    std::ostringstream fps_ostream;
    fps_ostream << "FPS: " << fps;
    renderText(10, 580, fps_ostream.str());

    glutSwapBuffers();
}

