#include "StdAfx.h"
#include "ConsoleCommandCVars.h"
#include <ISystem.h>
#include <CloseAllNetworkPeers/CloseAllNetworkPeersRequestBus.h>

using namespace CloseAllNetworkPeers;

void ConsoleCommandCVars::RegisterCVars()
{
    if (gEnv && !gEnv->IsEditor())
    {
        REGISTER_COMMAND(m_commandName.c_str(), CloseAll, 0,
            "Closes the server and all connected clients.");
    }
}

void ConsoleCommandCVars::UnregisterCVars()
{
    if (gEnv && !gEnv->IsEditor())
    {
        UNREGISTER_CVAR(m_commandName.c_str());
    }
}

void ConsoleCommandCVars::CloseAll(IConsoleCmdArgs* /*args*/)
{
    EBUS_EVENT(CloseAllNetworkPeersRequestBus,
        CloseAllNetworkPeers);
}
