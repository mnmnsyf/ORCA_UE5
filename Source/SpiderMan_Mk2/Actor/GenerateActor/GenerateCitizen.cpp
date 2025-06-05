// Fill out your copyright notice in the Description page of Project Settings.


#include "GenerateCitizen.h"
#include "CitizenBase.h"
#include "SplineActor.h"
// Sets default values
AGenerateCitizen::AGenerateCitizen()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGenerateCitizen::StartSpawning()
{
	if (!ActorClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActorClassToSpawn is not set!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get world context!"));
		return;
	}
	if (TargetSplineActor == nullptr)
	{
		return;
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//沿着路网生成
	float PawnPathDis = 0.f;

	// 根据配置数量生成
	for (int32 i = 0; i < MaxSpawnCount; ++i)
	{
		const FVector SpawnLocation = TargetSplineActor->GetSplineComponent()->GetLocationAtDistanceAlongSpline(PawnPathDis, ESplineCoordinateSpace::World);
		const FRotator SpawnRotation = TargetSplineActor->GetSplineComponent()->GetRotationAtDistanceAlongSpline(PawnPathDis, ESplineCoordinateSpace::World);
		AActor* NewActor = World->SpawnActor<AActor>(
			ActorClassToSpawn,
			SpawnLocation + FVector(0, 0, 100.f), // 简单垂直偏移示例
			SpawnRotation,
			SpawnParams
		);

		if (NewActor)
		{
			// 初始化生成后的Actor
			if (ACitizenBase* Citizen = Cast<ACitizenBase>(NewActor))
			{
				Citizen->SetMoveSpline(TargetSplineActor.Get(), PawnPathDis);
				//Citizens.Add(Citizen);
			}
			
		}
		PawnPathDis += IntervalDis;
	}
}

void AGenerateCitizen::BeginPlay()
{
	Super::BeginPlay();
	StartSpawning();
}

// Called every frame
void AGenerateCitizen::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
