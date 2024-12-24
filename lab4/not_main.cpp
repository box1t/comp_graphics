//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <chrono>
#include <random>
#include <thread>
#include <cmath>
#include <unordered_map>


const char* ray_tracing_fragment = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 viewPos;      // Позиция камеры
uniform vec3 lightPos;     // Позиция источника света
uniform int maxBounces;    // Максимальное количество отражений
uniform int materialType;  // Тип материала: 0 = матовый, 1 = зеркальный, 2 = преломляющий
uniform vec3 objectColor;  // Цвет объекта

const float airIOR = 1.0;  // Индекс преломления воздуха
const float glassIOR = 1.5; // Индекс преломления стекла

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    int materialType;
};

// Сцена (максимально 10 объектов)
uniform Sphere spheres[10];
uniform int sphereCount;

// Функция пересечения луча со сферой
bool intersectSphere(Ray ray, Sphere sphere, out float t) {
    vec3 oc = ray.origin - sphere.center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0.0) return false;

    t = (-b - sqrt(discriminant)) / (2.0 * a);
    return t > 0.0;
}

// Отражение вектора
vec3 reflectRay(vec3 dir, vec3 normal) {
    return dir - 2.0 * dot(dir, normal) * normal;
}

// Преломление вектора
vec3 refractRay(vec3 dir, vec3 normal, float ior) {
    float cosi = clamp(-1.0, 1.0, dot(dir, normal));
    float etai = airIOR, etat = ior;
    vec3 n = normal;
    if (cosi < 0.0) {
        cosi = -cosi;
    } else {
        swap(etai, etat);
        n = -normal;
    }
    float eta = etai / etat;
    float k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    return k < 0.0 ? vec3(0.0) : eta * dir + (eta * cosi - sqrt(k)) * n;
}

// Основная функция трассировки
vec3 traceRay(Ray ray, int depth) {
    if (depth > maxBounces) return vec3(0.0);

    float closestT = 1e6;
    Sphere hitSphere;
    bool hit = false;

    // Найти ближайшее пересечение
    for (int i = 0; i < sphereCount; ++i) {
        float t;
        if (intersectSphere(ray, spheres[i], t) && t < closestT) {
            closestT = t;
            hitSphere = spheres[i];
            hit = true;
        }
    }

    if (!hit) return vec3(0.1); // Фоновый цвет

    vec3 hitPoint = ray.origin + closestT * ray.direction;
    vec3 normal = normalize(hitPoint - hitSphere.center);
    vec3 lightDir = normalize(lightPos - hitPoint);

    // Освещение
    vec3 ambient = 0.1 * hitSphere.color;
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * hitSphere.color;

    // Проверка тени
    Ray shadowRay = Ray(hitPoint + 0.001 * normal, lightDir);
    for (int i = 0; i < sphereCount; ++i) {
        float t;
        if (intersectSphere(shadowRay, spheres[i], t)) {
            return ambient;
        }
    }

    // Рефлексия или преломление
    vec3 specular = vec3(0.0);
    if (hitSphere.materialType == 1) { // Зеркальный
        Ray reflectedRay = Ray(hitPoint + 0.001 * normal, reflectRay(ray.direction, normal));
        specular = 0.8 * traceRay(reflectedRay, depth + 1);
    } else if (hitSphere.materialType == 2) { // Преломляющий
        Ray refractedRay = Ray(hitPoint - 0.001 * normal, refractRay(ray.direction, normal, glassIOR));
        specular = 0.8 * traceRay(refractedRay, depth + 1);
    }

    return ambient + diffuse + specular;
}

void main() {
    Ray ray;
    ray.origin = viewPos;
    ray.direction = normalize(FragPos - viewPos);

    vec3 color = traceRay(ray, 0);
    FragColor = vec4(color, 1.0);
}
)";


// Вершинный шейдер
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec3 GouraudColor; // Цвет для Gouraud shading
flat out vec3 FlatColor; // Цвет для Flat shading

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;    // Позиция источника света
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // Позиция фрагмента и нормаль
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Рассчёт освещения (Ambient + Diffuse)
    vec3 norm = normalize(Normal);
    vec3 lightDirNormalized = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDirNormalized), 0.0);
    vec3 diffuse = diff * lightColor;

    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Итоговые цвета для затенений
    GouraudColor = (ambient + diffuse) * objectColor; // Gouraud shading
    FlatColor = GouraudColor; // Flat shading использует цвет вершины

    // Позиция вершины
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

)";

// Фрагментный шейдер
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 GouraudColor; // Для Gouraud shading
flat in vec3 FlatColor; // Для Flat shading

