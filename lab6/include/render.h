#ifndef RENDER_H
#define RENDER_H

#include <vector>


// Текстура для заднего фона
extern GLuint backgroundTexture;
extern GLuint cubeTexture;

// Внешние переменные камеры
extern float yaw, pitch;
extern float camX, camY, camZ;

// Внешние переменные вращения
extern float objAngle;

// FPS для отображения
extern float fps;

void display();
void drawBackground();
void loadTexture(const char* filename, GLuint& texture);

#endif // RENDER_H
