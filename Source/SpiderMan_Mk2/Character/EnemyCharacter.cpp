// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "SplineActor.h"
#include "DrawDebugHelpers.h"
#include "Components/SplineComponent.h"
#include <MathAlgorithm.h>
#include <CustomMovementComponent.h>

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCustomMovementComponent()->SetCalVelocityMode(ECalVelocityMode::CustomVelocity);
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

AWraithCharacter::AWraithCharacter()
{
	// 确保每帧调用 Tick
	PrimaryActorTick.bCanEverTick = true;
}

AWraithCharacter::AWraithCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AWraithCharacter::Initialize(ASplineActor* InVehicleActor)
{
	FollowedSplineActor = InVehicleActor;
}

// 重写 BeginPlay 以初始化随机移动
void AWraithCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 初始化移动方向和速度
	CurrentMoveDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
	CurrentMoveSpeed = FMath::RandRange(MinMoveSpeed, MaxMoveSpeed);
	
	// 设置定时器随机改变方向
	GetWorldTimerManager().SetTimer(
		DirectionChangeTimer, 
		this, 
		&AWraithCharacter::ChangeRandomDirection, 
		DirectionChangeInterval, 
		true
	);
}

// 修改 Tick 函数实现智能移动
void AWraithCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 调试绘制
	if (bShowSplineDistanceDebug && FollowedSplineActor)
	{
		DrawSplineDistanceDebug();
	}
	
	// 更新随机移动
	UpdateRandomMovement(DeltaTime);
}

// 更新随机移动逻辑
void AWraithCharacter::UpdateRandomMovement(float DeltaTime)
{
	if (!FollowedSplineActor)
	{
		return;
	}
	
	// 检查是否离 Spline 太远
	if (IsTooFarFromSpline())
	{
		// 如果太远，则向 Spline 移动
		CurrentMoveDirection = FMath::VInterpTo(
			CurrentMoveDirection,
			GetDirectionTowardSpline(),
			DeltaTime,
			MovementInterpSpeed * 2.0f  // 加快向 Spline 靠近的速度
		);
	}
	// 检查是否离 Spline 太近
	else if (IsTooCloseToSpline())
	{
		// 如果太近，则远离 Spline
		CurrentMoveDirection = FMath::VInterpTo(
			CurrentMoveDirection,
			GetDirectionAwayFromSpline(),
			DeltaTime,
			MovementInterpSpeed * 1.5f  // 中等速度远离
		);
	}
	
	// 更新移动和旋转
	UpdateMovementAndRotation(DeltaTime);
}

// 随机改变移动方向
void AWraithCharacter::ChangeRandomDirection()
{
	// 仅在距离合适时随机改变方向
	if (!IsTooFarFromSpline() && !IsTooCloseToSpline())
	{
		FVector NewDirection = CalculateNewMoveDirection();
		
		// 平滑过渡到新方向
		CurrentMoveDirection = FMath::VInterpTo(
			CurrentMoveDirection,
			NewDirection,
			0.1f,  // 小增量以开始过渡
			MovementInterpSpeed
		);
		
		// 随机改变速度
		CurrentMoveSpeed = FMath::RandRange(MinMoveSpeed, MaxMoveSpeed);
	}
}

// 计算新的随机移动方向
FVector AWraithCharacter::CalculateNewMoveDirection()
{
	// 生成水平面内的随机方向
	FVector RandomDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f);
	return RandomDirection.GetSafeNormal();
}

// 检查是否距离 Spline 过远
bool AWraithCharacter::IsTooFarFromSpline()
{
	if (!FollowedSplineActor)
	{
		return false;
	}
	
	// 获取样条组件
	USplineComponent* SplineComp = FollowedSplineActor->FindComponentByClass<USplineComponent>();
	if (!SplineComp)
	{
		return false;
	}
	
	// 获取当前位置
	FVector CurrentLocation = GetActorLocation();
	
	// 找到样条上最近的点
	float DisToSpline;
	float RetPointSplineInputKey;
	FVector ClosestPoint = FMathAlgorithm::FindNearestPointAlongSplineFromPointSquared(
		SplineComp, 
		CurrentLocation, 
		DisToSpline, 
		RetPointSplineInputKey
	);
	
	// 计算距离
	float Distance = FVector::Distance(CurrentLocation, ClosestPoint);
	
	// 如果距离大于最大允许距离，返回真
	return Distance > MaxDistanceToSpline;
}

// 获取朝向 Spline 的方向
FVector AWraithCharacter::GetDirectionTowardSpline()
{
	if (!FollowedSplineActor)
	{
		return FVector::ZeroVector;
	}
	
	// 获取样条组件
	USplineComponent* SplineComp = FollowedSplineActor->FindComponentByClass<USplineComponent>();
	if (!SplineComp)
	{
		return FVector::ZeroVector;
	}
	
	// 获取当前位置
	FVector CurrentLocation = GetActorLocation();
	
	// 找到样条上最近的点
	float DisToSpline;
	float RetPointSplineInputKey;
	FVector ClosestPoint = FMathAlgorithm::FindNearestPointAlongSplineFromPointSquared(
		SplineComp, 
		CurrentLocation, 
		DisToSpline, 
		RetPointSplineInputKey
	);
	
	// 计算朝向 Spline 的方向向量
	FVector DirectionToSpline = (ClosestPoint - CurrentLocation).GetSafeNormal();
	DirectionToSpline.Z = 0.0f;  // 保持在水平面内
	
	return DirectionToSpline;
}