uniform vec3 lightPos;    // Позиция источника света
uniform vec3 lightDir;    // Направление света (не используется для Flat и Gouraud)
uniform vec3 viewPos;     // Позиция камеры
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform int shadingMode;  // Режим затенения: 0 = Flat, 1 = Gouraud, 2 = Phong

void main()
{
    vec3 result = vec3(0.0);

    if (shadingMode == 0) {
        // Flat shading
        result = FlatColor;
    } else if (shadingMode == 1) {
        // Gouraud shading
        result = GouraudColor;
    } else if (shadingMode == 2) {
        // Phong shading (освещение рассчитывается в фрагментном шейдере)
        vec3 norm = normalize(Normal);
        vec3 lightDirNormalized = normalize(lightPos - FragPos);

        // Diffuse lighting
        float diff = max(dot(norm, lightDirNormalized), 0.0);
        vec3 diffuse = diff * lightColor;

        // Ambient lighting
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;

        // Specular lighting
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDirNormalized, norm);
        float specularStrength = 0.5;
        float shininess = 32.0;
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * lightColor;

        result = (ambient + diffuse + specular) * objectColor;
    }

    FragColor = vec4(result, 1.0);
}
)";

/////////////////           /////////////////
            // Константы и освещение
/////////////////           /////////////////

// Обработка клавиш для управления камерой
std::unordered_map<int, bool> keyState;

// параметр освещения
int shadingMode = 2; // По умолчанию Phong Shading


// Сфера для рейтрейсинга
struct Sphere {
    glm::vec3 center;
    float radius;
    glm::vec3 color;
    int materialType; // 0 = матовый, 1 = зеркальный, 2 = преломляющий

    Sphere(glm::vec3 c, float r, glm::vec3 col, int type)
        : center(c), radius(r), color(col), materialType(type) {}
};

/////
enum RenderMode {
    FORWARD_RENDERING,
    RAY_TRACING
};

RenderMode currentMode = FORWARD_RENDERING;



unsigned int rayTracingShader;


// Размеры окна
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;
const int MAX_OBJECTS = 100; // Максимальное количество объектов
bool fightModeActive = false; // Глобальная переменная
const float COLLISION_RADIUS_SQ = 1.0f; // Квадрат радиуса коллизи
int lightMode = 0; // 0 = Ambient, 1 = Diffuse, 2 = All




/////////////////           /////////////////
            // Камера и мышь
/////////////////           /////////////////

// Переменные для управления камерой
float camX = 0.0f, camY = 1.0f, camZ = 5.0f; // Позиция камеры
float yaw = -90.0f, pitch = 0.0f;           // Углы поворота камеры
float moveSpeed = 2.5f;                     // Скорость движения камеры
float mouseSensitivity = 1.1f;             // Чувствительность мыши
float zoomLevel = 45.0f;                   // Угол обзора
bool isPerspective = true;                 // Перспективная или ортографическая проекция
bool firstMouse = true;                    // Для сброса позиции мыши
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;


// Камера
struct Camera {
    
    glm::vec3 position, front, up, right, worldUp;
    float yaw, pitch, moveSpeed = 2.5f, mouseSensitivity = 0.1f, zoomLevel = 45.0f;

        Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
        : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch) {
        updateCameraVectors();
    }

        glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }
        void rotate(float deltaYaw) {
        yaw += deltaYaw;
        updateCameraVectors();
    }

    void changeHeight(float deltaHeight) {
        position.y += deltaHeight;
    }

    glm::vec3 getPosition() const {
        return position;
    }

        void processKeyboard(int direction, float deltaTime) {
        float velocity = moveSpeed * deltaTime;
        if (direction == GLFW_KEY_W)
            position += front * velocity;
        if (direction == GLFW_KEY_S)
            position -= front * velocity;
        if (direction == GLFW_KEY_A)
            position -= right * velocity;
        if (direction == GLFW_KEY_D)
            position += right * velocity;
    }

    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (constrainPitch) {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        updateCameraVectors();
            std::cout << "Yaw: " << yaw << ", Pitch: " << pitch << std::endl;
    }

    void processMouseScroll(float yoffset) {
        if (zoomLevel >= 1.0f && zoomLevel <= 45.0f)
            zoomLevel -= yoffset;
        if (zoomLevel <= 1.0f)
            zoomLevel = 1.0f;
        if (zoomLevel >= 45.0f)
            zoomLevel = 45.0f;
    }

private:
    void updateCameraVectors() {
        glm::vec3 frontVec;
        frontVec.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        frontVec.y = sin(glm::radians(pitch));
        frontVec.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(frontVec);
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};

Camera* globalCamera = nullptr; // Указатель на камеру


