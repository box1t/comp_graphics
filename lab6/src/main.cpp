
#include "stb_image.h"
#include <GL/glut.h>

#include "scene.h"   // Для управления сценой
#include "render.h"  // Для рендеринга
#include "camera.h"  // Для работы с камерой
#include "hud.h"     // Для отображения интерфейса


// Флаг автоматического вращения
bool autoRotate = true;
float objAngle = 0.0f; // Угол вращения объекта


// Обновление состояния
void update() {
    updateCamera();
    if (autoRotate) {
        objAngle += 0.5f;
        if (objAngle >= 360.0f) objAngle -= 360.0f;
    }
    glutPostRedisplay();
}


// Настройка OpenGL
void setup() {
    glEnable(GL_DEPTH_TEST); // Включаем тест глубины
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Чёрный фон
    loadTexture("/home/snowwy/Desktop/MAI/computer_graphics/comp_graphics/lab6/assets/background.jpg", backgroundTexture); // Загрузка текстуры для фона
    loadTexture("/home/snowwy/Desktop/MAI/computer_graphics/comp_graphics/lab6/assets/cube.jpg", cubeTexture);             // Загрузка текстуры для куба

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    //initializeObjects();
    setupProjection(); // Устанавливаем перспективу
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
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);


    glutMainLoop();
    return 0;
}

