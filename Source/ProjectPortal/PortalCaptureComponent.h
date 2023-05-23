// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PortalCaptureComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTPORTAL_API UPortalCaptureComponent : public UActorComponent
{
	GENERATED_BODY()
  
private:
  APlayerCameraManager* GetPlayerCameraManager();

protected:
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class USceneCaptureComponent2D* View;

  void SetupCapture(class USceneCaptureComponent2D* View);

  void CutCurrentFrame();

  void UpdateView(class APlayerCameraManager* CameraManager, class AActor* ViewTarget);

	virtual void BeginPlay() override;

public:
  UPROPERTY(BlueprintReadWrite)
  class AActor* Target;

  UPortalCaptureComponent();

  UFUNCTION(BlueprintCallable)
  void SetRTT(class UTextureRenderTarget2D* Texture);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
