#include <atolla/sink.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

AtollaSink sink;

int main(int argc, const char* argv[])
{
    AtollaSinkSpec spec;
    spec.lights_count = 1;
    spec.port = 10042;

    sink = atolla_sink_make(&spec);

    GLFWwindow* window;

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(640, 480, "Local Atolla Sink", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        AtollaSinkState state = atolla_sink_state(sink);
        if(state == ATOLLA_SINK_STATE_OPEN) {
            glClearColor(0.0, 1.0, 0.0, 1.0);
        } else if(state == ATOLLA_SINK_STATE_ERROR) {
            glClearColor(1.0, 0.0, 0.0, 1.0);
        } else {
            uint8_t frame[3];
            atolla_sink_get(sink, frame, 3);
            GLfloat r = frame[0] / 255.0f;
            GLfloat g = frame[1] / 255.0f;
            GLfloat b = frame[2] / 255.0f;
            glClearColor(r, g, b, 1.0);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
