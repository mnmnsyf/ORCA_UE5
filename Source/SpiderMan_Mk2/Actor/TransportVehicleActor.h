#pragma once

#include "CoreMinimal.h"
#include "MachineActor.h"
#include "TransportVehicleActor.generated.h"

class ASplineActor;
enum class EMovementDirection : uint8;

UCLASS()
class SPIDERMAN_MK2_API ATransportVehicleActor : public AMachineActor
{
    GENERATED_BODY()

public:
    ATransportVehicleActor();

public:
	// 当前在Spline上的距离
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float CurrentSplineDistance;

	// 自定义速度修正系数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float SpeedMultiplier = 1.0f;

	// 车辆距离地面的偏移高度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundOffset = .0f;

	// 是否启用地面检测
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bSnapToGround = false;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetWheelSpeed(float CurSpeed);

public:	 
	// 初始化函数，设置跟随的路径和初始距离
	void Initialize(ASplineActor* InSplineActor, float InitialDistance);
	void InitDirAndSpeed(EMovementDirection Direction, float VehicleSpeed);

private:
	EMovementDirection MovementDirection;

	// 车辆速度(厘米/秒)
	float VehicleSpeed = 300.0f;

	float SplineLength = .0f;

	// 所跟随的Spline Actor引用
	UPROPERTY()
	TObjectPtr<ASplineActor> FollowedSplineActor;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
	// 当车辆被销毁时调用
	virtual void Destroyed() override;

private:    
    // 更新车辆位置和朝向
    void UpdatePositionAndRotation();
};
