#pragma once

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0);


#define MAX_IN_FLIGHT 2

#define GRASS_COUNT 262144

#define WIND_TEX_DIM 512

#define PATCH_SIZE 5 

#define CUBEMAP_DIM 64

#define SPECULAR_DIM 512

#define BRDF_DIM 512

#define M_PI 3.1415926536

#define SHADOW_DIM 4096

#define BLUR_MIP 6