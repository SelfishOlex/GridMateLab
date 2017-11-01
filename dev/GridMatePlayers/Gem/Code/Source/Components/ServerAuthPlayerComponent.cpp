#include "StdAfx.h"
#include "ServerAuthPlayerComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Replica/RemoteProcedureCall.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMatePlayers/LocalClientBus.h>
#include <AzCore/Component/TransformBus.h>
#include <LmbrCentral/Physics/PhysicsComponentBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

class ServerAuthPlayerComponent::Chunk
    : public GridMate::ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);
    explicit Chunk(
        MemberIDCompact playerId = 0,
        const Vector3& pos = Vector3::CreateZero())
        : m_owningPlayer("Owning Player", playerId)
        , m_startingPosition("Starting Position", pos) {}

    static const char* GetChunkName()
    {
        return "ServerAuthPlayerComponent::Chunk";
    }

    bool IsReplicaMigratable() override { return true; }

    DataSet<MemberIDCompact> m_owningPlayer;
    DataSet<Vector3> m_startingPosition;

    class Desc : public GridMate::ReplicaChunkDescriptor
    {
    public:
        Desc() : ReplicaChunkDescriptor(
            Chunk::GetChunkName(), sizeof(Chunk)) {}

        ReplicaChunkBase* CreateFromStream(
            UnmarshalContext& mc) override
        {
            if (!mc.m_hasCtorData)
            {
                return CreateReplicaChunk<Chunk>();
            }

            Vector3 position;
            mc.m_iBuf->Read(position);
            MemberIDCompact playerId;
            mc.m_iBuf->Read(playerId);

            return CreateReplicaChunk<Chunk>(playerId, position);
        }

        void DiscardCtorStream(UnmarshalContext& mc) override
        {
            Vector3 position;
            mc.m_iBuf->Read(position);
            MemberIDCompact playerId;
            mc.m_iBuf->Read(playerId);
        }

        void DeleteReplicaChunk(
            ReplicaChunkBase* chunkInstance) override
        {
            delete chunkInstance;
        }

        void MarshalCtorData(ReplicaChunkBase* chunkInstance,
            WriteBuffer& wb) override
        {
            if (Chunk* chunk = static_cast<Chunk*>(chunkInstance))
            {
                ServerAuthPlayerComponent* component =
                    static_cast<ServerAuthPlayerComponent*>(
                        chunk->GetHandler());

                Vector3 position = Vector3::CreateZero();
                EBUS_EVENT_ID_RESULT(position,
                    component->GetEntityId(), TransformBus,
                    GetWorldTranslation);

                wb.Write(position);
                wb.Write(chunk->m_owningPlayer.Get());
            }
        }
    };
};

void ServerAuthPlayerComponent::Reflect(
    AZ::ReflectContext* context)
{
    if (auto sc = azrtti_cast<SerializeContext*>(context))
    {
        sc->Class<ServerAuthPlayerComponent, Component>()
          ->Version(1);

        if (EditContext* ec = sc->GetEditContext())
        {
            ec->Class<ServerAuthPlayerComponent>(
                  "Server Auth Player Body",
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

    auto& descTable = ReplicaChunkDescriptorTable::Get();
    if (!descTable.FindReplicaChunkDescriptor(
        ReplicaChunkClassId(Chunk::GetChunkName())))
    {
        descTable.RegisterChunkType<Chunk, Chunk::Desc>();
    }
}

void ServerAuthPlayerComponent::SetAssociatedPlayerId(
    const GridMate::MemberIDCompact& player)
{
    if (Chunk* chunk = static_cast<Chunk*>(m_chunk.get()))
        chunk->m_owningPlayer.Set(player);
}

ReplicaChunkPtr ServerAuthPlayerComponent::GetNetworkBinding()
{
    m_chunk = GridMate::CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void ServerAuthPlayerComponent::SetNetworkBinding(
    GridMate::ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void ServerAuthPlayerComponent::UnbindFromNetwork()
{
    if (m_chunk)
    {
        m_chunk->SetHandler(nullptr);
        m_chunk = nullptr;
    }
}

void ServerAuthPlayerComponent::Activate()
{
    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        ServerPlayerBodyBus::Handler::BusConnect(GetEntityId());
    }
    else if (Chunk* chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        const Vector3 pos = chunk->m_startingPosition.Get();
        EBUS_EVENT_ID(GetEntityId(), TransformBus,
            SetWorldTranslation, pos);

        EBUS_EVENT_ID(GetEntityId(),
            LmbrCentral::PhysicsComponentRequestBus,
            EnablePhysics);

        EBUS_EVENT(LocalClientBus, AttachToBody,
            chunk->m_owningPlayer.Get(), GetEntityId());
    }
}

void ServerAuthPlayerComponent::Deactivate()
{
    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        ServerPlayerBodyBus::Handler::BusDisconnect();
    }
}