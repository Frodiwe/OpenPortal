#include "ReplicationComponent.h"
#include "Tool.h"


UReplicationComponent::UReplicationComponent()
{
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UReplicationComponent::Add(AActor *Source, AActor* ReplicationTarget)
{
  auto Params = FActorSpawnParameters{};
  Params.Template = Source;

  auto Copy = GetWorld()->SpawnActor<AActor>(
    Source->GetClass(),
    UTool::ConvertLocationToActorSpace(Source->GetActorLocation(), GetOwner(), ReplicationTarget),
    UTool::ConvertRotationToActorSpace(Source->GetActorRotation(), GetOwner(), ReplicationTarget),
    Params
  );

  check(Copy);
  // Copy->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
  
  Units.Emplace(FReplicationUnit{
    .Source = Source,
    .Copy = Copy,
    .Target = ReplicationTarget
  });
}

void UReplicationComponent::Remove(AActor* Actor)
{
  Units.RemoveAll([&](const auto& Unit) {
    return Unit.Source == Actor && Unit.Copy->Destroy();
  });
}

void UReplicationComponent::Swap(class AActor* Source)
{
  UE_LOG(LogTemp, Warning, TEXT("Swapping..."));
  if (auto Unit = Units.FindByPredicate([&](const auto& Unit) { return Unit.Source == Source; }))
  {
    Unit->Swap();
    UE_LOG(LogTemp, Warning, TEXT("Swap done"));
  }
}

void UReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
  
  for (const auto& Unit : Units) {
    // UTool::Teleport(Unit.Copy, UTool::ConvertLocationToActorSpace(Unit.Source->GetActorLocation(), GetOwner(), Unit.Target));
    // Unit.Copy->SetActorRotation(UTool::ConvertRotationToActorSpace(Unit.Source->GetActorRotation(), GetOwner(), Unit.Target));
  }
}
