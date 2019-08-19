#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;

    vertex(glm::vec3 const& position, glm::vec3 const& normal, glm::vec2 const& tex_coord)
        : position(position)
        , normal(normal)
        , tex_coord(tex_coord)
    {
    }

    vertex(glm::vec3 const& position, glm::vec3 const& normal)
        : position(position)
        , normal(normal)
        , tex_coord(0.f)
    {
    }

    vertex(glm::vec3 const& position, glm::vec2 const& tex_coord)
        : position(position)
        , normal(0.f)
        , tex_coord(tex_coord)
    {
    }

    vertex(glm::vec3 const& position)
        : position(position)
        , normal(0.f)
        , tex_coord(0.f)
    {
    }
};

struct mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    std::vector<size_t> indexes;
    std::vector<vertex> vertexes;
};

bool background_state = false;

glm::mat4 model(1.f);
glm::mat4 projection(1.f);

std::vector<std::string> split(std::string const& str, char delimeter)
{
    std::stringstream ss(str);
    std::string item;
    std::vector<std::string> split_strs;
    while (std::getline(ss, item, delimeter))
    {
        split_strs.push_back(item);
    }

    return split_strs;
}

std::vector<std::string> split(std::string const& str, std::string delimeter)
{
    std::vector<std::string> split_str;
    size_t start = 0;
    size_t end = 0;
    while ((end = str.find(delimeter, start)) < str.size())
    {
        std::string val = str.substr(start, end - start);
        split_str.push_back(val);
        start = end + delimeter.size();
    }

    if (start < str.size())
    {
        std::string val = str.substr(start);
        split_str.push_back(val);
    }

    return split_str;
}

void read_triangle_mesh(
    std::string const& filename,
    std::vector<glm::vec3>& positions,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec2>& uvs,
    std::vector<size_t>& position_idxs,
    std::vector<size_t>& normal_idxs,
    std::vector<size_t>& uv_idxs)
{
    std::ifstream f;
    f.open(filename);
    if (!f.is_open())
    {
        std::cout << "File not open!\n";
        return;
    }

    std::stringstream ss;
    std::string line;
    while (std::getline(f, line))
    {
        ss << line;
        std::string op;
        ss >> op;

        if (op == "v")
        {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            positions.push_back(v);
        }
        else if (op == "vn")
        {
            glm::vec3 vn;
            ss >> vn.x >> vn.y >> vn.z;
            normals.push_back(vn);
        }
        else if (op == "vt")
        {
            glm::vec2 vt;
            ss >> vt.x >> vt.y;
            uvs.push_back(vt);
        }
        else if (op == "f")
        {
            std::vector<std::string> strs = split(line, ' ');
            strs.erase(strs.begin());
            for (std::string const& str : strs)
            {
                ss.str("");
                ss.clear();
                if (str.find("//") != std::string::npos)
                {
                    std::vector<std::string> nums = split(str, "//");

                    ss << nums[0];
                    size_t pos;
                    ss >> pos;
                    position_idxs.push_back(pos - 1);

                    ss.str("");
                    ss.clear();
                    ss << nums[1];
                    size_t normal;
                    ss >> normal;
                    normal_idxs.push_back(normal - 1);
                }
                else if (str.find('/') != std::string::npos)
                {
                    std::vector<std::string> nums = split(str, '/');
                    if (nums.size() == 2)
                    {
                        size_t pos;
                        ss << nums[0];
                        ss >> pos;
                        position_idxs.push_back(pos - 1);

                        ss.str("");
                        ss.clear();
                        size_t tex;
                        ss << nums[1];
                        ss >> tex;
                        uv_idxs.push_back(tex - 1);
                    }
                    else
                    {
                        size_t pos;
                        ss << nums[0];
                        ss >> pos;
                        position_idxs.push_back(pos - 1);

                        ss.str("");
                        ss.clear();
                        size_t tex;
                        ss << nums[1];
                        ss >> tex;
                        uv_idxs.push_back(tex - 1);

                        ss.str("");
                        ss.clear();
                        size_t normal;
                        ss << nums[2];
                        ss >> normal;
                        normal_idxs.push_back(normal - 1);
                    }
                }
                else
                {
                    ss << str;
                    size_t pos;
                    ss >> pos;
                    position_idxs.push_back(pos - 1);
                }
            }
        }

        ss.str("");
        ss.clear();
    }

    f.close();
}

