#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/vec2.h>
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



void set_shader_mat4(unsigned int shader, const char *name, mat4 value)
{
    glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE, &value[0][0]);
}


unsigned int load_texture(const char *path)
{
    unsigned int id;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    // texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nr_channels;
    unsigned char *data = stbi_load(path, &width, &height, &nr_channels, 0);
    if (!data) {
        printf("Failed to load texture: %s\n", path);
        return -1;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return id;
}


void render_texture(unsigned int id, unsigned int shader, mat4 view_projection, mat4 model)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);

    set_shader_mat4(shader, "view_projection", view_projection);
    set_shader_mat4(shader, "model", model);



    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

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

    // enable texture transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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


    stbi_set_flip_vertically_on_load(1);
    // chess board tiles
    unsigned int white_tile_texture  = load_texture("assets/pack/PNGs/no_shadow/1024h/square_brown_light_png_1024px.png");
    unsigned int black_tile_texture  = load_texture("assets/pack/PNGs/no_shadow/1024h/square_gray_light_png_1024px.png");

    unsigned int white_king_texture   = load_texture("assets/pack/PNGs/no_shadow/1024h/w_king_png_1024px.png");
    unsigned int white_queen_texture  = load_texture("assets/pack/PNGs/no_shadow/1024h/w_queen_png_1024px.png");
    unsigned int white_bishop_texture = load_texture("assets/pack/PNGs/no_shadow/1024h/w_bishop_png_1024px.png");
    unsigned int white_knight_texture = load_texture("assets/pack/PNGs/no_shadow/1024h/w_knight_png_1024px.png");
    unsigned int white_rook_texture   = load_texture("assets/pack/PNGs/no_shadow/1024h/w_rook_png_1024px.png");
    unsigned int white_pawn_texture   = load_texture("assets/pack/PNGs/no_shadow/1024h/w_pawn_png_1024px.png");

    unsigned int black_king_texture   = load_texture("assets/pack/PNGs/no_shadow/1024h/b_king_png_1024px.png");
    unsigned int black_queen_texture  = load_texture("assets/pack/PNGs/no_shadow/1024h/b_queen_png_1024px.png");
    unsigned int black_bishop_texture = load_texture("assets/pack/PNGs/no_shadow/1024h/b_bishop_png_1024px.png");
    unsigned int black_knight_texture = load_texture("assets/pack/PNGs/no_shadow/1024h/b_knight_png_1024px.png");
    unsigned int black_rook_texture   = load_texture("assets/pack/PNGs/no_shadow/1024h/b_rook_png_1024px.png");
    unsigned int black_pawn_texture   = load_texture("assets/pack/PNGs/no_shadow/1024h/b_pawn_png_1024px.png");


    // build chess board
    int tile_count = 8;
    vec2 tile_positions[tile_count][tile_count];



    assert(tile_count == 8);

    unsigned int x_index = 0;
    unsigned int y_index = 0;
    float center = 0.5f / ((float)tile_count / 2.0f);
    for (int y = -tile_count / 2; y < tile_count / 2; ++y, ++y_index, x_index = 0) {
        for (int x = -tile_count / 2; x < tile_count / 2; ++x, ++x_index) {
            vec2 position = { (float)x / ((float)tile_count / 2.0f) + center, (float)y / ((float)tile_count / 2.0f) + center };
            glm_vec2(position, tile_positions[x_index][y_index]);
        }
    }


    // set chess pieces starting position
    unsigned int piece_positions[tile_count][tile_count];
    memset(piece_positions, -1, sizeof(piece_positions));
    piece_positions[0][0] = white_rook_texture;
    piece_positions[1][0] = white_knight_texture;
    piece_positions[2][0] = white_bishop_texture;
    piece_positions[3][0] = white_queen_texture;
    piece_positions[4][0] = white_king_texture;
    piece_positions[5][0] = white_bishop_texture;
    piece_positions[6][0] = white_knight_texture;
    piece_positions[7][0] = white_rook_texture;
    piece_positions[0][1] = white_pawn_texture;
    piece_positions[1][1] = white_pawn_texture;
    piece_positions[2][1] = white_pawn_texture;
    piece_positions[3][1] = white_pawn_texture;
    piece_positions[4][1] = white_pawn_texture;
    piece_positions[5][1] = white_pawn_texture;
    piece_positions[6][1] = white_pawn_texture;
    piece_positions[7][1] = white_pawn_texture;


    piece_positions[0][7] = black_rook_texture;
    piece_positions[1][7] = black_knight_texture;
    piece_positions[2][7] = black_bishop_texture;
    piece_positions[3][7] = black_queen_texture;
    piece_positions[4][7] = black_king_texture;
    piece_positions[5][7] = black_bishop_texture;
    piece_positions[6][7] = black_knight_texture;
    piece_positions[7][7] = black_rook_texture;
    piece_positions[0][6] = black_pawn_texture;
    piece_positions[1][6] = black_pawn_texture;
    piece_positions[2][6] = black_pawn_texture;
    piece_positions[3][6] = black_pawn_texture;
    piece_positions[4][6] = black_pawn_texture;
    piece_positions[5][6] = black_pawn_texture;
    piece_positions[6][6] = black_pawn_texture;
    piece_positions[7][6] = black_pawn_texture;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        mat4 view_projection_matrix;
        glm_ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, view_projection_matrix);

        glUseProgram(shader);
        glBindVertexArray(quad_vao);

        // render chess board
        for (int y = 0; y < tile_count; ++y) {
            for (int x = 0; x < tile_count; ++x) {
                float tile_scale = 1.0f / ((float)tile_count / 2.0f);
                //float piece_scale = tile_scale + 0.4f;
                mat4 transform;
                glm_mat4_identity(transform);

                glm_translate(transform, tile_positions[x][y]);
                glm_scale_uni(transform, tile_scale);


                if (y % 2 != 0) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, x % 2 == 0 ? black_tile_texture : white_tile_texture);
                } else {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, x % 2 == 0 ? white_tile_texture : black_tile_texture);
                }


                set_shader_mat4(shader, "view_projection", view_projection_matrix);
                set_shader_mat4(shader, "model", transform);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);






                // render chess pieces
                if (piece_positions[x][y] != -1) {
                    glm_scale_uni(transform, 0.8f);
                    render_texture(piece_positions[x][y], shader, view_projection_matrix, transform);
                }

            }
        }



        glfwSwapBuffers(window);
        glfwPollEvents();
    }





    glDeleteTextures(1, &black_pawn_texture);
    glDeleteTextures(1, &black_rook_texture);
    glDeleteTextures(1, &black_knight_texture);
    glDeleteTextures(1, &black_bishop_texture);
    glDeleteTextures(1, &black_queen_texture);
    glDeleteTextures(1, &black_king_texture);

    glDeleteTextures(1, &white_pawn_texture);
    glDeleteTextures(1, &white_rook_texture);
    glDeleteTextures(1, &white_knight_texture);
    glDeleteTextures(1, &white_bishop_texture);
    glDeleteTextures(1, &white_queen_texture);
    glDeleteTextures(1, &white_king_texture);

    glDeleteTextures(1, &black_tile_texture);
    glDeleteTextures(1, &white_tile_texture);



    glDeleteShader(shader);


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
