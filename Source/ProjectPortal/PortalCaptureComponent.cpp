#include "PortalCaptureComponent.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Character.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Tool.h"


UPortalCaptureComponent::UPortalCaptureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
  
  // Requered to have TG_PostUpdateWork tick group to remove capture render lag
  PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

  View = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
  SetupCapture(View);
}

void UPortalCaptureComponent::SetupCapture(USceneCaptureComponent2D* view)
{
  view->bCaptureEveryFrame = false;
  view->bCaptureOnMovement = false;

  // Should be enabled for post process to work correctly (when bCaptureEveryFrame is false)
  // Pips on the Internet said that it seems like engine issue
  view->bAlwaysPersistRenderingState = true;

  // Force bigger LODs for faster computations
  view->LODDistanceFactor = 3;
  
  view->TextureTarget = nullptr;
  view->bEnableClipPlane = true;
  view->bUseCustomProjectionMatrix = true;
  
  view->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;

  FPostProcessSettings CaptureSettings;

  CaptureSettings.bOverride_AmbientOcclusionQuality = true;
  CaptureSettings.bOverride_MotionBlurAmount = true;
  CaptureSettings.bOverride_SceneFringeIntensity = true;
  CaptureSettings.bOverride_ScreenSpaceReflectionQuality = true;

  CaptureSettings.AmbientOcclusionQuality = 0.0f;
  CaptureSettings.MotionBlurAmount = 0.0f;
  CaptureSettings.SceneFringeIntensity = 0.0f;
  CaptureSettings.ScreenSpaceReflectionQuality = 0.0f;

  view->PostProcessSettings = CaptureSettings;
}

void UPortalCaptureComponent::CutCurrentFrame()
{
  UE_LOG(LogTemp, Warning, TEXT("Cutting this frame"));
  
  View->bCameraCutThisFrame = true;
}

void UPortalCaptureComponent::UpdateView(APlayerCameraManager* CameraManager, AActor* ViewTarget)
{
  if (ViewTarget == nullptr)
  {
    return;
  }

  View->SetWorldLocation(UTool::ConvertLocationToActorSpace(CameraManager->GetCameraLocation(), GetOwner(), ViewTarget));
  View->SetWorldRotation(UTool::ConvertRotationToActorSpace(CameraManager->GetCameraRotation(), GetOwner(), ViewTarget));

  View->ClipPlaneNormal = ViewTarget->GetActorForwardVector();
  View->ClipPlaneBase = ViewTarget->GetActorLocation() + (View->ClipPlaneNormal * 0.5f);
  View->CustomProjectionMatrix = UTool::GetCameraProjectionMatrix(GetWorld()->GetFirstPlayerController()->GetLocalPlayer());
  
  View->CaptureScene();
}

void UPortalCaptureComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UPortalCaptureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  UpdateView(GetPlayerCameraManager(), Target);
}

APlayerCameraManager* UPortalCaptureComponent::GetPlayerCameraManager()
{
  return GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
}

void UPortalCaptureComponent::SetRTT(UTextureRenderTarget2D* Texture)
{
  View->TextureTarget = Texture;
}
