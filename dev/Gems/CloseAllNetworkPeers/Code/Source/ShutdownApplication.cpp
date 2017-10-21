#include "StdAfx.h"
#include <CloseAllNetworkPeers/ShutdownApplication.h>
#include <ISystem.h>
#include <IConsole.h>

void CloseAllNetworkPeers::ShutdownApplication()
{
    ISystem* system = nullptr;
    EBUS_EVENT_RESULT(system, CrySystemRequestBus, GetCrySystem);
    if (system)
    {
        system->GetIConsole()->ExecuteString("quit");
    }
}
