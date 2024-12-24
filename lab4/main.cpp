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

unsigned int rayTracingShader;

// Размеры окна
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;
const int MAX_OBJECTS = 100; // Максимальное количество объектов
bool fightModeActive = false; // Глобальная переменная
const float COLLISION_RADIUS_SQ = 1.0f; // Квадрат радиуса коллизи
int lightMode = 0; // 0 = Ambient, 1 = Diffuse, 2 = All

float randomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

void countdownAndActivateFightMode(bool& fightModeActive) {
    std::cout << "Get ready..." << std::endl;
    for (int i = 5; i > 0; --i) {
        std::cout << i << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "FIGHT!" << std::endl;
    fightModeActive = true;
}

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
int shadingMode = 2; // По умолчанию Phong Shading

////////

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

enum ObjectType {
    ENEMY,
    FOOD,
    AMBIENT
};

struct Camera {
    float radius, angle, height;
    Camera(float r, float a, float h) : radius(r), angle(a), height(h) {}
    glm::vec3 getPosition() const {
        return { radius * std::cos(angle), height, radius * std::sin(angle) }; // x, y, z
    }
    void rotate(float deltaAngle) { angle += deltaAngle; }
    void changeHeight(float deltaHeight) { height += deltaHeight;}
};




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

struct SceneObject {
    unsigned int VAO;
    glm::vec3 position;
    int vertexCount;
    bool useIndices;
    ObjectType type; // нужно обновить конструктор

    SceneObject(unsigned int vao, glm::vec3 pos, int vCount, bool indices, ObjectType objType)
        : VAO(vao), position(pos), vertexCount(vCount), useIndices(indices), type(objType) {}

};

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

    void loadScene(const std::string& filename, unsigned int cubeVAO, unsigned int pyramidVAO, unsigned int sphereVAO) {
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
                }
                addObject(SceneObject(vao, glm::vec3(x, y, z), 36, false, static_cast<ObjectType>(type)));
            }

            file.close();
            std::cout << "Scene loaded from " << filename << ". "
                      << "Objects: " << getObjectCounter() << ", Food consumed: " << foodConsumed << ", "
                      << "Enemy: " << enemyCounter << ", Food: " << foodCounter << ", Ambient: " << ambientCounter << std::endl;
        }
    }


    void render(unsigned int shaderProgram, const glm::vec3& cameraPosition) {
        int modelLoc = glGetUniformLocation(shaderProgram, "model");

        for (auto& obj : objects) {
            if (obj.type == ENEMY) {
                glm::vec3 direction = glm::normalize(cameraPosition - obj.position);
                obj.position += direction * 0.01f; // Скорость движения

                float distanceSq = glm::length(cameraPosition - obj.position);
                distanceSq *= distanceSq; // Квадрат расстояния
                if (distanceSq < COLLISION_RADIUS_SQ) {
                    obj = objects.back();
                    objects.pop_back();
                    enemyCounter--;
                    std::cout << "Enemy reached the camera! Enemies left: " << enemyCounter << std::endl;
                }
            } else if (obj.type == FOOD) {
                float distanceSq = glm::length(cameraPosition - obj.position);
                distanceSq *= distanceSq; // Квадрат расстояния
                if (distanceSq < COLLISION_RADIUS_SQ) {
                    obj = objects.back();
                    objects.pop_back();
                    foodCounter--;
                    foodConsumed++;
                    std::cout << "Food consumed! Total: " << foodConsumed << std::endl;
                    continue;
                }
                obj.position = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(0, 1, 0)) * glm::vec4(obj.position, 1.0);
            }

            glm::mat4 model = glm::translate(glm::mat4(1.0f), obj.position);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glBindVertexArray(obj.VAO);
            if (obj.useIndices) {
                glDrawElements(GL_TRIANGLES, obj.vertexCount, GL_UNSIGNED_INT, nullptr);
            } else {
                glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
            }
        }
    


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
    }



};




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


