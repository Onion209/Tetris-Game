#pragma region Includes
#include <random>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <vector>

using namespace std;
using namespace glm;

#pragma endregion

#pragma region Settings
// Thiết lập window
const unsigned int SCR_WIDTH = 800; // đơn vị là pixel 
const unsigned int SCR_HEIGHT = 800;
// Thiết lập board game
const int mWidth = 9; // đơn vị là ô
const int mHeight = 16;
const int totalCell = mWidth * mHeight;

// khởi tạo một biến cameraPos 3 chiều tọa độ (8,8,0)
vec3 cameraPos = vec3(8, 8, 0); 

// khởi tạo 1 hằng số tickTime = 0.25 không thể thay đổi trong toàn chương trình
const double tickTime = 0.25f;
 // sử dụng thư viện <random> để tạo và quản lý các số ngẫu nhiên, cụ thể là sử dụng phân phối đều để sinh số nguyên từ 0 đến 7.
#pragma region Random

std::random_device dev;  // đặt tên cho object ranndom_device là dev
std::mt19937 rng(dev());  // đặt tên cho object mt19937 là rng, và khởi tạo nó với dev
std::uniform_int_distribution<std::mt19937::result_type> dist7(0, 6); 

#pragma endregion
 
//  Định nghĩa hai shader cơ bản trong OpenGL, một cho việc biến đổi đỉnh (Vertex Shader) và một cho việc xử lý màu sắc của pixel (Fragment Shader).
#pragma region Shader

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform vec3 tint;\n"
"out vec3 ourColor;\n"
"uniform mat4 transform;\n"
"void main()\n"
"{\n"
"   gl_Position = transform * vec4(aPos, 1.0);\n"
"   ourColor = tint / 255;\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(ourColor, 1.0f);\n"
"}\n\0";
#pragma endregion

#pragma endregion

// định nghĩa 1 vecto 2 chiều,
#pragma region Data Stucture
// định nghĩa một vector 2 chiều với tên Vec2Int
struct Vec2Int {
    int x, y;
    Vec2Int(int _x, int _y) : x(_x), y(_y) {}
    Vec2Int() : x(0), y(0) {}
};
// định nghĩa struct transform với 2 tham số ( vector2Int(position), rotation)
struct Transform {
    Vec2Int position;
    int rotation;

    Transform(Vec2Int pos, int rot) : position(pos), rotation(rot) {}
    Transform() : position(Vec2Int()), rotation(0) {}
};
// định nghĩa 1 lớp color
struct Color {
    float data[3];

    Color(float r, float g, float b)
    {
        data[0] = r;
        data[1] = g;
        data[2] = b;
    }

    Color() {
        for (int i = 0; i < 3; i++)
            data[i] = 0;
    }
};

struct Block {
    Vec2Int data[4];
    Color* color;
    // định nghĩa một block với 2 tham số (vector2Int(position), color)
    Block(Vec2Int p[4], Color* c)
    {
        for (int i = 0; i < 4; i++)
            data[i] = p[i];
        color = c;
    }

    Block(int p[8], Color* c)
    {
        for (int i = 0; i < 4; i++)
        {
            data[i].x = p[2 * i];
            data[i].y = p[2 * i + 1];
        }
        color = c;
    }
};
// tạo 1 tetromino có 2 thuộc tính là transform(position, rotate), block(array 4 phần tử , color)
struct Tetromino {
    Transform transform;
    Block* block;

    Tetromino(Transform trans, Block* b) {
        transform = trans;
        block = b;
    }
};

#pragma endregion

#pragma region Asset
// khởi tạo các biến VBO, VAO, EBO
unsigned int VBO, VAO, EBO;
 
// khởi tạo một array gồm 12 phần tử, bộ 3 phần tử là tọa độ của 1 đỉnh hình vuông
float mesh_vertex_square[] = {
    -0.5f,  0.5f, 0.0f, // top left
    -0.5f, -0.5f, 0.0f, // bot left
     0.5f, -0.5f, 0.0f, // bot right
     0.5f,  0.5f, 0.0f  // top right
};
// xác định thứ tự vẽ đỉnh của hình vuông trong open gl( vẽ từng tam giác một)
unsigned int mesh_index_square[] = {
    0, 1, 2,
    0, 2, 3
};

