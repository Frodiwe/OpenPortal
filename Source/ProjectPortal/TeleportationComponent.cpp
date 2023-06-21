#include "TeleportationComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Portal.h"
#include "Tool.h"


UTeleportationComponent::UTeleportationComponent()
{
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UTeleportationComponent::SetPortal(APortal* Portal)
{
  ActivePortal = Portal;
}

void UTeleportationComponent::BeginPlay()
{
  Super::BeginPlay();

  Tracked = FTeleportationUnit{
    .Subject = GetOwner(),
    .LastPosition = GetOwner()->GetActorLocation(),
    .LastInFront = false
  };
}

void UTeleportationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (ActivePortal == nullptr)
  {
    return;
  }

  if (HasCrossedSinceLastTracked(Tracked, ActivePortal->GetPlane()))
  {
    UE_LOG(LogTemp, Warning, TEXT("character? %s"), Tracked.Subject->IsA<ACharacter>() ? TEXT("true") : TEXT("false"));
    Teleport(
      Tracked.Subject->IsA<ACharacter>() ? Cast<ACharacter>(Tracked.Subject) : Tracked.Subject,
      ActivePortal,
      ActivePortal->GetTarget()
    );
    OnActorTeleported.Broadcast(Tracked.Subject);

    return;
  }

  UpdateTracking(Tracked, ActivePortal->GetPlane());
}

bool UTeleportationComponent::IsInFront(const FVector &Point, const FPlane &PortalPlane) const
{
  return PortalPlane.PlaneDot(Point) >= 0;
}

bool UTeleportationComponent::DoesIntersect(const FVector& Start, const FVector& End, const FPlane& PortalPlane) const
{
  auto IntersectionPoint = FVector{};
  return FMath::SegmentPlaneIntersection(Start, End, PortalPlane, IntersectionPoint);
}

bool UTeleportationComponent::HasCrossedSinceLastTracked(const FTeleportationUnit& Unit, const FPlane& TeleportationPlane)
{
  return DoesIntersect(Unit.LastPosition, Unit.Subject->GetActorLocation(), TeleportationPlane)
    && !IsInFront(Unit.Subject->GetActorLocation(), TeleportationPlane)
    && Unit.LastInFront;
}

template<>
void UTeleportationComponent::Teleport(AActor* Subject, AActor* Portal, AActor* TeleportationTarget)
{  
  UTool::Teleport(Subject, UTool::ConvertLocationToActorSpace(Subject->GetActorLocation(), Portal, TeleportationTarget));
  Subject->SetActorRotation(UTool::ConvertRotationToActorSpace(Subject->GetActorRotation(), Portal, TeleportationTarget));
}

template<>
void UTeleportationComponent::Teleport(ACharacter* Subject, AActor* Portal, AActor* TeleportationTarget)
{
  FVector SavedVelocity = Subject->GetCharacterMovement()->Velocity;
  GetWorld()->GetFirstPlayerController()->PlayerCameraManager->bGameCameraCutThisFrame = true;
  UE_LOG(LogTemp, Warning, TEXT("cutting this frame? %s"), GetWorld()->GetFirstPlayerController()->PlayerCameraManager->bGameCameraCutThisFrame ? TEXT("true") : TEXT("false"));

  Teleport(Cast<AActor>(Subject), Portal, TeleportationTarget);

  if (auto Controller = Subject->GetController(); Controller != nullptr)
  {
    Controller->SetControlRotation(UTool::ConvertRotationToActorSpace(Controller->GetControlRotation(), Portal, TeleportationTarget));
  }

  Subject->GetCharacterMovement()->Velocity =
      (SavedVelocity | Portal->GetActorForwardVector()) * TeleportationTarget->GetActorForwardVector()
    + (SavedVelocity | Portal->GetActorRightVector()) * TeleportationTarget->GetActorRightVector()
    + (SavedVelocity | Portal->GetActorUpVector()) * TeleportationTarget->GetActorUpVector();
}

void UTeleportationComponent::UpdateTracking(FTeleportationUnit& Unit, const FPlane& TeleportationPlane)
{
  Unit.LastInFront = IsInFront(Unit.Subject->GetActorLocation(), TeleportationPlane);
  Unit.LastPosition = Unit.Subject->GetActorLocation();
}