// Настройка проекции
glm::mat4 setupProjection(const Camera& camera) {
    if (isPerspective) {
        return glm::perspective(glm::radians(camera.zoomLevel), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    } else {
        float scale = camera.zoomLevel / 45.0f;
        return glm::ortho(-10.0f * scale, 10.0f * scale, -10.0f * scale, 10.0f * scale, 0.1f, 100.0f);
    }
}

void toggleProjection() {
    isPerspective = !isPerspective;
        std::cout << (isPerspective ? "Perspective projection enabled" : "Orthographic projection enabled") << std::endl;
}


// Обработка движения мыши
void mouseCallback(GLFWwindow* window, double xpos, double ypos, Camera& camera) {
    static bool firstMouse = true;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Перевёрнуто, так как ось Y в оконных системах растёт вниз

    lastX = xpos;
    lastY = ypos;

    std::cout << "Mouse moved. X offset: " << xoffset << ", Y offset: " << yoffset << std::endl;

    if (globalCamera) {
        globalCamera->processMouseMovement(xoffset, yoffset);
    }
}



/////////////////           /////////////////
            // Игровые объекты
/////////////////           /////////////////

// Функция для создания куба
unsigned int createCube() {
    float vertices[] = {
            // Позиции                      // Нормали
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, // Задняя грань
            0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, // Передняя грань
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,

            -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f,  0.0f, // Левая грань
            -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

            0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // Правая грань
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f, // Нижняя грань
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // Верхняя грань
            0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

// Функция для создания пирамиды
unsigned int createPyramid() {
    float vertices[] = {
            // Позиции                      // Нормали
            // Боковые грани
            0.0f,  0.5f,  0.0f,  -0.894f, 0.447f, 0.0f, // левый
            -0.5f, -0.5f,  0.5f, -0.894f, 0.447f, 0.0f,
            -0.5f, -0.5f, -0.5f, -0.894f, 0.447f, 0.0f,

            0.0f,  0.5f,  0.0f,  0.894f, 0.447f, 0.0f, // правый
            0.5f, -0.5f, -0.5f, 0.894f, 0.447f, 0.0f,
            0.5f, -0.5f,  0.5f, 0.894f, 0.447f, 0.0f,

            0.0f,  0.5f,  0.0f,  0.0f, 0.447f, 0.894f, // перед
            0.5f, -0.5f,  0.5f, 0.0f, 0.447f, 0.894f,
            -0.5f, -0.5f,  0.5f, 0.0f, 0.447f, 0.894f,

            0.0f,  0.5f,  0.0f,  0.0f, 0.447f, -0.894f, // зад
            -0.5f, -0.5f, -0.5f, 0.0f, 0.447f, -0.894f,
            0.5f, -0.5f, -0.5f, 0.0f, 0.447f, -0.894f,

            // Основание (два треугольника)
            // Первый треугольник
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,
            0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f,   0.0f, -1.0f, 0.0f,

            // Второй треугольник
            0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f,   0.0f, -1.0f, 0.0f,
            0.5f, -0.5f, 0.5f,    0.0f, -1.0f, 0.0f
    };
 
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

// Создание сферы
unsigned int createSphere(float radius, int stacks, int slices) {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<unsigned int> indices;

    // Генерация вершин и нормалей
    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI / 2 - M_PI * i / stacks; // Угол по вертикали
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * M_PI * j / slices; // Угол по горизонтали

            // Позиция вершины
            float x = radius * cos(phi) * cos(theta);
            float y = radius * cos(phi) * sin(theta);
            float z = radius * sin(phi);

            // Добавление координат вершины
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Нормали
            normals.push_back(x / radius);
            normals.push_back(y / radius);
            normals.push_back(z / radius);
        }
    }

    // Генерация индексов для треугольников
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;

            // Первое треугольник
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            // Второе треугольник
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    // Создание VAO и VBO
    unsigned int VAO, VBO[2], EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO); // Создаем два VBO: один для вершин, другой для нормалей
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Вершины
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Нормали
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Индексы
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return VAO;
}

// Создание пола

unsigned int createFloor(float width, float depth) {
    // Вершины пола с текстурными координатами
    float floorVertices[] = {
        // Позиции            // Текстурные координаты
         width / 2, 0.0f,  depth / 2,  1.0f, 1.0f, // Верхний правый
        -width / 2, 0.0f,  depth / 2,  0.0f, 1.0f, // Верхний левый
        -width / 2, 0.0f, -depth / 2,  0.0f, 0.0f, // Нижний левый

         width / 2, 0.0f,  depth / 2,  1.0f, 1.0f, // Верхний правый
        -width / 2, 0.0f, -depth / 2,  0.0f, 0.0f, // Нижний левый
         width / 2, 0.0f, -depth / 2,  1.0f, 0.0f  // Нижний правый
    };

    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);

    // Загрузка данных в буфер
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    // Установка атрибутов вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);          // Позиции
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // Текстурные координаты
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return floorVAO;
}


