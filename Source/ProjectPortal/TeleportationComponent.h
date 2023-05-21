// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "TeleportationComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTPORTAL_API UTeleportationComponent : public UActorComponent
{
	GENERATED_BODY()

private:
  FVector LastPosition = FVector::Zero();
  
  bool LastInFront = false;

protected:
  FPlane GetOwnerPlane() const;

  bool IsInFront(const FVector& Point, const FPlane& PortalPlane) const;

  bool DoesIntersect(const FVector& Start, const FVector& End, const FPlane& PortalPlane) const;

  bool IsPointCrossingPlane(const FVector& Point, const FPlane& PortalPlane);

public:
  UTeleportationComponent();

  bool HasCrossedSinceLastTracked(const FVector& CurrentLocation);

  void Teleport(class AActor* ActorToTeleport, class AActor* TeleportationTarget);

  void UpdateTracking(class AActor* TrackedActor);

  void Log(AActor* ActorToTeleport) const
  {
    UE_LOG(LogTemp, Warning, TEXT("does intersect: %s"), DoesIntersect(LastPosition, ActorToTeleport->GetActorLocation(), GetOwnerPlane()) ? *FString{"true"} : *FString{"false"});
    UE_LOG(LogTemp, Warning, TEXT("\tlast position: %s"), *LastPosition.ToString());
    UE_LOG(LogTemp, Warning, TEXT("\tcurr position: %s"), *ActorToTeleport->GetActorLocation().ToString());
    UE_LOG(LogTemp, Warning, TEXT("\tportal plane: %s"), *GetOwnerPlane().ToString());
    UE_LOG(LogTemp, Warning, TEXT("is in front: %s"), IsInFront(ActorToTeleport->GetActorLocation(), GetOwnerPlane()) ? *FString{"true"} : *FString{"false"});
    UE_LOG(LogTemp, Warning, TEXT("last in front: %s"), LastInFront ? *FString{"true"} : *FString{"false"});
  }
};
