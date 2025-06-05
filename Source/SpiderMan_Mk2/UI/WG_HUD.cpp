// Fill out your copyright notice in the Description page of Project Settings.

#include "WG_HUD.h"
#include "Components/CheckBox.h"
#include "EngineUtils.h"
#include "SpiderMan_Mk2/Actor/Bomb/IBombGameInterface.h"
#include "BombRule.h"

void UWG_HUD::OnEnterUIMode()
{
	OnEnterUIMode_BP();
	
	// 进入UI模式时更新复选框状态
	UpdateCheckBoxStates();
}

void UWG_HUD::OnOutUIMode()
{
	OnOutUIMode_BP();
}

void UWG_HUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 绑定复选框状态变更事件
	if (Bt_Debug_All)
	{
		Bt_Debug_All->OnCheckStateChanged.AddDynamic(this, &UWG_HUD::OnDebugAllCheckStateChanged);
	}
	
	if (Bt_Debug_Result)
	{
		Bt_Debug_Result->OnCheckStateChanged.AddDynamic(this, &UWG_HUD::OnDebugResultCheckStateChanged);
	}
	
	// 初始化时更新复选框状态
	UpdateCheckBoxStates();
}

// 更新复选框状态
void UWG_HUD::UpdateCheckBoxStates()
{
	// 查找第一个BombRule作为参考
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<ABombRule> It(World); It; ++It)
		{
			ABombRule* Rule = *It;
			if (Rule)
			{
				// 根据BombRule的状态更新复选框
				if (Bt_Debug_All)
				{
					Bt_Debug_All->SetCheckedState(Rule->IsAllPointsDebugEnabled() ? 
						ECheckBoxState::Checked : ECheckBoxState::Unchecked);
				}
				
				if (Bt_Debug_Result)
				{
					Bt_Debug_Result->SetCheckedState(Rule->IsResultPathDebugEnabled() ? 
						ECheckBoxState::Checked : ECheckBoxState::Unchecked);
				}
				
				// 只需要第一个BombRule即可
				break;
			}
		}
	}
}

// 复选框事件处理函数
void UWG_HUD::OnDebugAllCheckStateChanged(bool bIsChecked)
{
	// 查找所有BombRule并切换全部点的调试状态
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<ABombRule> It(World); It; ++It)
		{
			ABombRule* Rule = *It;
			if (Rule)
			{
				// 直接设置状态而不是切换
				Rule->SetAllPointsDebug(bIsChecked);
			}
		}
	}
}

void UWG_HUD::OnDebugResultCheckStateChanged(bool bIsChecked)
{
	// 查找所有BombRule并切换结果路径的调试状态
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<ABombRule> It(World); It; ++It)
		{
			ABombRule* Rule = *It;
			if (Rule)
			{
				// 直接设置状态而不是切换
				Rule->SetResultPathDebug(bIsChecked);
			}
		}
	}
}

void UWG_HUD::NativeDestruct()
{
	Super::NativeDestruct();
}

// 绑定炸弹规则事件
void UWG_HUD::BindBombGameEvents(UObject* BombGameObject)
{
	// 检查对象是否实现了接口
	IBombGameInterface* BombGame = Cast<IBombGameInterface>(BombGameObject);
	if (!BombGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("WG_HUD: 传入的对象未实现 IBombGameInterface"));
		return;
	}
	
	// 先解绑之前的事件（如果有）
	if (CurrentBombGame.GetObject())
	{
		IBombGameInterface* PrevGame = Cast<IBombGameInterface>(CurrentBombGame.GetObject());
		if (PrevGame)
		{
			PrevGame->GetOnBombGameStartDelegate().RemoveDynamic(this, &UWG_HUD::OnBombGameStart);
			PrevGame->GetOnBombGameCompleteDelegate().RemoveDynamic(this, &UWG_HUD::OnBombGameComplete);
			PrevGame->GetOnBombGameFailDelegate().RemoveDynamic(this, &UWG_HUD::OnBombGameFail);
		}
	}
	
	// 保存新的炸弹游戏接口
	CurrentBombGame.SetObject(BombGameObject);
	CurrentBombGame.SetInterface(BombGame);
	
	// 绑定事件
	BombGame->GetOnBombGameStartDelegate().AddDynamic(this, &UWG_HUD::OnBombGameStart);
	BombGame->GetOnBombGameCompleteDelegate().AddDynamic(this, &UWG_HUD::OnBombGameComplete);
	BombGame->GetOnBombGameFailDelegate().AddDynamic(this, &UWG_HUD::OnBombGameFail);
	
	UE_LOG(LogTemp, Log, TEXT("WG_HUD: 已绑定炸弹游戏事件"));
}

// 游戏开始事件处理
void UWG_HUD::OnBombGameStart()
{
	UE_LOG(LogTemp, Log, TEXT("WG_HUD: 收到炸弹游戏开始事件"));
	
	// 调用蓝图实现的显示提示方法
	ShowStartBombGameTips();
}

// 游戏完成事件处理
void UWG_HUD::OnBombGameComplete()
{
	UE_LOG(LogTemp, Log, TEXT("WG_HUD: 收到炸弹游戏完成事件"));
	
	// 调用蓝图实现的显示提示方法
	ShowCompleteBombGameTips();
}

// 添加游戏失败事件处理方法
void UWG_HUD::OnBombGameFail()
{
	UE_LOG(LogTemp, Log, TEXT("WG_HUD: 收到炸弹游戏失败事件"));
	
	// 调用蓝图实现的显示提示方法
	ShowFailBombGameTips();
}