/////////////////           /////////////////
            // Менеджер сцены
/////////////////           /////////////////

// Загрузка текстуры
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Параметры текстуры
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}


// Тип объекта
enum ObjectType {
    ENEMY,
    FOOD,
    AMBIENT,
    GROUND
};


// Объект сцены
struct SceneObject {
    unsigned int VAO;
    glm::vec3 position;
    glm::mat4 rotation = glm::mat4(1.0f); // Матрица для хранения вращения
    int vertexCount;
    bool useIndices;
    ObjectType type; // нужно обновить конструктор

    SceneObject(unsigned int vao, glm::vec3 pos, int vCount, bool indices, ObjectType objType)
        : VAO(vao), position(pos), vertexCount(vCount), useIndices(indices), type(objType) {}

};

// Сцена (не переделать, пока камера неисправна)
// а исправление камеры влечет за собой изменение сцены

class Scene {
public:
    RenderMode renderMode = FORWARD_RENDERING;
    std::vector<SceneObject> objects;
    int enemyCounter = 0;
    int foodCounter = 0;
    int ambientCounter = 0;
    int foodConsumed = 0; // Счетчик съеденной еды

    int getObjectCounter() const {
        return enemyCounter + foodCounter + ambientCounter;
    }

    void addObject(const SceneObject& obj) {
        if (objects.size() < MAX_OBJECTS) {
            objects.push_back(obj);
            if (obj.type == ENEMY) enemyCounter++;
            else if (obj.type == FOOD) foodCounter++;
            else if (obj.type == AMBIENT) ambientCounter++;
        }
    }

    void clearObjects() {
        objects.clear();
        enemyCounter = 0;
        foodCounter = 0;
        ambientCounter = 0;
        foodConsumed = 0;
        std::cout << "All objects cleared from the scene." << std::endl;
    }

    void saveScene(const std::string& filename) {
        std::ofstream file(filename, std::ios::out);
        if (file.is_open()) {
            file << getObjectCounter() << " " << foodConsumed << "\n";
            file << enemyCounter << " " << foodCounter << " " << ambientCounter << "\n";

            for (const auto& obj : objects) {
                file << obj.type << " "
                     << obj.position.x << " "
                     << obj.position.y << " "
                     << obj.position.z << "\n";
            }
            file.close();
            std::cout << "Scene saved to " << filename << std::endl;
        }
    }

    void loadScene(const std::string& filename, unsigned int cubeVAO, unsigned int pyramidVAO, unsigned int sphereVAO, unsigned int floorVAO) {
        std::ifstream file(filename, std::ios::in);
        if (file.is_open()) {
            objects.clear();
            enemyCounter = 0;
            foodCounter = 0;
            ambientCounter = 0;
            foodConsumed = 0;
            int objectCount;

            file >> objectCount >> foodConsumed; // Читаем количество объектов и съеденной еды

            file >> enemyCounter >> foodCounter >> ambientCounter; // Читаем детализацию объектов

            int type;
            float x, y, z;
            while (file >> type >> x >> y >> z) {
                unsigned int vao = 0;
                switch (static_cast<ObjectType>(type)) {
                    case ENEMY: vao = cubeVAO; break;
                    case FOOD: vao = pyramidVAO; break;
                    case AMBIENT: vao = sphereVAO; break;
                    case GROUND: vao = floorVAO; break; // Добавляем пол
                }
                addObject(SceneObject(vao, glm::vec3(x, y, z), 36, false, static_cast<ObjectType>(type)));
            }

            file.close();
            std::cout << "Scene loaded from " << filename << ". "
                      << "Objects: " << getObjectCounter() << ", Food consumed: " << foodConsumed << ", "
                      << "Enemy: " << enemyCounter << ", Food: " << foodCounter << ", Ambient: " << ambientCounter << std::endl;
        }
    }

    void updateEnemy(SceneObject& enemy, const glm::vec3& cameraPosition) {
        glm::vec3 direction = glm::normalize(cameraPosition - enemy.position);
        enemy.position += direction * 0.01f; // Скорость движения
    }

    bool checkCollision(const glm::vec3& objPosition, const glm::vec3& cameraPosition, float collisionRadiusSq) {
        float distanceSq = glm::length(cameraPosition - objPosition);
        return distanceSq * distanceSq < collisionRadiusSq;
    }