std::vector<glm::vec3> calculate_normals(
    std::vector<glm::vec3> const& positions,
    std::vector<size_t> const& position_idxs)
{
    std::vector<glm::vec3> normals;
    normals.reserve(position_idxs.size());
    for (size_t i = 0; i < position_idxs.size(); i += 3)
    {
        glm::vec3 const v0 = positions[position_idxs[i + 0]];
        glm::vec3 const v1 = positions[position_idxs[i + 1]];
        glm::vec3 const v2 = positions[position_idxs[i + 2]];
        glm::vec3 const n = glm::normalize(glm::cross(v2 - v0, v1 - v0));
        normals.push_back(n);
        normals.push_back(n);
        normals.push_back(n);
    }

    return normals;
}

std::vector<glm::vec2> calculate_uvs(
    std::vector<glm::vec3> const& positions,
    std::vector<size_t> const& position_idxs,
    std::vector<glm::vec3> const& normals)
{
    std::vector<glm::vec2> uvs;
    uvs.reserve(position_idxs.size());
    for (size_t i = 0; i < position_idxs.size(); i += 3)
    {
        glm::vec3 const v0 = positions[position_idxs[i + 0]];
        glm::vec3 const v1 = positions[position_idxs[i + 1]];
        glm::vec3 const v2 = positions[position_idxs[i + 2]];
        glm::vec3 const n = normals[position_idxs[i]];

        glm::quat const r = glm::inverse(glm::quatLookAt(n, glm::vec3(0.f, 1.f, 0.f)));

        uvs.emplace_back(r * v0);
        uvs.emplace_back(r * v1);
        uvs.emplace_back(r * v2);
    }

    return uvs;
}

mesh load_triangle_mesh(
    std::vector<glm::vec3> const& positions,
    std::vector<glm::vec3> const& normals,
    std::vector<glm::vec2> const& tex_coords,
    std::vector<size_t> const& position_idxs,
    std::vector<size_t> const& normal_idxs,
    std::vector<size_t> const& tex_coord_idxs,
    GLenum const usage)
{
    std::vector<glm::vec3> ns;
    std::vector<glm::vec2> uvs;
    std::vector<size_t> const& indexes = position_idxs;

    //std::cout
    //    << positions.size() << ' ' << normals.size() << ' ' << tex_coords.size() << '\n'
    //    << position_idxs.size() << ' ' << normal_idxs.size() << ' ' << tex_coord_idxs.size() << '\n';

    std::vector<vertex> vertexes;
    vertexes.reserve(positions.size());
    for (glm::vec3 const position : positions)
    {
        vertexes.emplace_back(position);
    }

    if (normal_idxs.empty())
    {
        ns = calculate_normals(positions, position_idxs);
        for (size_t i = 0; i < ns.size(); ++i)
        {
            vertexes[position_idxs[i]].normal = ns[i];
        }
    }
    else
    {
        ns = normals;
        for (size_t i = 0; i < normal_idxs.size(); ++i)
        {
            vertexes[position_idxs[i]].normal = ns[normal_idxs[i]];
        }
    }

    if (tex_coords.empty())
    {
        uvs = calculate_uvs(positions, position_idxs, ns);
        for (size_t i = 0; i < uvs.size(); ++i)
        {
            vertexes[position_idxs[i]].tex_coord = uvs[i];
        }
    }
    else
    {
        uvs = tex_coords;
        for (size_t i = 0; i < tex_coord_idxs.size(); ++i)
        {
            vertexes[position_idxs[i]].tex_coord = uvs[tex_coord_idxs[i]];
        }
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertexes.size() * sizeof(vertex),
        vertexes.data(),
        usage);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indexes.size() * sizeof(size_t),
        indexes.data(),
        usage);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(vertex),
        reinterpret_cast<void*>(offsetof(vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(vertex),
        reinterpret_cast<void*>(offsetof(vertex, normal)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(vertex),
        reinterpret_cast<void*>(offsetof(vertex, tex_coord)));
    glEnableVertexAttribArray(2);
    
    return { vao, vbo, ebo, indexes, vertexes };
}

void load_square()
{
    GLfloat vertexes[] =
    {
        // First triangle:
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,

        // Second triangle:
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertexes),
        vertexes, // or: 18 * sizeof(GLfloat)
        GL_STATIC_DRAW);

    // Configure vertex position:
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        nullptr);
    glEnableVertexAttribArray(0);
}

