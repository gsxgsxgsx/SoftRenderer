

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
unsigned int loadTexture(char const *path);
int initWindow();
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

