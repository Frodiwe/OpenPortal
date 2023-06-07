// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "TeleportationComponent.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FActorTeleported, AActor*);


USTRUCT()
struct FTeleportationUnit
{
  GENERATED_BODY()

  UPROPERTY()
	class AActor* Subject;

  FVector LastPosition = FVector::Zero();
  
  bool LastInFront = false;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTPORTAL_API UTeleportationComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
  UPROPERTY(EditAnywhere)
  TArray<FTeleportationUnit> Tracked;

  FPlane GetOwnerPlane() const;

  bool IsInFront(const FVector& Point, const FPlane& PortalPlane) const;

  bool DoesIntersect(const FVector& Start, const FVector& End, const FPlane& PortalPlane) const;

  void UpdateTracking(FTeleportationUnit& Unit);

  bool HasCrossedSinceLastTracked(const FTeleportationUnit& Unit);

  void Teleport(class AActor* Subject, class AActor* TeleportationTarget);
  void Teleport(class ACharacter* Subject, class AActor* TeleportationTarget);

public:
  UPROPERTY()
  class AActor* Target;

  FActorTeleported OnActorTeleported;

  UTeleportationComponent();

  void Track(class AActor* Actor);

  void Untrack(class AActor* Actor);

  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction);
};