bool compile_shader(std::string const& filename, GLenum type, GLuint& id)
{
    std::ifstream file(filename, std::ifstream::in);

    if (!file.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    std::string source;

    buffer << file.rdbuf();
    source = buffer.str();

    // Create shader
    GLuint shader_id = glCreateShader(type);

    // Setup shader source code
    GLchar const* src = source.data();
    glShaderSource(shader_id, 1, &src, nullptr);

    // Compile shader
    glCompileShader(shader_id);

    // Get compilation status
    GLint status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);

    // Check compilation errors
    if (status != GL_TRUE)
    {
        // Get log message size of the compilation process
        GLint size = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &size);

        std::string message;
        message.resize(size);

        // Get log message of the compilation process
        glGetShaderInfoLog(shader_id, size, nullptr, (GLchar*)message.data());

        // Print log message
        std::cout << message << std::endl;

        // Delete shader
        glDeleteShader(shader_id);

        return false;
    }

    // Return shader id
    id = shader_id;

    return true;
}

// Create shader program
bool create_program(std::string const& name, GLuint& id)
{
    GLuint vertex_shader_id, fragment_shader_id;

    // Load and compile vertex shader
    if (!compile_shader(name + ".vert", GL_VERTEX_SHADER, vertex_shader_id))
    {
        return false;
    }

    // Load and compile fragment shader
    if (!compile_shader(name + ".frag", GL_FRAGMENT_SHADER, fragment_shader_id))
    {
        return false;
    }

    // Create shader program
    GLuint program_id = glCreateProgram();

    // Attach compiled shaders to program
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Link attached shaders to create an executable
    glLinkProgram(program_id);

    // Delete compiled shaders
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Get linkage status
    GLint status;
    glGetProgramiv(program_id, GL_LINK_STATUS, &status);

    // Check linkage errors
    if (status != GL_TRUE)
    {
        // Get log message size of the linkage process
        GLint size = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &size);

        std::string message;
        message.resize(size);

        // Get log message of the linkage process
        glGetProgramInfoLog(program_id, size, nullptr, (GLchar*)message.data());

        // Print log message
        std::cout << message << std::endl;

        // Delete shader program
        glDeleteProgram(program_id);

        return false;
    }

    // Return shader program id
    id = program_id;

    return true;
}

void resize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    projection = glm::perspective(45.f, width / float(height), 0.001f, 1000.0f);
}

void keyboard(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int modifier)
{
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        background_state = !background_state;
    }

    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        model = glm::rotate(model, 0.1f, glm::vec3(0.0f, 1.0f, 0.0f)); // RzRyRxM = M
    }
}

void cursor_pos(GLFWwindow* window, double x, double y)
{
    model = glm::translate(
        glm::mat4(1.f),
        glm::vec3(static_cast<float>(x) * -0.001f, static_cast<float>(y) * 0.001f, 0.f));
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Could not initialize GLFW.\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Aula 1", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Could not instantiate the main window.\n";
        glfwTerminate();
        return -2;
    }

    glfwSetFramebufferSizeCallback(window, resize);
    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, cursor_pos);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Could not initialize GLAD.\n";
        glfwTerminate();
        return -3;
    }

    GLuint program_id;
    if (!create_program("../res/shaders/triangle", program_id))
    {
        std::cerr << "Could not create shader program.\n";
        glfwTerminate();
        return -4;
    }

    glUseProgram(program_id);

    std::string const filename = "../res/meshes/bunny.obj";
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<size_t> position_idxs;
    std::vector<size_t> normal_idxs;
    std::vector<size_t> uv_idxs;
    //readTriangleMesh(filename, positions, normals, uvs, position_idxs, normal_idxs, uv_idxs);
    read_triangle_mesh(filename, positions, normals, uvs, position_idxs, normal_idxs, uv_idxs);
    mesh m = load_triangle_mesh(positions, normals, uvs, position_idxs, normal_idxs, uv_idxs, GL_STATIC_DRAW);

    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    GLint const view_id = glGetUniformLocation(program_id, "view");
    GLint const model_id = glGetUniformLocation(program_id, "model");
    GLint const projection_id = glGetUniformLocation(program_id, "projection");

    resize(window, 800, 600);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (background_state)
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
        else
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        }

        glUniformMatrix4fv(view_id, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(model_id, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(projection_id, 1, GL_FALSE, glm::value_ptr(projection));

        glDrawElements(GL_TRIANGLES, position_idxs.size(), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(program_id);
    glDeleteVertexArrays(1, &m.vao);

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cin.get();
    return 0;
}
