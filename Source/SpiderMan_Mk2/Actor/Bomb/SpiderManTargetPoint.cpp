// Fill out your copyright notice in the Description page of Project Settings.

#include "SpiderManTargetPoint.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "SpiderManTargetPointManager.h"

// 构造函数
ASpiderManTargetPoint::ASpiderManTargetPoint()
{
	// 设置这个actor在每一帧都调用Tick()
	PrimaryActorTick.bCanEverTick = true;

	// 设置默认属性
	PointID = -1; // 使用-1表示未初始化
	bIsStart = false;
	bIsEnd = false;
}

// 游戏开始时调用
void ASpiderManTargetPoint::BeginPlay()
{
	Super::BeginPlay();
	
	// 初始化唯一ID
	InitializeUniqueID();
	
	RefreshStartEndState();

	// 注册到管理器
	USpiderManTargetPointManager::GetInstance()->RegisterTargetPoint(this);
}

// 每帧调用
void ASpiderManTargetPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 初始化唯一ID（改进版）
void ASpiderManTargetPoint::InitializeUniqueID()
{
	// 如果已经初始化过，则跳过
	if (bHasInitializedID && PointID >= 0)
	{
		return;
	}
	
	// 从管理器获取下一个可用 ID
	USpiderManTargetPointManager* Manager = USpiderManTargetPointManager::GetInstance();
	if (Manager)
	{
		// 确保管理器已初始化
		UWorld* World = GetWorld();
		if (World && Manager->GetAllTargetPoints().Num() == 0)
		{
			Manager->Initialize(World);
		}
		
		PointID = Manager->GetNextAvailableID();

		bHasInitializedID = true;

		UE_LOG(LogTemp, Log, TEXT("TargetPoint自动分配ID: %d"), PointID);
	}
}

// 设置是否为起点
void ASpiderManTargetPoint::SetIsStart(bool bNewIsStart)
{
	// 如果设置为起点，则不能同时是终点
	if (bNewIsStart && bIsEnd)
	{
		bIsEnd = false;
		UE_LOG(LogTemp, Warning, TEXT("TargetPoint ID %d: 同一个点不能同时是起点和终点，已自动取消终点标记"), PointID);
	}
	
	bIsStart = bNewIsStart; 
	RefreshStartEndState();
}

// 设置是否为终点
void ASpiderManTargetPoint::SetIsEnd(bool bNewIsEnd)
{
	// 如果设置为终点，则不能同时是起点
	if (bNewIsEnd && bIsStart)
	{
		bIsStart = false;
		UE_LOG(LogTemp, Warning, TEXT("TargetPoint ID %d: 同一个点不能同时是起点和终点，已自动取消起点标记"), PointID);
	}
	
	bIsEnd = bNewIsEnd;
	RefreshStartEndState();
}

// 游戏结束或销毁时调用
void ASpiderManTargetPoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 从管理器注销
	USpiderManTargetPointManager::GetInstance()->UnregisterTargetPoint(this);
	
	Super::EndPlay(EndPlayReason);
}