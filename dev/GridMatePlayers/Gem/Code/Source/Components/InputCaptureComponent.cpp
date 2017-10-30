#include "StdAfx.h"
#include "InputCaptureComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <GridMatePlayers/PlayerControlsBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMatePlayers;

void InputCaptureComponent::Reflect(ReflectContext* context)
{
    if (auto sc = azrtti_cast<SerializeContext *>(context))
    {
        sc->Class<InputCaptureComponent>()
            ->Version(1);

        if (EditContext* ec = sc->GetEditContext())
        {
            ec->Class<InputCaptureComponent>("Input Capture",
                "Captures player input")
                ->ClassElement(Edit::ClassElements::EditorData,
                    "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMate Players")
                ->Attribute(
                    Edit::Attributes::AppearsInAddComponentMenu,
                    AZ_CRC("Game"));
        }
    }
}

void InputCaptureComponent::Activate()
{
    InputChannelEventListener::Connect();
}

void InputCaptureComponent::Deactivate()
{
    InputChannelEventListener::Disconnect();
}

bool InputCaptureComponent::OnInputChannelEventFiltered(
    const InputChannel& inputChannel)
{
    const InputDeviceId deviceId =
        inputChannel.GetInputDevice().GetInputDeviceId();
    if (deviceId == InputDeviceKeyboard::Id)
        return OnKeyboardEvent(inputChannel);

    return false; // key not consumed
}

bool InputCaptureComponent::OnKeyboardEvent(
    const InputChannel& inputChannel)
{
    const InputChannelId inputType = inputChannel.
        GetInputChannelId();
    if (inputType == InputDeviceKeyboard::Key::AlphanumericW)
    {
        const bool pressed = !!inputChannel.GetValue();
        CheckAndUpdateForward(pressed);

        return true; // key consumed
    }

    if (inputType == InputDeviceKeyboard::Key::EditSpace)
    {
        const bool pressed = !!inputChannel.GetValue();
        CheckAndUpdateFire(pressed);

        return true; // key consumed
    }

    return false; // key not consumed
}

void InputCaptureComponent::CheckAndUpdateForward(
    bool pressed)
{
    if (m_isForwardPressed == pressed) return;

    if (pressed)
        EBUS_EVENT_ID(
            GetEntityId(),
            GridMatePlayers::PlayerControlsBus,
            ForwardKeyPressed);
    else
        EBUS_EVENT_ID(
            GetEntityId(),
            GridMatePlayers::PlayerControlsBus,
            ForwardKeyReleased);

    m_isForwardPressed = pressed;
}

void InputCaptureComponent::CheckAndUpdateFire(bool pressed)
{
    if (m_isFiring == pressed) return;

    if (!pressed)
        EBUS_EVENT_ID(
            GetEntityId(),
            GridMatePlayers::PlayerControlsBus,
            FireKeyReleased);

    m_isFiring = pressed;
}
