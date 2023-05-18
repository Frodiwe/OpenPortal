#include "Portal.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/BoxComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PostUpdateWork;

  PortalRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRoot"));
  SetRootComponent(PortalRoot);

  Gate = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalGateMesh"));
  Gate->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

  View = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("PortalSceneCapture"));
  View->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

  Area = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalArea"));
  Area->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

  SetupView();
}

void APortal::SetupView()
{
  View->bCaptureEveryFrame = false;
  View->bCaptureOnMovement = false;
  View->LODDistanceFactor = 3; //Force bigger LODs for faster computations
  
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

  UpdateView(GetPlayerCameraManager());

  auto Character = GetWorld()->GetFirstPlayerController()->GetCharacter();

  if (Character == nullptr)
  {
    return;
  }

  if (
    // UKismetMathLibrary::IsPointInBox(Character->GetActorLocation(), Area->GetComponentLocation(), Area->GetScaledBoxExtent())
    IsPointCrossingPortal(Character->GetActorLocation(), GetActorLocation(), GetActorUpVector())
  )
  {
    auto PortalPlane = FPlane{GetActorLocation(), GetActorUpVector()};

    UE_LOG(LogTemp, Warning, TEXT("does intersect: %s"), DoesIntersect(LastPosition, Character->GetActorLocation(), PortalPlane) ? *FString{"true"} : *FString{"false"});
    UE_LOG(LogTemp, Warning, TEXT("\tlast position: %s"), *LastPosition.ToString());
    UE_LOG(LogTemp, Warning, TEXT("\tcurr position: %s"), *Character->GetActorLocation().ToString());
    UE_LOG(LogTemp, Warning, TEXT("\tportal plane: %s"), *PortalPlane.ToString());
    UE_LOG(LogTemp, Warning, TEXT("is in front: %s"), IsInFront(Character->GetActorLocation(), PortalPlane) ? *FString{"true"} : *FString{"false"});
    UE_LOG(LogTemp, Warning, TEXT("last in front: %s"), LastInFront ? *FString{"true"} : *FString{"false"});

    TeleportActor(Character);
  }

  LastInFront = IsInFront(Character->GetActorLocation(), FPlane{GetActorLocation(), GetActorUpVector()});
  LastPosition = Character->GetActorLocation();
}

void APortal::UpdateView(APlayerCameraManager* CameraManager)
{
  if (Target == nullptr)
  {
    return;
  }

  View->SetWorldLocation(ConvertLocationToActorSpace(CameraManager->GetCameraLocation(), Gate, Target));
  View->SetWorldRotation(ConvertRotationToActorSpace(CameraManager->GetCameraRotation(), Gate, Target));

  View->ClipPlaneNormal = -Target->GetActorUpVector();
  View->ClipPlaneBase = Target->GetActorLocation() + (View->ClipPlaneNormal * 0.5f);
  View->CustomProjectionMatrix = GetCameraProjectionMatrix();
  
  View->CaptureScene();
}

FVector APortal::ConvertLocationToActorSpace(const FVector& Location, UStaticMeshComponent* SourceComponent, AActor* TargetActor) const
{
  if (SourceComponent == nullptr || TargetActor == nullptr)
  {
    return FVector::ZeroVector;
  }

  FVector Direction = Location - SourceComponent->GetComponentLocation();

  return TargetActor->GetActorLocation()
    + FVector::DotProduct(Direction, SourceComponent->GetForwardVector()) * TargetActor->GetActorForwardVector()
    + FVector::DotProduct(Direction, SourceComponent->GetRightVector()) * TargetActor->GetActorRightVector()
    + FVector::DotProduct(Direction, SourceComponent->GetUpVector()) * TargetActor->GetActorUpVector();
}

