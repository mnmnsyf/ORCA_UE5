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
#include "GenerateActorBase.h"
#include "GenerateVehicle.generated.h"

// 定义运动方向枚举
UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	Clockwise        UMETA(DisplayName = "顺时针"),
	CounterClockwise UMETA(DisplayName = "逆时针")
};

// 前向声明
class ATransportVehicleActor;
class ASplineActor;

UCLASS()
class SPIDERMAN_MK2_API AGenerateVehicle : public AGenerateActorBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGenerateVehicle();

	// 运动方向配置（顺时针/逆时针）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transport")
	EMovementDirection MovementDirection = EMovementDirection::Clockwise;

	// 车辆速度(厘米/秒)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transport", meta = (ClampMin = "0.0"))
	float VehicleSpeed = 300.0f;

	// 所跟随的Spline Actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transport")
	TSoftObjectPtr<ASplineActor> SplineActor;
protected:
	//生成逻辑
	virtual void StartSpawning() override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// Called every frame
	virtual void Tick(float DeltaTime) override; 

private:
	// 存储生成的所有车辆
	UPROPERTY()
	TArray<TObjectPtr<ATransportVehicleActor>> SpawnedVehicles;
};
