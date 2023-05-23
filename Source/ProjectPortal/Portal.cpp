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
#include "PortalCaptureComponent.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;

  // Requered to have TG_PostPhysics (essentialy not TG_PostUpdateWork) tick group to remove teleportation lag
  PrimaryActorTick.TickGroup = TG_PostPhysics;

  PortalRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRoot"));
  SetRootComponent(PortalRoot);

  Teleportation = CreateDefaultSubobject<UTeleportationComponent>(TEXT("Teleportation"));
  PortalCapture = CreateDefaultSubobject<UPortalCaptureComponent>(TEXT("Capture"));
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

  PortalCapture->Target = Target;
}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

  auto Character = GetWorld()->GetFirstPlayerController()->GetCharacter();

  if (Character == nullptr)
  {
    return;
  }

  if (Teleportation->HasCrossedSinceLastTracked(Character->GetActorLocation()))
  {
    // CutCurrentFrame();
    Teleportation->Teleport(Character, Target);
  }

  Teleportation->UpdateTracking(Character);
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