// Fill out your copyright notice in the Description page of Project Settings.


#include "GenerateEnemy.h"
#include "SplineActor.h"
#include "EnemyCharacter.h"

// Sets default values
AGenerateEnemy::AGenerateEnemy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AGenerateEnemy::StartSpawning()
{
	if (!SplineActor || !SplineActor->GetSplineComponent())
		return;
	// 清除之前生成的敌人
	for (AWraithCharacter* Enemy : SpawnedEnemies)
	{
		if (Enemy)
		{
			Enemy->Destroy();
		}
	}
	SpawnedEnemies.Empty();

	// 获取SplineComponent
	USplineComponent* SplineComp = SplineActor->GetSplineComponent();
	// 检查是否设置了敌人类
	if (!ActorClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("没有设置敌人类，无法生成敌人"));
		return;
	}

	// 计算Spline总长度
	float SplineLength = SplineComp->GetSplineLength();

	// 生成配置的数量的敌人
	for (int32 i = 0; i < MaxSpawnCount; i++)
	{
		// 计算初始分布位置（均匀分布）
		float DistanceRatio = static_cast<float>(i) / MaxSpawnCount;
		float InitialDistance = SplineLength * DistanceRatio;

		// 获取Spline上的位置和朝向
		FVector SplineLocation = SplineComp->GetLocationAtDistanceAlongSpline(InitialDistance, ESplineCoordinateSpace::World);
		FRotator SplineRotation = SplineComp->GetRotationAtDistanceAlongSpline(InitialDistance, ESplineCoordinateSpace::World);
		
		// 获取Spline在该点的右向量，用于左右两侧偏移
		FVector RightVector = SplineRotation.RotateVector(FVector::RightVector);
		
		// 决定敌人在Spline的左侧还是右侧（奇数索引在左侧，偶数索引在右侧）
		float SideOffset = (i % 2 == 0) ? EnemySpawnDistance : -EnemySpawnDistance;
		
		// 计算最终生成位置
		FVector SpawnLocation = SplineLocation + (RightVector * SideOffset);
		
		// 进行射线检测以确保敌人在地面上
		FHitResult HitResult;
		FVector TraceStart = SpawnLocation + FVector(0, 0, 100.0f);
		FVector TraceEnd = SpawnLocation - FVector(0, 0, 300.0f);

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			CollisionParams
		);

		if (bHit)
		{
			SpawnLocation = HitResult.Location + FVector(0, 0, 100.0f); // 稍微抬高一点防止陷入地面
		}

		// 生成敌人
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AWraithCharacter* NewEnemy = GetWorld()->SpawnActor<AWraithCharacter>(
			ActorClassToSpawn,
			SpawnLocation,
			SplineRotation,
			SpawnParams
		);

		if (NewEnemy)
		{
			// 初始化敌人
			NewEnemy->Initialize(SplineActor.Get());
			SpawnedEnemies.Add(NewEnemy);
			UE_LOG(LogTemp, Warning, TEXT("成功生成敌人 %s"), *NewEnemy->GetName());
		}
	}
}

// Called when the game starts or when spawned
void AGenerateEnemy::BeginPlay()
{
	Super::BeginPlay();
	// 游戏开始时生成敌人
	StartSpawning();
}

void AGenerateEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清除生成的敌人
	for (AWraithCharacter* Enemy : SpawnedEnemies)
	{
		if (Enemy)
		{
			Enemy->Destroy();
		}
	}
	SpawnedEnemies.Empty();

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AGenerateEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

