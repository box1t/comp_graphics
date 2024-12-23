#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
 
// Размеры окна
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Вершинный шейдер
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

// Фрагментный шейдер
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos; // Позиция точечного источника света
uniform vec3 lightDir; // Направление направленного источника света
uniform vec3 viewPos;  // Позиция камеры
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse для точечного источника света
    vec3 norm = normalize(Normal);
    vec3 lightDirNormalized = normalize(lightPos - FragPos);
    float diffPoint = max(dot(norm, lightDirNormalized), 0.0);
    vec3 diffusePoint = diffPoint * lightColor;

    // Diffuse для направленного источника света
    float diffDir = max(dot(norm, lightDir), 0.0);
    vec3 diffuseDir = diffDir * lightColor;

    // Combine results
    vec3 result = (ambient + diffusePoint + diffuseDir) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

struct Camera {
    float radius, angle, height;
    Camera(float r, float a, float h) : radius(r), angle(a), height(h) {}
    glm::vec3 getPosition() const {
        return { radius * std::cos(angle), height, radius * std::sin(angle) }; // x, y, z
    }
    void rotate(float deltaAngle) { angle += deltaAngle; }
    void changeHeight(float deltaHeight) { height += deltaHeight;}
};

void processInput(GLFWwindow* window, Camera& camera) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Изменение угла вращения
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.rotate(0.05f); // Вращение влево
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.rotate(-0.05f); // Вращение вправо

    // Изменение высоты камеры
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.changeHeight(0.05f); // Подъем
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.changeHeight(-0.05f); // Спуск
}

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

    // Создание объектов
    unsigned int cubeVAO = createCube();
    unsigned int pyramidVAO = createPyramid();
    unsigned int sphereVAO = createSphere(0.5f, 20, 20);

    // Инициализация камеры
    Camera camera(5.0f, 90.0f, 0.0f);

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        processInput(window, camera);

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

        // Установить uniform-переменные в шейдере
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 0.0f, 0.0f, 5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightDir"), 0.0f, 0.0f, -5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 0.0f, 0.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.2f, 0.3f, 1.0f);

        // Рендеринг куба
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Рендеринг пирамиды
        model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(pyramidVAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        // Рендеринг сферы
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, 2400, GL_UNSIGNED_INT, nullptr);

        // Обмен буферов и обработка событий
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Освобождение ресурсов
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteVertexArrays(1, &sphereVAO);
    glfwTerminate();
    return 0;
}