#pragma once

#include <IConsole.h>

namespace CloseAllNetworkPeers
{
    class ConsoleCommandCVars
    {
    public:
        void RegisterCVars();
        void UnregisterCVars();

        static void CloseAll(IConsoleCmdArgs* args);
    };
}
