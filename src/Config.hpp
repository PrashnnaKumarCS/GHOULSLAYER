#pragma once
#include <string>

// Central configuration file for GHOULSLAYER.
// All gameplay numbers and engine settings are kept here
// so they can be easily tweaked without digging through logic code.
namespace Config {

    namespace Window {
        constexpr unsigned int WIDTH = 1280;
        constexpr unsigned int HEIGHT = 720;
        constexpr unsigned int FPS_LIMIT = 60;
        inline const std::string TITLE = "GHOULSLAYER - RISE. FIGHT. SURVIVE.";
    }

} // namespace Config
