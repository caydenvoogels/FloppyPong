#include "KineticPlayerControllerBase.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

#if PLATFORM_WINDOWS
#include "Framework/Application/SlateApplication.h"
#include "Windows/WindowsApplication.h"
#include "Windows/WindowsHWrapper.h"
#endif

AKineticPlayerControllerBase::AKineticPlayerControllerBase()
{
	static ConstructorHelpers::FClassFinder<AActor> GameCameraBlueprint(TEXT("/Game/Controller/BP_GameCamera"));
	if (GameCameraBlueprint.Succeeded())
	{
		GameCameraClass = GameCameraBlueprint.Class;
	}
}

void AKineticPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("KineticPlayerControllerBase::BeginPlay reached."));

	// Both paddles are controlled inside the play window; the shared OS cursor
	// must not be visible or become the control surface.
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	SetupRawMouseInput();

	if (!GameCameraClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("KineticPlayerControllerBase could not find BP_GameCamera class."));
		return;
	}

	AActor* GameCamera = UGameplayStatics::GetActorOfClass(this, GameCameraClass);
	if (!GameCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("KineticPlayerControllerBase found no BP_GameCamera actor in the level."));
		return;
	}

	SetViewTargetWithBlend(GameCamera, CameraBlendTime, CameraBlendFunction, CameraBlendExp);
}

void AKineticPlayerControllerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Blueprint subclasses can replace the BeginPlay event without calling
	// the native parent. Retry here so input setup is never skipped and so the
	// PIE window can finish creating before registration is attempted.
	SetupRawMouseInput();
}

void AKineticPlayerControllerBase::EnsureRawMouseInput()
{
	SetupRawMouseInput();
}

#if PLATFORM_WINDOWS
void AKineticPlayerControllerBase::SetupRawMouseInput()
{
	if (RawInputWindowHandle || !FSlateApplication::IsInitialized())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Raw mouse setup: attempting initialization."));
	FWindowsApplication* WindowsApplication = static_cast<FWindowsApplication*>(FSlateApplication::Get().GetPlatformApplication().Get());
	if (!WindowsApplication)
	{
		UE_LOG(LogTemp, Error, TEXT("Raw mouse setup: platform application is not FWindowsApplication."));
		return;
	}

	WindowsApplication->AddMessageHandler(*this);
	TSharedPtr<SWindow> InputWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (!InputWindow.IsValid() && GEngine && GEngine->GameViewport)
	{
		InputWindow = GEngine->GameViewport->GetWindow();
	}
	if (!InputWindow.IsValid() || !InputWindow->GetNativeWindow().IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Raw mouse setup: native window is not ready; will retry."));
		return;
	}

	RAWINPUTDEVICE MouseDevice{};
	MouseDevice.usUsagePage = 0x01;
	MouseDevice.usUsage = 0x02;
	MouseDevice.dwFlags = RIDEV_INPUTSINK;
	MouseDevice.hwndTarget = static_cast<HWND>(InputWindow->GetNativeWindow()->GetOSWindowHandle());
	if (!RegisterRawInputDevices(&MouseDevice, 1, sizeof(MouseDevice)))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to register raw mouse input: %lu"), GetLastError());
		return;
	}

	RawInputWindowHandle = MouseDevice.hwndTarget;
	UE_LOG(LogTemp, Warning, TEXT("Raw mouse input registered. Move each mouse to assign Mouse 1 and Mouse 2."));
}
#else
void AKineticPlayerControllerBase::SetupRawMouseInput() {}
#endif

void AKineticPlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if PLATFORM_WINDOWS
	if (RawInputWindowHandle)
	{
		RAWINPUTDEVICE MouseDevice{};
		MouseDevice.usUsagePage = 0x01;
		MouseDevice.usUsage = 0x02;
		MouseDevice.dwFlags = RIDEV_REMOVE;
		MouseDevice.hwndTarget = nullptr;
		RegisterRawInputDevices(&MouseDevice, 1, sizeof(MouseDevice));
		RawInputWindowHandle = nullptr;
	}
	if (FSlateApplication::IsInitialized())
	{
		if (FWindowsApplication* WindowsApplication = static_cast<FWindowsApplication*>(FSlateApplication::Get().GetPlatformApplication().Get()))
		{
			WindowsApplication->RemoveMessageHandler(*this);
		}
	}
