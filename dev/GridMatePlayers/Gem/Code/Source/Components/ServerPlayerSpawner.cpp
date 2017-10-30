#include "StdAfx.h"
#include "ServerPlayerSpawner.h"
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <ISystem.h>
#include <INetwork.h>
#include <LmbrCentral/Scripting/SpawnerComponentBus.h>
#include <GridMatePlayers/ServerPlayerBodyBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

void ServerPlayerSpawner::Reflect(ReflectContext* context)
{
    if (auto sc = azrtti_cast<SerializeContext*>(context))
    {
        sc->Class<ServerPlayerSpawner, Component>()
            ->Version(1);

        if (EditContext* ec = sc->GetEditContext())
        {
            ec->Class<ServerPlayerSpawner>(
                "Server Player Spawner",
                "Server Authoritative")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMate Players")
                ->Attribute(
                    Edit::Attributes::
                    AppearsInAddComponentMenu,
                    AZ_CRC("Game"));
        }
    }
}

void ServerPlayerSpawner::Activate()
{
    if (gEnv && gEnv->IsDedicated())
    {
        SessionEventBus::Handler::BusConnect(
            gEnv->pNetwork->GetGridMate());
    }
}

void ServerPlayerSpawner::Deactivate()
{
    if (gEnv && gEnv->IsDedicated())
    {
        SessionEventBus::Handler::BusDisconnect();
    }
}

void ServerPlayerSpawner::OnMemberJoined(
    GridMate::GridSession* session,
    GridMate::GridMember* member)
{
    const MemberIDCompact playerId = member->GetIdCompact();
    if (session->GetMyMember()->GetIdCompact() == playerId)
        return; // ignore ourselves, the server

    const auto t = Transform::CreateTranslation(
        Vector3::CreateAxisY(10.f - m_playerCount * 1.f));
    m_playerCount++;

    AZ_Printf("GridMatePlayers", "Spawning player @ %f %f %f",
        static_cast<float>(t.GetTranslation().GetX()),
        static_cast<float>(t.GetTranslation().GetY()),
        static_cast<float>(t.GetTranslation().GetZ()));

    AzFramework::SliceInstantiationTicket ticket;
    EBUS_EVENT_ID_RESULT(ticket, GetEntityId(),
        LmbrCentral::SpawnerComponentRequestBus,
        SpawnRelative, t);

    m_spawningPlayer = playerId;
    SliceInstantiationResultBus::MultiHandler::BusConnect(
        ticket);
}

void ServerPlayerSpawner::OnSliceInstantiated(
    const AZ::Data::AssetId&,
    const SliceComponent::SliceInstanceAddress& address)
{
    // TODO support clients joining at the same time

    SliceInstantiationResultBus::MultiHandler::BusDisconnect();
    for (auto& entity : address.second->
        GetInstantiated()->m_entities)
    {
        EBUS_EVENT_ID(entity->GetId(), ServerPlayerBodyBus,
            SetAssociatedPlayerId, m_spawningPlayer);
    }

    m_spawningPlayer = 0;
}