Color color_black = Color(20, 20, 20);
Color color_white = Color(248,248,248);
Color color_grey = Color(225,225,225);
Color color_dgrey = Color(18, 174, 179);
Color color_cyan = Color(0, 183, 235);
Color color_blue = Color(13, 100, 166);
Color color_orange = Color(225, 156, 19);
Color color_yellow = Color(189, 223, 52);
Color color_green = Color(9, 154, 86);
Color color_purple = Color(139, 16, 176);
Color color_red = Color(198, 45, 45);
// tạo ra một array Block[] chưa 7 phần tử là 7 block mà chưa 4 ô
Block blocks[] =
{ 
    // Block I
    Block(new int[8]{
        1,  2,
        1,  1,
        1,  0,
        1, -1
    }, &color_cyan),
    // Block J
    Block(new int[8] {
        1, 2,
        1, 1,
        1, 0,
        0, 0
    }, &color_blue),
    // Block L
    Block(new int[8] {
        0, 2,
        0, 1,
        0, 0,
        1, 0
    }, &color_orange),
    // Block O
    Block(new int[8] {
        0, 1,
        1, 1,
        0, 0,
        1, 0
    }, &color_yellow),
    // Block S
    Block(new int[8] {
        1, 2,
        1, 1,
        0, 1,
        0, 0
    }, &color_green),
    // Block Z
    Block(new int[8] {
        0, 2,
        0, 1,
        1, 1,
        1, 0
    }, &color_red),
    // Block T
    Block(new int[8] {
        0, 2,
        0, 1,
        1, 1,
        0, 0
    }, &color_purple)
};

map<int, int> track_key_state = {
    { GLFW_KEY_UP, GLFW_RELEASE },
    { GLFW_KEY_DOWN, GLFW_RELEASE},
    { GLFW_KEY_LEFT, GLFW_RELEASE },
    { GLFW_KEY_RIGHT, GLFW_RELEASE },
    { GLFW_KEY_E, GLFW_RELEASE },
    { GLFW_KEY_W, GLFW_RELEASE }
};

#pragma endregion
// khởi tạo các hàm để dùng trong main
#pragma region Declaration

// Input
void framebuffer_size_callback(GLFWwindow* window, int width, int height); 
// dùng để thay đổi độ dài window tùy ý
void processInput(); 
// xử lý event đầu vào như nhấn phím hoặc di chuyển chuột
void updateTrackKeyInput(); 
// cập nhật trạng thái các phím
bool IsKeyDown(int key);
 // kiểm tra xem phím có được nhấn không - nếu true thì sẽ là nhấn còn false thì sẽ là nhả
bool IsKeyUp(int key); 
// kiểm tra xem phím có được nhả không - nếu true thì sẽ là nhả còn false thì sẽ là nhấn

// Render
template <size_t n_vert, size_t n_index> 
void binding(float(&vertices)[n_vert], unsigned int(&indices)[n_index]); 
void clearColor(Color& color); 
//hàm để trỏ đến một màu cụ thể có thể dùng để đặt màu cho hình nền,...

// Core Game
void ProcessPlayerControl(); 
// hàm này để xử lý các sự kiện đầu vào như di chuyển hay xoay các block
void TickDown(); 
// dùng để cập nhật trạng thái của trò chơi sau mỗi tick thời gian
void Update(); 
// dùng để cập nhật trạng thái của toàn bộ trò chơi bao gồm: xử lý đầu vào từ người chơi, cập nhật vị trí của các khối tetro, và kiểm tra xem có dòng nào được hoàn thành hay không
void OnLanded();
 // hàm này đước gọi khi khối tetro hạ cánh - nó dùng để kiểm tra xem có dòng nào được hoàn thành sau khi khối tetro hạ cánh hay không
