// Fill out your copyright notice in the Description page of Project Settings.

#include "SpiderManGameModeBase.h"
#include "UI/WG_HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "SpiderManPlayerController.h"
#include "SpiderManTargetPointManager.h"
#include "SpiderManGraphManager.h"

ASpiderManGameModeBase::ASpiderManGameModeBase()
{
	// 设置默认值
	
	// 设置默认使用我们的玩家控制器
	PlayerControllerClass = ASpiderManPlayerController::StaticClass();
}

void ASpiderManGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 游戏开始时创建并显示主界面
	CreateAndShowMainUI();

	// 初始化目标点管理器
	USpiderManTargetPointManager::GetInstance()->Initialize(GetWorld());

	// 可选：输出调试信息
	USpiderManTargetPointManager::GetInstance()->DebugPrintAllTargetPoints();
}

void ASpiderManGameModeBase::CreateAndShowMainUI()
{
	// 检查是否已设置UI类
	if (!MainUIClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("未设置主界面UI类，无法创建UI"));
		return;
	}
	
	// 检查UI是否已创建
	if (MainUI)
	{
		// 如果已创建，只需显示
		MainUI->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		return;
	}
	
	// 获取本地玩家控制器
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("无法获取玩家控制器，UI创建失败"));
		return;
	}
	
	// 创建主界面UI
	MainUI = CreateWidget<UWG_HUD>(PC, MainUIClass);
	if (MainUI)
	{
		// 将UI添加到视口
		MainUI->AddToViewport();
		
		// 如果是我们自定义的控制器，传递UI引用
		ASpiderManPlayerController* SpiderManPC = Cast<ASpiderManPlayerController>(PC);
		if (SpiderManPC)
		{
			SpiderManPC->SetMainUI(MainUI);
		}
		
		UE_LOG(LogTemp, Display, TEXT("主界面UI创建成功并添加到视口"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("主界面UI创建失败"));
	}
}

UWG_HUD* ASpiderManGameModeBase::GetMainHUD()
{
	return MainUI;
}

void ASpiderManGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 销毁单例管理器
	USpiderManGraphManager::DestroyInstance();
	USpiderManTargetPointManager::DestroyInstance();
	
	UE_LOG(LogTemp, Log, TEXT("SpiderManGameModeBase: 已销毁图算法管理器和目标点管理器单例"));
	
	Super::EndPlay(EndPlayReason);
} 