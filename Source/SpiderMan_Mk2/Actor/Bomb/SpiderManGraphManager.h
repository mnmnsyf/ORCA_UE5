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
#include "SpiderManGraphManager.generated.h"

class ASpiderManTargetPoint;

/**
 * 图算法管理器
 * 使用弗洛伊德算法计算场景中所有目标点之间的最短路径
 */
UCLASS()
class SPIDERMAN_MK2_API USpiderManGraphManager : public UObject
{
    GENERATED_BODY()
    
public:
    // 获取单例实例
    static USpiderManGraphManager* GetInstance();
    
    // 销毁单例实例
    static void DestroyInstance();
    
    // 初始化管理器，收集场景中的所有目标点并构建图
    // DistanceLimit: 点之间的最大连接距离，超过此距离的点将不被连接
    void Initialize(UWorld* World, float DistanceLimit = -1.0f);
    
	// 反初始化管理器，清理资源
	void UnInitialize();

private:
	// 清除数据
	void Clear();

public:
    // 获取从起点到终点的最短路径ID集
    TArray<int32> GetShortestPath(int32 StartID, int32 EndID);
    
    // 获取两点之间的最短距离
    float GetShortestDistance(int32 StartID, int32 EndID);

private:
	// 使用弗洛伊德算法计算所有点之间的最短路径
	void ComputeAllShortestPaths();

	// 构建最短路径数组
	TArray<int32> ReconstructPath(int32 StartIndex, int32 EndIndex);

	// 获取两点之间的距离
	float GetDistanceBetweenPoints(ASpiderManTargetPoint* Point1, ASpiderManTargetPoint* Point2);

	// 递归构建路径
	void RecursivePathConstruction(TArray<int32>& Path, int32 StartIndex, int32 EndIndex);
private:
    // 单例实例
    static USpiderManGraphManager* Instance;
    
    // 世界引用
    UPROPERTY()
    TObjectPtr<UWorld> WorldContext;
    
private:
    // 图的邻接矩阵 - 存储点之间的距离
    TArray<TArray<float>> AdjacencyMatrix;
    
    // 路径矩阵 - 存储通过哪个点可以到达目标点
    TArray<TArray<int32>> PathMatrix;
    
    // 目标点ID到索引的映射
    TMap<int32, int32> PointIDToIndex;
    
    // 索引到目标点ID的映射
    TMap<int32, int32> IndexToPointID;
    
   
    bool bIsInitial = false;

    // 添加距离限制成员变量
    float ConnectionDistanceLimit;
}; 