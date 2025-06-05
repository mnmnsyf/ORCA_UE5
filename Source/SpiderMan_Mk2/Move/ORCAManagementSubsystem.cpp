#include "ORCAManagementSubsystem.h"
#include "GameFramework/Actor.h"
#include "ORCAComponent.h"
#include "CustomCharacterBase.h"
#include "CustomMovementComponent.h"
UORCAManagementSubsystem* UORCAManagementSubsystem::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {   
        return World->GetSubsystem<UORCAManagementSubsystem>();
    }
    return nullptr;
}

void UORCAManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
	 // 绑定 Tick 函数到当前子系统实例
	MyTickFunction.TargetSubsystem = this;

	// 设置 Tick 组
	MyTickFunction.TickGroup = TG_PrePhysics; // 选择 Tick 阶段

	// 注册 Tick 函数到当前 World 的 Level
	MyTickFunction.RegisterTickFunction(GetWorld()->PersistentLevel);
}

void UORCAManagementSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

}

void UORCAManagementSubsystem::Tick(float DeltaTime)
{
	//准备数据
	for (auto CurAgentCom : AgentComponents)
	{
		IIORCAInterface* CurAgent = Cast<IIORCAInterface>(CurAgentCom->GetOwner());
		if(!CurAgent)
			continue;
		CurAgentCom->CurVelocity = CurAgent->Execute_CalcORCACurVelocity(CurAgentCom->GetOwner());
		CurAgentCom->PrefVelocity = CurAgent->Execute_CalcORCAPrefVelocity(CurAgentCom->GetOwner(), DeltaTime);
		CurAgentCom->CurPos = CurAgent->Execute_CalcORCACurPos(CurAgentCom->GetOwner());
		CurAgentCom->CurRotDir = CurAgent->Execute_CalcORCACurRotDir(CurAgentCom->GetOwner());
		CurAgentCom->MaxSpeed = CurAgent->Execute_CalcORCAMaxSpeed(CurAgentCom->GetOwner(), DeltaTime);
		CurAgentCom->AgentRadius = CurAgent->Execute_CalcORCARadius(CurAgentCom->GetOwner());
		CurAgentCom->UpdateAgentNeighbors();
	}
	//计算ORCA新速度
	for (auto CurAgentCom : AgentComponents)
	{
		CurAgentCom->ComputeAgentNewVelocity(DeltaTime);
	}

}

void UORCAManagementSubsystem::RegisterMovementComponent(UORCAComponent* Component)
{
	if (Component && !AgentComponents.Contains(Component))
	{
		AgentComponents.Add(Component);
	}
}

void UORCAManagementSubsystem::UnregisterMovementComponent(UORCAComponent* Component)
{
    AgentComponents.Remove(Component);
}

const TArray<TWeakObjectPtr<class UORCAComponent>>* UORCAManagementSubsystem::GetAgentComponents()
{
	return &AgentComponents;
}


bool UORCAManagementSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE /*|| WorldType == EWorldType::Editor*/;
}

void FORCAManagementTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	if (TargetSubsystem.IsValid())
	{
		TargetSubsystem->Tick(DeltaTime);
	}
}
