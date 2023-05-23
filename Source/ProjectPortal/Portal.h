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
  class AActor* Target;
 
  UPROPERTY()
  class UTeleportationComponent* Teleportation;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class UPortalCaptureComponent* PortalCapture;

  FMatrix GetCameraProjectionMatrix() const;

  virtual void BeginPlay() override;

public:
	APortal();

  UFUNCTION(BlueprintCallable)
  UTextureRenderTarget2D* GeneratePortalTexture();

	virtual void Tick(float DeltaTime) override;
};
