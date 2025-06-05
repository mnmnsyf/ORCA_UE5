// Fill out your copyright notice in the Description page of Project Settings.


#include "CitizenBase.h"
#include "CustomMovementComponent.h"
#include "SplineActor.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

// 初始化静态原子计数器
std::atomic<int32> ACitizenBase::NextID{ 0 };
// Sets default values
ACitizenBase::ACitizenBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ORCAMoveCom = CreateDefaultSubobject<UORCAComponent>(TEXT("DynamicORCAMoveComponent"));
	// 只在首次生成时分配ID
	if (UniqueID == INDEX_NONE)
	{
		UniqueID = NextID.fetch_add(1, std::memory_order_relaxed);
	}
}

ACitizenBase::ACitizenBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	ORCAMoveCom = CreateDefaultSubobject<UORCAComponent>(TEXT("DynamicORCAMoveComponent"));
	// 只在首次生成时分配ID
	if (UniqueID == INDEX_NONE)
	{
		UniqueID = NextID.fetch_add(1, std::memory_order_relaxed);
	}
}

FVector ACitizenBase::CalcORCACurRotDir_Implementation()
{
	return GetActorRotation().Vector();
}

float ACitizenBase::CalcORCAMaxSpeed_Implementation(float DeltaTime)
{
	return GetCustomMovementComponent()->MaxWalkSpeed;
}

float ACitizenBase::CalcORCARadius_Implementation()
{
	/*return GetCapsuleComponent()->GetScaledCapsuleRadius();*/
	return ORCAMoveCom->GetAgentConfigRadius();
}

FVector ACitizenBase::CalcORCACurPos_Implementation()
{
	return GetActorLocation();
}

FVector ACitizenBase::CalcORCACurVelocity_Implementation()
{
	return GetCustomMovementComponent()->Velocity;
}

FVector ACitizenBase::CalcORCAPrefVelocity_Implementation(float DeltaTime)
{
	FVector AnsPrefV = FVector::ZeroVector;
	if (TargetSplineActor == nullptr)
	{
		return AnsPrefV;
	}
	FVector CurPos = GetActorLocation();
	float WantedSplineDis = CurrentSplineDistance;
	FVector WantedRoadPos = TargetSplineActor->GetSplineComponent()->GetLocationAtDistanceAlongSpline(WantedSplineDis, ESplineCoordinateSpace::World);
	FVector WantedRoadDir = TargetSplineActor->GetSplineComponent()->GetRotationAtDistanceAlongSpline(WantedSplineDis, ESplineCoordinateSpace::World).Vector().GetSafeNormal2D();
	FVector OffsetPos = WantedRoadPos - CurPos;
	OffsetPos.Z = 0.f;
	AnsPrefV = OffsetPos.GetSafeNormal() * GetCustomMovementComponent()->MaxWalkSpeed;
	return AnsPrefV;
}

// Called when the game starts or when spawned
void ACitizenBase::BeginPlay()
{
	Super::BeginPlay();
	GetCustomMovementComponent()->SetCalVelocityMode(ECalVelocityMode::CustomVelocity);
}

void ACitizenBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACitizenBase, UniqueID);
}

// Called every frame
void ACitizenBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateMoveSpline(DeltaTime);
}

// Called to bind functionality to input
void ACitizenBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACitizenBase::SetMoveSpline(ASplineActor* SplineActorPtr, float CurDis)
{
	TargetSplineActor = SplineActorPtr;
	CurrentSplineDistance = CurDis;
	SpawnSplineDistance = CurDis;
}

float ACitizenBase::GetSpeed()
{
	return GetCustomMovementComponent()->Velocity.Size2D();
}

void ACitizenBase::UpdateMoveSpline(float DeltaTime)
{
	
	FVector CurPos = GetActorLocation();
	FVector WantedRoadPos = TargetSplineActor->GetSplineComponent()->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
	FVector OffsetPos = WantedRoadPos - CurPos;
	OffsetPos.Z = 0.f;
	//更新朝向
	FVector TargetRot = OffsetPos.GetSafeNormal2D();
	FVector WantedRoadDir = TargetSplineActor->GetSplineComponent()->GetRotationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World).Vector().GetSafeNormal2D();
	if (OffsetPos.IsNearlyZero())
	{
		TargetRot = WantedRoadDir;
	}
	SetActorRotation(TargetRot.Rotation());
	//判断当前位置和路上目标位置离多远，超过一定距离,目标位置需要等待
	if (OffsetPos.Size() < 250.f)
	{
		CurrentSplineDistance += GetCustomMovementComponent()->MaxWalkSpeed * DeltaTime;
	}
	//更新速度
	FVector MoveVelocity = ORCAMoveCom->GetORCAVelocity();
	GetCustomMovementComponent()->SetCustomVelocity(MoveVelocity);
	//CurrentSplineDistance += GetCustomMovementComponent()->MaxWalkSpeed * DeltaTime;

	if (CurrentSplineDistance >= TargetSplineActor->GetSplineComponent()->GetSplineLength())
	{
		if (!TargetSplineActor->GetSplineComponent()->IsClosedLoop())
		{
			CurrentSplineDistance = 0.f;
			FVector ZeorPos = TargetSplineActor->GetSplineComponent()->GetLocationAtDistanceAlongSpline(SpawnSplineDistance, ESplineCoordinateSpace::World);
			SetActorLocation(ZeorPos);
		}
		else
		{
			CurrentSplineDistance -= TargetSplineActor->GetSplineComponent()->GetSplineLength();
		}
	}
	
}

