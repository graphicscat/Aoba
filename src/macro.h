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