// 检查是否距离 Spline 过近
bool AWraithCharacter::IsTooCloseToSpline()
{
	if (!FollowedSplineActor)
	{
		return false;
	}
	
	// 获取样条组件
	USplineComponent* SplineComp = FollowedSplineActor->FindComponentByClass<USplineComponent>();
	if (!SplineComp)
	{
		return false;
	}
	
	// 获取当前位置
	FVector CurrentLocation = GetActorLocation();
	
	// 找到样条上最近的点
	float DisToSpline;
	float RetPointSplineInputKey;
	FVector ClosestPoint = FMathAlgorithm::FindNearestPointAlongSplineFromPointSquared(
		SplineComp, 
		CurrentLocation, 
		DisToSpline, 
		RetPointSplineInputKey
	);
	
	// 计算距离
	float Distance = FVector::Distance(CurrentLocation, ClosestPoint);
	
	// 如果距离小于最小允许距离，返回真
	return Distance < MinDistanceToSpline;
}

// 获取远离 Spline 的方向
FVector AWraithCharacter::GetDirectionAwayFromSpline()
{
	if (!FollowedSplineActor)
	{
		return FVector::ZeroVector;
	}
	
	// 获取样条组件
	USplineComponent* SplineComp = FollowedSplineActor->FindComponentByClass<USplineComponent>();
	if (!SplineComp)
	{
		return FVector::ZeroVector;
	}
	
	// 获取当前位置
	FVector CurrentLocation = GetActorLocation();
	
	// 找到样条上最近的点
	float DisToSpline;
	float RetPointSplineInputKey;
	FVector ClosestPoint = FMathAlgorithm::FindNearestPointAlongSplineFromPointSquared(
		SplineComp, 
		CurrentLocation, 
		DisToSpline, 
		RetPointSplineInputKey
	);
	
	// 计算远离 Spline 的方向向量 (从最近点指向当前位置)
	FVector DirectionAwayFromSpline = (CurrentLocation - ClosestPoint).GetSafeNormal();
	DirectionAwayFromSpline.Z = 0.0f;  // 保持在水平面内
	
	return DirectionAwayFromSpline;
}

// 更新移动和旋转
void AWraithCharacter::UpdateMovementAndRotation(float DeltaTime)
{
	// 确保方向向量是规范化的
	CurrentMoveDirection = CurrentMoveDirection.GetSafeNormal();
	
	// 设置自定义速度
	FVector Velocity = CurrentMoveDirection * CurrentMoveSpeed;
	GetCustomMovementComponent()->SetCustomVelocity(Velocity);
	
	// 计算角色应该面向的旋转
	FRotator TargetRotation = CurrentMoveDirection.Rotation();
	
	// 平滑地旋转角色
	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		RotationInterpSpeed
	);
	
	// 只关心 Yaw 轴的旋转(水平旋转)
	NewRotation.Pitch = CurrentRotation.Pitch;
	NewRotation.Roll = CurrentRotation.Roll;
	
	// 应用新旋转
	SetActorRotation(NewRotation);
}

// 实现调试绘制函数
void AWraithCharacter::DrawSplineDistanceDebug()
{
	if (!FollowedSplineActor)
	{
		return;
	}
	
	// 获取样条组件
	USplineComponent* SplineComp = FollowedSplineActor->FindComponentByClass<USplineComponent>();
	if (!SplineComp)
	{
		return;
	}
	
	// 获取当前位置
	FVector CurrentLocation = GetActorLocation();
	
	// 找到样条上最近的点
	float DisToSpline;
	float RetPointSplineInputKey;
	FVector ClosestPoint = FMathAlgorithm::FindNearestPointAlongSplineFromPointSquared(SplineComp, CurrentLocation, DisToSpline, RetPointSplineInputKey);
	
	// 计算距离
	float Distance = FVector::Distance(CurrentLocation, ClosestPoint);
	
	// 计算连线中点位置（用于显示文本）
	FVector MidPoint = (CurrentLocation + ClosestPoint) * 0.5f;
	
	// 绘制连线
	DrawDebugLine(
		GetWorld(),
		CurrentLocation,
		ClosestPoint,
		DebugLineColor,
		false, // 持续性
		-1.0f, // 持续时间（负值表示单帧）
		0, // DepthPriority
		DebugLineThickness // 线条粗细
	);
	
	// 在中点绘制距离文本
	FString DistanceText = FString::Printf(TEXT("距离: %.2f\n键值: %.2f"), Distance, RetPointSplineInputKey);
	DrawDebugString(
		GetWorld(),
		MidPoint,
		DistanceText,
		nullptr, // Actor
		DebugTextColor,
		0.0f, // 持续时间
		true, // 背景描边
		DebugTextSize // 文本大小
	);
	
	// 额外：在最近点绘制一个小球，便于观察
	DrawDebugSphere(
		GetWorld(),
		ClosestPoint,
		10.0f, // 半径
		8, // 分段数
		FColor::Red,
		false,
		-1.0f,
		0,
		2.0f
	);
}
