// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"


// 计算速度的方式
UENUM(BlueprintType)
enum class ECalVelocityMode : uint8
{
	Acceleration,		 // 通过加速度的方式就是ue原生计算速度的方式
	CustomVelocity,		// 通过自定义速度
};
/**
 *重定义移动组件 
 */
UCLASS()
class SPIDERMAN_MK2_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:


	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration);

	UFUNCTION(BlueprintCallable, Category = "MOCharacterMovement", meta = (DisplayName = "设置计算速度的方式"))
	void SetCalVelocityMode(ECalVelocityMode NewCalVelocityMode);

	UFUNCTION(BlueprintCallable, Category = "MOCharacterMovement", meta = (DisplayName = "设置外部自定义速度"))
	void SetCustomVelocity(const FVector& NewCustomVelocity);

	//获取加速度
	FVector GetAcceleration() {return Acceleration;}
	
private:
	ECalVelocityMode CalVelocityMode = ECalVelocityMode::Acceleration;
	FVector CustomVelocity = FVector::ZeroVector;
};
