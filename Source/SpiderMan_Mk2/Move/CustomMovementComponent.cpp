// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomMovementComponent.h"

void UCustomMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	if (CalVelocityMode == ECalVelocityMode::Acceleration)
	{
		UCharacterMovementComponent::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
	}
	else if(CalVelocityMode == ECalVelocityMode::CustomVelocity)
	{
		Velocity = CustomVelocity;
	}
}

void UCustomMovementComponent::SetCalVelocityMode(ECalVelocityMode NewCalVelocityMode)
{
	CalVelocityMode = NewCalVelocityMode;
}

void UCustomMovementComponent::SetCustomVelocity(const FVector& NewCustomVelocity)
{
	CustomVelocity = NewCustomVelocity;
}
