#include<window/mwindow.h>

#define STB_IMAGE_IMPLEMENTATION
#include<tinygltf/stb_image.h>

void MWindow::callbackOnMouseMove(GLFWwindow* window,double xpos, double ypos)
{
    auto ptr = reinterpret_cast<MWindow*>(glfwGetWindowUserPointer(window));
    ptr->processMouseMove(xpos,ypos);
}

MWindow::MWindow(const std::string& title, int w,int h):m_title(title),m_width(w),m_height(h){};

void MWindow::init()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(m_width,m_height,m_title.c_str(),nullptr,nullptr);


    //init members
    m_cam = std::make_shared<Camera>();

    m_mousePos = glm::vec2(0.0f);

    m_mouseOffset = glm::vec2(0.0f);

    firstMouse = true;

    m_active = false;

    //

    GLFWimage ico;

    ico.pixels = stbi_load("../../src/aoba.png",&ico.width,&ico.height,0,4);
    
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, (void*)(this));
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPos(window, m_width, m_height);
    glfwSetWindowIcon(window, 1, &ico);

    //set callback
    glfwSetCursorPosCallback(window,callbackOnMouseMove);
    
    LOG_TRACE("init GLFWwindow success");
}

void MWindow::setWinTitle(const std::string& title)
{
    std::string t = m_title + " - " + title;
    glfwSetWindowTitle(window,t.c_str());
}

void MWindow::tick(float delta)
{
    m_active = false;

    m_mouseOffset = glm::vec2(0.0f);

    if(firstMouse)
    {
        m_mouseLastPos = m_mousePos;
        firstMouse = false;
    }

    if(isMouseButtonPressed(Mouse::ButtonLeft)) m_active = true;

    if(m_active)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_mouseOffset.x = m_mousePos.x - m_mouseLastPos.x;
        m_mouseOffset.y = m_mouseLastPos.y - m_mousePos.y;
        m_cam->ProcessMouseMovement(m_mouseOffset.x,m_mouseOffset.y);
    }
    else{
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    m_mouseLastPos = m_mousePos;

    if(isKeyPressed(Key::W)) m_cam->ProcessKeyboard(FORWARD,delta);
    if(isKeyPressed(Key::A)) m_cam->ProcessKeyboard(LEFT,delta);
    if(isKeyPressed(Key::S)) m_cam->ProcessKeyboard(BACKWARD,delta);
    if(isKeyPressed(Key::D)) m_cam->ProcessKeyboard(RIGHT,delta);
    if(isKeyPressed(Key::Escape)) glfwSetWindowShouldClose(window,true);
}

MWindow::~MWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int MWindow::getWidth()
{
    return m_width;
}

int MWindow::getHeight()
{
    return m_height;
}

void MWindow::callbackOnResize(GLFWwindow* window,int width, int height)
{

}


void MWindow::callbackOnMouseButton(GLFWwindow* window,int button, int action, int mods)
{

}

void MWindow::callbackOnScroll(GLFWwindow* window, double xoffset, double yoffset)
{

}

void MWindow::callbackOnSetFoucus(GLFWwindow* window,bool bFoucus)
{

}

void MWindow::processMouseMove(double xpos, double ypos)
{
    float xposf = static_cast<float>(xpos);
    float yposf = static_cast<float>(ypos);

    // if (firstMouse)
    // {
    //     m_mousePos.x = xposf;
    //     m_mousePos.y = yposf;
    //     firstMouse = false;
    // }

    // m_mouseOffset.x = xposf - m_mousePos.x;
    // m_mouseOffset.y = m_mousePos.y - yposf; // reversed since y-coordinates go from bottom to top

    m_mousePos.x = xposf;
    m_mousePos.y = yposf;

}

bool MWindow::isMouseButtonPressed(MouseCode button) const
{
    auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
	return state == GLFW_PRESS;
}

bool MWindow::isKeyPressed(const KeyCode key) const
{
    auto state = glfwGetKey(window, static_cast<int32_t>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}