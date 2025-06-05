// Fill out your copyright notice in the Description page of Project Settings.


#include "GenerateActorBase.h"

// Sets default values
AGenerateActorBase::AGenerateActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AGenerateActorBase::StartSpawning()
{
}

// Called when the game starts or when spawned
void AGenerateActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGenerateActorBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AGenerateActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

