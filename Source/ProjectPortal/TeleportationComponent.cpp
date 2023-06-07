#include "TeleportationComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tool.h"


UTeleportationComponent::UTeleportationComponent()
{
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UTeleportationComponent::Track(AActor *Actor)
{
  Tracked.Add(FTeleportationUnit{
    .Subject = Actor,
    .LastPosition = FVector::ZeroVector,
    .LastInFront = false
  });
}

void UTeleportationComponent::Untrack(AActor *Actor)
{
  Tracked.RemoveAll([&](const auto& Unit) { return Unit.Subject == Actor; });
}

void UTeleportationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  for (auto& Unit : Tracked)
  {
    if (HasCrossedSinceLastTracked(Unit))
    {
      // PortalCapture->CutCurrentFrame();
      Teleport(Unit.Subject->IsA<ACharacter>() ? Cast<ACharacter>(Unit.Subject) : Unit.Subject, Target);
    }

    UpdateTracking(Unit);
  }
}

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

bool UTeleportationComponent::HasCrossedSinceLastTracked(const FTeleportationUnit& Unit)
{
  return DoesIntersect(Unit.LastPosition, Unit.Subject->GetActorLocation(), GetOwnerPlane()) && !IsInFront(Unit.Subject->GetActorLocation(), GetOwnerPlane()) && Unit.LastInFront;
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

  OnActorTeleported.Broadcast(Subject);
}

void UTeleportationComponent::Teleport(ACharacter* Subject, AActor* TeleportationTarget)
{
  FVector SavedVelocity = Subject->GetCharacterMovement()->Velocity;

  Teleport(Cast<AActor>(Subject), TeleportationTarget);

  if (auto Controller = Subject->GetController(); Controller != nullptr)
  {
    Controller->SetControlRotation(UTool::ConvertRotationToActorSpace(Controller->GetControlRotation(), GetOwner(), TeleportationTarget));
  }

  Subject->GetCharacterMovement()->Velocity =
      (SavedVelocity | GetOwner()->GetActorForwardVector()) * TeleportationTarget->GetActorForwardVector()
    + (SavedVelocity | GetOwner()->GetActorRightVector()) * TeleportationTarget->GetActorRightVector()
    + (SavedVelocity | GetOwner()->GetActorUpVector()) * TeleportationTarget->GetActorUpVector();
}

void UTeleportationComponent::UpdateTracking(FTeleportationUnit& Unit)
{
  Unit.LastInFront = IsInFront(Unit.Subject->GetActorLocation(), GetOwnerPlane());
  Unit.LastPosition = Unit.Subject->GetActorLocation();
}
