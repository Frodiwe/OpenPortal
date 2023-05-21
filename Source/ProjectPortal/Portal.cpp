#include "Portal.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/BoxComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Tool.h"
#include "TeleportationComponent.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PostUpdateWork;

  PortalRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRoot"));
  SetRootComponent(PortalRoot);

  View = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
  View->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

  Teleportation = CreateDefaultSubobject<UTeleportationComponent>(TEXT("Teleportation"));

  SetupView();
}

void APortal::SetupView()
{
  View->bCaptureEveryFrame = false;
  View->bCaptureOnMovement = false;
  // Force bigger LODs for faster computations
  View->LODDistanceFactor = 3;
  
  View->TextureTarget = nullptr;
  View->bEnableClipPlane = true;
  View->bUseCustomProjectionMatrix = true;
  
  View->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;

  FPostProcessSettings CaptureSettings;

  CaptureSettings.bOverride_AmbientOcclusionQuality = true;
  CaptureSettings.bOverride_MotionBlurAmount = true;
  CaptureSettings.bOverride_SceneFringeIntensity = true;
  CaptureSettings.bOverride_ScreenSpaceReflectionQuality = true;

  CaptureSettings.AmbientOcclusionQuality = 0.0f;
  CaptureSettings.MotionBlurAmount = 0.0f;
  CaptureSettings.SceneFringeIntensity = 0.0f;
  CaptureSettings.ScreenSpaceReflectionQuality = 0.0f;

  View->PostProcessSettings = CaptureSettings;
}

void APortal::BeginPlay()
{
	Super::BeginPlay();
}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

  // auto Character = GetWorld()->GetFirstPlayerController()->GetCharacter();

  // if (Character == nullptr)
  // {
  //   return;
  // }

  UpdateView(GetPlayerCameraManager());

  // if (Teleportation->HasCrossedSinceLastTracked(Character->GetActorLocation()))
  // {
  //   Teleportation->Teleport(Character, Target);
  // }

  // Teleportation->UpdateTracking(Character);
}

void APortal::UpdateView(APlayerCameraManager* CameraManager)
{
  if (Target == nullptr)
  {
    return;
  }

  View->SetWorldLocation(UTool::ConvertLocationToActorSpace(CameraManager->GetCameraLocation(), this, Target));
  View->SetWorldRotation(UTool::ConvertRotationToActorSpace(CameraManager->GetCameraRotation(), this, Target));

  View->ClipPlaneNormal = Target->GetActorForwardVector();
  View->ClipPlaneBase = Target->GetActorLocation() + (View->ClipPlaneNormal * 0.5f);
  View->CustomProjectionMatrix = GetCameraProjectionMatrix();
  
  View->CaptureScene();
}

FMatrix APortal::GetCameraProjectionMatrix() const
{
  FMatrix ProjectionMatrix;
  auto player = GetWorld()->GetFirstPlayerController()->GetLocalPlayer();

  if (player != nullptr)
  {
    FSceneViewProjectionData PlayerProjectionData;
    player->GetProjectionData(player->ViewportClient->Viewport, PlayerProjectionData);

    ProjectionMatrix = PlayerProjectionData.ProjectionMatrix;
  }

  return ProjectionMatrix;
}

APlayerCameraManager* APortal::GetPlayerCameraManager()
{
  if (PlayerCameraManager == nullptr)
  {
    PlayerCameraManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager; 
  }

  return PlayerCameraManager;
}

UTextureRenderTarget2D* APortal::GeneratePortalTexture()
{
  auto CurrentSizeX = int32{1920};
  auto CurrentSizeY = int32{1080};

  if (GetWorld()->GetFirstPlayerController() != nullptr)
  {
    GetWorld()->GetFirstPlayerController()->GetViewportSize(CurrentSizeX, CurrentSizeY);
  }

  CurrentSizeX = FMath::Clamp(int32(CurrentSizeX / 1.7), 128, 1920);
  CurrentSizeY = FMath::Clamp(int32(CurrentSizeY / 1.7), 128, 1080);

  auto PortalTexture = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), *FString("PortalRenderTarget"));
  
  check(PortalTexture);

  PortalTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
  PortalTexture->Filter = TextureFilter::TF_Bilinear;
  PortalTexture->SizeX = CurrentSizeX;
  PortalTexture->SizeY = CurrentSizeY;
  PortalTexture->ClearColor = FLinearColor::Black;
  PortalTexture->TargetGamma = 2.2f;
  PortalTexture->bNeedsTwoCopies = false;
  PortalTexture->AddressX = TextureAddress::TA_Clamp;
  PortalTexture->AddressY = TextureAddress::TA_Clamp;
  PortalTexture->UpdateResource();

  return PortalTexture;
}