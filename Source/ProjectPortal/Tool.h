#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Tool.generated.h"


UCLASS()
class PROJECTPORTAL_API UTool : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable)
  static FVector ConvertLocationToActorSpace(const FVector& Location, class AActor* SourceActor, AActor* TargetActor);

  UFUNCTION(BlueprintCallable)
  static FRotator ConvertRotationToActorSpace(const FRotator& Rotation, class AActor* SourceActor, AActor* TargetActor);

  UFUNCTION(BlueprintCallable)
  static FMatrix GetCameraProjectionMatrix(class ULocalPlayer* Player);
};
