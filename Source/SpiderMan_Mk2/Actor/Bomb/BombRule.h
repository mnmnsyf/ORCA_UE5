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
#include "GameFramework/Actor.h"
#include "IBombGameInterface.h"
#include "BombRule.generated.h"

class ASpiderManTargetPoint;

/**
 * 炸弹规则类，定义炸弹从起点到终点的路径规则
 */
UCLASS()
class SPIDERMAN_MK2_API ABombRule : public AActor, public IBombGameInterface
{
	GENERATED_BODY()
	
public:	
	// 设置默认值
	ABombRule();
public:
	// 起点目标点的软引用
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb Rule")
	TSoftObjectPtr<ASpiderManTargetPoint> StartPoint;

	// 终点目标点的软引用
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb Rule")
	TSoftObjectPtr<ASpiderManTargetPoint> EndPoint;

	// 规则ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb Rule")
	int32 RuleID = 0;

	// 减少炸弹时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomb Rule")
	float DecBombTime = 5;

	// 点之间的最大连接距离，超过此距离的点将不被连接
	UPROPERTY(EditAnywhere, Category = "Bomb Rule", meta = (ClampMin = "100.0"))
	float ConnectionDistanceLimit = 1000.0f;

	// 是否显示结果路径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowResultPath = true;

	// 调试线条颜色
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bShowResultPath"))
	FColor PathLineColor = FColor::Yellow;

	// 调试线条粗细
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bShowResultPath", ClampMin = "1.0", ClampMax = "1000.0"))
	float PathLineThickness = 2.0f;

	// 是否绘制所有点之间的连线
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawAllPointsConnections = false;

	// 所有点连线的颜色
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bDrawAllPointsConnections"))
	FColor AllPointsLineColor = FColor::Blue;

	// 所有点连线的粗细
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bDrawAllPointsConnections", ClampMin = "1.0", ClampMax = "1000.0"))
	float AllPointsLineThickness = 1.0f;

	// 距离文字颜色
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bDrawAllPointsConnections"))
	FColor DistanceTextColor = FColor::Yellow;

	// 距离文字大小
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bDrawAllPointsConnections", ClampMin = "0.5", ClampMax = "5.0"))
	float DistanceTextSize = 1.5f;

	// 玩家触发起点/终点提示的距离
	UPROPERTY(EditAnywhere, Category = "Tips", meta = (ClampMin = "100.0"))
	float StartEndNotifyDistance = 400.0f;

	// 玩家需要接近点多近才会触发下一个提示点(厘米)
	UPROPERTY(EditAnywhere, Category = "Tips", meta = (ClampMin = "100.0"))
	float TipsActivationDistance = 300.0f;

	// 是否启用提示功能
	UPROPERTY(EditAnywhere, Category = "Tips")
	bool bEnablePathTips = true;

public:
	// 获取所有点调试状态
	bool IsAllPointsDebugEnabled() const { return bDrawAllPointsConnections; }

	// 直接设置所有点调试状态
	void SetAllPointsDebug(bool bEnabled);

	// 获取结果路径调试状态
	bool IsResultPathDebugEnabled() const { return bShowResultPath; }

	// 直接设置结果路径调试状态
	void SetResultPathDebug(bool bEnabled);

	// 游戏失败方法（由定时器或其他组件调用）
	void OnGameFailed();

protected:
	// 炸弹游戏开始事件（玩家接近起点）
	UPROPERTY(BlueprintAssignable, Category = "Bomb Game")
	FOnBombGameStartDelegate OnBombGameStart;

	// 炸弹游戏完成事件（玩家接近终点）
	UPROPERTY(BlueprintAssignable, Category = "Bomb Game")
	FOnBombGameCompleteDelegate OnBombGameComplete;
	
	// 炸弹游戏失败事件（时间耗尽）
	UPROPERTY(BlueprintAssignable, Category = "Bomb Game")
	FOnBombGameFailDelegate OnBombGameFail;

	UPROPERTY()
	TSoftObjectPtr<ASpiderManTargetPoint> CurStartPoint;

	UPROPERTY()
	TSoftObjectPtr<ASpiderManTargetPoint> CurEndPoint;

public:
    // 实现接口方法
    virtual FOnBombGameStartDelegate& GetOnBombGameStartDelegate() override { return OnBombGameStart; }
    virtual FOnBombGameCompleteDelegate& GetOnBombGameCompleteDelegate() override { return OnBombGameComplete; }
    virtual FOnBombGameFailDelegate& GetOnBombGameFailDelegate() override { return OnBombGameFail; }
    
private:
#if WITH_EDITOR
    // 编辑器中属性变更时调用
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	// 游戏开始或生成时调用
	virtual void BeginPlay() override;
	// 每帧调用
	virtual void Tick(float DeltaTime) override;

private:
	// 根据ID查找对应的TargetPoint
	ASpiderManTargetPoint* FindTargetPointByID(int32 ID) const;
	
	// 绘制调试路径
	void DrawDebugPath();

	// 绘制所有目标点之间的连线
	void DrawAllPointDebugPath();

	// 检查当前提示点
	void CheckAndUpdatePathTips();
	
	// 隐藏所有提示点
	void HideAllPathTips();
	
	// 更新栈顶提示点显示
	void UpdateStackTopTip();

	// 绑定 HUD 事件的内部方法
	void BindHUDEventsIfNeeded();

	void BindBombTimeUIEventsIfNeeded();

	// 检查玩家是否接近起点
	bool IsPlayerNearStartPoint();

	// 检查玩家是否接近终点
	bool IsPlayerNearEndPoint();

	// 设置起点目标点
	void SetStartPoint();

	// 设置终点目标点
	void SetEndPoint();

	// 更新目标点的起点/终点状态
	void UpdateTargetPointsState();

	// 更新路径点的提示状态
	void UpdatePathPointsTips();

	// 计算并设置从起点到终点的最短路径
	void ComputeShortestPath();

private:
	// 路径点提示栈，LIFO(后进先出)
	UPROPERTY()
	TArray<int32> TipsPointsStack;

	UPROPERTY()
	TSet<int32> ResultPoints;

	// 跟踪是否已触发起点/终点事件，防止重复触发
	bool bHasTriggeredStartEvent = false;

	bool bHasTriggeredEndEvent = false;

	bool bHasTriggeredFailEvent = false;

	// 追踪是否已绑定 HUD 事件
	bool bHasBindedHUDEvents = false;

	bool bHasBindedBombTimeUIEvents = false;
}; 