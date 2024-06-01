#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <random>

#include<util/logger.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include<engine/context.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include<glm/glm/vec4.hpp>
#include<glm/glm/vec3.hpp>
#include<glm/glm/vec2.hpp>
#include<glm/glm/mat4x4.hpp>

//#include<tinygltf/tiny_gltf.h>

