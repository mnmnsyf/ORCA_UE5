// Fill out your copyright notice in the Description page of Project Settings.


#include "GenerateVehicle.h"
#include "TransportVehicleActor.h"
#include "SplineActor.h"

// Sets default values
AGenerateVehicle::AGenerateVehicle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AGenerateVehicle::StartSpawning()
{
	if (!SplineActor || !SplineActor->GetSplineComponent())
		return;
	// 清除之前生成的车辆
	for (ATransportVehicleActor* Vehicle : SpawnedVehicles)
	{
		if (Vehicle)
		{
			Vehicle->Destroy();
		}
	}
	SpawnedVehicles.Empty();

	// 获取SplineComponent
	USplineComponent* SplineComp = SplineActor->GetSplineComponent();
	
	// 检查是否设置了车辆类
	if (!ActorClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("没有设置车辆类，无法生成车辆"));
		return;
	}

	// 计算Spline总长度
	float SplineLength = SplineComp->GetSplineLength();

	// 生成配置的数量的车辆
	for (int32 i = 0; i < MaxSpawnCount; i++)
	{
		// 计算初始分布位置（均匀分布）
		float DistanceRatio = static_cast<float>(i) / MaxSpawnCount;
		float InitialDistance = SplineLength * DistanceRatio;

		// 计算生成位置
		FVector SpawnLocation = SplineComp->GetLocationAtDistanceAlongSpline(InitialDistance, ESplineCoordinateSpace::World);
		FRotator SpawnRotation = SplineComp->GetRotationAtDistanceAlongSpline(InitialDistance, ESplineCoordinateSpace::World);

		// 生成车辆
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ATransportVehicleActor* NewVehicle = GetWorld()->SpawnActor<ATransportVehicleActor>(
			ActorClassToSpawn,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		if (NewVehicle)
		{
			// 设置车辆的路径和初始距离
			NewVehicle->Initialize(SplineActor.Get(), InitialDistance);
			NewVehicle->InitDirAndSpeed(MovementDirection, VehicleSpeed);
			SpawnedVehicles.Add(NewVehicle);

			// 添加调试日志
			UE_LOG(LogTemp, Warning, TEXT("成功生成车辆 %s 并初始化"), *NewVehicle->GetName());
		}
	}
}

// Called when the game starts or when spawned
void AGenerateVehicle::BeginPlay()
{
	Super::BeginPlay();
	// 游戏开始时生成敌人
	StartSpawning();
}

void AGenerateVehicle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清除生成的车辆
	for (ATransportVehicleActor* Vehicle : SpawnedVehicles)
	{
		if (Vehicle)
		{
			Vehicle->Destroy();
		}
	}
	SpawnedVehicles.Empty();

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AGenerateVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

