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
#include "Blueprint/UserWidget.h"
#include "Actor/Bomb/IBombGameInterface.h"
#include "WG_HUD.generated.h"

/**
 * 游戏主界面UI基类
 */
UCLASS()
class SPIDERMAN_MK2_API UWG_HUD : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<class UCheckBox> Bt_Debug_All;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<class UCheckBox> Bt_Debug_Result;
public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnEnterUIMode_BP();

	UFUNCTION(BlueprintImplementableEvent)
	void OnOutUIMode_BP();

	// 显示炸弹游戏开始提示（蓝图实现）
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Game")
	void ShowStartBombGameTips();

	// 显示炸弹游戏完成提示（蓝图实现）
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Game")
	void ShowCompleteBombGameTips();

	// 显示炸弹游戏失败提示（蓝图实现）
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Game")
	void ShowFailBombGameTips();

	// 修改事件处理函数
	UFUNCTION()
	void OnDebugAllCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnDebugResultCheckStateChanged(bool bIsChecked);

public:
	
	void OnEnterUIMode();

	void OnOutUIMode();

	// 更新复选框状态的函数
	void UpdateCheckBoxStates();

	// 绑定炸弹游戏事件 - 使用接口类型
	void BindBombGameEvents(UObject* BombGameObject);

private:
	// 游戏开始事件处理
	UFUNCTION()
	void OnBombGameStart();
	
	// 游戏完成事件处理
	UFUNCTION()
	void OnBombGameComplete();
	
	// 游戏失败事件处理
	UFUNCTION()
	void OnBombGameFail();

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

private:
	// 当前绑定的炸弹游戏接口
	UPROPERTY()
	TScriptInterface<IBombGameInterface> CurrentBombGame;
}; 