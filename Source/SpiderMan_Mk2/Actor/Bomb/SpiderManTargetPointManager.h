//*******************************************************************************
//																				//
//	Project Name:			SpiderMan_Mk2										//
//	Author:					Wangzhongpeng									//
//	Completion Date:												//
//	Copyright Declaration:									//
// (C) Copyright        成都数字天空
//																				//
//																				//
//*******************************************************************************
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SpiderManTargetPointManager.generated.h"

class ASpiderManTargetPoint;

/**
 * 目标点管理器
 * 用于集中管理和缓存场景中的所有目标点，避免频繁遍历
 */
UCLASS()
class SPIDERMAN_MK2_API USpiderManTargetPointManager : public UObject
{
	GENERATED_BODY()
	
public:
	// 获取单例实例
	static USpiderManTargetPointManager* GetInstance();
	
	// 销毁单例实例
	static void DestroyInstance();
	
	// 初始化管理器，收集场景中的所有目标点
	void Initialize(UWorld* World);

	// 反初始化管理器，清理资源
	void UnInitialize();

public:
	// 注册一个目标点到管理器
	void RegisterTargetPoint(ASpiderManTargetPoint* TargetPoint);
	
	// 从管理器注销一个目标点
	void UnregisterTargetPoint(ASpiderManTargetPoint* TargetPoint);
	
	// 根据ID查找目标点
	ASpiderManTargetPoint* FindTargetPointByID(int32 ID) const;
	
	// 获取所有目标点
	const TMap<int32, ASpiderManTargetPoint*>& GetAllTargetPoints() const;
	
	// 清除所有注册的目标点
	void ClearAllTargetPoints();
	
	// 输出调试信息
	void DebugPrintAllTargetPoints() const;
	
	// 获取下一个可用的目标点 ID
	int32 GetNextAvailableID() const;
	
private:
	// 单例实例
	static USpiderManTargetPointManager* Instance;
	
	// 存储所有目标点的映射表
	UPROPERTY()
	TMap<int32, ASpiderManTargetPoint*> TargetPointsMap;
	
	// 世界引用，用于在需要时查找目标点
	UPROPERTY()
	TObjectPtr<UWorld> WorldContext;
}; 