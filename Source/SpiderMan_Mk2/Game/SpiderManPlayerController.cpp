// Fill out your copyright notice in the Description page of Project Settings.

#include "SpiderManPlayerController.h"
#include "UI/WG_HUD.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyCharacter.h"  // 假设这是敌人类的正确路径
#include "Actor/DetectionRangeActor.h"

ASpiderManPlayerController::ASpiderManPlayerController()
{
	bIsInUIMode = false;
}

void ASpiderManPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// 绑定N键切换UI模式
	InputComponent->BindAction("ToggleUI", IE_Pressed, this, &ASpiderManPlayerController::ToggleUIMode);
	
	// 修改为按下和释放事件
	InputComponent->BindAction("Detect", IE_Pressed, this, &ASpiderManPlayerController::StartDetect);
	InputComponent->BindAction("Detect", IE_Released, this, &ASpiderManPlayerController::StopDetect);
}

void ASpiderManPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 默认设置为游戏输入模式
	bIsInUIMode = false;
	
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void ASpiderManPlayerController::ToggleUIMode()
{
	// 切换UI模式状态
	bIsInUIMode = !bIsInUIMode;
	
	if (bIsInUIMode)
	{
		// 切换到UI和游戏混合模式，而不是纯UI模式
		// 这样即使显示UI时也能接收到游戏按键输入
		FInputModeGameAndUI InputMode;
		if (MainUI)
		{
			InputMode.SetWidgetToFocus(MainUI->TakeWidget());
			MainUI->OnEnterUIMode();
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		
		// 显示鼠标
		bShowMouseCursor = true;
		
		// 禁用玩家角色输入
		if (GetPawn())
		{
			GetPawn()->DisableInput(this);
			UE_LOG(LogTemp, Display, TEXT("已禁用角色输入"));
		}
		
		UE_LOG(LogTemp, Display, TEXT("切换到UI模式 (GameAndUI)"));
	}
	else
	{
		// 切换回游戏模式
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		
		// 隐藏鼠标
		bShowMouseCursor = false;
		
		// 重新启用玩家角色输入
		if (GetPawn())
		{
			GetPawn()->EnableInput(this);
			UE_LOG(LogTemp, Display, TEXT("已启用角色输入"));
		}
		if (MainUI)
		{
			MainUI->OnOutUIMode();
		}
		UE_LOG(LogTemp, Display, TEXT("切换到游戏模式"));
	}
}

void ASpiderManPlayerController::StartDetect()
{
	// 只在非检测状态时处理
	if (!bInDetect)
	{
		StartDetect_BP();
		bInDetect = true;
		
		// 创建检测范围可视化
		if (!DetectionRangeActor && DetectionRangeActorClass)
		{
			// 获取玩家位置
			FVector CurSpawnLocation = GetPawn() ? GetPawn()->GetActorLocation() : GetSpawnLocation();
			
			// 生成检测范围Actor
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = this;
			
			DetectionRangeActor = GetWorld()->SpawnActor<ADetectionRangeActor>(
				DetectionRangeActorClass, 
				CurSpawnLocation,
				FRotator::ZeroRotator,
				SpawnParams
			);
			
			// 设置拥有者控制器（用于跟随）
			if (DetectionRangeActor)
			{
				DetectionRangeActor->SetOwnerController(this);
				
				// 设置检测范围Actor的最大比例以匹配我们的检测范围
				DetectionRangeActor->SetMaxScale(DetectionRangeInMeters);
				
				UE_LOG(LogTemp, Display, TEXT("创建检测范围可视化"));
			}
		}
	}
}

void ASpiderManPlayerController::StopDetect()
{
	// 只在检测状态时处理
	if (bInDetect)
	{
		StopDetect_BP();
		bInDetect = false;
		
		// 关闭检测模式，销毁检测范围可视化
		if (DetectionRangeActor)
		{
			DetectionRangeActor->Destroy();
			DetectionRangeActor = nullptr;
			UE_LOG(LogTemp, Display, TEXT("销毁检测范围可视化"));
		}
		
		// 清除所有敌人的检测状态
		ClearEnemyDetection();
	}
}

void ASpiderManPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 如果处于检测状态，持续更新敌人检测状态
	if (bInDetect)
	{
		UpdateEnemyDetection();
	}
}

void ASpiderManPlayerController::UpdateEnemyDetection()
{
	// 搜索场上所有距离玩家定义范围内的敌人，更新检测状态
	APawn* PlayerPawn = GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	// 获取玩家位置
	FVector PlayerLocation = PlayerPawn->GetActorLocation();

	// 搜索范围（厘米）
	float DetectionRange = DetectionRangeInMeters * 100.0f; // 转换为厘米
	
	// 获取所有敌人
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWraithCharacter::StaticClass(), FoundEnemies);

	// 遍历所有敌人并更新其检测状态
	for (AActor* Actor : FoundEnemies)
	{
		AWraithCharacter* Enemy = Cast<AWraithCharacter>(Actor);
		if (Enemy)
		{
			// 使用3D距离计算
			FVector EnemyLocation = Enemy->GetActorLocation();
			float Distance3D = FVector::Distance(PlayerLocation, EnemyLocation);
			
			// 根据距离设置敌人的检测状态
			bool bInRange = (Distance3D <= DetectionRange);
			Enemy->SetEnemyBeDetected(bInRange);
		}
	}
}

void ASpiderManPlayerController::ClearEnemyDetection()
{
	// 获取所有AWraithCharacter并重置检测状态
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWraithCharacter::StaticClass(), FoundEnemies);

	int32 ClearedCount = 0;
	for (AActor* Actor : FoundEnemies)
	{
		AWraithCharacter* Enemy = Cast<AWraithCharacter>(Actor);
		if (Enemy)
		{
			Enemy->SetEnemyBeDetected(false);
			ClearedCount++;
		}
	}

	UE_LOG(LogTemp, Display, TEXT("检测结束: 清除 %d 个敌人的检测状态"), ClearedCount);
}
