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

class USplineComponent;
class SPIDERMAN_MK2_API FMathAlgorithm
{
public:
	static FVector FindNearestPointAlongSplineFromPointSquared(const USplineComponent* TargetSpline, FVector Point, float& DisToSpline, float& RetPointSplineInputKey);
	
private:
	static FVector FindNearestPointAlongSplineSegmentFromPoint(const USplineComponent* TargetSpline, FVector TargetPoint, int32 SegmentIndex, float& SquaredDistoSegment, float& RetPointSplineInputKey);
	
	// 新增一个用于调试绘制整个样条的方法
	static void DebugDrawEntireSpline(const USplineComponent* TargetSpline, float Duration = 10.0f);
private:
	static const FVector InvalidVector;
};