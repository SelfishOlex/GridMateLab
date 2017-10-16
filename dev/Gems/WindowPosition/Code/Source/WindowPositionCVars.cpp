#include "StdAfx.h"
#include "WindowPositionCVars.h"
#include <windows.h>
#include <Winuser.h>
#include <ISystem.h>

using namespace WindowPosition;

void WindowPositionCVars::RegisterCVars()
{
    if (gEnv && !gEnv->IsEditor())
    {
        REGISTER_COMMAND("windowx", WindowX, 0,
            "set X position of the window");
        REGISTER_COMMAND("windowy", WindowY, 0,
            "set X position of the window");
        REGISTER_COMMAND("windowxy", WindowXY, 0,
            "set X and Y position of the window");
    }
}

void WindowPositionCVars::UnregisterCVars()
{
    if (gEnv && !gEnv->IsEditor())
    {
        UNREGISTER_CVAR("windowx");
        UNREGISTER_CVAR("windowy");
        UNREGISTER_CVAR("windowxy");
    }
}

void WindowPositionCVars::WindowX(IConsoleCmdArgs* args)
{
    if (args->GetArgCount() > 1)
    {
        const auto x = atoi(args->GetArg(1));

        if (const auto windowHandle = GetActiveWindow())
        {
            WINDOWINFO windowInfo;
            windowInfo.cbSize = sizeof(WINDOWINFO);
            if (GetWindowInfo(windowHandle, &windowInfo))
                SetWindowPos(windowHandle, nullptr,
                    x,
                    windowInfo.rcWindow.top,
                    0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE);
        }
    }
}

void WindowPositionCVars::WindowY(IConsoleCmdArgs* args)
{
    if (args->GetArgCount() > 1)
    {
        const auto y = atoi(args->GetArg(1));

        if (const auto windowHandle = GetActiveWindow())
        {
            WINDOWINFO windowInfo;
            windowInfo.cbSize = sizeof(WINDOWINFO);
            if (GetWindowInfo(windowHandle, &windowInfo))
                SetWindowPos(windowHandle, nullptr,
                    windowInfo.rcWindow.left,
                    y,
                    0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE);
        }
    }
}

void WindowPositionCVars::WindowXY(IConsoleCmdArgs* args)
{
    if (args->GetArgCount() > 2)
    {
        const auto x = atoi(args->GetArg(1));
        const auto y = atoi(args->GetArg(2));

        if (const auto windowHandle = GetActiveWindow())
            SetWindowPos(windowHandle, nullptr,
                x,
                y,
                0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE);
    }
}
