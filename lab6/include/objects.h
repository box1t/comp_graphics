#ifndef OBJECTS_H
#define OBJECTS_H

#include <vector>

struct Object {
    float x, y, z;
    float radius;
    bool isObstacle;

    Object(float x, float y, float z, float radius, bool isObstacle);
};

extern std::vector<Object> objects;
extern int removedObjectsCount;

void initializeObjects();
void drawCube();
void drawAxes();

#endif // OBJECTS_H
