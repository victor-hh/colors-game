#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

// Estrutura para representar um bloco
struct Block {
    int colorIndex;
    bool selecionado; // Flag pra pretea o bloco
};

struct Pontuacao {
    float total;
    int round;
};

vector<Block> blocks;
vector<vector<float>> colorPalette;
vector<int> numberOfBlockPerColorIndex;
int rows = 10; 
int cols = 10; 

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
vector<vector<float>> generateColorPalette(int numberOfColors);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void updateBlockColors(int clickedColorIndex);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "   ourColor = aColor;\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "in vec3 ourColor;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0);\n"
    "}\0";

int main() {
    // printf("INICIALIZANDO O PROGRAMA");
    cout << "INICIALIZANDO O PROGRAMA" << endl;

    // Inicializar GLFW e definir versão
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Jogo das Cores", NULL, NULL);
    if (window == NULL) {
        cout << "Falha ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Falha ao inicializar GLAD" << endl;
        return -1;
    }

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // Gerar paleta de cores dinamicamente, 
    // Desta forma os blocos que compartilham cores podem apenas se referir ao indice da cor
    int numberOfColors = 25;
    colorPalette = generateColorPalette(numberOfColors);
    numberOfBlockPerColorIndex.resize(numberOfColors);

    // Criar blocos
    blocks.resize(rows * cols);
    for (int i = 0; i < blocks.size(); ++i) {
        int selectedColorIndex = rand() % colorPalette.size();
        blocks[i].colorIndex = selectedColorIndex;
        blocks[i].selecionado = false;
        numberOfBlockPerColorIndex[selectedColorIndex]++;
    }

    // Criar VAO e VBO para os blocos
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    vector<float> vertices;

    float blockWidth = 2.0f / cols;
    float blockHeight = 2.0f / rows;

    srand(static_cast<unsigned int>(time(0)));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Definir posição dos 4 vértices de cada quadrado (normalizado para OpenGL)
            float x = -1.0f + j * blockWidth;
            float y = 1.0f - i * blockHeight;

            int colorIndex = rand() % colorPalette.size();
            auto color = colorPalette[colorIndex];

            // Primeiro triângulo
            vertices.insert(vertices.end(), { x, y, color[0], color[1], color[2] });
            vertices.insert(vertices.end(), { x + blockWidth, y, color[0], color[1], color[2] });
            vertices.insert(vertices.end(), { x + blockWidth, y - blockHeight, color[0], color[1], color[2] });
            // Segundo triângulo
            vertices.insert(vertices.end(), { x, y, color[0], color[1], color[2] });
            vertices.insert(vertices.end(), { x + blockWidth, y - blockHeight, color[0], color[1], color[2] });
            vertices.insert(vertices.end(), { x, y - blockHeight, color[0], color[1], color[2] });
        }
    }

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Atributo de posição
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atributo de cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Loop de renderização
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Limpar tela
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Usar o programa de shader
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        for (int i = 0; i < blocks.size(); ++i) {
            // Se o bloco está selecionado, mudar sua cor para preto
            vector<float> color = blocks[i].selecionado ? vector<float>{0.0f, 0.0f, 0.0f} : colorPalette[blocks[i].colorIndex];

            // Criar um novo buffer de vértices para este bloco
            vector<float> blockVertices;

            // Definir posição dos 4 vértices de cada quadrado (normalizado para OpenGL)
            float blockX = -1.0f + (i % cols) * (2.0f / cols);
            float blockY = 1.0f - (i / cols) * (2.0f / rows);
            float blockWidth = 2.0f / cols;
            float blockHeight = 2.0f / rows;

            // Primeiro triângulo
            blockVertices.insert(blockVertices.end(), { blockX, blockY, color[0], color[1], color[2] });
            blockVertices.insert(blockVertices.end(), { blockX + blockWidth, blockY, color[0], color[1], color[2] });
            blockVertices.insert(blockVertices.end(), { blockX + blockWidth, blockY - blockHeight, color[0], color[1], color[2] });
            // Segundo triângulo
            blockVertices.insert(blockVertices.end(), { blockX, blockY, color[0], color[1], color[2] });
            blockVertices.insert(blockVertices.end(), { blockX + blockWidth, blockY - blockHeight, color[0], color[1], color[2] });
            blockVertices.insert(blockVertices.end(), { blockX, blockY - blockHeight, color[0], color[1], color[2] });

            // Atualizar o VBO com os vértices do bloco
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, i * 6 * 5 * sizeof(float), blockVertices.size() * sizeof(float), blockVertices.data());

            // Desenhar os blocos
            glDrawArrays(GL_TRIANGLES, i * 6, 6);
        }

        glBindVertexArray(0);

        // Swap buffers e poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpar recursos
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

// Função para gerar uma paleta de cores dinamicamente
vector<vector<float>> generateColorPalette(int numberOfColors) {
    vector<vector<float>> palette;
    srand(static_cast<unsigned int>(time(0)));

    for (int i = 0; i < numberOfColors; ++i) {
        // Gerar três valores aleatórios entre 0 e 1 para RGB
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        palette.push_back({ r, g, b });
    }

    return palette;
}

// Função callback para o tamanho da janela
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Função para processar entradas do usuário
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Função chamada ao clicar no mouse
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Converter coordenadas de tela para coordenadas do OpenGL
        float x = (xpos / SCR_WIDTH) * 2.0f - 1.0f;
        float y = -((ypos / SCR_HEIGHT) * 2.0f - 1.0f); // Inverter Y

        int clickedIndex = -1;
        // Verificar qual bloco foi clicado
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                float blockX = -1.0f + j * (2.0f / cols);
                float blockY = 1.0f - i * (2.0f / rows);
                if (x >= blockX && x <= blockX + (2.0f / cols) && y >= blockY - (2.0f / rows) && y <= blockY) {
                    clickedIndex = i * cols + j; // Calcular o índice do bloco
                    break;
                }
            }
            if (clickedIndex != -1) break;
        }

        // Se um bloco foi clicado, atualizar as cores dos blocos correspondentes
        if (clickedIndex != -1) {
            cout << "colorIndex: " << blocks[clickedIndex].colorIndex << endl;
            cout << "Numero de blocos apagados: " << numberOfBlockPerColorIndex[blocks[clickedIndex].colorIndex] << endl;
            // printf("colorIndex: %i", blocks[clickedIndex].colorIndex);
            updateBlockColors(blocks[clickedIndex].colorIndex);
        }
    }
}

// Função para atualizar as cores dos blocos
void updateBlockColors(int clickedColorIndex) {
    for (auto &block : blocks) {
        if (block.colorIndex == clickedColorIndex) {
            block.selecionado = true; // Marcar bloco como selecionado
        }
    }
}
