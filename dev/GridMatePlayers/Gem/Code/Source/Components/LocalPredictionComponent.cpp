#include "StdAfx.h"
#include "LocalPredictionComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include "GridMate/Replica/ReplicaFunctions.h"
#include "AzFramework/Network/NetBindingHandlerBus.h"
#include "GridMate/Replica/ReplicaMgr.h"
#include "LmbrCentral/Physics/CryCharacterPhysicsBus.h"
#include <GridMatePlayers/NetworkTimeRequestBus.h>
#include <GridMatePlayers/InterpolationBus.h>
#include <GridMatePlayers/PlayerBodyBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

class LocalPredictionComponent::Chunk : public ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);
    Chunk() : m_serverCheckpoint("Server Checkpoint") {}
    bool IsReplicaMigratable() override { return true; }

    static const char* GetChunkName()
    {
        return "LocalPredictionComponent::Chunk";
    }

    DataSetVectorInTime::BindInterface<
            LocalPredictionComponent,
            &LocalPredictionComponent::OnNewServerCheckpoint>
        m_serverCheckpoint;
};

void LocalPredictionComponent::Reflect(ReflectContext* reflect)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflect))
    {
        sc->Class<LocalPredictionComponent, Component>()
            ->Version(1);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<LocalPredictionComponent>(
                "Local Prediction",
                "[Guesses how server will move the character]")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMate Players")
                ->Attribute(Edit::Attributes::
                    AppearsInAddComponentMenu,
                    AZ_CRC("Game"));
        }
    }

    auto& descTable = ReplicaChunkDescriptorTable::Get();
    if (!descTable.FindReplicaChunkDescriptor(
        ReplicaChunkClassId(Chunk::GetChunkName())))
    {
        descTable.RegisterChunkType<Chunk>();
    }
}

void LocalPredictionComponent::Activate()
{
    const auto self = GetEntityId();
    LocalPredictionRequestBus::Handler::BusConnect(self);

    TransformNotificationBus::Handler::BusConnect(self);

    if (!NetQuery::IsEntityAuthoritative(GetEntityId()))
        TickBus::Handler::BusConnect();

    m_isActive = true;
}

void LocalPredictionComponent::Deactivate()
{
    m_isActive = false;

    LocalPredictionRequestBus::Handler::BusDisconnect();
    TransformNotificationBus::Handler::BusDisconnect();

    if (!NetQuery::IsEntityAuthoritative(GetEntityId()))
        TickBus::Handler::BusDisconnect();
}

ReplicaChunkPtr LocalPredictionComponent::GetNetworkBinding()
{
    m_chunk = GridMate::CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void LocalPredictionComponent::SetNetworkBinding(
    ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void LocalPredictionComponent::UnbindFromNetwork()
{
    m_chunk->SetHandler(nullptr);
    m_chunk = nullptr;
}

void LocalPredictionComponent::OnCharacterMoveForward(
    float speed, u32 time)
{
    m_movingForward = true;
    m_speed = speed;
    AZ_Printf("Book", "forward, local (-- %f --) @ %d",
        static_cast<float>(GetPosition().GetY()), time);
    m_history.AddDataPoint(GetPosition(), time);
}

void LocalPredictionComponent::OnCharacterStop(u32 time)
{
    m_movingForward = false;
    m_speed = 0;
    AZ_Printf("Book", "stopped, local (-- %f --) @ %d",
        static_cast<float>(GetPosition().GetY()), time);
    m_history.AddDataPoint(GetPosition(), time);
}

void LocalPredictionComponent::OnTransformChanged(
    const AZ::Transform&, const AZ::Transform& world)
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        if (chunk->IsMaster())
        {
            if (world.GetTranslation() !=
                chunk->m_serverCheckpoint.Get().m_vector)
            {
                AZ_Printf("Book", "server (-- %f --) @ %d",
                    static_cast<float>(
                        world.GetTranslation().GetY()),
                    GetTime());
            }

            chunk->m_serverCheckpoint.Set(
                { world.GetTranslation(), GetTime() });
        }
        else if (IsLocallyControlled())
        {
            m_history.AddDataPoint(world.GetTranslation(),
                GetTime());
        }
        else
        {
            TransformNotificationBus::Handler::BusDisconnect();
        }
    }
}

void LocalPredictionComponent::OnTick(float deltaTime,
    ScriptTimePoint)
{
    if (IsLocallyControlled())
    {
        EBUS_EVENT_ID(GetEntityId(),
            LmbrCentral::CryCharacterPhysicsRequestBus,
            RequestVelocity,
            Vector3::CreateAxisY(m_speed), 0);
    }
    else
    {
        TickBus::Handler::BusDisconnect();
    }
}

Vector3 LocalPredictionComponent::GetPosition() const
{
    auto v = AZ::Vector3::CreateZero();
    EBUS_EVENT_ID_RESULT(v, GetEntityId(), TransformBus,
        GetWorldTranslation);
    return v;
}

AZ::u32 LocalPredictionComponent::GetTime() const
{
    AZ::u32 t = 0;
    EBUS_EVENT_RESULT(t, NetworkTimeRequestBus, GetLocalTime);
    return t;
}

bool LocalPredictionComponent::IsLocallyControlled() const
{
    auto localClient = false;
    EBUS_EVENT_ID_RESULT(localClient, GetEntityId(),
        PlayerBodyRequestBus, IsAttachedToLocalClient);
    return localClient;
}

void LocalPredictionComponent::OnNewServerCheckpoint(
    const VectorInTime& value, const GridMate::TimeContext&)
{
    if (m_isActive &&
        !NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        const auto& serverPos = value.m_vector;
        const auto serverTime = value.m_time;

        if (m_history.HasHistory() && IsLocallyControlled())
        {
            const auto backThen =
                m_history.GetPositionAt(serverTime);
            const auto diff = backThen - serverPos;
            const auto diffLength = diff.GetLength();

            const auto allowedDeviation =
                (GetTime() - serverTime) * 0.001f *
                AZStd::GetMax(m_speed, .5f);

            if (diffLength > allowedDeviation)
            {
                m_history.DeleteAfter(serverTime);
                m_history.AddDataPoint(serverPos, serverTime);

                EBUS_EVENT_ID(GetEntityId(), TransformBus,
                    SetWorldTranslation, serverPos);

                AZ_Printf("Book", "new (-- %f --) @%d; dy %f,"
                    " dev %f, speed %f",
                    static_cast<float>(serverPos.GetY()),
                    GetTime(),
                    static_cast<float>(diff.GetY()),
                    allowedDeviation, m_speed);
            }
        }
        else
        {
            m_history.AddDataPoint(serverPos, serverTime);
            EBUS_EVENT_ID(GetEntityId(), InterpolationBus,
                SetWorldTranslation, serverPos, serverTime);
        }
    }
}
