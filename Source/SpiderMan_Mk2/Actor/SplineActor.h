#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/ActorComponent.h"
#include "SplineActor.generated.h"

UCLASS()
class SPIDERMAN_MK2_API ASplineActor : public AActor
{
    GENERATED_BODY()

public:
	ASplineActor();

	// Spline组件声明
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	TObjectPtr<USplineComponent> SplineComponent;

public:
	// 获取Spline组件
	UFUNCTION(BlueprintCallable, Category = "Spline")
	USplineComponent* GetSplineComponent() const { return SplineComponent; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};