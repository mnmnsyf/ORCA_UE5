// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenerateActorBase.generated.h"

UCLASS()
class SPIDERMAN_MK2_API AGenerateActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGenerateActorBase();

protected:

	// 生成参数配置
	UPROPERTY(EditAnywhere, Category = "Spawn Settings")
	TSubclassOf<AActor> ActorClassToSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawn Settings", meta = (ClampMin = "0"))
	int32 MaxSpawnCount = 5;

	//生成逻辑
	UFUNCTION(BlueprintCallable, Category = "Spawn Control")
	virtual void StartSpawning();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
