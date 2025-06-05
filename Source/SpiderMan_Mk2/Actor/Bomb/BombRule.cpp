// Fill out your copyright notice in the Description page of Project Settings.

#include "BombRule.h"
#include "SpiderManTargetPoint.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "SpiderManTargetPointManager.h"
#include "SpiderManGraphManager.h"
#include "Game/SpiderManGameModeBase.h"
#include "UI/WG_HUD.h"
#include "WG_BombTime.h"

// 构造函数
ABombRule::ABombRule()
{
 	// 设置这个actor在每一帧都调用Tick()
	PrimaryActorTick.bCanEverTick = true;
}

// 游戏开始时调用
void ABombRule::BeginPlay()
{
	Super::BeginPlay();
	
	// 重置触发状态
	bHasTriggeredStartEvent = false;
	bHasTriggeredEndEvent = false;
    bHasTriggeredFailEvent = false;
	bHasBindedHUDEvents = false;  // 重置绑定标志
    bHasBindedBombTimeUIEvents = false;

	// 确保目标点状态与规则匹配
	UpdateTargetPointsState();
	
	// 延迟调用 UpdatePathPointsTips，给目标点足够的时间初始化
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ABombRule::UpdatePathPointsTips, 0.1f, false);
}

// 每帧调用
void ABombRule::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 绘制调试路径
	DrawDebugPath();
	
	// 绘制所有点之间的连线
	DrawAllPointDebugPath();
	
	// 检查并更新提示点
	CheckAndUpdatePathTips();
}

void ABombRule::SetStartPoint()
{
    // 如果新的起点与当前终点相同，则清除终点
    if (StartPoint.Get() && EndPoint.Get() == StartPoint.Get())
	{
        EndPoint = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("BombRule %d: 起点和终点不能相同，已清除终点设置"), RuleID);
    }
    
    // 更新之前起点的状态
    if (CurStartPoint.IsValid())
    {
        CurStartPoint->SetIsStart(false);
    }
    
    // 设置新的起点
    CurStartPoint = StartPoint;
    
    // 更新新起点的状态
    UpdateTargetPointsState();
}

void ABombRule::SetEndPoint()
{
    // 如果新的终点与当前起点相同，则清除起点
    if (EndPoint.Get() && StartPoint.Get() == EndPoint.Get())
    {
        StartPoint = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("BombRule %d: 起点和终点不能相同，已清除起点设置"), RuleID);
    }
    
    // 更新之前终点的状态
    if (CurEndPoint.IsValid())
    {
        CurEndPoint->SetIsEnd(false);
    }
    
    // 设置新的终点
    CurEndPoint = EndPoint;
    
    // 更新新终点的状态
    UpdateTargetPointsState();
}

void ABombRule::UpdateTargetPointsState()
{
    // 更新起点状态
    if (StartPoint.IsValid())
    {
        StartPoint->SetIsStart(true);
    }
    
    // 更新终点状态
    if (EndPoint.IsValid())
    {
        EndPoint->SetIsEnd(true);
    }
}

// 根据ID查找对应的TargetPoint
ASpiderManTargetPoint* ABombRule::FindTargetPointByID(int32 ID) const
{
	// 使用全局管理器查找目标点
	return USpiderManTargetPointManager::GetInstance()->FindTargetPointByID(ID);
}