void processInput(GLFWwindow* window, Camera& camera, int& currentSceneIndex, int sceneCount, Scene& currentScene, unsigned int cubeVAO, unsigned int pyramidVAO, unsigned int sphereVAO) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        currentMode = (currentMode == FORWARD_RENDERING) ? RAY_TRACING : FORWARD_RENDERING;
        std::cout << (currentMode == RAY_TRACING ? "Ray Tracing Mode" : "Forward Rendering Mode") << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.rotate(0.05f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.rotate(-0.05f);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.changeHeight(0.05f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.changeHeight(-0.05f);

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

    static int lastSceneIndex = currentSceneIndex;
    static bool saveKeyPressed = false;
    static bool loadKeyPressed = false;
    static bool toggleKeyJPressed = false;
    static bool toggleKeyKPressed = false;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !saveKeyPressed) {
        saveKeyPressed = true;
        currentScene.saveScene("scene.txt");
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
        saveKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !loadKeyPressed) {
        loadKeyPressed = true;
        currentScene.loadScene("scene.txt", cubeVAO, pyramidVAO, sphereVAO);
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
        loadKeyPressed = false;
    }

        // Переключение между Ambient, Diffuse
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && !toggleKeyJPressed) {
        toggleKeyJPressed = true;
        lightMode = (lightMode + 1) % 3; // 0, 1, 2
        std::cout << "Light mode: " << (lightMode == 0 ? "Ambient" :
                                        lightMode == 1 ? "Diffuse" : "All") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE) {
        toggleKeyJPressed = false;
    }

    // Включение всех режимов сразу
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !toggleKeyKPressed) {
        toggleKeyKPressed = true;
        lightMode = 2; // All modes
        std::cout << "Light mode: All" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE) {
        toggleKeyKPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        currentSceneIndex = 0;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        currentSceneIndex = 1;
    if (currentSceneIndex != lastSceneIndex) {
        std::cout << "Switched to scene " << currentSceneIndex + 1 << std::endl;
        lastSceneIndex = currentSceneIndex;
    }

    // Отсчет и активация fightMode на клавишу 8
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

    static bool addCubePressed = false;
    static bool addPyramidPressed = false;
    static bool addSpherePressed = false;

    float radius = 5.0f;
    float fixedHeight = 0.0f; // Высота, на которой размещаются объекты

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        if (!addCubePressed) {
            addCubePressed = true;
            glm::vec3 randomPosition = glm::vec3(
                randomFloat(-radius, radius),
                fixedHeight,
                randomFloat(-radius, radius)
            );
            currentScene.addObject({cubeVAO, randomPosition, 36, false, FOOD});
            std::cout << "Added cube at position: (" 
                      << randomPosition.x << ", " 
                      << randomPosition.y << ", " 
                      << randomPosition.z << ")" << std::endl;
        }
    } else {
        addCubePressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        if (!addPyramidPressed) {
            addPyramidPressed = true;
            glm::vec3 randomPosition = glm::vec3(
                randomFloat(-radius, radius),
                fixedHeight,
                randomFloat(-radius, radius)
            );
            currentScene.addObject({pyramidVAO, randomPosition, 18, false, AMBIENT});
            std::cout << "Added pyramid at position: (" 
                      << randomPosition.x << ", " 
                      << randomPosition.y << ", " 
                      << randomPosition.z << ")" << std::endl;
        }
    } else {
        addPyramidPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        if (!addSpherePressed) {
            addSpherePressed = true;
            glm::vec3 randomPosition = glm::vec3(
                randomFloat(-radius, radius),
                fixedHeight,
                randomFloat(-radius, radius)
            );
            currentScene.addObject({sphereVAO, randomPosition, 2400, true, ENEMY});
            std::cout << "Added sphere at position: (" 
                      << randomPosition.x << ", " 
                      << randomPosition.y << ", " 
                      << randomPosition.z << ")" << std::endl;
        }
    } else {
        addSpherePressed = false;
    }
        // Очистка объектов на клавишу 9
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
        currentScene.clearObjects();
    }
}




int main() {
    // Инициализация GLFW
    glfwInit();


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
    // unsigned int cubeVAO = createCube();
    // unsigned int pyramidVAO = createPyramid();
    // unsigned int sphereVAO = createSphere(0.5f, 20, 20);
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
    Camera camera(5.0f, glm::radians(90.0f), 1.0f); // Убедитесь, что радиус и высота разумны
    
    // debug camera
    
    std::cout << "Camera Position: " << glm::to_string(camera.getPosition()) << std::endl;
    for (const auto& obj : scenes[currentSceneIndex].objects) {
    std::cout << "Object Position: " << glm::to_string(obj.position) << std::endl;
}




    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        processInput(window, camera, currentSceneIndex, scenes.size(), scenes[currentSceneIndex], cubeVAO, pyramidVAO, sphereVAO);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "shadingMode"), shadingMode);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 0.0f, 5.0f, 5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.8f, 0.2f, 0.2f);
        glUniform1i(glGetUniformLocation(shaderProgram, "lightMode"), lightMode);

        //scenes[currentSceneIndex].render(shaderProgram, camera.getPosition());
//raytrace
//
        glUniform3f(glGetUniformLocation(rayTracingShader, "viewPos"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
        glUniform3f(glGetUniformLocation(rayTracingShader, "lightPos"), 0.0f, 5.0f, 5.0f);
        glUniform1i(glGetUniformLocation(rayTracingShader, "maxBounces"), 3);
        glUniform1i(glGetUniformLocation(rayTracingShader, "sphereCount"), spheres.size());
        for (int i = 0; i < spheres.size(); ++i) {
            Sphere& s = spheres[i];
            glUniform3fv(glGetUniformLocation(rayTracingShader, ("spheres[" + std::to_string(i) + "].center").c_str()), 1, glm::value_ptr(s.center));
            glUniform1f(glGetUniformLocation(rayTracingShader, ("spheres[" + std::to_string(i) + "].radius").c_str()), s.radius);
            glUniform3fv(glGetUniformLocation(rayTracingShader, ("spheres[" + std::to_string(i) + "].color").c_str()), 1, glm::value_ptr(s.color));
            glUniform1i(glGetUniformLocation(rayTracingShader, ("spheres[" + std::to_string(i) + "].materialType").c_str()), s.materialType);
        }
        if (currentMode == FORWARD_RENDERING) {
            scenes[currentSceneIndex].render(shaderProgram, camera.getPosition());
        } else if (currentMode == RAY_TRACING) {
            glUseProgram(rayTracingShader);
            glUniform3f(glGetUniformLocation(rayTracingShader, "viewPos"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
            glDrawArrays(GL_TRIANGLES, 0, 6); // Полноэкранный квадрат
        }


        glfwSwapBuffers(window);
        glfwPollEvents();


        // Очистка экрана
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Установка шейдера
        glUseProgram(shaderProgram);

        // Установка матриц
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(camera.getPosition(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

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
    }

    // Освобождение ресурсов
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteVertexArrays(1, &sphereVAO);
    glfwTerminate();
    return 0;
}