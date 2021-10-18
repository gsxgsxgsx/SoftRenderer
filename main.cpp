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

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

GLFWwindow *window;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Vec3f eye(-3.0, 3.4, -4);
Vec3f eye(0.3, 0.05, 4);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);
Vec3f front(0, 0, -1);

Camera camera(eye, center, up);

int main(int argc, char **argv)
{
    initWindow();
    FrameBuffer fbuffer(SCR_WIDTH, SCR_HEIGHT);

    Matrix viewMat = lookat(eye, center, up);
    Matrix viewPortMat = viewport(SCR_WIDTH, SCR_HEIGHT);
    // Matrix projMat = projection(89.f, SCR_WIDTH / SCR_HEIGHT, 0.1f, -0.9f);
    Matrix projMat = projection(50.f, SCR_WIDTH / SCR_HEIGHT, 9.f, -0.1f);

    //------------------------------------------------------------------------------------------

    std::vector<std::string> _aos = {
        "resources/pbr/streaky-metal1-unity/streaky-metal1_ao.png",
        "resources/pbr/worn-shiny-metal-unity/worn-shiny-metal-ao.png",
        "resources/pbr/rock-slab-wall1-unity/rock-slab-wall_ao.png",
        "resources/pbr/scuffed-plastic-1-bl/scuffed-plastic5-ao.png"};
    std::vector<std::string> _albedos = {
        "resources/pbr/streaky-metal1-unity/streaky-metal1_albedo.png",
        "resources/pbr/worn-shiny-metal-unity/worn-shiny-metal-albedo.png",
        "resources/pbr/rock-slab-wall1-unity/rock-slab-wall_albedo.png",
        "resources/pbr/scuffed-plastic-1-bl/scuffed-plastic5-alb.png",
        "resources/pbr/oxidized-copper-ue/oxidized-copper-albedo.png"};

    std::vector<std::string> _tnormals = {
        "resources/pbr/streaky-metal1-unity/streaky-metal1_normal-ogl.png",
        "resources/pbr/worn-shiny-metal-unity/worn-shiny-metal-Normal-ogl.png",
        "resources/pbr/rock-slab-wall1-unity/rock-slab-wall_normal-ogl.png",
        "resources/pbr/scuffed-plastic-1-bl/scuffed-plastic-normal.png",
        "resources/pbr/oxidized-copper-ue/oxidized-copper-normal-ue.png"};

    std::vector<std::string> _materials = {
        "resources/pbr/streaky-metal1-unity/streaky-metal1_metallic.psd",
        "resources/pbr/worn-shiny-metal-unity/worn-shiny-metal-Metallic.psd",
        "resources/pbr/rock-slab-wall1-unity/rock-slab-wall_metallic.psd",
        "resources/pbr/scuffed-plastic-1-bl/scuffed-plastic-metal.png",
        "resources/pbr/oxidized-copper-ue/oxidized-copper-metal.png"};

    //std::string _rough = "resources/pbr/scuffed-plastic-1-bl/scuffed-plastic-rough.png";
    std::string _rough = "resources/pbr/oxidized-copper-ue/oxidized-coppper-roughness.png";

    std::string _env = "resources/hdr/snow_machine/test8_Env.hdr";
    std::string _bg = "resources/hdr/snow_machine/test8_Bg.jpg";
    std::string _ref = "resources/hdr/snow_machine/test8_Ref.hdr";

    std::string _brdf = "resources/hdr/ibl_brdf_lut.png";

    Texture texture;
    /*  int id_aoes[4], id_albedoes[4], id_tns[4], id_ms[4];
    for (int i = 0; i < 4; i++)
    {
        id_aoes[i] = texture.loadTexture(_aos[i]);
        id_albedoes[i] = texture.loadTexture(_albedos[i]);
        id_tns[i] = texture.loadTexture(_tnormals[i]);
        id_ms[i] = texture.loadTexture(_materials[i]);
    }*/

    int id_r = texture.loadTexture(_rough);

    int id_bg = texture.loadTexture(_bg);
    int id_irr = texture.loadTexture(_env);
    int id_ref = texture.loadTexture(_ref);
    int id_ibl_irradiance = 0;
    int id_ibl_specular = 0;
    int id_brdf = texture.loadTexture(_brdf);

    //读取辐照度ibl贴图------------------------------------------------------------------------------------------
    Model *obj_cube = new Model("resources/obj/cube/cube.obj");
    std::vector<Matrix> cubemap_viewmats;
    for (int i = 0; i < 6; i++)
        cubemap_viewmats.push_back(lookat(Vec3f(0, 0, 0), cubemap_dirs[i], cubemap_ups[i]));

    Matrix projMat_sp = projection(90.f, SCR_WIDTH / SCR_HEIGHT, 2, -10.f);
    ///
    std::vector<Image> irramaps;
    for (int i = 0; i < 6; i++)
        irramaps.push_back(Image(SCR_WIDTH, SCR_HEIGHT, 3, FLOAT));

    GenSkyboxShader gshader(Matrix::identity(), Matrix::identity(), projMat_sp, viewPortMat);
    gshader.setTexture(&texture);
    gshader.setData(obj_cube);
    gshader.id = id_irr;
    gshader.type = HDR;

    for (int i = 0; i < 6; i++)
    {
        gshader.view = cubemap_viewmats[i];
        gshader.image = &irramaps[i];
        render(gshader, &fbuffer, TRIANGLE);

        int id_ = texture.setTexture(irramaps[i]);
        if (i == 0)
            id_ibl_irradiance = id_;

        //stbi_flip_vertically_on_write(true);
        //stbi_write_png(cubemap_output_paths[i].c_str(), SCR_WIDTH, SCR_HEIGHT, 4, fbuffer.colorBuffer, SCR_WIDTH * 4);
        fbuffer.clearBuffer();
    }

    //读取镜面ibl贴图------------------------------------------------------------------------------------------
    std::vector<Image> specmaps;
    for (int i = 0; i < 6; i++)
        specmaps.push_back(Image(SCR_WIDTH, SCR_HEIGHT, 3, FLOAT));
    gshader.id = id_ref;
    gshader.type = HDR;

    for (int i = 0; i < 6; i++)
    {
        gshader.view = cubemap_viewmats[i];
        gshader.image = &specmaps[i];
        render(gshader, &fbuffer, TRIANGLE);

        int id_ = texture.setTexture(specmaps[i]);
        if (i == 0)
            id_ibl_specular = id_;

        fbuffer.clearBuffer();

        //stbi_flip_vertically_on_write(true);
        //stbi_write_png(cubemap_output_paths[i].c_str(), SCR_WIDTH, SCR_HEIGHT, 4, fbuffer.colorBuffer, SCR_WIDTH * 4);
    }
    //LUT------------------------------------------------------------------------------------------
    /*Image lut(300, 300, 3, FLOAT);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            float nw = i / 300.f;
            float roughness = i / 300.f;
        }
    }*/
    //渲染背景------------------------------------------------------------------------------------------
    std::vector<Image> skyboxmaps;
    for (int i = 0; i < 6; i++)
        skyboxmaps.push_back(Image(SCR_WIDTH, SCR_HEIGHT, 3, UINT));

    gshader.id = id_bg;
    gshader.type = LDR;
    int id_skybox;
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
    render(skyshader, &fbuffer, TRIANGLE);

    //------------------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        fbuffer.clearBuffer();

        glDrawPixels(SCR_WIDTH, SCR_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, fbuffer.colorBuffer);

        stbi_flip_vertically_on_write(true);
        stbi_write_png("output.png", SCR_WIDTH, SCR_HEIGHT, 4, fbuffer.colorBuffer, SCR_WIDTH * 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //delete objmodel;
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
    glfwInit();

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "tinyRender", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //隐藏光标
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    return 0;
}