FVector APortal::ConvertLocationToActorSpace(const FVector& Location, AActor* SourceActor, AActor* TargetActor) const
{
  if (SourceActor == nullptr || TargetActor == nullptr)
  {
    return FVector::ZeroVector;
  }

  FVector Direction = Location - SourceActor->GetActorLocation();

  return TargetActor->GetActorLocation()
    + FVector::DotProduct(Direction, SourceActor->GetActorForwardVector()) * TargetActor->GetActorForwardVector()
    + FVector::DotProduct(Direction, SourceActor->GetActorRightVector()) * TargetActor->GetActorRightVector()
    + FVector::DotProduct(Direction, SourceActor->GetActorUpVector()) * TargetActor->GetActorUpVector();
}

FRotator APortal::ConvertRotationToActorSpace(const FRotator& Rotation, UStaticMeshComponent* SourceComponent, AActor* TargetActor) const
{
  if (SourceComponent == nullptr || TargetActor == nullptr)
  {
    return FRotator::ZeroRotator;
  }

  return (
    TargetActor->GetActorTransform().GetRotation()
    * SourceComponent->GetComponentTransform().GetRotation().Inverse()
    * FQuat{Rotation}
  ).Rotator();
}

FRotator APortal::ConvertRotationToActorSpace(const FRotator& Rotation, AActor* SourceActor, AActor* TargetActor) const
{
  if (SourceActor == nullptr || TargetActor == nullptr)
  {
    return FRotator::ZeroRotator;
  }

  return (
    TargetActor->GetActorTransform().GetRotation()
    * SourceActor->GetActorTransform().GetRotation().Inverse()
    * FQuat{Rotation}
  ).Rotator();
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

bool APortal::IsPointCrossingPortal(const FVector& Point, const FVector& PortalLocation, const FVector& PortalNormal)
{
  const auto PortalPlane = FPlane{PortalLocation, PortalNormal};

  return DoesIntersect(LastPosition, Point, PortalPlane) && !IsInFront(Point, PortalPlane) && LastInFront;
}

bool APortal::IsInFront(const FVector& Point, const FPlane& PortalPlane) const
{
  return PortalPlane.PlaneDot(Point) >= 0;
}

bool APortal::DoesIntersect(const FVector& Start, const FVector& End, const FPlane& PortalPlane) const
{
  auto IntersectionPoint = FVector{};
  return FMath::SegmentPlaneIntersection(Start, End, PortalPlane, IntersectionPoint);
}

void APortal::TeleportActor(AActor* ActorToTeleport)
{
  if (ActorToTeleport == nullptr || Target == nullptr)
  {
    return;
  }

  FVector SavedVelocity = FVector::ZeroVector;
  ACharacter* Character = nullptr;

  if (ActorToTeleport->IsA(ACharacter::StaticClass()))
  {
    Character = Cast<ACharacter>(ActorToTeleport);
    SavedVelocity = Character->GetCharacterMovement()->Velocity;
  }

  auto HitResult = FHitResult{};
  auto NewLocation = ConvertLocationToActorSpace(ActorToTeleport->GetActorLocation(), this, Target);
  
  ActorToTeleport->SetActorLocation(
    NewLocation,
    false,
    &HitResult,
    ETeleportType::TeleportPhysics
  );
  ActorToTeleport->SetActorRotation(ConvertRotationToActorSpace(ActorToTeleport->GetActorRotation(), this, Target));

  if (Character != nullptr)
  {
    auto Controller = Character->GetController();

    if (Controller != nullptr)
    {
      Controller->SetControlRotation(ConvertRotationToActorSpace(Controller->GetControlRotation(), this, Target));
    }

    FVector Dots{
      FVector::DotProduct(SavedVelocity, GetActorForwardVector()),
      FVector::DotProduct(SavedVelocity, GetActorRightVector()),
      FVector::DotProduct(SavedVelocity, GetActorUpVector())
    };

    Character->GetCharacterMovement()->Velocity = Dots.X * Target->GetActorForwardVector()
      + Dots.Y * Target->GetActorRightVector()
      + Dots.Z * Target->GetActorUpVector();
  }

  LastPosition = NewLocation;
}