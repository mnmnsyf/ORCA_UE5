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
#include "CustomCharacterBase.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class SPIDERMAN_MK2_API AEnemyCharacter : public ACustomCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();
	AEnemyCharacter(const FObjectInitializer& ObjectInitializer);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};

class ASplineActor;

UCLASS()
class SPIDERMAN_MK2_API AWraithCharacter : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWraithCharacter();
	AWraithCharacter(const FObjectInitializer& ObjectInitializer);

public:
	// 是否显示与样条最近点的调试信息
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowSplineDistanceDebug = false;

	// 调试线条颜色
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bShowSplineDistanceDebug"))
	FColor DebugLineColor = FColor::Green;

	// 调试文本颜色
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bShowSplineDistanceDebug"))
	FColor DebugTextColor = FColor::Yellow;

	// 调试线条粗细
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bShowSplineDistanceDebug", ClampMin = "1.0", ClampMax = "10.0"))
	float DebugLineThickness = 2.0f;

	// 调试文本大小
	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bShowSplineDistanceDebug", ClampMin = "0.5", ClampMax = "5.0"))
	float DebugTextSize = 1.5f;

	// 移动相关配置
	UPROPERTY(EditAnywhere, Category = "AI|Movement", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
	float MaxDistanceToSpline = 700.0f;

	// 在移动相关配置中添加最小距离参数
	UPROPERTY(EditAnywhere, Category = "AI|Movement", meta = (ClampMin = "5.0", ClampMax = "500.0"))
	float MinDistanceToSpline = 300.0f;

	UPROPERTY(EditAnywhere, Category = "AI|Movement", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
	float MinMoveSpeed = 100.0f;

	UPROPERTY(EditAnywhere, Category = "AI|Movement", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
	float MaxMoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "AI|Movement", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float DirectionChangeInterval = 2.0f;

	UPROPERTY(EditAnywhere, Category = "AI|Movement", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float MovementInterpSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category = "AI|Movement", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float RotationInterpSpeed = 5.0f;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetEnemyBeDetected(bool bDetected);

public:
	// 初始化函数，设置跟随的Vehicle
	void Initialize(ASplineActor* InVehicleActor);

protected:
	// 重写 BeginPlay 以初始化随机移动
	virtual void BeginPlay() override;
	// 重写Tick函数以添加调试绘制
	virtual void Tick(float DeltaTime) override;

private:
	// 绘制与样条最近点的调试信息
	void DrawSplineDistanceDebug();
	// 内部辅助方法
	void UpdateRandomMovement(float DeltaTime);

	void ChangeRandomDirection();

	FVector CalculateNewMoveDirection();

	FVector GetDirectionTowardSpline();

	FVector GetDirectionAwayFromSpline();

	void UpdateMovementAndRotation(float DeltaTime);

	// 在内部辅助方法中添加新方法
	bool IsTooCloseToSpline();

	bool IsTooFarFromSpline();

private:
	// 所跟随的Spline Actor引用
	UPROPERTY()
	TObjectPtr<ASplineActor> FollowedSplineActor;

	// 当前运动状态
	FVector CurrentMoveDirection;

	float CurrentMoveSpeed;

	FTimerHandle DirectionChangeTimer;
};