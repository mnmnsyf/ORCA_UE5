// Fill out your copyright notice in the Description page of Project Settings.

#include "SpiderManTargetPointManager.h"
#include "SpiderManTargetPoint.h"
#include "EngineUtils.h"

// 静态实例初始化为nullptr
USpiderManTargetPointManager* USpiderManTargetPointManager::Instance = nullptr;

// 获取单例实例
USpiderManTargetPointManager* USpiderManTargetPointManager::GetInstance()
{
	if (!Instance)
	{
		Instance = NewObject<USpiderManTargetPointManager>();
		Instance->AddToRoot(); // 防止被垃圾回收
	}
	return Instance;
}

// 销毁单例实例
void USpiderManTargetPointManager::DestroyInstance()
{
	if (Instance)
	{
		Instance->UnInitialize();
		Instance->RemoveFromRoot();
		Instance = nullptr;
	}
}

// 初始化管理器
void USpiderManTargetPointManager::Initialize(UWorld* World)
{
	WorldContext = World;
	ClearAllTargetPoints();
	
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("无法初始化TargetPointManager：无效的World引用"));
		return;
	}
	
	// 收集场景中的所有目标点
	for (TActorIterator<ASpiderManTargetPoint> It(World); It; ++It)
	{
		ASpiderManTargetPoint* TargetPoint = *It;
		if (TargetPoint)
		{
			RegisterTargetPoint(TargetPoint);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("目标点管理器初始化完成，共收集 %d 个目标点"), TargetPointsMap.Num());
}

// 注册目标点
void USpiderManTargetPointManager::RegisterTargetPoint(ASpiderManTargetPoint* TargetPoint)
{
	if (!TargetPoint)
	{
		return;
	}
	
	int32 PointID = TargetPoint->GetPointID();
	if (PointID < 0)
		return;
	// 检查ID是否已被注册
	if (TargetPointsMap.Contains(PointID))
	{
		ASpiderManTargetPoint* ExistingPoint = TargetPointsMap[PointID];
		if (ExistingPoint != TargetPoint)
		{
			UE_LOG(LogTemp, Warning, 
				TEXT("目标点ID冲突：ID %d 已被 %s 使用，无法注册 %s"), 
				PointID, 
				*ExistingPoint->GetName(), 
				*TargetPoint->GetName());
		}
		return;
	}
	
	// 注册新目标点
	TargetPointsMap.Add(PointID, TargetPoint);
	UE_LOG(LogTemp, Verbose, TEXT("已注册目标点：ID=%d, Name=%s"), PointID, *TargetPoint->GetName());
}

// 注销目标点
void USpiderManTargetPointManager::UnregisterTargetPoint(ASpiderManTargetPoint* TargetPoint)
{
	if (!TargetPoint)
	{
		return;
	}
	
	int32 PointID = TargetPoint->GetPointID();
	
	// 检查要注销的目标点是否存在
	if (TargetPointsMap.Contains(PointID))
	{
		ASpiderManTargetPoint* RegisteredPoint = TargetPointsMap[PointID];
		if (RegisteredPoint == TargetPoint)
		{
			TargetPointsMap.Remove(PointID);
			UE_LOG(LogTemp, Verbose, TEXT("已注销目标点：ID=%d, Name=%s"), PointID, *TargetPoint->GetName());
		}
	}
}

// 查找目标点
ASpiderManTargetPoint* USpiderManTargetPointManager::FindTargetPointByID(int32 ID) const
{
	// 从映射表中查找
	if (const ASpiderManTargetPoint* const* FoundPoint = TargetPointsMap.Find(ID))
	{
		return const_cast<ASpiderManTargetPoint*>(*FoundPoint);
	}
	
	return nullptr;
}

// 获取所有目标点
const TMap<int32, ASpiderManTargetPoint*>& USpiderManTargetPointManager::GetAllTargetPoints() const
{
	return TargetPointsMap;
}

// 清除所有目标点
void USpiderManTargetPointManager::ClearAllTargetPoints()
{
	TargetPointsMap.Empty();
}

// 输出调试信息
void USpiderManTargetPointManager::DebugPrintAllTargetPoints() const
{
	UE_LOG(LogTemp, Log, TEXT("====== 目标点管理器状态 ======"));
	UE_LOG(LogTemp, Log, TEXT("已注册目标点数量：%d"), TargetPointsMap.Num());
	
	for (const auto& Pair : TargetPointsMap)
	{
		if (Pair.Value)
		{
			UE_LOG(LogTemp, Log, TEXT("目标点ID: %d, Name: %s, Location: %s"), 
				Pair.Key, 
				*Pair.Value->GetName(), 
				*Pair.Value->GetActorLocation().ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("目标点ID: %d 存在无效引用"), Pair.Key);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("============================"));
}

// 获取下一个可用的目标点 ID
int32 USpiderManTargetPointManager::GetNextAvailableID() const
{
	TSet<int32> UsedIDs;
	
	// 收集已使用的所有 ID
	for (const auto& Pair : TargetPointsMap)
	{
		UsedIDs.Add(Pair.Key);
	}
	
	// 找到第一个未使用的 ID
	int32 NextID = 0;
	while (UsedIDs.Contains(NextID))
	{
		NextID++;
	}
	
	return NextID;
}

// 添加 UnInitialize 实现
void USpiderManTargetPointManager::UnInitialize()
{
	ClearAllTargetPoints();
	WorldContext = nullptr;
	UE_LOG(LogTemp, Log, TEXT("TargetPointManager: 已反初始化"));
} 