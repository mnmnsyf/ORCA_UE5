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
#include "DetectionRangeActor.generated.h"

UCLASS()
class SPIDERMAN_MK2_API ADetectionRangeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADetectionRangeActor();

	// 最小缩放比例
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.1"))
	float MinScale = 1.0f;

	// 最大缩放比例
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "1.0"))
	float MaxScale = 100.0f;

	// 脉冲速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.1"))
	float PulseSpeed = 1.0f;

public:
	// 设置所有者控制器
	void SetOwnerController(APlayerController* InController) { OwnerController = InController; }

	// 设置最大缩放值
	void SetMaxScale(float NewMaxScale) { MaxScale = NewMaxScale; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// 更新大小变化
	void UpdateScale();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	// 当前时间
	float CurrentTime = 0.0f;

	// 所有者玩家控制器
	UPROPERTY()
	TObjectPtr<APlayerController> OwnerController;
}; 