    void handleEnemyCollision(SceneObject& enemy, std::vector<SceneObject>& objects, int& enemyCounter) {
        enemy = objects.back();
        objects.pop_back();
        enemyCounter--;
        std::cout << "Enemy reached the camera! Enemies left: " << enemyCounter << std::endl;
    }

    void updateFood(SceneObject& food, const glm::vec3& cameraPosition, std::vector<SceneObject>& objects, int& foodCounter, int& foodConsumed) {
        if (checkCollision(food.position, cameraPosition, COLLISION_RADIUS_SQ)) {
            food = objects.back();
            objects.pop_back();
            foodCounter--;
            foodConsumed++;
            std::cout << "Food consumed! Total: " << foodConsumed << std::endl;
        } else {
            food.rotation = glm::rotate(food.rotation, glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }

    void renderObject(const SceneObject& obj, int modelLoc) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), obj.position);
        model *= obj.rotation; // Добавляем вращение объекта
        
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(obj.VAO);
        if (obj.useIndices) {
            glDrawElements(GL_TRIANGLES, obj.vertexCount, GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
        }
    }

    void render(unsigned int shaderProgram, const glm::vec3& cameraPosition) {
        int modelLoc = glGetUniformLocation(shaderProgram, "model");

        for (size_t i = 0; i < objects.size(); ++i) {
            SceneObject& obj = objects[i];

            if (obj.type == ENEMY) {
                updateEnemy(obj, cameraPosition);

                if (checkCollision(obj.position, cameraPosition, COLLISION_RADIUS_SQ)) {
                    handleEnemyCollision(obj, objects, enemyCounter);
                    --i; // Так как объект удалён, корректируем индекс.
                    continue;
                }
            } else if (obj.type == FOOD) {
                updateFood(obj, cameraPosition, objects, foodCounter, foodConsumed);
            }

            renderObject(obj, modelLoc);
        }
    }

    void renderFloor(SceneObject& obj, unsigned int shaderProgram, unsigned int floorTexture) {
        glBindTexture(GL_TEXTURE_2D, floorTexture);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), obj.position);
        model *= obj.rotation;

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(obj.VAO);
        glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
    }

    // void render(unsigned int shaderProgram, const glm::vec3& cameraPosition) {
    //     int modelLoc = glGetUniformLocation(shaderProgram, "model");

    //     for (auto& obj : objects) {
    //         if (obj.type == ENEMY) {
    //             glm::vec3 direction = glm::normalize(cameraPosition - obj.position);
    //             obj.position += direction * 0.01f; // Скорость движения

    //             float distanceSq = glm::length(cameraPosition - obj.position);
    //             distanceSq *= distanceSq; // Квадрат расстояния
    //             if (distanceSq < COLLISION_RADIUS_SQ) {
    //                 obj = objects.back();
    //                 objects.pop_back();
    //                 enemyCounter--;
    //                 std::cout << "Enemy reached the camera! Enemies left: " << enemyCounter << std::endl;
    //             }
    //         } else if (obj.type == FOOD) {
    //             float distanceSq = glm::length(cameraPosition - obj.position);
    //             distanceSq *= distanceSq; // Квадрат расстояния
    //             if (distanceSq < COLLISION_RADIUS_SQ) {
    //                 obj = objects.back();
    //                 objects.pop_back();
    //                 foodCounter--;
    //                 foodConsumed++;
    //                 std::cout << "Food consumed! Total: " << foodConsumed << std::endl;
    //                 continue;
    //             }
    //             obj.position = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(0, 1, 0)) * glm::vec4(obj.position, 1.0);
    //         }

    //         glm::mat4 model = glm::translate(glm::mat4(1.0f), obj.position);
    //         glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //         glBindVertexArray(obj.VAO);
    //         if (obj.useIndices) {
    //             glDrawElements(GL_TRIANGLES, obj.vertexCount, GL_UNSIGNED_INT, nullptr);
    //         } else {
    //             glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
    //         }
    //     }
    
    // }


        // for (const auto& obj : objects) {
        //     glm::mat4 model = glm::translate(glm::mat4(1.0f), obj.position);
        //     glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        //     glBindVertexArray(obj.VAO);
        //     if (obj.useIndices) {
        //         glDrawElements(GL_TRIANGLES, obj.vertexCount, GL_UNSIGNED_INT, nullptr);
        //     } else {
        //         glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
        //     }
        // }

};


/////////////////           /////////////////
            // Игровая логика
/////////////////           /////////////////

// Генерация точек для спавна объектов сцены
float randomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

