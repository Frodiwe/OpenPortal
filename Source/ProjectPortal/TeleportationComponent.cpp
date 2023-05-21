#include "TeleportationComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tool.h"


UTeleportationComponent::UTeleportationComponent()
{ }

FPlane UTeleportationComponent::GetOwnerPlane() const
{
  // should be forward vector
  return FPlane{GetOwner()->GetActorLocation(), GetOwner()->GetActorUpVector()};
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

bool UTeleportationComponent::IsPointCrossingPlane(const FVector& Point, const FPlane& PortalPlane)
{
  return DoesIntersect(LastPosition, Point, PortalPlane) && !IsInFront(Point, PortalPlane) && LastInFront;
}

bool UTeleportationComponent::HasCrossedSinceLastTracked(const FVector &CurrentLocation)
{
  return IsPointCrossingPlane(CurrentLocation, GetOwnerPlane());
}

void UTeleportationComponent::Teleport(AActor* ActorToTeleport, AActor* TeleportationTarget)
{
  if (ActorToTeleport == nullptr || TeleportationTarget == nullptr)
  {
    return;
  }

  FVector SavedVelocity = FVector::ZeroVector;
  ACharacter* Character = Cast<ACharacter>(ActorToTeleport);

  if (Character != nullptr)
  {
    SavedVelocity = Character->GetCharacterMovement()->Velocity;
  }

  auto HitResult = FHitResult{};
  ActorToTeleport->SetActorLocation(
    UTool::ConvertLocationToActorSpace(ActorToTeleport->GetActorLocation(), GetOwner(), TeleportationTarget),
    false,
    &HitResult,
    ETeleportType::TeleportPhysics
  );
  ActorToTeleport->SetActorRotation(UTool::ConvertRotationToActorSpace(ActorToTeleport->GetActorRotation(), GetOwner(), TeleportationTarget));

  if (Character == nullptr)
  {
    return;
  }

  if (auto Controller = Character->GetController(); Controller != nullptr)
  {
    Controller->SetControlRotation(UTool::ConvertRotationToActorSpace(Controller->GetControlRotation(), GetOwner(), TeleportationTarget));
  }

  Character->GetCharacterMovement()->Velocity =
      (SavedVelocity | GetOwner()->GetActorForwardVector()) * TeleportationTarget->GetActorForwardVector()
    + (SavedVelocity | GetOwner()->GetActorRightVector()) * TeleportationTarget->GetActorRightVector()
    + (SavedVelocity | GetOwner()->GetActorUpVector()) * TeleportationTarget->GetActorUpVector();
}

void UTeleportationComponent::UpdateTracking(AActor* TrackedActor)
{
  LastInFront = IsInFront(TrackedActor->GetActorLocation(), GetOwnerPlane());
  LastPosition = TrackedActor->GetActorLocation();
}