void CheckClearTetromino(); 
// hàm này để kiểm tra và xóa các dòng đã hoàn thành trên bảng chơi (full màu của 1 dòng)

// Tetromino
void ClearTetromino(Tetromino* tetro); 
// hàm này dùng để xóa 1 khối tetromino bất kì
void DrawTetromino(Tetromino* tetro); 
// hàm này dùng để vẽ 1 khối tetromino lên board game 
bool CheckTetrominoTransform(Tetromino* tetro);
 // hàm này để check transform của khối đó

// Utilities
Vec2Int ApplyRotate(Vec2Int pos, int rot);
 // hàm này dùng để xoay 1 vị trí 2D một góc cụ thể
#pragma endregion

#pragma region Working var

GLFWwindow* window;
double passTickTime = 0;
const Vec2Int startPos = Vec2Int(mWidth / 2, mHeight);
float* cell[totalCell];
int cTetroId = 3;
Transform oldTrans;
Tetromino cTetro = Tetromino(
    Transform(Vec2Int(3, 10), 0),
    &blocks[cTetroId]
);
vector<int> scanLines;

#pragma endregion


int main()
{
#pragma region Initialization

#pragma region Init GLFW

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#pragma endregion

#pragma region Init Window

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tetris Game", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

#pragma endregion

// khởi tạo thư viện GLAD và check xem nó có được kết nối tới thư viện đó hay không
#pragma region Init GLAD 

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

#pragma endregion
// Shader (dùng để vẽ hình trong cảnh 3D )
#pragma region Shader Compile
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

#pragma endregion

#pragma region Binding 

    binding(mesh_vertex_square, mesh_index_square);
    glBindVertexArray(VAO);
    glUseProgram(shaderProgram);

    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    unsigned int colorLoc = glGetUniformLocation(shaderProgram, "tint");

    for (int i = 0; i < totalCell; i++) // tô màu nền cho các cell
        cell[i] = color_grey.data;

#pragma endregion

#pragma endregion

#pragma region Core Loop

    // Game loop // nếu window chưa nhận được tín hiệu close thì nó sẽ thực hiện các câu lệnh bên trong vòng while
    while (!glfwWindowShouldClose(window))
    {
        ClearTetromino(&cTetro); // hàm này dùng để xóa 1 khối tetromino bất kì( logic: đặt lại giá trị  trong mảng biểu diễn bảng chơi từ giá trị biểu diễn khối tetromino thành giá trị biểu diễn màu nền)

        processInput();// xử lý event đầu vào như nhấn phím hoặc di chuyển chuột
        Update(); // dùng để cập nhật trạng thái của toàn bộ trò chơi bao gồm: xử lý đầu vào từ người chơi, cập nhật vị trí của các khối tetro, và kiểm tra xem có dòng nào được hoàn thành hay không
        updateTrackKeyInput(); // cập nhật trạng thái các phím
        DrawTetromino(&cTetro); // hàm để vẽ tetro lên màn hình
        // render
#pragma region Render

        clearColor(color_white); 
        {
            mat4 trans = mat4(1.0f); // khởi tạo ma trận đơn vị 4*4
            trans = scale(trans, 0.12f * vec3(mWidth, mHeight, 1));
            trans = translate(trans, vec3(mWidth / 2, mHeight / 2, 0) - cameraPos + vec3(0.49f, 0.015f, 0.0f));
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
            glUniform3fv(colorLoc, 1, color_dgrey.data); // lỗi màu biên
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // vẽ biên dưới dạng từng tam giác 1
        }

        // Cells
        for (int i = 0; i < totalCell; i++)
        {
            int pos_x = i % mWidth; // tính toán tọa độ x của cell[i] trên board game
            int pos_y = i / mWidth; // tính toán tọa độ y của cell[i] trên board game
            // tương tự vẽ biên thì vẽ các cell lên board game
            mat4 trans = mat4(1.0f);
            trans = scale(trans, vec3(1.0f) * 0.1f);
            trans = translate(trans, 1.1f * vec3(pos_x, pos_y, 0) - cameraPos);

            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
            glUniform3fv(colorLoc, 1, cell[i]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

#pragma endregion
// để xử lý window với event đầu vào
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

#pragma endregion
// xóa resource khi ko cần sử dụng đến nữa 
#pragma region Destruction

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;

#pragma endregion

}

#pragma region Implementation

#pragma region Input
// hàm này để set kích cỡ của window 
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
void processInput() // xử lý event đầu vào như nhấn phím hoặc di chuyển chuột
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // Kiểm tra phím "up" và "down"
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        // Xử lý khi phím "up" được nhấn
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
       // Xử lý khi phím "down" được nhấn
    }
}
// duyệt qua hết các phím trong list các phím và cập nhật trạng thái cho nó
void updateTrackKeyInput()
{
    for (map<int, int>::iterator i = track_key_state.begin(); i != track_key_state.end(); i++)
        i->second = glfwGetKey(window, i->first);
}
// check xem nó có được nhấn không - nếu true thì sẽ là nhấn còn false thì sẽ là nhả
bool IsKeyDown(int key) {
    return track_key_state[key] == GLFW_RELEASE && glfwGetKey(window, key) == GLFW_PRESS;
}
// check xem nó có thả ra hay ko
bool IsKeyUp(int key)   
{
    return track_key_state[key] == GLFW_PRESS && glfwGetKey(window, key) == GLFW_RELEASE;
}

#pragma endregion

#pragma region Render

template <size_t n_vert, size_t n_index>
void binding(float(&vertices)[n_vert], unsigned int(&indices)[n_index])
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_vert, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * n_index, indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
//
void clearColor(Color& color) // hàm để tô màu
{
    float r = color.data[0] / 255.0f;
    float g = color.data[1] / 255.0f;
    float b = color.data[2] / 255.0f;

    glClearColor(r, g, b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

#pragma endregion

#pragma region Core Game
// hàm để xử lý sự kiện điều khiển từ người chơi trong game
void ProcessPlayerControl()
{
    oldTrans = cTetro.transform;

    if (IsKeyDown(GLFW_KEY_E)) // đổi khối tetro đang di chuyển
    {
        cTetroId = (cTetroId + 1) % 7; // lấy phần dư để tránh trường hợp cTetroId > 7
        cTetro.block = &blocks[cTetroId]; // gán id thành id +1
    }

    if (IsKeyDown(GLFW_KEY_UP))
        cTetro.transform.rotation = (cTetro.transform.rotation + 1) % 4; // chưa hiểu tại sao 0,1,2,3 mà máy nó hiểu là 0,90,180,270 độ

    if (IsKeyDown(GLFW_KEY_LEFT))
        cTetro.transform.position.x -= 1;

    if (IsKeyDown(GLFW_KEY_RIGHT))
        cTetro.transform.position.x += 1;

    if (IsKeyDown(GLFW_KEY_DOWN))
        cTetro.transform.position.y -= 1;

    if (!CheckTetrominoTransform(&cTetro))
        cTetro.transform = oldTrans;
}

bool isGameOver = false;

void TickDown()
{
    if (isGameOver)
    {
        return;
    }

    while (passTickTime + tickTime < glfwGetTime())
    {
        passTickTime += tickTime;

        oldTrans = cTetro.transform;

        cTetro.transform.position.y -= 1;

        if (!CheckTetrominoTransform(&cTetro))
        {
            cTetro.transform = oldTrans;
            OnLanded();
        }
    }
}

void Update()
{
    ProcessPlayerControl();
    TickDown();
}
void OnLanded()
{
    DrawTetromino(&cTetro);
    CheckClearTetromino();
    // Khi game chưa kết thúc thì thực hiện lệnh dưới
    // Kiểm tra nếu khối tetromino vượt ra khỏi cạnh trên của bảng
    if (cTetro.transform.position.y < mHeight)
    {
        cTetro.block = &blocks[dist7(rng)];
        cTetro.transform.position = startPos;
        cTetro.transform.rotation = 0;
        isGameOver = false; 
    }
    else
    {
        isGameOver = true;
    }
}
void CheckClearTetromino() // hàm này để kiểm tra và xóa các dòng đã hoàn thành trên bảng chơi (full màu của 1 dòng)
{
    int countEmpty = 0;
    int stopLine = 0;
    for (int y = 0; y < mHeight; y++)
    {
        countEmpty = 0;
        for (int x = 0; x < mWidth; x++)
            if (cell[y * mWidth + x] == color_grey.data)
                countEmpty++;

        cout << "Count Empty " << countEmpty << " of " << y << endl;
        if (countEmpty == mWidth)
        {
            stopLine = y + 1;
            break;
        }
        if (countEmpty == 0)
            scanLines.push_back(y);
    }

    cout << stopLine << endl;

    int sLineIndex = 0;
    for (int y = 0; y < stopLine; y++)
    {
        while (sLineIndex < scanLines.size())
        {
            if (y + sLineIndex != scanLines[sLineIndex])
                break;

            sLineIndex++;
        }

        if (sLineIndex > 0)
            for (int x = 0; x < mWidth; x++)
                cell[y * mWidth + x] = cell[(y + sLineIndex) * mWidth + x];
    }

    scanLines.clear();
}

#pragma endregion

#pragma region Tetromino
// kiểm tra vị trí hợp lệ của khối tetro đó trên màn hình
bool CheckTetrominoTransform(Tetromino* tetro)
{
    for (int i = 0; i < 4; i++)
    {
        Vec2Int local_pos = tetro->block->data[i];
        local_pos = ApplyRotate(local_pos, tetro->transform.rotation);
        int px = tetro->transform.position.x + local_pos.x;
        int py = tetro->transform.position.y + local_pos.y;
        if (0 <= px && px < mWidth && 0 <= py && py < mHeight)
        {
            if (cell[py * mWidth + px] != color_grey.data)
                return false;
        }
        else {
            if (px < 0 || px >= mWidth || py < 0)
                return false;
        }
    }

    return true;
}
// xóa một tetromino trên board game
void ClearTetromino(Tetromino* tetro)
{
    for (int i = 0; i < 4; i++)
    {
        Vec2Int local_pos = tetro->block->data[i];
        local_pos = ApplyRotate(local_pos, tetro->transform.rotation);
        int px = tetro->transform.position.x + local_pos.x;
        int py = tetro->transform.position.y + local_pos.y;
        if (px < 0 || px >= mWidth || py < 0 || py >= mHeight)
            continue;
        cell[py * mWidth + px] = color_grey.data;
    }
}
// vẽ một tetromino lên board game
void DrawTetromino(Tetromino* tetro)
{
    for (int i = 0; i < 4; i++)
    {
        Vec2Int local_pos = tetro->block->data[i];
        local_pos = ApplyRotate(local_pos, tetro->transform.rotation);
        int px = tetro->transform.position.x + local_pos.x;
        int py = tetro->transform.position.y + local_pos.y;
        if (px < 0 || px >= mWidth || py < 0 || py >= mHeight)
            continue;
        cell[py * mWidth + px] = tetro->block->color->data;
    }
}

#pragma endregion

#pragma region Utilities
// hàm này để xoay một khối tetromino
Vec2Int ApplyRotate(Vec2Int pos, int rot)
{
    int t = pos.x;
    switch (rot)
    {
    case 1: // 90 deg
        pos.x = pos.y;
        pos.y = 1 - t;
        break;
    case 2: // 180 deg
        pos.x = 1 - pos.x;
        pos.y = 1 - pos.y;
        break;
    case 3: // 270 deg
        pos.x = 1 - pos.y;
        pos.y = t;
        break;
    }

    return pos;
}

#pragma endregion
// int score = 0;

// void calculateScore(int rowsCleared) {
//     int points;

//     switch(rowsCleared) {
//         case 1:
//             points = 40;
//             break;
//         case 2:
//             points = 100;
//             break;
//         case 3:
//             points = 300;
//             break;
//         case 4:
//             points = 1200;
//             break;
//         default:
//             points = 0;
//     }

//     score += points;
// }

#pragma endregion
