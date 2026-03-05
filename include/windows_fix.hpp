/**
 * @file windows_fix.hpp
 * @brief Windows API 与 raylib 冲突修复
 *
 * 此文件必须在包含 CodingSnake.hpp 之前包含，用于重命名与 raylib 冲突的 Windows API 函数
 *
 * 使用方法：
 * 1. 在包含 CodingSnake.hpp 之前包含此文件
 * 2. 在包含 raylib.h 之后，如果需要可以使用 #undef 取消宏定义
 */

#ifndef WINDOWS_FIX_HPP
#define WINDOWS_FIX_HPP

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN  // 排除 Windows.h 中不常用的 API
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX             // 防止 Windows.h 定义 min/max 宏
    #endif

    // 重命名与 raylib 冲突的 Windows API 函数
    #ifndef Rectangle
        #define Rectangle RL_Rectangle
    #endif
    #ifndef CloseWindow
        #define CloseWindow RL_CloseWindow
    #endif
    #ifndef ShowCursor
        #define ShowCursor RL_ShowCursor
    #endif
    #ifndef DrawText
        #define DrawText RL_DrawText
    #endif
    #ifndef LoadImage
        #define LoadImage RL_LoadImage
    #endif
    #ifndef PlaySound
        #define PlaySound RL_PlaySound
    #endif
    #ifndef DrawTextEx
        #define DrawTextEx RL_DrawTextEx
    #endif
#endif

#endif // WINDOWS_FIX_HPP
