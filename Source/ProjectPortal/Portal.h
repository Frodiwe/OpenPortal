// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

UCLASS()
class PROJECTPORTAL_API APortal : public AActor
{
	GENERATED_BODY()
private:
  FVector LastPosition;
  
  bool LastInFront;

protected:
  UPROPERTY()
  class USceneComponent* PortalRoot;

  UPROPERTY()
  class APlayerCameraManager* PlayerCameraManager;

  UPROPERTY(EditAnywhere)
  class AActor* Target;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class UStaticMeshComponent* Gate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class USceneCaptureComponent2D* View;

  UPROPERTY(EditAnywhere)
  class UBoxComponent* Area;

  bool IsInFront(const FVector& Point, const FPlane& PortlaPlane) const;

  bool DoesIntersect(const FVector& Start, const FVector& End, const FPlane& PortalPlane) const;

  void SetupView();

  FVector ConvertLocationToActorSpace(const FVector& Location, class UStaticMeshComponent* SourceComponent, AActor* TargetActor) const;
  FVector ConvertLocationToActorSpace(const FVector& Location, class AActor* SourceActor, AActor* TargetActor) const;

  FRotator ConvertRotationToActorSpace(const FRotator& Rotation, class UStaticMeshComponent* SourceComponent, AActor* TargetActor) const;
  FRotator ConvertRotationToActorSpace(const FRotator& Rotation, class AActor* SourceActor, AActor* TargetActor) const;

  FMatrix GetCameraProjectionMatrix() const;

  void UpdateView(class APlayerCameraManager* PlayerCamera);

  virtual void BeginPlay() override;

  class APlayerCameraManager* GetPlayerCameraManager();

public:
	APortal();

  UFUNCTION(BlueprintCallable)
  UTextureRenderTarget2D* GeneratePortalTexture();

  UFUNCTION(BlueprintCallable)
  bool IsPointCrossingPortal(const FVector& Point, const FVector& PortalLocation, const FVector& PortalNormal);

  void TeleportActor(AActor* ActorToTeleport);

	virtual void Tick(float DeltaTime) override;
};
