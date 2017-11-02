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

        if (EditContext* ec = sc->GetEditContext())
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
    const EntityId self = GetEntityId();
    LocalPredictionRequestBus::Handler::BusConnect(self);
    TransformNotificationBus::Handler::BusConnect(self);

    if (!NetQuery::IsEntityAuthoritative(self))
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
    if (Chunk* chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        const Vector3 newLocal = world.GetTranslation();
        if (chunk->IsMaster())
        {
            const float diff = newLocal.GetDistance(
                chunk->m_serverCheckpoint.Get().m_vector);
            if (diff < 0.01f) return;

            AZ_Printf("Book", "server (-- %f --) @ %d",
                static_cast<float>(newLocal.GetY()),
                GetLocalTime());

            chunk->m_serverCheckpoint.Set(
                { newLocal, GetLocalTime() });
        }
        else if (IsLocallyControlled())
        {
            m_history.AddDataPoint(newLocal, GetLocalTime());
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
}

Vector3 LocalPredictionComponent::GetPosition() const
{
    Vector3 v = AZ::Vector3::CreateZero();
    EBUS_EVENT_ID_RESULT(v, GetEntityId(), AZ::TransformBus,
        GetWorldTranslation);
    return v;
}

AZ::u32 LocalPredictionComponent::GetLocalTime() const
{
    AZ::u32 t = 0;
    EBUS_EVENT_RESULT(t, NetworkTimeRequestBus, GetLocalTime);
    return t;
}

bool LocalPredictionComponent::IsLocallyControlled() const
{
    bool localClient = false;
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
        const Vector3& serverPos = value.m_vector;
        const AZ::u32 serverTime = value.m_time;

        if (IsLocallyControlled())
        {
            if (m_history.HasHistory())
            {
                const Vector3 backThen =
                    m_history.GetPositionAt(serverTime);
                const float diff = backThen.GetDistance(serverPos);
                const float allowedDeviation =
                    (GetLocalTime() - serverTime) * 0.001f *
                    AZStd::GetMax(m_speed, .5f);

                if (diff > allowedDeviation)
                {
                    m_history.DeleteAfter(serverTime);
                    m_history.AddDataPoint(serverPos, serverTime);

                    EBUS_EVENT_ID(GetEntityId(), TransformBus,
                        SetWorldTranslation, serverPos);

                    AZ_Printf("Book", "new (-- %f --) @%d; dy %f,"
                        " dev %f, speed %f",
                        static_cast<float>(serverPos.GetY()),
                        GetLocalTime(),
                        diff,
                        allowedDeviation, m_speed);
                }
            }
            else
            {
                EBUS_EVENT_ID(GetEntityId(), TransformBus,
                    SetWorldTranslation, serverPos);
            }
        }
        else
        {
            AZ_Printf("Book", "interpolate (-- %f --) @%d",
                static_cast<float>(serverPos.GetY()),
                serverTime);

            EBUS_EVENT_ID(GetEntityId(), InterpolationBus,
                SetWorldTranslation, serverPos, serverTime);
        }
    }
}
