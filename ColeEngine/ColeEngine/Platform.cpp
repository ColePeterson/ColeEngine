#include "Platform.h"

#include<iostream>

static void error_callback(int error, const char* msg)
{
    fputs(msg, stderr);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //glViewport(0, 0, width, height);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
   // glViewport(0, 0, width, height);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
   
}

static void initGlew()
{
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (GLEW_OK != err)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    
}



Platform::Platform(int _width, int _height)
    : width(_width), height(_height), focused(true), menu(true), mouseLeftRelease(false), mouseRightRelease(false),
    mouseLeft(false), mouseRight(false)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        std::cout << "GLFW Failed to initialize!\n";
        exit(-1);
    }

    glfwWindowHint(GLFW_RESIZABLE, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 0);

    window = glfwCreateWindow(width, height, "Cole Engine", NULL, NULL);

    if (!window) { glfwTerminate();  exit(-1); }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

   // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    //glfwSetCursorPosCallback(window, cursor_position_callback);
    //glfwSetMouseButtonCallback(window, mouse_button_callback);

    initGlew();
}

Platform::~Platform()
{
    glfwTerminate();
}

void Platform::setWindowSize(int _width, int _height)
{
    width = _width;
    height = _height;

    glfwSetWindowSize(window, width, height);
}

void Platform::update()
{
    // Set window width and height members
    int sWidth, sHeight;
    glfwGetFramebufferSize(window, &width, &height);

    updateMouse();
    updateKeyboard();
}


void Platform::updateMouse()
{
    int mLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    mouseLeftRelease = false;
    mouseRightRelease = false;

    if (mLeft == GLFW_PRESS)
    {
        mouseLeft = true;
    }
    else if (mLeft == GLFW_RELEASE)
    {
        if (mouseLeft)
        {
            mouseLeftRelease = true;
        }
        mouseLeft = false;
    }

    int mRight = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

    if (mRight == GLFW_PRESS)
    {
        mouseRight = true;
    }
    else if (mRight == GLFW_RELEASE)
    {
        if (mouseRight)
        {
            mouseRightRelease = true;
        }
        mouseRight = false;
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);


    dx = xpos - mouseX;
    dy = ypos - mouseY;

    mouseX = xpos;
    mouseY = ypos;
}

void Platform::updateKeyboard()
{
    int key_w = glfwGetKey(window, GLFW_KEY_W);
    if (key_w == GLFW_PRESS)
        w_down = true;
    else if (key_w == GLFW_RELEASE)
        w_down = false;
    
    int key_s = glfwGetKey(window, GLFW_KEY_S);
    if (key_s == GLFW_PRESS)
        s_down = true;
    else if (key_w == GLFW_RELEASE)
        s_down = false;

    int key_a = glfwGetKey(window, GLFW_KEY_A);
    if (key_a == GLFW_PRESS)
        a_down = true;
    else if (key_a == GLFW_RELEASE)
        a_down = false;

    int key_d = glfwGetKey(window, GLFW_KEY_D);
    if (key_d == GLFW_PRESS)
        d_down = true;
    else if (key_w == GLFW_RELEASE)
        d_down = false;

    int key_lsfift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    if (key_lsfift == GLFW_PRESS)
        lshift_down = true;
    else if (key_lsfift == GLFW_RELEASE)
        lshift_down = false;


    int key_q = glfwGetKey(window, GLFW_KEY_Q);
    if (key_q == GLFW_PRESS) {
        q_down = true;

        if (menu) menu = false;
        else menu = true;
    }
    else if (key_q == GLFW_RELEASE){
        q_down = false;

       
    }
        
    //if(menu) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    //else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (menu) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    
   
}
