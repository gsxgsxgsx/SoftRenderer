#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "glad.c"
#include "include/camera.h"
#include "include/main.h"
#include "include/data_model.h"
#include "include/math.h"
#include "include/our_gl.h"
#include "include/fbuffer.h"
#include "include/data_input.h"
#include "include/data.h"
#include "shader/shader.h"
#include "shader/Simple_Shader.h"
#include "shader/Simple_BP_Shader.h"
#include "shader/PBR_Shader.h"
#include "shader/HDR_Shader.h"
#include "shader/pre_genSkyBox_Shader.h"
#include "shader/Skybox_Shader.h"

#include "imgui-master/imgui.h"
#include "imgui-master/imconfig.h"
#include "imgui-master/imgui_internal.h"
#include "imgui-master/imstb_rectpack.h"
#include "imgui-master/imstb_textedit.h"
#include "imgui-master/imstb_truetype.h"

#include "imgui-master/backends/imgui_impl_glfw.h"
#include "imgui-master/backends/imgui_impl_opengl3.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

GLFWwindow *window;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Vec3f eye(3.0, 3.4, 4);
Vec3f eye(0.03, 0.0, -0.4);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);
Vec3f front(0, 0, -1);

Camera camera(eye, center, up);
float fov = 40;

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
Vec4i getBackGroundC(ImVec4 &clear_color)
{
    return Vec4i(255 * clear_color.x * clear_color.w, 255 * clear_color.y * clear_color.w, 255 * clear_color.z * clear_color.w, 255 * clear_color.w);
}
void saveImage(FrameBuffer &fbuffer)
{
    stbi_flip_vertically_on_write(true);
    stbi_write_png("output.png", SCR_WIDTH, SCR_HEIGHT, 4, fbuffer.colorBuffer, SCR_WIDTH * 4);
}
int main(int argc, char **argv)
{
    initWindow();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    // GL 3.2 + GLSL 150
    const char *glsl_version = "#version 150";

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_render_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //------------------------------------------------------------------------------------------

    FrameBuffer fbuffer(SCR_WIDTH, SCR_HEIGHT);

    Matrix viewMat = lookat(eye, center, up);
    Matrix viewPortMat = viewport(SCR_WIDTH, SCR_HEIGHT);
    // Matrix projMat = projection(89.f, SCR_WIDTH / SCR_HEIGHT, 0.1f, -0.9f);
    Matrix projMat = projection(fov, SCR_WIDTH / SCR_HEIGHT, 0.9f, -0.1f);
    Matrix modelMat = Matrix::identity();

    Model *obj_cube = new Model("resources/obj/cube/cube.obj");
    SimpleShader shader(modelMat, viewMat, projMat, viewPortMat); //default
    shader.setData(obj_cube);

    //skybox
    /*
    std::vector<std::string> _skyboxes = {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg",
    };
    Texture texture;
    int id_skybox;
    for (int i = 0; i < _skyboxes.size(); i++)
    {
        int tmp = texture.loadTexture(_skyboxes[i]);
        if (i == 0)
            id_skybox = tmp;
    }
    SkyboxShader skyshader(Matrix::identity(), viewMat, projMat, viewPortMat);
    skyshader.setTexture(&texture);
    skyshader.setData(obj_cube);

    skyshader.id_ = id_skybox;
    skyshader.type = LDR;
*/
    Texture texture;

    std::string _bg = "resources/hdr/snow_machine/test8_Bg.jpg";
    int id_bg = texture.loadTexture(_bg);
    Matrix projMat_sp = projection(90.f, SCR_WIDTH / SCR_HEIGHT, 2, -10.f);

    GenSkyboxShader gshader(Matrix::identity(), Matrix::identity(), projMat_sp, viewPortMat);
    gshader.setTexture(&texture);
    gshader.setData(obj_cube);

    std::vector<Image> skyboxmaps;
    for (int i = 0; i < 6; i++)
        skyboxmaps.push_back(Image(SCR_WIDTH, SCR_HEIGHT, 3, UINT));

    gshader.id = id_bg;
    gshader.type = LDR;
    int id_skybox;
    std::vector<Matrix> cubemap_viewmats;
    for (int i = 0; i < 6; i++)
        cubemap_viewmats.push_back(lookat(camera.eye, camera.eye+cubemap_dirs[i], cubemap_ups[i]));

    for (int i = 0; i < 6; i++)
    {
        gshader.view = cubemap_viewmats[i];
        gshader.image = &skyboxmaps[i];
        render(gshader, &fbuffer, TRIANGLE);

        int id_ = texture.setTexture(skyboxmaps[i]);
        if (i == 0)
            id_skybox = id_;

        stbi_flip_vertically_on_write(true);
        stbi_write_png(cubemap_output_paths[i].c_str(), SCR_WIDTH, SCR_HEIGHT, 4, fbuffer.colorBuffer, SCR_WIDTH * 4);
        fbuffer.clearBuffer();
    }
    SkyboxShader skyshader(Matrix::identity(), viewMat, projMat, viewPortMat);
    skyshader.setTexture(&texture);
    skyshader.setData(obj_cube);
    // skyshader.id_ = id_ibl_irradiance;
    //skyshader.type=HDR;
    skyshader.id_ = id_skybox;
    skyshader.type = LDR;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        fbuffer.clearColor(getBackGroundC(clear_color));
        fbuffer.clearZBuffer();
        shader.view = camera.update();
        render(shader, &fbuffer, TRIANGLE);
        projMat = projection(fov, SCR_WIDTH / SCR_HEIGHT, 0.9f, -0.1f);
        skyshader.projection = projMat;
        //render(skyshader, &fbuffer, TRIANGLE);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 教程1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        //ImGui::ShowDemoWindow(&show_demo_window);

        {
            ImGui::Begin("Render Window", &show_render_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

            glEnable(GL_TEXTURE_2D);
            unsigned int textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Set texture wrapping to GL_REPEAT
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Set texture filtering
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 800, 800, 0, GL_RGBA, GL_UNSIGNED_BYTE, fbuffer.colorBuffer);

            ImGui::Image((void *)(intptr_t)textureID, ImVec2(SCR_WIDTH, SCR_HEIGHT));

            ImGui::End();
        }
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("^u^ let's start rendering!"); // Create a window called "Hello, world!" and append into it.

            ImGui::RadioButton("campera", true);

            ImGui::SliderFloat("fov", &fov, 0, 90);
            ImGui::SliderFloat("camera pos-x:", &camera.eye.x, -20, 20);
            ImGui::SliderFloat("camera pos-y:", &camera.eye.y, -20, 20);
            ImGui::SliderFloat("camera pos-z:", &camera.eye.z, -20, 20);

            //ImGui::Checkbox("Ambient Occlusion", true); // Edit bools storing our window open/close state

            ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            //if (ImGui::Button("load obj~")) // Buttons return true when clicked (most widgets return true when edited/activated)
            //  counter++;
            //ImGui::SameLine();
            //ImGui::Text("counter = %d", counter);

            if (ImGui::Button("save"))
                saveImage(fbuffer);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        glfwSwapBuffers(window);
    }

    delete obj_cube;
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(ROTATE_LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(ROTATE_RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    std::cout << "yes";
    // if (action == GLFW_PRESS)
    //{//
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    //   }
    // else
    //{
    //  glfwSetCursorPosCallback(window, nullptr);
    //glfwSetScrollCallback(window, nullptr);
    // }
}
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

int initWindow()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "tinyRender", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); // Enable vsync

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glfwSetMouseButtonCallback(window, mouse_button_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetScrollCallback(window, scroll_callback);

    //隐藏光标
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    return 0;
}