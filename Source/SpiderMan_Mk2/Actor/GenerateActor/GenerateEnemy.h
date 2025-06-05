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
#include "GenerateEnemy.generated.h"

class AWraithCharacter;
class ASplineActor;

UCLASS()
class SPIDERMAN_MK2_API AGenerateEnemy : public AGenerateActorBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGenerateEnemy();

	// 敌人距离Spline的距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemies", meta = (ClampMin = "50.0"))
	float EnemySpawnDistance = 200.0f;

	// 所跟随的Spline Actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemies")
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
	// 存储生成的所有敌人
	UPROPERTY()
	TArray<TObjectPtr<AWraithCharacter>> SpawnedEnemies;
};
