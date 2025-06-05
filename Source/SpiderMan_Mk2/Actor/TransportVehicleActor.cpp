#include "TransportVehicleActor.h"
#include "SplineActor.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyCharacter.h"
#include "GenerateVehicle.h"

// ATransportVehicleActor 实现
ATransportVehicleActor::ATransportVehicleActor()
{
    // 确保 Tick 被启用
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics; // 优先级设置为物理更新后
}

void ATransportVehicleActor::BeginPlay()
{
    Super::BeginPlay();
    // 确保在游戏开始时启用 Tick
    SetActorTickEnabled(true);
    
    // 添加调试日志，确认 BeginPlay 被调用
    UE_LOG(LogTemp, Warning, TEXT("TransportVehicleActor BeginPlay 被调用"));
}

void ATransportVehicleActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 如果没有设置 Spline，则不执行
    if (!FollowedSplineActor)
    {
        return;
    }
    
    if (SplineLength <= 0.0f)
    {
        return;
    }
    
    // 获取移动方向和速度
	float CurVehicleSpeed = VehicleSpeed * SpeedMultiplier;  
    // 调用蓝图函数设置车轮速度
	SetWheelSpeed(CurVehicleSpeed);

    bool bIsMovingClockwise = MovementDirection == EMovementDirection::Clockwise;
    
    // 根据方向更新距离
    float DirectionMultiplier = bIsMovingClockwise ? 1.0f : -1.0f;
    CurrentSplineDistance += DirectionMultiplier * VehicleSpeed * DeltaTime;
    
    // 处理循环
    if (CurrentSplineDistance < 0.0f)
    {
        CurrentSplineDistance += SplineLength;
    }
    else if (CurrentSplineDistance > SplineLength)
    {
        CurrentSplineDistance -= SplineLength;
    }
    
    // 更新位置和朝向
    UpdatePositionAndRotation();
}

void ATransportVehicleActor::Initialize(ASplineActor* InSplineActor, float InitialDistance)
{
    FollowedSplineActor = InSplineActor;
    CurrentSplineDistance = InitialDistance;
    
	// 获取 Spline 组件
	USplineComponent* SplineComponent = FollowedSplineActor->GetSplineComponent();
	if (FollowedSplineActor && SplineComponent)
	{
		// 获取Spline总长度
		SplineLength = SplineComponent->GetSplineLength();
	}
	
    // 立即更新位置和朝向
    UpdatePositionAndRotation();
    
    // 确保 Tick 启用
    SetActorTickEnabled(true);
    
    // 添加调试日志
    UE_LOG(LogTemp, Warning, TEXT("TransportVehicleActor 初始化完成"));
}

void ATransportVehicleActor::InitDirAndSpeed(EMovementDirection Direction, float InVehicleSpeed)
{
    MovementDirection = Direction;
    VehicleSpeed = InVehicleSpeed;
}

void ATransportVehicleActor::Destroyed()
{
    Super::Destroyed();
}

void ATransportVehicleActor::UpdatePositionAndRotation()
{
    if (!FollowedSplineActor)
    {
        return;
    }
    
    USplineComponent* SplineComponent = FollowedSplineActor->GetSplineComponent();
    if (!SplineComponent)
    {
        return;
    }
    
    // 获取当前位置和朝向
    FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(
        CurrentSplineDistance, 
        ESplineCoordinateSpace::World
    );
    
    FRotator NewRotation = SplineComponent->GetRotationAtDistanceAlongSpline(
        CurrentSplineDistance, 
        ESplineCoordinateSpace::World
    );
    
    // 执行射线检测以确定地面位置
    if (bSnapToGround)
    {
        FHitResult HitResult;
        FVector TraceStart = NewLocation + FVector(0, 0, 100.0f); // 从当前位置上方开始射线
        FVector TraceEnd = NewLocation - FVector(0, 0, 300.0f);   // 向下检测一定距离
        
        // 定义射线检测需要的参数
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(this);  // 忽略自身
        
        // 进行射线检测
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,  // 使用可见性通道或地面通道
            CollisionParams
        );
        
        // 如果检测到地面，则调整高度
        if (bHit)
        {
            // 计算车辆底部到地面的偏移量
            float HeightOffset = GroundOffset;
            
            // 调整位置，使车辆贴合地面
            NewLocation = HitResult.Location + FVector(0, 0, HeightOffset);
            
            // 可选：根据地面法线调整旋转（使车辆与地面平行）
            FVector UpVector = HitResult.Normal;
            FVector ForwardVector = NewRotation.Vector();
            // 确保前向量与上向量正交
            ForwardVector = FVector::VectorPlaneProject(ForwardVector, UpVector).GetSafeNormal();
            
            // 生成新的旋转
            NewRotation = UKismetMathLibrary::MakeRotFromXZ(ForwardVector, UpVector);
        }
    }
    
    // 应用位置和朝向
    SetActorLocation(NewLocation);
    SetActorRotation(NewRotation);
} 