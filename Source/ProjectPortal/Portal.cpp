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
#include "ReplicationComponent.h"
#include "PortalCaptureComponent.h"

APortal::APortal()
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PostPhysics;

  PortalRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRoot"));
  SetRootComponent(PortalRoot);

  PortalArea = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalArea"));
  PortalArea->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

  Replication = CreateDefaultSubobject<UReplicationComponent>(TEXT("Replication"));
  PortalCapture = CreateDefaultSubobject<UPortalCaptureComponent>(TEXT("Capture"));
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

  PortalCapture->Target = Target;

  PortalArea->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnBeginOverlap);
  PortalArea->OnComponentEndOverlap.AddDynamic(this, &APortal::OnEndOverlap);
}

void APortal::OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
  if (auto Teleportation = OtherActor->GetComponentByClass<UTeleportationComponent>())
  {
    Teleportation->SetPortal(this);
    Teleportation->OnActorTeleported.AddLambda([this](const auto& Actor) {
      Replication->Swap(Actor);
    });
  }

  if (OtherActor->IsA<ACharacter>())
  {
    return;
  }

  Replication->Add(OtherActor, Target);
}

void APortal::OnEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
  if (auto TeleportationComponent = OtherActor->GetComponentByClass<UTeleportationComponent>())
  {
    TeleportationComponent->SetPortal(nullptr);
  }

  if (OtherActor->IsA<ACharacter>())
  {
    return;
  }

  Replication->Remove(OtherActor);
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