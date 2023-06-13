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

  FVector LastPosition = FVector::ZeroVector;
  
  bool LastInFront = false;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTPORTAL_API UTeleportationComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
  UPROPERTY(VisibleAnywhere)
  FTeleportationUnit Tracked;

  bool IsInFront(const FVector& Point, const FPlane& PortalPlane) const;

  bool DoesIntersect(const FVector& Start, const FVector& End, const FPlane& PortalPlane) const;

  void UpdateTracking(FTeleportationUnit& Unit, const FPlane& TeleportationPlane);

  bool HasCrossedSinceLastTracked(const FTeleportationUnit& Unit, const FPlane& TeleportationPlane);

  void Teleport(class AActor* Subject, class AActor* Portal, class AActor* TeleportationTarget);
  void Teleport(class ACharacter* Subject, class AActor* Portal, class AActor* TeleportationTarget);

  virtual void BeginPlay() override;

public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class APortal* ActivePortal;

  FActorTeleported OnActorTeleported;

  UTeleportationComponent();

  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction);
};
