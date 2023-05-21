#include "Tool.h"


FVector UTool::ConvertLocationToActorSpace(const FVector &Location, AActor *SourceActor, AActor *TargetActor)
{
  if (SourceActor == nullptr || TargetActor == nullptr)
  {
    return FVector::ZeroVector;
  }

  FVector Direction = Location - SourceActor->GetActorLocation();

  return TargetActor->GetActorLocation()
    + (Direction | SourceActor->GetActorForwardVector()) * TargetActor->GetActorForwardVector()
    + (Direction | SourceActor->GetActorRightVector()) * TargetActor->GetActorRightVector()
    + (Direction | SourceActor->GetActorUpVector()) * TargetActor->GetActorUpVector();
}

FRotator UTool::ConvertRotationToActorSpace(const FRotator& Rotation, AActor* SourceActor, AActor* TargetActor)
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
