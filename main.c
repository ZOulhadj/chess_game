#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/vec3.h>

void glfw_error_callback(int code, const char *description);
void glfw_framebuffer_callback(GLFWwindow *window, int width, int height);

int main(void)
{
    if (!glfwInit())
    {
        printf("Error: failed to initialize GLFW\n");
        return -1;
    }


    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    // TODO: GLFW_OPENGL_DEBUG_CONTEXT for debug builds

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Chess", NULL, NULL);
    if (!window)
    {
        printf("Error: failed to create window\n");
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_callback);


    glewExperimental = 1;
    if (glewInit() != GLEW_OK)
    {
        printf("Error: failed to initialize GLEW\n");
        return -1;
    }


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void glfw_error_callback(int code, const char* description)
{
    printf("GLFW error (%d) - %s\n", code, description);
}

void glfw_framebuffer_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
