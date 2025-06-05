// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ORCAComponent.generated.h"

const float RVO_EPSILON = 0.00001f;
struct FLine
{
	FVector2D Point;	 // 在线上的一个点
	FVector2D Direction; // 线的方向
};
struct FORCAConoid // ORCA里定义的圆锥形
{
	FVector2D LeftLegDir = FVector2D::ZeroVector;
	FVector2D RightLegDir = FVector2D::ZeroVector;
};
UINTERFACE(MinimalAPI, BlueprintType)
class UIORCAInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * ORCA系统交互接口*(因为不同类型的actor的计算一些数据的方式不同，所以提成接口，分摊到各自actor里去实现）
 */
class SPIDERMAN_MK2_API IIORCAInterface
{
	GENERATED_BODY()

public:
	
	// 计算代理的最优速度
	UFUNCTION(BlueprintNativeEvent, Category = "ORCA")
	FVector CalcORCAPrefVelocity(float DeltaTime);
	// 计算代理的当前速度
	UFUNCTION(BlueprintNativeEvent, Category = "ORCA")
	FVector CalcORCACurVelocity();
	// 计算代理的当前位置
	UFUNCTION(BlueprintNativeEvent, Category = "ORCA")
	FVector CalcORCACurPos();
	// 计算代理的当前朝向
	UFUNCTION(BlueprintNativeEvent, Category = "ORCA")
	FVector CalcORCACurRotDir();
	// 计算代理的最大速度
	UFUNCTION(BlueprintNativeEvent, Category = "ORCA")
	float CalcORCAMaxSpeed(float DeltaTime);
	// 计算代理的当前半径
	UFUNCTION(BlueprintNativeEvent, Category = "ORCA")
	float CalcORCARadius();
};
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPIDERMAN_MK2_API UORCAComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class UORCAManagementSubsystem;
	// Sets default values for this component's properties
	UORCAComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	//获取ORCA速度
	FVector GetORCAVelocity();

	//获取代理的初始配置半径
	float GetAgentConfigRadius() { return AgentConfigRadius; };
private:
	// 代理的初始半径
	UPROPERTY(Category = "ORCAConfig", EditAnywhere)
	float AgentConfigRadius = 50.f;

	// 在多大范围内搜所邻居
	UPROPERTY(Category = "ORCAConfig", EditAnywhere)
	float SearchRange = 200.f;

	// 代理的优先级，优先级小的会绕优先级大的。
	UPROPERTY(Category = "ORCAConfig", EditAnywhere)
	int32 AgentSteerPriority = 1;

	// 和其他士兵的时间窗口
	UPROPERTY(Category = "ORCAConfig", EditAnywhere)
	float TimeHorizon = 0.1f;

	// 和静态物体碰撞的时间窗口
	UPROPERTY(Category = "ORCAConfig", EditAnywhere)
	float TimeHorizonObst = 0.25f;

	// 无解时的最大速度
	UPROPERTY(Category = "ORCAConfig", EditAnywhere)
	float MaxSpeedNoSolve; //无解时的最大速度 

	FVector CurVelocity = FVector::ZeroVector;			// 代理当前速度
	FVector NewVelocity = FVector::ZeroVector;			// ORCA计算出来的代理新速度
	FVector PrefVelocity = FVector::ZeroVector;			// 代理期望速度

	FVector CurPos = FVector::ZeroVector;
	FVector CurRotDir = FVector::ZeroVector;

	float AgentRadius = 100.f;
	float MaxSpeed = 100.f;
	TArray<TWeakObjectPtr<class UORCAComponent>>Neighbors;

public:
	/**
	* 计算给定代理的新速度
	*/
	void ComputeAgentNewVelocity(float DeltaTime);

	/*
	* 更新代理周围的邻居
	*/
	void UpdateAgentNeighbors();

	/*
	* HasValidORCAInterface
	*/
	bool HasValidORCAInterface();
private:
	/**
	 * 线性方程求解1
	 */
	bool linearProgram1(const TArray<FLine>& lines, int lineNo, float radius, const FVector2D& optVelocity, const FVector2D& curVelocity, bool directionOpt, FVector2D& result);

	/**
	 * 线性方程求解2
	 */
	int linearProgram2(const TArray<FLine>& lines, float radius, const FVector2D& optVelocity, const FVector2D& curVelocity, bool directionOpt, FVector2D& result);

	// 线性方程求解3(不考虑静态障碍物）
	void linearProgram3(const TArray<FLine>& lines, int beginLine, float radius, const FVector2D& curVelocity, FVector2D& result);
};
