#include "InputComponent.h"
#include "WindowsCursor.h"

void UInputComponent::ProcessInput(float DeltaTime)
{
    if (PressedKeys.Contains(EKeys::RightMouseButton))
    {
        if (PressedKeys.Contains(EKeys::W))
        {
            KeyBindDelegate[FString("EditorW")].Broadcast(DeltaTime);
        }
        if (PressedKeys.Contains(EKeys::A))
        {
            KeyBindDelegate[FString("EditorA")].Broadcast(DeltaTime);
        }
        if (PressedKeys.Contains(EKeys::S))
        {
            KeyBindDelegate[FString("EditorS")].Broadcast(DeltaTime);
        }
        if (PressedKeys.Contains(EKeys::D))
        {
            KeyBindDelegate[FString("EditorD")].Broadcast(DeltaTime);
        }
        if (PressedKeys.Contains(EKeys::Q))
        {
            KeyBindDelegate[FString("EditorQ")].Broadcast(DeltaTime);
        }
        if (PressedKeys.Contains(EKeys::E))
        {
            KeyBindDelegate[FString("EditorE")].Broadcast(DeltaTime);
        }
    }
}

void UInputComponent::SetPossess()
{
    BindInputDelegate();

    //TODO: Possess일때 기존에 있던거 다시 넣어줘야할수도
}

void UInputComponent::BindInputDelegate()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    BindKeyDownDelegateHandles.Add(Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
        {
            InputKey(InKeyEvent);
        }));

    BindKeyUpDelegateHandles.Add(Handler->OnKeyUpDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
        {
            InputKey(InKeyEvent);
        }));
    BindMouseMoveDelegateHandles.Add(Handler->OnMouseMoveDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
        {
            const FVector2D& Delta = InMouseEvent.GetCursorDelta();

            if (PressedKeys.Contains(EKeys::RightMouseButton))
            {
                if (MouseBindDelegate.Contains("Turn"))
                {
                    MouseBindDelegate["Turn"].Broadcast(Delta.X);
                }

                if (MouseBindDelegate.Contains("LookUp"))
                {
                    MouseBindDelegate["LookUp"].Broadcast(Delta.Y);
                }
            }
        }));

    BindMouseDownDelegateHandles.Add(Handler->OnMouseDownDelegate.AddLambda([this](const FPointerEvent& InMouseEvent) {
        InputMouse(InMouseEvent);
        }));
    BindMouseUpDelegateHandles.Add(Handler->OnMouseUpDelegate.AddLambda([this](const FPointerEvent& InMouseEvent) {
        InputMouse(InMouseEvent);
        }));
}

void UInputComponent::UnPossess()
{
    ClearBindDelegate();
}

void UInputComponent::ClearBindDelegate()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    for (FDelegateHandle DelegateHandle : BindKeyDownDelegateHandles)
    {
        Handler->OnKeyDownDelegate.Remove(DelegateHandle);
    }

    for (FDelegateHandle DelegateHandle : BindKeyUpDelegateHandles)
    {
        Handler->OnKeyUpDelegate.Remove(DelegateHandle);
    }

    for (FDelegateHandle DelegateHandle : BindMouseMoveDelegateHandles)
    {
        Handler->OnMouseMoveDelegate.Remove(DelegateHandle);
    }
    for (FDelegateHandle DelegateHandle : BindMouseDownDelegateHandles)
    {
        Handler->OnMouseDownDelegate.Remove(DelegateHandle);
    }
    for (FDelegateHandle DelegateHandle : BindMouseUpDelegateHandles)
    {
        Handler->OnMouseUpDelegate.Remove(DelegateHandle);
    }

    BindKeyDownDelegateHandles.Empty();
    BindKeyUpDelegateHandles.Empty();
    BindMouseMoveDelegateHandles.Empty();
    BindMouseDownDelegateHandles.Empty();
    BindMouseUpDelegateHandles.Empty();
}

void UInputComponent::InputKey(const FKeyEvent& InKeyEvent)
{
    // 일반적인 단일 키 이벤트
    switch (InKeyEvent.GetCharacter())
    {
    case 'W':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::W);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::W);
        }
        break;
    }
    case 'A':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::A);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::A);
        }
        break;
    }
    case 'S':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::S);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::S);
        }
        break;
    }
    case 'D':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::D);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::D);
        }
        break;
    }
    case 'Q':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::Q);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::Q);
        }
        break;
    }
    case 'E':
    {
        if (InKeyEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::E);
        }
        else if (InKeyEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::E);
        }
        break;
    }
    default:
        break;
    }
}

void UInputComponent::InputMouse(const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        if (InMouseEvent.GetInputEvent() == IE_Pressed)
        {
            PressedKeys.Add(EKeys::RightMouseButton);
            FWindowsCursor::SetShowMouseCursor(false);
            MousePinPosition = InMouseEvent.GetScreenSpacePosition();
        }
        else if (InMouseEvent.GetInputEvent() == IE_Released)
        {
            PressedKeys.Remove(EKeys::RightMouseButton);
            FWindowsCursor::SetShowMouseCursor(true);
            FWindowsCursor::SetPosition(
                static_cast<int32>(MousePinPosition.X),
                static_cast<int32>(MousePinPosition.Y)
            );
        }
    }
}

void UInputComponent::BindAction(const FString& Key, const std::function<void(float)>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }

    KeyBindDelegate[Key].AddLambda([this, Callback](float DeltaTime)
        {
            Callback(DeltaTime);
        });
}

void UInputComponent::BindAxis(const FString& Axis, const std::function<void(float)>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }

    MouseBindDelegate[Axis].AddLambda([this, Callback](float DeltaTime)
        {
            Callback(DeltaTime);
        });
}
