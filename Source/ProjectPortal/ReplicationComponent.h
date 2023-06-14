// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReplicationComponent.generated.h"


USTRUCT()
struct FReplicationUnit
{
  GENERATED_BODY()

  UPROPERTY()
	class AActor* Source;

  UPROPERTY()
  class AActor* Copy;

  UPROPERTY()
  class AActor* Target;

  void Swap()
  {
    std::swap(Source, Copy);
  }
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTPORTAL_API UReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
  UPROPERTY(VisibleAnywhere)
  TArray<FReplicationUnit> Units;

public:
	UReplicationComponent();

  void Add(class AActor* Source, class AActor* ReplicationTarget);

  void Remove(class AActor* AActor);

  void Swap(class AActor* Source);

  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