// Игровой режим
void fightMode(Scene& currentScene, unsigned int cubeVAO, unsigned int sphereVAO) {
    if (!fightModeActive) return;

    static auto lastAddTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();

    float minRadius = 5.0f;
    float maxRadius = 15.0f;
    float fixedHeight = 0.0f; // Высота, на которой размещаются объекты

    if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastAddTime).count() >= 5) {
        lastAddTime = currentTime;

        if (currentScene.objects.size() + 3 > MAX_OBJECTS) {
            fightModeActive = false;
            std::cout << "Fight mode deactivated. Maximum object limit reached." << std::endl;
            return;
        }

            
         // Добавляем один куб
        float cubeAngle = randomFloat(0.0f, 360.0f);
        float cubeRadius = randomFloat(minRadius, maxRadius);
        glm::vec3 randomCubePosition = glm::vec3(
            cubeRadius * cos(glm::radians(cubeAngle)),
            fixedHeight,
            cubeRadius * sin(glm::radians(cubeAngle))
        );

        currentScene.addObject({cubeVAO, randomCubePosition, 36, false, FOOD});
        std::cout << "Появился сыр" << std::endl;

        // Добавляем две сферы
        for (int i = 0; i < 2; ++i) {
            float sphereAngle = randomFloat(0.0f, 360.0f);
            float sphereRadius = randomFloat(minRadius, maxRadius);
            glm::vec3 randomSpherePosition = glm::vec3(
                sphereRadius * cos(glm::radians(sphereAngle)),
                fixedHeight,
                sphereRadius * sin(glm::radians(sphereAngle))
            );

            currentScene.addObject({sphereVAO, randomSpherePosition, 2400, true, ENEMY});
        }
        std::cout << "Появились кошки. Держите ушки на макушке!" << std::endl; 
        
    }
}


// Отсчет до боевого режима
void countdownAndActivateFightMode(bool& fightModeActive) {
    std::cout << "Get ready..." << std::endl;
    for (int i = 5; i > 0; --i) {
        std::cout << i << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "FIGHT!" << std::endl;
    fightModeActive = true;
}



/////////////////           /////////////////
        // Менеджер введенных команд
/////////////////           /////////////////

// Обработка клавиш для выхода
void handleExitInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void processZoom(GLFWwindow* window, Camera& camera, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) { // Увеличение масштаба
        camera.zoomLevel -= 2.0f * deltaTime;
        if (camera.zoomLevel < 10.0f)
            camera.zoomLevel = 10.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) { // Уменьшение масштаба
        camera.zoomLevel += 2.0f * deltaTime;
        if (camera.zoomLevel > 90.0f)
            camera.zoomLevel = 90.0f;
    }
}


// Обработка ввода для управления камерой
void handleCameraInput(GLFWwindow* window, Camera& camera, float deltaTime) {
    static bool toggleKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_W, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_S, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_A, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_D, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !toggleKeyPressed) {
        toggleProjection();
        toggleKeyPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        toggleKeyPressed = false;
    }
}

// А есть ли всё ещё параметр высоты? или уже устарел?
// Обработка вращения и высоты камеры
void handleCameraMovement(GLFWwindow* window, Camera& camera) {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.rotate(0.05f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.rotate(-0.05f);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.changeHeight(0.05f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.changeHeight(-0.05f);
}

// Обработка переключения режимов
void handleModeSwitch(GLFWwindow* window) {
    static bool keyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !keyPressed) {
        keyPressed = true;
        currentMode = (currentMode == FORWARD_RENDERING) ? RAY_TRACING : FORWARD_RENDERING;
        std::cout << (currentMode == RAY_TRACING ? "Ray Tracing Mode" : "Forward Rendering Mode") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        keyPressed = false;
    }
}

// Переключение режимов шейдинга
void handleShadingMode(GLFWwindow* window) {
    static bool keyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !keyPressed) {
        keyPressed = true;
        shadingMode = (shadingMode + 1) % 3; // Переключаем между 0, 1, 2
        if (shadingMode == 0) std::cout << "Flat Shading enabled." << std::endl;
        else if (shadingMode == 1) std::cout << "Gouraud Shading enabled." << std::endl;
        else if (shadingMode == 2) std::cout << "Phong Shading enabled." << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        keyPressed = false;
    }
}

// Управление освещением
void handleLightingInput(GLFWwindow* window) {
    static bool toggleKeyJPressed = false;
    static bool toggleKeyKPressed = false;

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && !toggleKeyJPressed) {
        toggleKeyJPressed = true;
        lightMode = (lightMode + 1) % 3;
        std::cout << "Light mode: " << (lightMode == 0 ? "Ambient" : (lightMode == 1 ? "Diffuse" : "All")) << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE) {
        toggleKeyJPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !toggleKeyKPressed) {
        toggleKeyKPressed = true;
        lightMode = 2; // All modes
        std::cout << "Light mode: All" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE) {
        toggleKeyKPressed = false;
    }
}

