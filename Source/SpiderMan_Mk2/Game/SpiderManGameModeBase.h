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
#include "GameFramework/GameModeBase.h"
#include "SpiderManGameModeBase.generated.h"

class UWG_HUD;

/**
 * 蜘蛛侠游戏模式基类
 */
UCLASS()
class SPIDERMAN_MK2_API ASpiderManGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ASpiderManGameModeBase();

	// 主界面UI的类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UWG_HUD> MainUIClass;

public:
	// 获取当前的 UWG_HUD 实例
	UFUNCTION(BlueprintCallable, Category = "UI")
	class UWG_HUD* GetMainHUD();

protected:
	virtual void BeginPlay() override;
	
	// 添加 EndPlay 方法来清理单例
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// 创建并显示主界面UI
	void CreateAndShowMainUI();

private:
	// 主界面UI实例的引用
	UPROPERTY()
	TObjectPtr<UWG_HUD> MainUI;
}; 