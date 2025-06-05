// Fill out your copyright notice in the Description page of Project Settings.


#include "MathAlgorithm.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetSystemLibrary.h"

FVector FMathAlgorithm::FindNearestPointAlongSplineFromPointSquared(const USplineComponent* TargetSpline, FVector Point, float& DisToSpline, float& RetPointSplineInputKey)
{
	if (!TargetSpline)
	{
		DisToSpline = FLT_MAX;
		RetPointSplineInputKey = -1.0f;
		return InvalidVector;
	}

	// 在方法开始处添加一次性调试绘制
	static bool bHasDrawnDebug = false;
	if (!bHasDrawnDebug && TargetSpline)
	{
		bHasDrawnDebug = true;
		DebugDrawEntireSpline(TargetSpline, 120.0f);  // 绘制120秒
	}

	auto SplineCurves = TargetSpline->SplineCurves;
	float MinDis = FLT_MAX / 2.0f;
	int32 i = 0, MinIndex = -1;
	// 先找到最小的一个节点
	for (auto SplinePoint : SplineCurves.Position.Points)
	{
		float NowDis = FVector::DistSquared(Point, TargetSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
		if (NowDis < MinDis)
		{
			MinDis = NowDis;
			MinIndex = i;
		}
		i++;
	}
	const int32 NumSegments = SplineCurves.Position.Points.Num() - 1;
	
	// 判断样条是否闭合
	bool bIsClosedLoop = TargetSpline->IsClosedLoop();
	
	float PreDis = -1.0f, NextDis = -1.0f;
	float PreKey = -1.0f, NextKey = -1.0f;
	// 然后左右各取一个检测
	FVector PreVector = InvalidVector, NextVector = InvalidVector;
	
	// 处理前一段
	if (MinIndex > 0)
	{
		PreVector = FindNearestPointAlongSplineSegmentFromPoint(TargetSpline, Point, MinIndex - 1, PreDis, PreKey);
	}
	else if (bIsClosedLoop && MinIndex == 0) // 如果是闭合样条，并且当前是第一个点，则前一段是最后一个段
	{
		PreVector = FindNearestPointAlongSplineSegmentFromPoint(TargetSpline, Point, NumSegments, PreDis, PreKey);
	}

	// 处理后一段
	if (MinIndex < NumSegments)
	{
		NextVector = FindNearestPointAlongSplineSegmentFromPoint(TargetSpline, Point, MinIndex, NextDis, NextKey);
	}
	else if (bIsClosedLoop && MinIndex == NumSegments) // 如果是闭合样条，并且当前是最后一个点，则后一段连接到第一个点
	{
		// 使用最后一个点到第一个点的段
		NextVector = FindNearestPointAlongSplineSegmentFromPoint(TargetSpline, Point, MinIndex, NextDis, NextKey);
	}

	// 比较两者，返回更近的点
	if (NextVector == InvalidVector || (PreVector != InvalidVector && FVector::DistSquared(PreVector, Point) < FVector::DistSquared(NextVector, Point)))
	{
		DisToSpline = PreDis;
		RetPointSplineInputKey = PreKey;
		return PreVector;
	}
	else
	{
		DisToSpline = NextDis;
		RetPointSplineInputKey = NextKey;
		return NextVector;
	}
}

FVector FMathAlgorithm::FindNearestPointAlongSplineSegmentFromPoint(const USplineComponent* TargetSpline, FVector TargetPoint, int32 SegmentIndex, float& SquaredDistoSegment, float& RetPointSplineInputKey)
{
	if (!TargetSpline)
	{
		return FVector::ZeroVector;
	}
	auto SplineCurves = TargetSpline->SplineCurves;
	auto SplineTransform = TargetSpline->GetComponentTransform();
	auto GetLocationAtSplineInputKey = [&](float InKey) ->FVector {
		FVector Location = SplineCurves.Position.Eval(InKey, FVector::ZeroVector);
		Location = SplineTransform.TransformPosition(Location);
		return Location;
	};

	// 获取初始条件，会影响最终曲线哟！！！
	const int32 NumPoints = SplineCurves.Position.Points.Num();
	const int32 LastPoint = NumPoints - 1;
	
	// 处理闭合样条的边界情况
	bool bIsClosedLoop = TargetSpline->IsClosedLoop();
	int32 NextPointIndex = (SegmentIndex == LastPoint) ? 0 : (SegmentIndex + 1);
	
	const auto& StartPoint = SplineCurves.Position.Points[SegmentIndex];
	const auto& EndPoint = SplineCurves.Position.Points[NextPointIndex];

	FVector Location1 = SplineTransform.TransformPosition(StartPoint.OutVal);
	FVector Location2 = SplineTransform.TransformVector(StartPoint.LeaveTangent);
	// FVector Location2 = StartPoint.LeaveTangent;
	FVector Location3 = SplineTransform.TransformPosition(EndPoint.OutVal);
	FVector Location4 = SplineTransform.TransformVector(EndPoint.ArriveTangent);
	// FVector Location4 = EndPoint.ArriveTangent;
	FVector Result_Point;

	const auto& P0 = Location1;
	const auto& T0 = Location2;
	const auto& P1 = Location3;
	const auto& T1 = Location4;

	// 三次hermite曲线核心公式
	float a0 = P0.X, b0 = P0.Y, c0 = P0.Z;
	float a1 = P1.X, b1 = P1.Y, c1 = P1.Z;
	float _x = TargetPoint.X, _y = TargetPoint.Y, _z = TargetPoint.Z;
	float e0 = T0.X, f0 = T0.Y, g0 = T0.Z;
	float e1 = T1.X, f1 = T1.Y, g1 = T1.Z;
	float A1 = (2.0f * a0 - 2.0f * a1 + e0 + e1) /** 3.0f*/;
	float A2 = (2.0f * b0 - 2.0f * b1 + f0 + f1);
	float A3 = (2.0f * c0 - 2.0f * c1 + g0 + g1);
	float B1 = (-3.0f * a0 + 3.0f * a1 - 2.0f * e0 - e1);
	float B2 = (-3.0f * b0 + 3.0f * b1 - 2.0f * f0 - f1);
	float B3 = (-3.0f * c0 + 3.0f * c1 - 2.0f * g0 - g1);
	float C1 = e0;
	float C2 = f0;
	float C3 = g0;
	float D1 = a0;
	float D2 = b0;
	float D3 = c0;
	float a = A1 * A1 + A2 * A2 + A3 * A3;
	float b = 2.0f * (A1 * B1 + A2 * B2 + A3 * B3);
	float c = B1 * B1 + B2 * B2 + B3 * B3 + 2.0f * (A1 * C1 + A2 * C2 + A3 * C3);
	float d = 2.0f * (A1 * D1 + A2 * D2 + A3 * D3 + C1 * B1 + C2 * B2 + C3 * B3 - A1 * _x - A2 * _y - A3 * _z);
	float e = C1 * C1 + C2 * C2 + C3 * C3 + 2.0f * (B1 * D1 + B2 * D2 + B3 * D3 - B1 * _x - B2 * _y - B3 * _z);
	float f = 2.0f * (C1 * D1 + C2 * D2 + C3 * D3 - C1 * _x - C2 * _y - C3 * _z);
	float g = D1 * D1 + D2 * D2 + D3 * D3 - 2.0f * (D1 * _x + D2 * _y + D3 * _z) + _x * _x + _y * _y + _z * _z;
	float A = 30.0f * a;
	float B = 20.0f * b;
	float C = 12.0f * c;
	float D = 6.0f * d;
	float E = 2.0f * e;

	// 四次函数求根
	int Sum_T = 0; // 四次函数实根总数
	double Result[4] = { 0.0f };


	// 天珩公式求四次函数实根法
	double sqr_A = A * A;
	double sqr_B = B * B;
	double _D = 3.0f * sqr_B - 8.0f * A * C;
	double _E = -1.0f * sqr_B * B + 4.0f * A * B * C - 8.0f * sqr_A * D;
	double _F = 3.0f * sqr_B * sqr_B + 16.0f * sqr_A * C * C - 16.0f * A * sqr_B * C + 16.0f * sqr_A * B * D - 64.0f * sqr_A * A * E;
	double _A = _D * _D - 3.0f * _F;
	double _B = _D * _F - 9.0f * _E * _E;
	double _C = _F * _F - 3.0f * _D * _E * _E;
	double Delta = _B * _B - 4.0f * _A * _C;
	double TT1 = 0.0f;
	double TT2 = 0.0f;
	double TT3 = 0.0f;
	double TT4 = 0.0f;
	double tt = 0.0f;
	float SGN = abs(_E) / _E;

	if (Delta > 0.0f)
	{
		double _Z1 = _A * _D + 3.0f * ((-1.0f * _B - sqrt(Delta)) / 2.0f);
		double _Z2 = _A * _D + 3.0f * ((-1.0f * _B + sqrt(Delta)) / 2.0f);
		double _Z3 = (cbrt(_Z1) + cbrt(_Z2));
		double _Z = _D * _D - _D * _Z3 + _Z3 * _Z3 - 3 * _A;
		TT1 = (-1.0f * B + SGN * sqrt((_D + _Z3) / 3.0f) - sqrt((2.0f * _D - _Z3 + 2.0f * sqrt(_Z)) / 3.0f)) / (4.0f * A);
		TT2 = (-1.0f * B + SGN * sqrt((_D + _Z3) / 3.0f) + sqrt((2.0f * _D - _Z3 + 2.0f * sqrt(_Z)) / 3.0f)) / (4.0f * A);
		Sum_T = 2;
		if (TT1 < TT2)
		{
		}
		else
		{
			tt = TT1;
			TT1 = TT2;
			TT2 = tt;
		}
		Result[0] = TT1;
		Result[1] = TT2;
	}
	else if (Delta < 0.0f)
	{

		if (_E != 0.0f && _D > 0.0f && _F > 0.0f)
		{
			double Theta = acos((3.0f * _B - 2.0f * _A * _D) / (2.0f * _A * sqrt(_A)));
			double _Y1 = sqrt((_D - 2.0f * sqrt(_A) * cos(Theta / 3.0f)) / 3.0f);
			double _Y2 = sqrt((_D + sqrt(_A) * (cos(Theta / 3.0f) - sqrt(3.0f) * sin(Theta / 3.0f))) / 3.0f);
			double _Y3 = sqrt((_D + sqrt(_A) * (cos(Theta / 3.0f) + sqrt(3.0f) * sin(Theta / 3.0f))) / 3.0f);

			TT1 = (-1.0f * B + SGN * _Y1 - (_Y2 + _Y3)) / (4.0 * A);
			TT2 = (-1.0f * B - SGN * _Y1 - (_Y2 - _Y3)) / (4.0 * A);
			TT3 = (-1.0f * B - SGN * _Y1 + (_Y2 - _Y3)) / (4.0 * A);
			TT4 = (-1.0f * B + SGN * _Y1 + (_Y2 + _Y3)) / (4.0 * A);
			Result[0] = TT1;
			Result[1] = TT2;
			Result[2] = TT3;
			Result[3] = TT4;
			std::sort(Result, Result + 4);
			Sum_T = 4;
		}
		else
		{
			Result[0] = 0.5f;
			Sum_T = 1;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("there is no root"));
	}

	// 牛顿迭代法部分
	double Min = 1.0f;

	auto Dis = [&TargetPoint](FVector x) { return FVector::DistSquared(x, TargetPoint); };
	if (Sum_T == 0)
	{
		if (Dis(P0) < Dis(P1))
		{
			Result_Point = P0;
			SquaredDistoSegment = Dis(P0);
			RetPointSplineInputKey = SegmentIndex;
		}
		else
		{
			Result_Point = P1;
			SquaredDistoSegment = Dis(P1);
			RetPointSplineInputKey = SegmentIndex + 1;
		}
	}
	else
	{
		for (int j = 0; j <= Sum_T; j++)
		{

			double t = 0.0f;
			if (j == 0)
			{
				t = Result[j] * 0.5f;
			}
			if (j < Sum_T && j > 0)
			{
				t = (Result[j] + Result[j - 1]) * 0.5f;
			}
			if (j == Sum_T)
			{
				t = (Result[j - 1] + 1.0f) * 0.5f;
			}
			for (int32 i = 0; i < 10; i++)
			{

				double sqr_t = t * t;
				// 牛顿迭代核心公式
				t = t - (6.0f * a * sqr_t * sqr_t * t + 5.0f * b * sqr_t * sqr_t + 4.0f * c * sqr_t * t + 3.0f * d * sqr_t + 2.0f * e * t + f) / (A * sqr_t * sqr_t + B * sqr_t * t + C * sqr_t + D * t + E);
			}
			if (t >= 0 && t <= 1)
			{
				Min = Dis(GetLocationAtSplineInputKey(t + SegmentIndex)) < Dis(GetLocationAtSplineInputKey(Min + SegmentIndex)) ? t : Min;

			}
			Min = Dis(GetLocationAtSplineInputKey(Min + SegmentIndex)) < Dis(GetLocationAtSplineInputKey(1.0f + SegmentIndex)) ? Min : 1.0f;
			Min = Dis(GetLocationAtSplineInputKey(Min + SegmentIndex)) < Dis(GetLocationAtSplineInputKey(SegmentIndex)) ? Min : 0.0f;
		}
		Sum_T = 0;



		// 输出最优的结果
		double _t = Min;
		RetPointSplineInputKey = _t + SegmentIndex;
		Result_Point = GetLocationAtSplineInputKey(_t + SegmentIndex);
		SquaredDistoSegment = Dis(Result_Point);
	}
	return Result_Point;
}

const FVector FMathAlgorithm::InvalidVector = FVector(FLT_MAX, FLT_MAX, FLT_MAX);

// 在 MathAlgorithm.cpp 中添加一个公共静态方法用于绘制完整的样条
void FMathAlgorithm::DebugDrawEntireSpline(const USplineComponent* TargetSpline, float Duration)
{
	if (!TargetSpline || !TargetSpline->GetWorld())
	{
		return;
	}
	
	// 获取样条的总段数
	auto SplineCurves = TargetSpline->SplineCurves;
	const int32 NumSegments = SplineCurves.Position.Points.Num() - 1;
	
	if (NumSegments < 1)
	{
		return;
	}
	
	// 是否闭合样条
	bool bIsClosedLoop = TargetSpline->IsClosedLoop();
	
	// 绘制每一段样条
	for (int32 SegmentIndex = 0; SegmentIndex <= NumSegments; SegmentIndex++)
	{
		// 对于最后一段，仅当样条是闭合的时才绘制
		if (SegmentIndex == NumSegments && !bIsClosedLoop)
		{
			continue;
		}
		
		int32 NextPointIndex = (SegmentIndex == NumSegments) ? 0 : (SegmentIndex + 1);
		
		// 获取当前段的控制点和切线
		const auto& SplineTransform = TargetSpline->GetComponentTransform();
		const auto& StartPoint = SplineCurves.Position.Points[SegmentIndex];
		const auto& EndPoint = SplineCurves.Position.Points[NextPointIndex];
		
		FVector P0 = SplineTransform.TransformPosition(StartPoint.OutVal);
		FVector T0 = SplineTransform.TransformVector(StartPoint.LeaveTangent);
		FVector P1 = SplineTransform.TransformPosition(EndPoint.OutVal);
		FVector T1 = SplineTransform.TransformVector(EndPoint.ArriveTangent);
		
		// 获取埃尔米特曲线系数
		float a0 = P0.X, b0 = P0.Y, c0 = P0.Z;
		float a1 = P1.X, b1 = P1.Y, c1 = P1.Z;
		float e0 = T0.X, f0 = T0.Y, g0 = T0.Z;
		float e1 = T1.X, f1 = T1.Y, g1 = T1.Z;
		
		// 绘制细分的线段近似曲线
		FVector PrevPoint = P0;
		FVector CurrPoint;
		
		// 为每个段使用不同颜色，使其更容易区分
		FColor SegmentColor;
		switch (SegmentIndex % 4)
		{
		case 0: SegmentColor = FColor::Red; break;
		case 1: SegmentColor = FColor::Green; break;
		case 2: SegmentColor = FColor::Blue; break;
		default: SegmentColor = FColor::Yellow; break;
		}

		for (double t = 0.01; t <= 1.0; t += 0.01)
		{
			double sqr_t = t * t;
			double _a = 2.0f * sqr_t * t - 3.0f * sqr_t + 1.0f;
			double _b = -2.0f * sqr_t * t + 3.0f * sqr_t;
			double _c = sqr_t * t - 2.0f * sqr_t + t;
			double _d = sqr_t * t - sqr_t;
			
			double Res_x = a0 * (_a) + a1 * (_b) + e0 * (_c) + e1 * (_d);
			double Res_y = b0 * (_a) + b1 * (_b) + f0 * (_c) + f1 * (_d);
			double Res_z = c0 * (_a) + c1 * (_b) + g0 * (_c) + g1 * (_d);
			
			CurrPoint = FVector(Res_x, Res_y, Res_z);
			
			UKismetSystemLibrary::DrawDebugLine(
				TargetSpline->GetWorld(), 
				PrevPoint, 
				CurrPoint, 
				SegmentColor, 
				Duration, 
				3.0f
			);
			
			PrevPoint = CurrPoint;
		}
	}
}
