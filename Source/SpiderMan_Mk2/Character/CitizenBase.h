// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomCharacterBase.h"
#include "ORCAComponent.h"
#include "CitizenBase.generated.h"

UCLASS()
class SPIDERMAN_MK2_API ACitizenBase : public ACustomCharacterBase, public IIORCAInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACitizenBase();
	ACitizenBase(const FObjectInitializer& ObjectInitializer);

	FVector CalcORCAPrefVelocity_Implementation(float DeltaTime) override;


	FVector CalcORCACurVelocity_Implementation() override;


	FVector CalcORCACurPos_Implementation() override;


	FVector CalcORCACurRotDir_Implementation() override;

	float CalcORCAMaxSpeed_Implementation(float DeltaTime) override;
	
	float CalcORCARadius_Implementation() override;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 唯一标识符（VisibleAnywhere保持只读）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Identifier", Replicated)
	int32 UniqueID = INDEX_NONE;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void SetMoveSpline(class ASplineActor* SplineActorPtr, float CurDis);

	UFUNCTION(BlueprintCallable, Category = "Identifier")
	int32 GetUniqueID() const { return UniqueID; };
	UFUNCTION(BlueprintCallable)
	float GetSpeed();
private:
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UORCAComponent> ORCAMoveCom;

	TWeakObjectPtr<class ASplineActor> TargetSplineActor;
	
	UPROPERTY(EditAnywhere, Category = "Spline Movement")
	bool bLoopMovement = true;

	float CurrentSplineDistance = 0.0f;
	float SpawnSplineDistance = 0.f;
	bool bIsMoving = false;
private:
	//更新市民沿道路移动
	void UpdateMoveSpline(float DeltaTime);

private:
	// 静态ID生成器（线程安全版本）
	static std::atomic<int32> NextID;
};