// 绘制调试路径
void ABombRule::DrawDebugPath()
{
    // 如果不需要显示结果路径，则跳过
    if (!bShowResultPath)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World || ResultPoints.Num() < 2)
    {
        return;
    }
    
    // 创建一个数组来存储所有要连接的点
    TArray<ASpiderManTargetPoint*> PointsToConnect;
    TArray<int32> SortedPointIDs(ResultPoints.Array());
    
    // 根据ResultPoints中的ID查找对应的TargetPoint
    for (int32 PointID : SortedPointIDs)
    {
        ASpiderManTargetPoint* Point = FindTargetPointByID(PointID);
        if (Point)
        {
            PointsToConnect.Add(Point);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BombRule %d: 无法找到ID为 %d 的TargetPoint"), RuleID, PointID);
        }
    }
    
    // 如果找到的点少于2个，无法绘制线
    if (PointsToConnect.Num() < 2)
    {
        return;
    }
    
    // 绘制连接线
    bool bPersistentLines = false; // 每帧重新绘制
    float LifeTime = 0.0f; // 只显示一帧，0.0f表示只显示当前帧
    
    for (int32 i = 0; i < PointsToConnect.Num() - 1; ++i)
    {
        FVector StartLocation = PointsToConnect[i]->GetActorLocation();
        FVector EndLocation = PointsToConnect[i+1]->GetActorLocation();
        StartLocation.Z += 100.0f;
        EndLocation.Z += 100.0f;
        
        // 绘制线段
        DrawDebugLine(
            World,
            StartLocation,
            EndLocation,
            PathLineColor,
            bPersistentLines,
            LifeTime,
            0, // 深度优先级
            PathLineThickness
        );
        
        // 计算线段中点
        FVector MidPoint = (StartLocation + EndLocation) / 2.0f;
        // 将中点向上偏移一点，避免文字与线重叠
        MidPoint.Z += 50.0f;
        
        // 显示序号（i+1表示第几个点）
        DrawDebugString(
            World,
            MidPoint,
            FString::Printf(TEXT("%d"), i+1),
            nullptr, // Actor
            FColor::White, // 文字颜色
            LifeTime,
            true, // 投影
            2.5f // 文字大小
        );
    }
}

// 绘制所有目标点之间的连线
void ABombRule::DrawAllPointDebugPath()
{
    // 如果没有启用绘制全部连线，则跳过
    if (!bDrawAllPointsConnections)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // 获取所有目标点
    USpiderManTargetPointManager* Manager = USpiderManTargetPointManager::GetInstance();
    if (!Manager)
    {
        return;
    }

    const TMap<int32, ASpiderManTargetPoint*>& AllPoints = Manager->GetAllTargetPoints();
    TArray<ASpiderManTargetPoint*> PointsArray;

    // 将Map中的目标点转为数组
    for (const auto& Pair : AllPoints)
    {
        if (Pair.Value)
        {
            PointsArray.Add(Pair.Value);
        }
    }

    // 如果点数量不足，无法绘制连线
    if (PointsArray.Num() < 2)
    {
        return;
    }

    // 绘制所有点之间的连线
    bool bPersistentLines = false; // 每帧重新绘制
    float LifeTime = 0.0f; // 只显示一帧，0.0f表示只显示当前帧

    // 双重循环，两两连接所有点
    for (int32 i = 0; i < PointsArray.Num(); ++i)
    {
        for (int32 j = i + 1; j < PointsArray.Num(); ++j)
        {
            FVector StartLocation = PointsArray[i]->GetActorLocation();
            FVector EndLocation = PointsArray[j]->GetActorLocation();
            
            // 计算XY平面上的距离（忽略Z轴）
            FVector2D Start2D(StartLocation.X, StartLocation.Y);
            FVector2D End2D(EndLocation.X, EndLocation.Y);
            float Distance2D = FVector2D::Distance(Start2D, End2D);
            
            // 如果距离超过限制，跳过这对点
            if (Distance2D > ConnectionDistanceLimit)
            {
                continue;
            }
            
            // 将Z值稍微提高，避免与地面重叠
            StartLocation.Z += 80.0f;
            EndLocation.Z += 80.0f;

            // 绘制连线
            DrawDebugLine(
                World,
                StartLocation,
                EndLocation,
                AllPointsLineColor,
                bPersistentLines,
                LifeTime,
                0, // 深度优先级
                AllPointsLineThickness
            );

            // 计算中点位置
            FVector MidPoint = (StartLocation + EndLocation) / 2.0f;
            // 将中点向上偏移一点，避免文字与线重叠
            MidPoint.Z += 80.0f;

            // 生成距离字符串
            FString DistanceStr = FString::Printf(TEXT("%.1f米"), Distance2D / 100.0f);
            
            // 显示距离信息
            DrawDebugString(
                World,
                MidPoint,
                DistanceStr,
                nullptr, // Actor
                DistanceTextColor,
                LifeTime,
                true, // 投影
                DistanceTextSize
            );
        }
    }
}

