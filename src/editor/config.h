#pragma once
#include <string>

    struct Config
    {
        /* data */
        std::string appName = "aoba";

        struct InitWindowInfo
		{
			// Window resizeable state.
			bool bResizeable = true;

			// Init windows size.
			uint32_t initWidth  = 800;
			uint32_t initHeight = 480;
		} windowInfo = { };


    };