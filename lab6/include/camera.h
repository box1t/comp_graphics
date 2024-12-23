#ifndef CAMERA_H
#define CAMERA_H

#include <unordered_map>
#include <cmath>
#include <cstdio>

void setupProjection();
void updateCamera();
void keyboardPress(unsigned char key, int x, int y);
void mouseMotion(int x, int y);
void mouseButton(int button, int state, int x, int y);

#endif // CAMERA_H
