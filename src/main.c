#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/vec3.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/cam.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void glfw_error_callback(int code, const char *description);
void glfw_window_resize_callback(GLFWwindow *window, int width, int height);
void glfw_framebuffer_callback(GLFWwindow *window, int width, int height);

struct vertex {
    vec3 position;
    vec2 texture_coordinate;
};


int window_width  = 720;
int window_height = 720;

int main(void)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        printf("Error: failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    // TODO: GLFW_OPENGL_DEBUG_CONTEXT for debug builds

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Chess", NULL, NULL);
    if (!window) {
        printf("Error: failed to create window\n");
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, glfw_window_resize_callback);
    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_callback);


    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
        printf("Error: failed to initialize GLEW\n");
        return -1;
    }



    struct vertex vertices[4] = {
            { 0.5f, 0.5f, 0.0f,    1.0f, 1.0f },
            { 0.5f, -0.5f, 0.0f,   1.0f, 0.0f },
            { -0.5f, -0.5f, 0.0f,  0.0f, 0.0f },
            { -0.5f, 0.5f, 0.0f,   0.0f, 1.0f }
    };

    unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
    };

    unsigned int quad_vao;
    unsigned int quad_vbo;
    unsigned int quad_ebo;

    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glGenBuffers(1, &quad_ebo);
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)(1 * sizeof(vec3)));


    unsigned int shader, vertex_shader, fragment_shader;
    // load shaders
    char vertex_shader_source[] =
            "#version 330 core\n"
            "layout (location = 0) in vec3 position;\n"
            "layout (location = 1) in vec2 texture_pos;\n"
            "out vec2 texture_coords;\n"
            "uniform mat4 view_projection;\n"
            "uniform mat4 model;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = view_projection * model * vec4(position, 1.0);\n"
            "    texture_coords = vec2(texture_pos.x, texture_pos.y);\n"
            "}\n";

    char fragment_shader_source[] =
            "#version 330 core\n"
            "out vec4 color;\n"
            "in vec2 texture_coords;\n"
            "uniform sampler2D texture1;\n"
            "void main()\n"
            "{\n"
            "    color = texture(texture1, texture_coords);\n"
            "}\n";

    int shader_success;
    char shader_log[1024];

    // vertex shader
    const char *v = vertex_shader_source;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &v, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_success);
    if (!shader_success) {
        glGetShaderInfoLog(vertex_shader, 1024, NULL, shader_log);
        printf("Error: vertex shader compile error - %s", shader_log);
        glfwTerminate();
        return -1;
    }

    // fragment shader
    const char *f = fragment_shader_source;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &f, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_success);
    if (!shader_success) {
        glGetShaderInfoLog(fragment_shader, 1024, NULL, shader_log);
        printf("Error: fragment shader compile error - %s", shader_log);
        glfwTerminate();
        return -1;
    }

    // shader program

    shader = glCreateProgram();
    glAttachShader(shader, vertex_shader);
    glAttachShader(shader, fragment_shader);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &shader_success);
    if (!shader_success) {
        glGetProgramInfoLog(shader, 1024, NULL, shader_log);
        printf("Error: shader link error - %s", shader_log);

        glfwTerminate();
        return -1;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // example texture
    unsigned int white_tile_texture;
    glGenTextures(1, &white_tile_texture);
    glBindTexture(GL_TEXTURE_2D, white_tile_texture);
    // texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width1, height1, nr_channels1;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load("assets/white_tile.jpg", &width1, &height1, &nr_channels1, 0);
    if (!data) {
        printf("Error: failed to load texture\n");
        glfwTerminate();
        return -1;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);


    unsigned int black_tile_texture;
    glGenTextures(1, &black_tile_texture);
    glBindTexture(GL_TEXTURE_2D, black_tile_texture);
    // texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width2, height2, nr_channels2;
    unsigned char *data1 = stbi_load("assets/black_tile.jpg", &width2, &height2, &nr_channels2, 0);
    if (!data1) {
        printf("Error: failed to load texture\n");
        glfwTerminate();
        return -1;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
    glGenerateMipmap(GL_TEXTURE_2D);

    //glUseProgram(shader);
    //glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
    //glUniform1i(glGetUniformLocation(shader, "texture1"), 0);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);



        mat4 view_projection_matrix;
        glm_ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, view_projection_matrix);

        // chess grid
        int tile_count = 8;
        for (int y = -tile_count / 2; y < tile_count / 2; ++y) {
            for (int x = -tile_count / 2; x < tile_count / 2; ++x) {
                mat4 transform;
                glm_mat4_identity(transform);

                // tile translations
                float center = 0.5f / ((float)tile_count / 2.0f);
                vec2 position = { (float)x / ((float)tile_count / 2.0f) + center, (float)y / ((float)tile_count / 2.0f) + center };
                glm_translate(transform, position);

                float scale = 1.0f / ((float)tile_count / 2);
                glm_scale_uni(transform, scale);


                // board texture toggling
                if (y % 2 != 0) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, x % 2 == 0 ? white_tile_texture : black_tile_texture);
                } else {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, x % 2 == 0 ? black_tile_texture : white_tile_texture);
                }



                glUseProgram(shader);
                glUniformMatrix4fv(glGetUniformLocation(shader, "view_projection"), 1, GL_FALSE, &view_projection_matrix[0][0]);
                glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &transform[0][0]);

                glBindVertexArray(quad_vao);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glDeleteBuffers(1, &quad_ebo);
    glDeleteBuffers(1, &quad_vbo);
    glDeleteVertexArrays(1, &quad_vao);

    glfwTerminate();
    return 0;
}

void glfw_error_callback(int code, const char* description)
{
    printf("GLFW error (%d) - %s\n", code, description);
}

void glfw_window_resize_callback(GLFWwindow *window, int width, int height)
{
    window_width = width;
    window_height = height;
}

void glfw_framebuffer_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
