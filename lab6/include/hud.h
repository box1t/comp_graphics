#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <chrono>


void calculateFPS();
void renderText(float x, float y, const std::string& text);
void keyboardRelease(unsigned char key, int x, int y);
void specialKeyPress(int key, int x, int y);
void specialKeyRelease(int key, int x, int y);

#endif // UTILS_H
