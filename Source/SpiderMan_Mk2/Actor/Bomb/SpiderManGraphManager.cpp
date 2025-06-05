// Fill out your copyright notice in the Description page of Project Settings.

#include "SpiderManGraphManager.h"
#include "SpiderManTargetPoint.h"
#include "SpiderManTargetPointManager.h"
#include "EngineUtils.h"

// 静态实例初始化为nullptr
USpiderManGraphManager* USpiderManGraphManager::Instance = nullptr;

// 获取单例实例
USpiderManGraphManager* USpiderManGraphManager::GetInstance()
{
    if (!Instance)
    {
        Instance = NewObject<USpiderManGraphManager>();
        Instance->AddToRoot(); // 防止被垃圾回收
    }
    return Instance;
}

// 销毁单例实例
void USpiderManGraphManager::DestroyInstance()
{
    if (Instance)
    {
        Instance->UnInitialize();
        Instance->RemoveFromRoot();
        Instance = nullptr;
    }
}

// 初始化管理器
void USpiderManGraphManager::Initialize(UWorld* World, float DistanceLimit)
{
    if (bIsInitial == true)
        return;
    bIsInitial = true;
    
    WorldContext = World;
    ConnectionDistanceLimit = DistanceLimit;
    Clear();
    
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("无法初始化GraphManager：无效的World引用"));
        return;
    }
    
    // 获取所有目标点
    USpiderManTargetPointManager* PointManager = USpiderManTargetPointManager::GetInstance();
    PointManager->Initialize(World);
    const TMap<int32, ASpiderManTargetPoint*>& AllPoints = PointManager->GetAllTargetPoints();
    
    // 如果没有足够的点，直接返回
    if (AllPoints.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("GraphManager: 目标点数量不足，无法构建图"));
        return;
    }
    
    // 构建目标点ID到索引的映射
    int32 Index = 0;
    for (const auto& Pair : AllPoints)
    {
        PointIDToIndex.Add(Pair.Key, Index);
        IndexToPointID.Add(Index, Pair.Key);
        Index++;
    }
    
    // 初始化邻接矩阵和路径矩阵
    int32 NumPoints = AllPoints.Num();
    AdjacencyMatrix.SetNum(NumPoints);
    PathMatrix.SetNum(NumPoints);
    
    // 设置默认值
    const float Infinity = FLT_MAX;
    for (int32 i = 0; i < NumPoints; i++)
    {
        AdjacencyMatrix[i].SetNum(NumPoints);
        PathMatrix[i].SetNum(NumPoints);
        
        for (int32 j = 0; j < NumPoints; j++)
        {
            if (i == j)
            {
                AdjacencyMatrix[i][j] = 0.0f; // 自己到自己的距离为0
            }
            else
            {
                AdjacencyMatrix[i][j] = Infinity; // 初始化为无穷大
            }
            
            PathMatrix[i][j] = -1; // 初始化为无中间节点
        }
    }
    
    // 填充邻接矩阵，计算每对点之间的距离
    for (const auto& Pair1 : AllPoints)
    {
        for (const auto& Pair2 : AllPoints)
        {
            // 跳过相同的点
            if (Pair1.Key == Pair2.Key)
            {
                continue;
            }
            
            int32 Index1 = PointIDToIndex[Pair1.Key];
            int32 Index2 = PointIDToIndex[Pair2.Key];
            
            // 计算两点之间的距离
            float Distance = GetDistanceBetweenPoints(Pair1.Value, Pair2.Value);
            
            // 如果设置了距离限制且距离超过限制，则设为无穷大（不连通）
            if (ConnectionDistanceLimit > 0.0f && Distance > ConnectionDistanceLimit)
            {
                continue; // 保持为无穷大，表示不连通
            }
            
            // 更新邻接矩阵
            AdjacencyMatrix[Index1][Index2] = Distance;
        }
    }
    
    UE_LOG(LogTemp, Display, TEXT("GraphManager: 初始化完成，共 %d 个目标点，距离限制: %.2f"), 
           NumPoints, ConnectionDistanceLimit > 0.0f ? ConnectionDistanceLimit : -1.0f);
    
    // 计算所有最短路径
    ComputeAllShortestPaths();
}

// 计算两点之间的距离
float USpiderManGraphManager::GetDistanceBetweenPoints(ASpiderManTargetPoint* Point1, ASpiderManTargetPoint* Point2)
{
    if (!Point1 || !Point2)
    {
        return FLT_MAX;
    }
    
    // 计算3D距离
    FVector Location1 = Point1->GetActorLocation();
    FVector Location2 = Point2->GetActorLocation();
    return FVector::Distance(Location1, Location2);
}

