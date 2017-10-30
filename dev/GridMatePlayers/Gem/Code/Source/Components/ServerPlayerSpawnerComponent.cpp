#include "StdAfx.h"
#include "ServerPlayerSpawnerComponent.h"
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

void ServerPlayerSpawnerComponent::Reflect(ReflectContext* rc)
{
    if (auto sc = azrtti_cast<SerializeContext*>(rc))
    {
        sc->Class<ServerPlayerSpawnerComponent, Component>()
            ->Version(1);

        if (EditContext* ec = sc->GetEditContext())
        {
            ec->Class<ServerPlayerSpawnerComponent>(
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

void ServerPlayerSpawnerComponent::Activate()
{
#if defined(DEDICATED_SERVER)
    ISystem* system = nullptr;
    EBUS_EVENT_RESULT(system, CrySystemRequestBus, GetCrySystem);
    if (system)
    {
        SessionEventBus::Handler::BusConnect(
            system->GetINetwork()->GetGridMate());
    }
#endif
}

void ServerPlayerSpawnerComponent::Deactivate()
{
#if defined(DEDICATED_SERVER)
    SessionEventBus::Handler::BusDisconnect();
#endif
}

void ServerPlayerSpawnerComponent::OnMemberJoined(
    GridMate::GridSession* session,
    GridMate::GridMember* member)
{
    const MemberIDCompact playerId = member->GetIdCompact();
    if (session->GetMyMember()->GetIdCompact() == playerId)
        return; // ignore ourselves, the server

    const Transform t = Transform::CreateTranslation(
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

    m_joiningplayers[ticket] = playerId;
    SliceInstantiationResultBus::MultiHandler::BusConnect(ticket);
}

void ServerPlayerSpawnerComponent::OnSliceInstantiated(
    const AZ::Data::AssetId&,
    const SliceComponent::SliceInstanceAddress& address)
{
    const SliceInstantiationTicket& ticket =
        *SliceInstantiationResultBus::GetCurrentBusId();
    const auto iter = m_joiningplayers.find(ticket);
    if (iter != m_joiningplayers.end())
    {
        const MemberIDCompact playerId = iter->second;
        SliceInstantiationResultBus::MultiHandler::BusDisconnect(
            ticket);

        for (Entity* entity : address.second->
            GetInstantiated()->m_entities)
        {
            EBUS_EVENT_ID(entity->GetId(), ServerPlayerBodyBus,
                SetAssociatedPlayerId, playerId);
        }
    }
    m_joiningplayers.erase(iter);
}