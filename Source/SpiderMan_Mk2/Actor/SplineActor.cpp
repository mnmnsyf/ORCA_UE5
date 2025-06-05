#include "SplineActor.h"
#include "TransportVehicleActor.h"


#if WITH_EDITOR

ASplineActor::ASplineActor()
{
	// 设置这个actor可以在每一帧都更新
	PrimaryActorTick.bCanEverTick = false;

	// 创建并设置根组件为Spline组件
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	RootComponent = SplineComponent;

	// 默认设置为闭环
	SplineComponent->SetClosedLoop(true);
}

void ASplineActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


