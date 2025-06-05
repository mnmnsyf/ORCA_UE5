// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenerateCitizen.generated.h"

UCLASS()
class SPIDERMAN_MK2_API AGenerateCitizen : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGenerateCitizen();

protected:

	// 生成参数配置
	UPROPERTY(EditAnywhere, Category = "Spawn Settings")
	TSubclassOf<AActor> ActorClassToSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawn Settings", meta = (ClampMin = "0"))
	int32 MaxSpawnCount = 5;
	// 间隔距离
	UPROPERTY(EditAnywhere, Category = "Spawn Settings")
	float IntervalDis = 150.f;

	UPROPERTY(EditAnywhere, Category = "Spawn Settings")
	TSoftObjectPtr<class ASplineActor> TargetSplineActor;

	//生成逻辑
	UFUNCTION(BlueprintCallable, Category = "Spawn Control")
	virtual void StartSpawning();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
//private:
//	TArray<TWeakObjectPtr<class ACitizenBase>>  Citizens;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
