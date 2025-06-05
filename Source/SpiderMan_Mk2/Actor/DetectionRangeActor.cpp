// Fill out your copyright notice in the Description page of Project Settings.

#include "DetectionRangeActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ADetectionRangeActor::ADetectionRangeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADetectionRangeActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADetectionRangeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 更新缩放
	UpdateScale();
	
	// 更新位置，跟随玩家
	if (OwnerController && OwnerController->GetPawn())
	{
		// 获取玩家位置并更新
		FVector PlayerLocation = OwnerController->GetPawn()->GetActorLocation();
		SetActorLocation(PlayerLocation);
	}
	
	// 更新时间
	CurrentTime += DeltaTime * PulseSpeed;
	if (CurrentTime > 2.0f * PI)
	{
		CurrentTime -= 2.0f * PI;
	}
}

void ADetectionRangeActor::UpdateScale()
{
	// 计算当前比例 (使用正弦波在MinScale和MaxScale之间变化)
	float ScaleFactor = (FMath::Sin(CurrentTime) + 1.0f) * 0.5f; // 0.0 - 1.0
	float CurrentScale = FMath::Lerp(MinScale, MaxScale, ScaleFactor);
	SetActorScale3D(FVector(CurrentScale));
}