// Обработка добавления объектов
void handleObjectCreation(GLFWwindow* window, Scene& currentScene, unsigned int cubeVAO, unsigned int pyramidVAO, unsigned int sphereVAO) {
    static bool addCubePressed = false;
    static bool addPyramidPressed = false;
    static bool addSpherePressed = false;

    float radius = 5.0f;
    float fixedHeight = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && !addCubePressed) {
        addCubePressed = true;
        glm::vec3 randomPosition = glm::vec3(randomFloat(-radius, radius), fixedHeight, randomFloat(-radius, radius));
        currentScene.addObject({cubeVAO, randomPosition, 36, false, FOOD});
        std::cout << "Added cube at position: (" << randomPosition.x << ", " << randomPosition.y << ", " << randomPosition.z << ")" << std::endl;
    } else {
        addCubePressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS && !addPyramidPressed) {
        addPyramidPressed = true;
        glm::vec3 randomPosition = glm::vec3(randomFloat(-radius, radius), fixedHeight, randomFloat(-radius, radius));
        currentScene.addObject({pyramidVAO, randomPosition, 18, false, AMBIENT});
        std::cout << "Added pyramid at position: (" << randomPosition.x << ", " << randomPosition.y << ", " << randomPosition.z << ")" << std::endl;
    } else {
        addPyramidPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS && !addSpherePressed) {
        addSpherePressed = true;
        glm::vec3 randomPosition = glm::vec3(randomFloat(-radius, radius), fixedHeight, randomFloat(-radius, radius));
        currentScene.addObject({sphereVAO, randomPosition, 2400, true, ENEMY});
        std::cout << "Added sphere at position: (" << randomPosition.x << ", " << randomPosition.y << ", " << randomPosition.z << ")" << std::endl;
    } else {
        addSpherePressed = false;
    }

            // Очистка объектов на клавишу 9
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
        currentScene.clearObjects();
    }
}

// Сохранение и загрузка сцены
void handleSceneSaveLoad(GLFWwindow* window, Scene& currentScene, unsigned int cubeVAO, unsigned int pyramidVAO, unsigned int sphereVAO, unsigned int floorVAO) {
    static bool saveKeyPressed = false;
    static bool loadKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !saveKeyPressed) {
        saveKeyPressed = true;
        currentScene.saveScene("scene.txt");
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
        saveKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !loadKeyPressed) {
        loadKeyPressed = true;
        currentScene.loadScene("scene.txt", cubeVAO, pyramidVAO, sphereVAO, floorVAO);
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
        loadKeyPressed = false;
    }
}

// Переключение между сценами
void handleSceneSwitch(GLFWwindow* window, int& currentSceneIndex, int sceneCount) {
    static int lastSceneIndex = currentSceneIndex;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        currentSceneIndex = 0;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        currentSceneIndex = 1;

    if (currentSceneIndex != lastSceneIndex) {
        std::cout << "Switched to scene " << currentSceneIndex + 1 << std::endl;
        lastSceneIndex = currentSceneIndex;
    }
}

// Активация и деактивация режима боя
void handleFightMode(GLFWwindow* window, Scene& currentScene, unsigned int cubeVAO, unsigned int sphereVAO) {
    static bool toggleKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
        if (!toggleKeyPressed) {
            toggleKeyPressed = true;
            if (fightModeActive) {
                fightModeActive = false;
                std::cout << "Fight mode deactivated." << std::endl;
            } else {
                std::thread(countdownAndActivateFightMode, std::ref(fightModeActive)).detach();
            }
        }
    } else {
        toggleKeyPressed = false;
    }
    fightMode(currentScene, cubeVAO, sphereVAO);
}

// разбитая на модули
void processInput(GLFWwindow* window, Camera& camera, int& currentSceneIndex, int sceneCount, Scene& currentScene, unsigned int cubeVAO, unsigned int pyramidVAO, unsigned int sphereVAO, float deltaTime) {
    handleExitInput(window);
    handleCameraInput(window, camera, deltaTime);
    handleCameraMovement(window, camera);
    handleModeSwitch(window);
    handleShadingMode(window);
    handleLightingInput(window);
    processZoom(window, camera, deltaTime);
    handleObjectCreation(window, currentScene, cubeVAO, pyramidVAO, sphereVAO);
    handleSceneSaveLoad(window, currentScene, cubeVAO, pyramidVAO, sphereVAO);
    handleSceneSwitch(window, currentSceneIndex, sceneCount);
    handleFightMode(window, currentScene, cubeVAO, sphereVAO);
}

