#include "WG_BombTime.h"
#include "Components/TextBlock.h"
#include "Actor/Bomb/BombRule.h"
#include "EngineUtils.h"

void UWG_BombTime::NativeConstruct()
{
	Super::NativeConstruct();
	CurLeftTime = Time;
}

void UWG_BombTime::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!bCanTick)
		return;
		
	if (CurLeftTime <= .0f)
	{
		// 游戏失败
		if (bIsRunning)
		{
			bIsRunning = false; // 停止计时
			
			// 如果已绑定BombGame则直接触发失败事件
			if (CurrentBombGame.GetObject())
			{
				IBombGameInterface* BombGame = Cast<IBombGameInterface>(CurrentBombGame.GetObject());
				if (BombGame)
				{
					// 找到BombRule并调用其失败方法
					ABombRule* BombRule = Cast<ABombRule>(CurrentBombGame.GetObject());
					if (BombRule)
					{
						BombRule->OnGameFailed();
						UE_LOG(LogTemp, Log, TEXT("BombTime: 触发炸弹游戏失败事件"));
					}
				}
			}
		}
	}
	else 
	{
		CurLeftTime -= InDeltaTime;
	}
	
	if (T_Time)
	{
		FString TimeStr = FString::Printf(TEXT("%.2f"), CurLeftTime > 0 ? CurLeftTime : 0);
		T_Time->SetText(FText::FromString(TimeStr));
	}
}

void UWG_BombTime::OnBombGameStart()
{
	bCanTick = true;
}

void UWG_BombTime::OnBombGameComplete()
{
	bCanTick = false;
}

void UWG_BombTime::OnBombGameFail()
{
	bCanTick = false;
}

// 实现绑定方法
void UWG_BombTime::BindBombGameEvents(UObject* BombGameObject)
{
	// 检查对象是否实现了接口
	IBombGameInterface* BombGame = Cast<IBombGameInterface>(BombGameObject);
	if (!BombGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("WG_BombTime: 传入的对象未实现 IBombGameInterface"));
		return;
	}

	// 先解绑之前的事件（如果有）
	if (CurrentBombGame.GetObject())
	{
		IBombGameInterface* PrevGame = Cast<IBombGameInterface>(CurrentBombGame.GetObject());
		if (PrevGame)
		{
			PrevGame->GetOnBombGameStartDelegate().RemoveDynamic(this, &UWG_BombTime::OnBombGameStart);
			PrevGame->GetOnBombGameCompleteDelegate().RemoveDynamic(this, &UWG_BombTime::OnBombGameComplete);
			PrevGame->GetOnBombGameFailDelegate().RemoveDynamic(this, &UWG_BombTime::OnBombGameFail);
		}
	}

	// 保存新的炸弹游戏接口
	CurrentBombGame.SetObject(BombGameObject);
	CurrentBombGame.SetInterface(BombGame);

	// 绑定事件
	BombGame->GetOnBombGameStartDelegate().AddDynamic(this, &UWG_BombTime::OnBombGameStart);
	BombGame->GetOnBombGameCompleteDelegate().AddDynamic(this, &UWG_BombTime::OnBombGameComplete);
	BombGame->GetOnBombGameFailDelegate().AddDynamic(this, &UWG_BombTime::OnBombGameFail);

	UE_LOG(LogTemp, Log, TEXT("WG_BombTime: 已绑定炸弹游戏"));
}

// 设置当前剩余时间
void UWG_BombTime::SetCurLeftTime(float NewTime)
{
	CurLeftTime = NewTime;
	UE_LOG(LogTemp, Log, TEXT("BombTime: 更新剩余时间为 %.2f"), CurLeftTime);
	
	// 如果时间被重置为大于0，重新启用倒计时
	if (CurLeftTime > 0.0f && !bIsRunning)
	{
		bIsRunning = true;
		UE_LOG(LogTemp, Log, TEXT("BombTime: 重新启用倒计时"));
	}
}