// 使用弗洛伊德算法计算所有点之间的最短路径
void USpiderManGraphManager::ComputeAllShortestPaths()
{
    int32 NumPoints = AdjacencyMatrix.Num();
    if (NumPoints == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("GraphManager: 没有足够的点来计算最短路径"));
        return;
    }
    
    // 初始化路径矩阵
    for (int32 i = 0; i < NumPoints; i++)
    {
        for (int32 j = 0; j < NumPoints; j++)
        {
            PathMatrix[i][j] = j;
        }
    }
    
    // 弗洛伊德算法
    for (int32 k = 0; k < NumPoints; k++)
    {
        for (int32 i = 0; i < NumPoints; i++)
        {
            for (int32 j = 0; j < NumPoints; j++)
            {
                if (AdjacencyMatrix[i][k] == FLT_MAX || AdjacencyMatrix[k][j] == FLT_MAX)
                {
                    continue; // 避免溢出
                }
                
                float NewDistance = AdjacencyMatrix[i][k] + AdjacencyMatrix[k][j];
                if (NewDistance < AdjacencyMatrix[i][j])
                {
                    AdjacencyMatrix[i][j] = NewDistance;
                    PathMatrix[i][j] = PathMatrix[i][k];
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Display, TEXT("GraphManager: 所有最短路径计算完成"));
}

// 修改路径重建方法，修复路径计算错误
TArray<int32> USpiderManGraphManager::ReconstructPath(int32 StartIndex, int32 EndIndex)
{
    TArray<int32> Path;
    
    // 如果起点和终点相同，返回只包含起点的路径
    if (StartIndex == EndIndex)
    {
        Path.Add(IndexToPointID[StartIndex]);
        return Path;
    }
    
    // 如果没有路径（距离为无穷大），返回空数组
    if (AdjacencyMatrix[StartIndex][EndIndex] == FLT_MAX)
    {
        UE_LOG(LogTemp, Warning, TEXT("GraphManager: 从 %d 到 %d 不存在路径（距离为无穷大）"), 
               IndexToPointID[StartIndex], IndexToPointID[EndIndex]);
        return Path;
    }
    
    // 使用递归方法重建路径
    RecursivePathConstruction(Path, StartIndex, EndIndex);
    
    return Path;
}

// 添加新的递归方法用于正确构建路径
void USpiderManGraphManager::RecursivePathConstruction(TArray<int32>& Path, int32 StartIndex, int32 EndIndex)
{
    // 如果中间没有其他节点（直接相连）
    if (PathMatrix[StartIndex][EndIndex] == EndIndex)
    {
        // 如果Path为空（第一次调用），添加起点
        if (Path.Num() == 0)
        {
            Path.Add(IndexToPointID[StartIndex]);
        }
        // 添加终点
        Path.Add(IndexToPointID[EndIndex]);
        return;
    }
    
    // 找到中间经过的节点
    int32 Intermediate = PathMatrix[StartIndex][EndIndex];
    
    // 如果Path为空（第一次调用），添加起点
    if (Path.Num() == 0)
    {
        Path.Add(IndexToPointID[StartIndex]);
    }
    
    // 递归构建左半部分路径（不包括中间节点）
    RecursivePathConstruction(Path, StartIndex, Intermediate);
    
    // 递归构建右半部分路径（包括终点）
    RecursivePathConstruction(Path, Intermediate, EndIndex);
}

// 获取从起点到终点的最短路径
TArray<int32> USpiderManGraphManager::GetShortestPath(int32 StartID, int32 EndID)
{
    // 检查映射是否存在
    if (!PointIDToIndex.Contains(StartID) || !PointIDToIndex.Contains(EndID))
    {
        UE_LOG(LogTemp, Warning, TEXT("GraphManager: 无效的起点ID(%d)或终点ID(%d)"), StartID, EndID);
        return TArray<int32>();
    }
    
    int32 StartIndex = PointIDToIndex[StartID];
    int32 EndIndex = PointIDToIndex[EndID];
    
    return ReconstructPath(StartIndex, EndIndex);
}

// 获取两点之间的最短距离
float USpiderManGraphManager::GetShortestDistance(int32 StartID, int32 EndID)
{
    // 检查映射是否存在
    if (!PointIDToIndex.Contains(StartID) || !PointIDToIndex.Contains(EndID))
    {
        UE_LOG(LogTemp, Warning, TEXT("GraphManager: 无效的起点ID(%d)或终点ID(%d)"), StartID, EndID);
        return FLT_MAX;
    }
    
    int32 StartIndex = PointIDToIndex[StartID];
    int32 EndIndex = PointIDToIndex[EndID];
    
    return AdjacencyMatrix[StartIndex][EndIndex];
}

// 清除数据
void USpiderManGraphManager::Clear()
{
    AdjacencyMatrix.Empty();
    PathMatrix.Empty();
    PointIDToIndex.Empty();
    IndexToPointID.Empty();
}

// 添加 UnInitialize 实现
void USpiderManGraphManager::UnInitialize()
{
    Clear();
    WorldContext = nullptr;
    bIsInitial = false;
    UE_LOG(LogTemp, Log, TEXT("GraphManager: 已反初始化"));
} 