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
#include "SpiderManTargetPoint.generated.h"

/**
 * 蜘蛛侠游戏中的目标点Actor
 * 可以用于标记特定位置、导航点或交互位置
 */
UCLASS()
class SPIDERMAN_MK2_API ASpiderManTargetPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// 设置默认值
	ASpiderManTargetPoint();
public:
	// 是否为炸弹路径的起点
	UPROPERTY(VisibleAnywhere, Category = "Target Point")
	bool bIsStart = false;

	// 是否为炸弹路径的终点
	UPROPERTY(VisibleAnywhere, Category = "Target Point")
	bool bIsEnd = false;

	// 目标点唯一标识符
	UPROPERTY(VisibleAnywhere, Category = "Target Point")
	int32 PointID;
public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetTipsState(bool bShow);
	
	UFUNCTION(BlueprintImplementableEvent)
	void RefreshStartEndState();

	UFUNCTION(BlueprintImplementableEvent)
	FVector GetTipActorLocation();

	// 获取是否为起点
	UFUNCTION(BlueprintCallable, Category = "Target Point")
	bool IsStart() const { return bIsStart; }
	
	// 获取是否为终点
	UFUNCTION(BlueprintCallable, Category = "Target Point")
	bool IsEnd() const { return bIsEnd; }
    	
	UFUNCTION(BlueprintImplementableEvent)
	class UWG_BombTime* GetBombTimeUI();

protected:
	// 每帧调用
	virtual void Tick(float DeltaTime) override;

	// 游戏开始或生成时调用
	virtual void BeginPlay() override;

	// 游戏结束或销毁时调用
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeUniqueID();
public:	
	// 设置是否为起点
	void SetIsStart(bool bNewIsStart);

	// 设置是否为终点
	void SetIsEnd(bool bNewIsEnd);

	// 获取位置标识符
	int32 GetPointID() const { return PointID; }

private:
	bool bHasInitializedID = false;
}; 