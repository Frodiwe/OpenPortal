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

  UPROPERTY()
  class APlayerCameraManager* PlayerCameraManager;

  UPROPERTY(EditAnywhere)
  class AActor* Target;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class USceneCaptureComponent2D* View;
 
  UPROPERTY()
  class UTeleportationComponent* Teleportation;

  UPROPERTY(BlueprintReadWrite)
  class UPortalCaptureComponent* PortalCapture;

  FMatrix GetCameraProjectionMatrix() const;

  class APlayerCameraManager* GetPlayerCameraManager();

  void SetupView();

  void CutCurrentFrame();

  void UpdateView(class APlayerCameraManager* PlayerCamera);

  virtual void BeginPlay() override;

public:
	APortal();

  UFUNCTION(BlueprintCallable)
  UTextureRenderTarget2D* GeneratePortalTexture();

	virtual void Tick(float DeltaTime) override;
};