// 修改更新路径点提示的方法，使用栈方式
void ABombRule::UpdatePathPointsTips()
{
    // 检查目标点管理器是否已初始化完成
    USpiderManTargetPointManager* Manager = USpiderManTargetPointManager::GetInstance();
    if (!Manager || Manager->GetAllTargetPoints().Num() == 0)
    {
        // 如果管理器未初始化或没有点，延迟再试
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, this, &ABombRule::UpdatePathPointsTips, 0.1f, false);
        return;
    }

	const TMap<int32, ASpiderManTargetPoint*>& AllPoints = Manager->GetAllTargetPoints();
	
	// 将Map中的目标点转为数组
	for (const auto& Pair : AllPoints)
	{
		if (Pair.Value)
		{
			if (Pair.Value == StartPoint.Get())
			{
				Pair.Value->bIsStart = true;
			}
            else 
            {
                Pair.Value->bIsStart = false;
            }

            if (Pair.Value == EndPoint.Get())
			{
				Pair.Value->bIsEnd = true;
			}
			else
			{
				Pair.Value->bIsEnd = false;
			}
		}
	}

    // 隐藏所有提示点
    HideAllPathTips();
    
    // 如果不启用提示，则直接返回
    if (!bEnablePathTips || ResultPoints.Num() == 0)
    {
        return;
    }

    ComputeShortestPath();
}

// 更新栈顶提示点显示
void ABombRule::UpdateStackTopTip()
{
    // 确保栈不为空
    if (TipsPointsStack.Num() == 0)
    {
        return;
    }
    
    // 获取栈顶元素（最后一个元素）
    int32 TopPointID = TipsPointsStack.Last();
    
    // 查找对应的目标点并显示提示
    ASpiderManTargetPoint* Point = FindTargetPointByID(TopPointID);
    if (Point)
    {
        Point->SetTipsState(true);
        UE_LOG(LogTemp, Log, TEXT("显示栈顶提示点ID: %d"), TopPointID);
    }
}

// 隐藏所有提示点
void ABombRule::HideAllPathTips()
{
    // 遍历ResultPoints中所有的点ID，隐藏所有提示
    for (int32 PointID : ResultPoints)
    {
        ASpiderManTargetPoint* Point = FindTargetPointByID(PointID);
        if (Point)
        {
            Point->SetTipsState(false);
        }
    }
}

// 添加绑定 HUD 事件的方法
void ABombRule::BindHUDEventsIfNeeded()
{
    // 如果已经绑定过，则跳过
    if (bHasBindedHUDEvents)
    {
        return;
    }
    
    // 获取游戏模式和 HUD
    ASpiderManGameModeBase* GameMode = Cast<ASpiderManGameModeBase>(GetWorld()->GetAuthGameMode());
    if (!GameMode)
    {
        return;
    }
    
    UWG_HUD* HUD = GameMode->GetMainHUD();
    if (!HUD)
    {
        return;
    }
    
    // 使用接口绑定事件
    HUD->BindBombGameEvents(this);
    bHasBindedHUDEvents = true;
    UE_LOG(LogTemp, Log, TEXT("BombRule: 已自动绑定HUD事件"));
}

void ABombRule::BindBombTimeUIEventsIfNeeded()
{
	// 如果已经绑定过，则跳过
	if (bHasBindedBombTimeUIEvents)
	{
		return;
	}
	if (!EndPoint.IsValid())
	{
		return;
	}
	if (UWG_BombTime* BombTimeUI = EndPoint->GetBombTimeUI())
	{
		BombTimeUI->BindBombGameEvents(this);
	}
    else 
    {
        return;
    }
	
    bHasBindedBombTimeUIEvents = true;
	UE_LOG(LogTemp, Log, TEXT("BombRule: 已自动绑定BombTimeUI事件"));
}

