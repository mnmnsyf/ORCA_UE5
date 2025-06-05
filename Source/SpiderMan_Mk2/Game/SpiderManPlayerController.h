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
#include "GameFramework/PlayerController.h"
#include "SpiderManPlayerController.generated.h"

class UWG_HUD;
class ADetectionRangeActor;

/**
 * 蜘蛛侠玩家控制器
 */
UCLASS()
class SPIDERMAN_MK2_API ASpiderManPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ASpiderManPlayerController();

	// 检测范围Actor类
	UPROPERTY(EditDefaultsOnly, Category = "Detection")
	TSubclassOf<ADetectionRangeActor> DetectionRangeActorClass;

	// 修改检测范围 (单位：米)
	UPROPERTY(EditDefaultsOnly, Category = "Detection", meta = (ClampMin = "1.0"))
	float DetectionRangeInMeters = 100.0f;

public:
	// 切换UI和游戏输入模式
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ToggleUIMode();
	
	// 开始侦察（按下按键时调用）
	UFUNCTION(BlueprintCallable, Category = "Input")
	void StartDetect();
	
	// 结束侦察（松开按键时调用）
	UFUNCTION(BlueprintCallable, Category = "Input")
	void StopDetect();

	// 开始侦察（按下按键时调用）
	UFUNCTION(BlueprintImplementableEvent)
	void StartDetect_BP();

	// 结束侦察（松开按键时调用）
	UFUNCTION(BlueprintImplementableEvent)
	void StopDetect_BP();

	// 获取UI参考
	void SetMainUI(UWG_HUD* InMainUI) { MainUI = InMainUI; }

protected:
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	// 覆盖Tick函数，用于持续更新检测
	virtual void Tick(float DeltaTime) override;
private:
	// 检测敌人
	void UpdateEnemyDetection();

	// 清除敌人检测状态
	void ClearEnemyDetection();

private:
	// 是否处于UI模式
	bool bIsInUIMode;

	bool bInDetect = false;

	// 主界面UI的引用
	UPROPERTY()
	TObjectPtr<UWG_HUD> MainUI;
	
	// 检测范围可视化Actor
	UPROPERTY()
	TObjectPtr<ADetectionRangeActor> DetectionRangeActor;
}; 