#include "StdAfx.h"
#include "ConsoleCommandCVars.h"
#include <ISystem.h>
#include <CloseAllNetworkPeers/CloseAllNetworkPeersBus.h>

using namespace CloseAllNetworkPeers;

void ConsoleCommandCVars::RegisterCVars()
{
    if (gEnv && !gEnv->IsEditor())
    {
        REGISTER_COMMAND("closeall", CloseAll, 0,
            "Closes the server and all connected clients.");
    }
}

void ConsoleCommandCVars::UnregisterCVars()
{
    if (gEnv && !gEnv->IsEditor())
    {
        UNREGISTER_CVAR("closeall");
    }
}

void ConsoleCommandCVars::CloseAll(IConsoleCmdArgs* /*args*/)
{
    AZ_Printf("Gem", "CloseAll called");

    EBUS_EVENT(CloseAllNetworkPeersRequestBus,
        CloseAllNetworkPeers);
}
