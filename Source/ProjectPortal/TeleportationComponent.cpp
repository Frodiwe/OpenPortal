#include "TeleportationComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tool.h"


UTeleportationComponent::UTeleportationComponent()
{ }

FPlane UTeleportationComponent::GetOwnerPlane() const
{
  return FPlane{GetOwner()->GetActorLocation(), -GetOwner()->GetActorForwardVector()};
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

void UTeleportationComponent::Teleport(AActor* Subject, AActor* TeleportationTarget)
{
  auto HitResult = FHitResult{};
  Subject->SetActorLocation(
    UTool::ConvertLocationToActorSpace(Subject->GetActorLocation(), GetOwner(), TeleportationTarget),
    false,
    &HitResult,
    ETeleportType::TeleportPhysics
  );
  Subject->SetActorRotation(UTool::ConvertRotationToActorSpace(Subject->GetActorRotation(), GetOwner(), TeleportationTarget));
}

void UTeleportationComponent::Teleport(ACharacter* Subject, AActor* TeleportationTarget)
{
  FVector SavedVelocity = Subject->GetCharacterMovement()->Velocity;

  Teleport(Cast<AActor>(Subject), TeleportationTarget);

  if (Subject == nullptr)
  {
    return;
  }

  if (auto Controller = Subject->GetController(); Controller != nullptr)
  {
    Controller->SetControlRotation(UTool::ConvertRotationToActorSpace(Controller->GetControlRotation(), GetOwner(), TeleportationTarget));
  }

  Subject->GetCharacterMovement()->Velocity =
      (SavedVelocity | GetOwner()->GetActorForwardVector()) * TeleportationTarget->GetActorForwardVector()
    + (SavedVelocity | GetOwner()->GetActorRightVector()) * TeleportationTarget->GetActorRightVector()
    + (SavedVelocity | GetOwner()->GetActorUpVector()) * TeleportationTarget->GetActorUpVector();
}

void UTeleportationComponent::UpdateTracking(AActor* TrackedActor)
{
  LastInFront = IsInFront(TrackedActor->GetActorLocation(), GetOwnerPlane());
  LastPosition = TrackedActor->GetActorLocation();
}
