// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

UCLASS()
class PROJECTPORTAL_API APortal : public AActor
{
	GENERATED_BODY()

protected:
  UPROPERTY()
  class USceneComponent* PortalRoot;

  UPROPERTY(EditAnywhere)
  class UBoxComponent* PortalArea;

  UPROPERTY(EditAnywhere)
  class AActor* Target;

  UPROPERTY(EditAnywhere)
  class UReplicationComponent* Replication;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class UPortalCaptureComponent* PortalCapture;

  UFUNCTION()
  void OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

  UFUNCTION()
  void OnEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

  FMatrix GetCameraProjectionMatrix() const;

  virtual void BeginPlay() override;

public:
	APortal();

  FPlane GetPlane() const
  {
    return {GetActorLocation(), -GetActorForwardVector()};
  }

  AActor* GetTarget() const
  {
    return Target;
  }

  UFUNCTION(BlueprintCallable)
  UTextureRenderTarget2D* GeneratePortalTexture();
};
