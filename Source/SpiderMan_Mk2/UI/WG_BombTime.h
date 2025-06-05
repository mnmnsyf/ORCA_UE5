//*******************************************************************************
//																				//
//	Project Name:			WG_BombTime										//
//	Author:					Wangzhongpeng									//
//	Completion Date:												//
//	Copyright Declaration:									//
//																				//
//																				//
//*******************************************************************************

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Actor/Bomb/IBombGameInterface.h"
#include "EngineUtils.h"
#include "Actor/Bomb/BombRule.h"
#include "WG_BombTime.generated.h"


UCLASS()
class SPIDERMAN_MK2_API UWG_BombTime : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> T_Time;

	UPROPERTY(EditAnywhere, Category = "BombTime", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float Time = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurLeftTime = Time;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString DecreaseTimeStr;
public:
	UFUNCTION(BlueprintImplementableEvent)
	void PlayDecTimeAnim();

	UFUNCTION(BlueprintCallable)
	FString GetCurDecreaseTimeStr() {
		return DecreaseTimeStr;
	}
public:
	// 绑定炸弹游戏事件
	void BindBombGameEvents(UObject* BombGameObject);

	// 获取当前倒计时
	float GetCurLeftTime() const { return CurLeftTime; }

	// 设置当前倒计时
	void SetCurLeftTime(float NewTime);

protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
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
private:
	// 当前绑定的炸弹游戏接口
	UPROPERTY()
	TScriptInterface<IBombGameInterface> CurrentBombGame;

	// 是否正在运行
	bool bIsRunning = true;

	// 设置是否可以倒计时
	bool bCanTick = false;

};