int main() {
    // Инициализация GLFW
    glfwInit();

    float lastFrame = 0.0f; // Время предыдущего кадра
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Scene", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);


    glewInit();

    // Настройка шейдеров
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
//
//
std::vector<Sphere> spheres;
spheres.push_back(Sphere(glm::vec3(0.0f, 0.0f, -5.0f), 1.0f, glm::vec3(1.0f, 0.0f, 0.0f), 1)); // Зеркальная сфера
spheres.push_back(Sphere(glm::vec3(2.0f, 0.0f, -5.0f), 1.0f, glm::vec3(0.0f, 1.0f, 0.0f), 0)); // Матовая сфера
spheres.push_back(Sphere(glm::vec3(-2.0f, 0.0f, -5.0f), 1.0f, glm::vec3(0.0f, 0.0f, 1.0f), 2)); // Преломляющая сфера

rayTracingShader = glCreateProgram();
//
//
    // // Создание объектов

    unsigned int cubeVAO = createCube();
    unsigned int pyramidVAO = createPyramid();
    unsigned int sphereVAO = createSphere(0.5f, 20, 20);
    SceneObject cubeObj = { cubeVAO, glm::vec3(0.0f, 0.0f, 0.0f), 36, false, ENEMY };
    SceneObject pyramidObj = { pyramidVAO, glm::vec3(2.0f, 0.0f, 0.0f), 18, false, FOOD };
    SceneObject sphereObj = { sphereVAO, glm::vec3(-2.0f, 0.0f, 0.0f), 2400, true, AMBIENT };


    Scene scene1, scene2;
//    scene1.renderMode = Scene::FORWARD_RENDERING;
//    scene2.renderMode = Scene::RAY_TRACING;
    scene1.addObject(cubeObj);
    scene1.addObject(pyramidObj);
    scene2.addObject(sphereObj);
    scene2.addObject(cubeObj);

    int currentSceneIndex = 0;
    std::vector<Scene> scenes = {scene1, scene2};

    
    // Инициализация камеры
    Camera camera(glm::vec3(0.0f, 1.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
    globalCamera = &camera;
    float deltaTime = 0.016f; // Пример значения    
    
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        if (globalCamera) {
            mouseCallback(window, xpos, ypos, *globalCamera);
        }
    });
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // debug camera
    std::cout << "Camera Position: " << glm::to_string(camera.getPosition()) << std::endl;
    for (const auto& obj : scenes[currentSceneIndex].objects) {
        std::cout << "Object Position: " << glm::to_string(obj.position) << std::endl;
    }





    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        processInput(window, camera, currentSceneIndex, scenes.size(), scenes[currentSceneIndex], cubeVAO, pyramidVAO, sphereVAO, deltaTime);

        // Рассчитать прошедшее время
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Очистка экрана
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Получение матриц проекции и вида
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = setupProjection(camera);

        // Установка шейдера
        glUseProgram(shaderProgram);

        // Передача матриц в шейдер
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Универсальная модельная матрица (если объекты не используют уникальные трансформации)
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // Установка параметров освещения
        glUniform1i(glGetUniformLocation(shaderProgram, "shadingMode"), shadingMode);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 0.0f, 5.0f, 5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.8f, 0.2f, 0.2f);
        glUniform1i(glGetUniformLocation(shaderProgram, "lightMode"), lightMode);

        // Рендеринг текущей сцены
        scenes[currentSceneIndex].render(shaderProgram, camera.getPosition());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

        // // Установить uniform-переменные в шейдере
        // glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 0.0f, 0.0f, 5.0f);
        // glUniform3f(glGetUniformLocation(shaderProgram, "lightDir"), 0.0f, 0.0f, -5.0f);
        // glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 0.0f, 0.0f, 1.0f);
        // glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        // glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.2f, 0.3f, 1.0f);

        // // Рендеринг куба
        // model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        // glBindVertexArray(cubeVAO);
        // glDrawArrays(GL_TRIANGLES, 0, 36);

        // // Рендеринг пирамиды
        // model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
        // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        // glBindVertexArray(pyramidVAO);
        // glDrawArrays(GL_TRIANGLES, 0, 18);

        // // Рендеринг сферы
        // model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f));
        // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        // glBindVertexArray(sphereVAO);
        // glDrawElements(GL_TRIANGLES, 2400, GL_UNSIGNED_INT, nullptr);

        // // Обмен буферов и обработка событий
        // glfwSwapBuffers(window);
        // glfwPollEvents();
    

    // Освобождение ресурсов
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteVertexArrays(1, &sphereVAO);
    glfwTerminate();
    return 0;
}