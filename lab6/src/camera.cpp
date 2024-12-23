#include <GL/glut.h>
#include "camera.h"

// Переменные камеры
bool isPerspective = true; // Флаг для переключения проекций
float camX = 0.0f, camY = 0.0f, camZ = 0.0f; // Позиция камеры
float moveSpeed = 0.1f;                       // Скорость движения камеры

float yaw = 0.0f, pitch = 0.0f;              // Углы вращения камеры
float mouseSensitivity = 0.04f;               // Чувствительность мыши
float zoomLevel = 60.0f;

bool isMouseButtonPressed = false; // Отвечает за левую кнопку мыши
int lastMouseX = 0, lastMouseY = 0; // Последняя позиция мыши

std::unordered_map<int, bool> keyState;



///////////////////                  //////////
                // Выбор проекции   //
/////////////////                  ///////////

void setupProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (isPerspective) {
        gluPerspective(zoomLevel, 800.0f / 600.0f, 0.1f, 100.0f);
    } else {
        float scale = zoomLevel / 45.0f; // Пример масштаба на основе zoomLevel
        glOrtho(-10.0f * scale, 10.0f * scale, -10.0f * scale, 10.0f * scale, 0.1f, 100.0f);
    }
    glMatrixMode(GL_MODELVIEW);
}

// Переключение проекций: Перспективной и Ортогональной
void toggleProjection() {
    isPerspective = !isPerspective;
    setupProjection();
    glutPostRedisplay();
}


///////////////////                  //////////
                // Состояние камеры //
/////////////////                  ///////////


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


// Для продолжения движения камеры при нажатии клавиш мыши
void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON) {
        return; // Игнорируем нажатие правой кнопки мыши
    }
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isMouseButtonPressed = true;
            lastMouseX = x;
            lastMouseY = y;
        } else if (state == GLUT_UP) {
            isMouseButtonPressed = false;
        }
    }
}

// Для вращения камеры мышью
void mouseMotion(int x, int y) {
    int deltaX = x - lastMouseX;
    int deltaY = lastMouseY - y;

    

    lastMouseX = x;
    lastMouseY = y;

    yaw += deltaX * mouseSensitivity * M_PI / 180.0f;
    pitch += deltaY * mouseSensitivity * M_PI / 180.0f;

    if (pitch > M_PI / 2) pitch = M_PI / 2;
    if (pitch < -M_PI / 2) pitch = -M_PI / 2;

    if (yaw > M_PI) yaw -= 2 * M_PI;
    if (yaw < -M_PI) yaw += 2 * M_PI;
}


///////////////////            ///////////
                // Клавиатура //
/////////////////            ///////////


// Обработка нажатия клавиш
void keyboardPress(unsigned char key, int x, int y) {
    switch (key) {
        case 'p':
            toggleProjection();
            break;
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