#endif
	Super::EndPlay(EndPlayReason);
}

#if PLATFORM_WINDOWS
bool AKineticPlayerControllerBase::ProcessMessage(HWND HWnd, uint32 Message, WPARAM WParam, LPARAM LParam, int32& OutResult)
{
	if (Message != WM_INPUT)
	{
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("Raw mouse WM_INPUT received. HWnd=%p"), HWnd);

	UINT DataSize = 0;
	if (GetRawInputData(reinterpret_cast<HRAWINPUT>(LParam), RID_INPUT, nullptr, &DataSize, sizeof(RAWINPUTHEADER)) != 0 || DataSize == 0)
	{
		return false;
	}

	TArray<uint8> RawData;
	RawData.SetNumUninitialized(DataSize);
	if (GetRawInputData(reinterpret_cast<HRAWINPUT>(LParam), RID_INPUT, RawData.GetData(), &DataSize, sizeof(RAWINPUTHEADER)) == static_cast<UINT>(-1))
	{
		return false;
	}

	const RAWINPUT* RawInput = reinterpret_cast<const RAWINPUT*>(RawData.GetData());
	if (RawInput->header.dwType != RIM_TYPEMOUSE)
	{
		return false;
	}

	void* Device = RawInput->header.hDevice;
	if (!PrimaryMouseDevice)
	{
		PrimaryMouseDevice = Device;
		UE_LOG(LogTemp, Log, TEXT("Raw Mouse 1 detected and assigned. Device=%p"), Device);
	}
	if (Device == PrimaryMouseDevice)
	{
		PrimaryMousePosition.X = FMath::Clamp(PrimaryMousePosition.X - (RawInput->data.mouse.lLastX * 2.5f), 40.0f, 1450.0f);
		PrimaryMousePosition.Y = FMath::Clamp(PrimaryMousePosition.Y - (RawInput->data.mouse.lLastY * 2.5f), -850.0f, 850.0f);
	}
	else if (Device && Device != PrimaryMouseDevice)
	{
		if (!SecondMouseDevice)
		{
			SecondMouseDevice = Device;
			bSecondMouseActive = true;
			UE_LOG(LogTemp, Log, TEXT("Raw Mouse 2 detected and assigned. Device=%p. Blue paddle is now using Mouse 2."), Device);
		}

		if (Device == SecondMouseDevice)
		{
			SecondMousePosition.X = FMath::Clamp(SecondMousePosition.X - (RawInput->data.mouse.lLastX * 2.5f), -1450.0f, -40.0f);
			SecondMousePosition.Y = FMath::Clamp(SecondMousePosition.Y - (RawInput->data.mouse.lLastY * 2.5f), -850.0f, 850.0f);
		}
	}

	const double CurrentTime = FPlatformTime::Seconds();
	if (CurrentTime - LastMouseDebugTime >= 1.0)
	{
		LastMouseDebugTime = CurrentTime;
		UE_LOG(LogTemp, Log, TEXT("Raw mouse status: Mouse1=%p Pos=(%.1f, %.1f), Mouse2=%p Pos=(%.1f, %.1f), Mouse2Active=%s"),
			PrimaryMouseDevice,
			PrimaryMousePosition.X,
			PrimaryMousePosition.Y,
			SecondMouseDevice,
			SecondMousePosition.X,
			SecondMousePosition.Y,
			bSecondMouseActive ? TEXT("true") : TEXT("false"));
	}

	// Consume raw mouse messages so Slate and legacy input cannot route them
	// through the shared OS cursor as well.
	OutResult = 0;
	return true;
}
#endif
