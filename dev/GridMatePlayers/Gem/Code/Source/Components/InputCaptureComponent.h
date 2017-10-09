#pragma once
#include <AzCore/Component/Component.h>
#include <AzFramework/Input/Events/InputChannelEventListener.h>

namespace GridMatePlayers
{
    class InputCaptureComponent
        : public AZ::Component
        , public AzFramework::InputChannelEventListener
    {
    public:
        AZ_COMPONENT(InputCaptureComponent,
            "{73009DB8-503A-4B60-95C1-B25B07CC5481}");

        static void Reflect(AZ::ReflectContext* context);

    protected:
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // AzFramework::InputChannelEventListener interface
        bool OnInputChannelEventFiltered(
            const AzFramework::InputChannel& inputChannel)
            override;

        bool OnKeyboardEvent(
            const AzFramework::InputChannel& inputChannel);

        void CheckAndUpdateForward(bool pressed);
        void CheckAndUpdateFire(bool pressed);

    private:
        bool m_isForwardPressed = false;
        bool m_isFiring = false;

        AZ::EntityId GetParent();
    };
}