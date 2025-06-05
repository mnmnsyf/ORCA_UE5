
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "ORCAManagementSubsystem.generated.h"


// 自定义 Tick 函数类型
struct FORCAManagementTickFunction : public FTickFunction
{
	TWeakObjectPtr<UORCAManagementSubsystem> TargetSubsystem;

	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
	virtual FString DiagnosticMessage() override { return TEXT("UORCAManagementSubsystem Tick"); }
};

UCLASS()
class SPIDERMAN_MK2_API UORCAManagementSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // 获取子系统实例
    static UORCAManagementSubsystem* Get(const UObject* WorldContext);

	// 是否需要在编辑器中Tick（默认false）
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

    // 子系统接口
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// 新增组件注册接口
	void RegisterMovementComponent(class UORCAComponent* Component);
	void UnregisterMovementComponent(class UORCAComponent* Component);
	//获取所有ORCA代理组件
	const TArray<TWeakObjectPtr<class UORCAComponent>>* GetAgentComponents();
	// Tick 逻辑实现
	void Tick(float DeltaTime);
private:
    FORCAManagementTickFunction MyTickFunction;

	//直接存储组件指针
	TArray<TWeakObjectPtr<class UORCAComponent>> AgentComponents;

	
}; 