#pragma once
#include <pch.h>
#include <engine/camera.h>
#include <keycode.h>
class MWindow
{
    public:

    MWindow() = default;
    MWindow(const std::string& title, int w,int h);
    ~MWindow();

    int getWidth();
    int getHeight();
    
    void init();

    void tick(float delta);

    GLFWwindow* window;

    std::shared_ptr<Camera>m_cam;

    private:
    int m_width;
    int m_height;
    std::string m_title;

   

    glm::vec2 m_mousePos;

    glm::vec2 m_mouseOffset;

    glm::vec2 m_mouseLastPos;

    bool firstMouse;

    bool m_active;

    public:

    void setWinTitle(const std::string& title);

    static void callbackOnResize(GLFWwindow* window, int width, int height);
	static void callbackOnMouseButton(GLFWwindow* window, int button, int action, int mods);
	static void callbackOnScroll(GLFWwindow* window, double xoffset, double yoffset);
	static void callbackOnSetFoucus(GLFWwindow* window, bool bFoucus);

    void processMouseMove(double xpos, double ypos);

    static void callbackOnMouseMove(GLFWwindow* window,double xpos, double ypos);

    bool isMouseButtonPressed(MouseCode button) const;

    bool isKeyPressed(const KeyCode key) const;
};

