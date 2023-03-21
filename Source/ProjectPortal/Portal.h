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
  class UStaticMeshComponent* Gate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class USceneCaptureComponent2D* View;

  void SetupView();

  FVector ConvertLocationToActorSpace(const FVector& Location, class UStaticMeshComponent* SourceComponent, AActor* TargetActor) const;

  FRotator ConvertRotationToActorSpace(const FRotator& Rotation, class UStaticMeshComponent* SourceComponent, AActor* TargetActor) const;

  FMatrix GetCameraProjectionMatrix() const;

  void UpdateView(class APlayerCameraManager* PlayerCamera);

  virtual void BeginPlay() override;

  class APlayerCameraManager* GetPlayerCameraManager();

  UFUNCTION(BlueprintCallable)
  UTextureRenderTarget2D* GeneratePortalTexture();

public:
	APortal();

	virtual void Tick(float DeltaTime) override;
};