// 修改 CheckAndUpdatePathTips 方法
void ABombRule::CheckAndUpdatePathTips()
{
    // 在检查前先尝试绑定 HUD 事件
    BindHUDEventsIfNeeded();
    BindBombTimeUIEventsIfNeeded();

    // 检查是否接近起点
    if (!bHasTriggeredStartEvent && IsPlayerNearStartPoint() && !bHasTriggeredFailEvent)
    {
        // 广播事件（接口方式）
        OnBombGameStart.Broadcast();
        bHasTriggeredStartEvent = true;
    }
    
    if (!bHasTriggeredStartEvent)
    {
        return;
    }

    // 检查是否接近终点
    if (!bHasTriggeredEndEvent && IsPlayerNearEndPoint() && !bHasTriggeredFailEvent)
    {
        // 广播事件（接口方式）
        OnBombGameComplete.Broadcast();
        bHasTriggeredEndEvent = true;   
    }
    
    // 确保栈不为空
    if (TipsPointsStack.Num() == 0)
    {
        return;
    }
    
    // 获取栈顶点ID
    int32 TopPointID = TipsPointsStack.Last();
    
    // 查找对应的目标点
    ASpiderManTargetPoint* CurrentPoint = FindTargetPointByID(TopPointID);
    
    if (!CurrentPoint)
    {
        return;
    }
    
    // 获取玩家角色
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    // 计算玩家与当前提示点的距离
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    FVector PointLocation = CurrentPoint->GetTipActorLocation();
    float Distance = FVector::Distance(PlayerLocation, PointLocation);
    
    // 如果玩家靠近当前提示点，则移除栈顶并更新下一个提示
    if (Distance <= TipsActivationDistance)
    {
        UE_LOG(LogTemp, Log, TEXT("玩家接近提示点ID: %d，距离: %.2f，移除栈顶"), 
            TopPointID, Distance);
        
        // 隐藏当前提示
        CurrentPoint->SetTipsState(false);
        
        // 移除栈顶元素
        TipsPointsStack.Pop();
        
        // 更新新的栈顶提示
        UpdateStackTopTip();
        
        if (EndPoint.IsValid())
        {
            if (UWG_BombTime* BombTimeUI = EndPoint->GetBombTimeUI())
			{
                BombTimeUI->DecreaseTimeStr = FString::Printf(TEXT("+ %.2f"), DecBombTime);
				BombTimeUI->PlayDecTimeAnim();
                BombTimeUI->SetCurLeftTime(BombTimeUI->GetCurLeftTime() + DecBombTime);
            }
        }

        if (TipsPointsStack.Num() == 0)
        {
            UE_LOG(LogTemp, Log, TEXT("所有提示点已显示完成"));
        }
    }
}

// 直接设置所有点调试状态
void ABombRule::SetAllPointsDebug(bool bEnabled)
{
    if (bDrawAllPointsConnections != bEnabled)
    {
        bDrawAllPointsConnections = bEnabled;
        UE_LOG(LogTemp, Log, TEXT("显示所有点连线: %s"), bDrawAllPointsConnections ? TEXT("开启") : TEXT("关闭"));
    }
}

// 直接设置结果路径调试状态
void ABombRule::SetResultPathDebug(bool bEnabled)
{
    if (bShowResultPath != bEnabled)
    {
        bShowResultPath = bEnabled;
        UE_LOG(LogTemp, Log, TEXT("显示结果路径: %s"), bShowResultPath ? TEXT("开启") : TEXT("关闭"));
    }
}

#if WITH_EDITOR
void ABombRule::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    
    // 如果修改了起点、终点或ResultPoints，更新相关状态
    if (PropertyName == GET_MEMBER_NAME_CHECKED(ABombRule, StartPoint))
    {
        SetStartPoint();
	} 
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ABombRule, EndPoint))
	{
        SetEndPoint();
	}
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ABombRule, ResultPoints) ||
             PropertyName == GET_MEMBER_NAME_CHECKED(ABombRule, bEnablePathTips))
    {
        // 如果在编辑器中修改了ResultPoints或提示设置，确保管理器已初始化
        USpiderManTargetPointManager::GetInstance()->Initialize(GetWorld());
        
        // 更新路径点提示
        UpdatePathPointsTips();
    }
}
#endif 

void ABombRule::ComputeShortestPath()
{
    // 确保图管理器已初始化，传入距离限制
    USpiderManGraphManager* GraphManager = USpiderManGraphManager::GetInstance();
    GraphManager->Initialize(GetWorld(), ConnectionDistanceLimit);
    
    // 检查起点和终点是否有效
    if (!StartPoint.IsValid() || !EndPoint.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("BombRule: 没有设置起点或终点，无法计算最短路径"));
        return;
    }
    
    // 安全获取指针
    ASpiderManTargetPoint* StartPointPtr = StartPoint.Get();
    ASpiderManTargetPoint* EndPointPtr = EndPoint.Get();
    
    // 再次检查获取的指针是否有效
    if (!StartPointPtr || !EndPointPtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("BombRule: 无法获取有效的起点或终点指针"));
        return;
    }
    
    // 获取起点和终点的ID
    int32 StartID = StartPointPtr->GetPointID();
    int32 EndID = EndPointPtr->GetPointID();
    
    // 从图管理器中获取最短路径
    TArray<int32> ShortestPath = GraphManager->GetShortestPath(StartID, EndID);
    
    if (ShortestPath.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("BombRule: 无法找到从ID %d 到 ID %d 的路径"), StartID, EndID);
        return;
    }
    
    // 清除并设置ResultPoints
    ResultPoints.Empty();
    for (int32 PointID : ShortestPath)
    {
        ResultPoints.Add(PointID);
    }
    
    // 获取路径总距离
    float TotalDistance = GraphManager->GetShortestDistance(StartID, EndID);
    
    UE_LOG(LogTemp, Display, TEXT("BombRule: 已计算最短路径，从ID %d 到 ID %d，路径长度: %d，总距离: %.2f"), 
        StartID, EndID, ShortestPath.Num(), TotalDistance);
    
    // 更新路径点提示（仅更新UI，不再递归计算路径）
    TipsPointsStack.Empty();
    TArray<int32> PointsArray = ResultPoints.Array();
    
    // 按照正常顺序添加到栈中（最后一个元素为栈顶）
    for (int32 i = PointsArray.Num() - 1; i >= 0; --i)
    {
        TipsPointsStack.Add(PointsArray[i]);
    }
    
    // 只更新提示显示，不递归调用
    UpdateStackTopTip();
}

// 检查玩家是否接近起点
bool ABombRule::IsPlayerNearStartPoint()
{
    if (!StartPoint.IsValid())
    {
        return false;
    }
    
    ASpiderManTargetPoint* StartPointPtr = StartPoint.Get();
    if (!StartPointPtr)
    {
        return false;
    }
    
    // 获取玩家角色
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return false;
    }
    
    // 计算玩家与起点的距离
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    FVector PointLocation = StartPointPtr->GetTipActorLocation();
    float Distance = FVector::Distance(PlayerLocation, PointLocation);
    
    // 返回是否在通知距离内
    return Distance <= StartEndNotifyDistance;
}

// 检查玩家是否接近终点
bool ABombRule::IsPlayerNearEndPoint()
{
    if (!EndPoint.IsValid())
    {
        return false;
    }
    
    ASpiderManTargetPoint* EndPointPtr = EndPoint.Get();
    if (!EndPointPtr)
    {
        return false;
    }
    
    // 获取玩家角色
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return false;
    }
    
    // 计算玩家与终点的距离
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    FVector PointLocation = EndPointPtr->GetTipActorLocation();
    float Distance = FVector::Distance(PlayerLocation, PointLocation);
    
    // 返回是否在通知距离内
    return Distance <= StartEndNotifyDistance;
}

// 实现游戏失败方法
void ABombRule::OnGameFailed()
{
	if (bHasTriggeredFailEvent)
	    return;

    // 广播失败事件
    OnBombGameFail.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("BombRule: 触发炸弹游戏失败事件"));
    
    bHasTriggeredFailEvent = true;
   
    // 清理提示栈并隐藏所有提示
    HideAllPathTips();
    
    if (EndPoint.IsValid())
    {
        if (UWG_BombTime* BombTimeUI = EndPoint->GetBombTimeUI())
        {
            // 重置为初始时间或其他值
            BombTimeUI->SetCurLeftTime(0);
        }
    }
} 