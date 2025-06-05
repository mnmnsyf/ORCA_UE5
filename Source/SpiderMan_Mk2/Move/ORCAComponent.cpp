// Fill out your copyright notice in the Description page of Project Settings.


#include "ORCAComponent.h"
#include "ORCAManagementSubsystem.h"

// Sets default values for this component's properties
UORCAComponent::UORCAComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UORCAComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UORCAManagementSubsystem* ORCASystem = GetWorld()->GetSubsystem<UORCAManagementSubsystem>())
	{
		if(HasValidORCAInterface())
			ORCASystem->RegisterMovementComponent(this);
	}
	
}

void UORCAComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UORCAManagementSubsystem* ORCASystem = GetWorld()->GetSubsystem<UORCAManagementSubsystem>())
	{
		ORCASystem->UnregisterMovementComponent(this);
	}

	Super::EndPlay(EndPlayReason);
}


// Called every frame
void UORCAComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FVector UORCAComponent::GetORCAVelocity()
{
	return NewVelocity;
}


void UORCAComponent::ComputeAgentNewVelocity(float DeltaTime)
{
	if (Neighbors.IsEmpty())
	{
		NewVelocity = PrefVelocity;
		return;
	}
	FVector SelfPos = CurPos;


	float InvTimeHorizon = 1.0f / TimeHorizon; // 时间窗口的倒数

	float SelfRadius = AgentRadius;

	// 求解线性规划所需的直线
	TArray<FLine> OrcaLines;

	for (const auto OtherAgentInfo : Neighbors)
	{
	

		if (OtherAgentInfo == nullptr)
			continue;

		if (AgentSteerPriority > OtherAgentInfo->AgentSteerPriority)
		{
			continue;
		}
		float OtherRadius = OtherAgentInfo->AgentRadius;
		FVector OtherPos = OtherAgentInfo->CurPos;
		FVector RelativePosition = OtherPos - SelfPos;
		FVector RelativeVelocity = CurVelocity - OtherAgentInfo->CurVelocity;
		RelativePosition.Z = 0.f;
		RelativeVelocity.Z = 0.f;

		const float DistSq = FVector::DotProduct(RelativePosition, RelativePosition);
		const float CombinedRadius = SelfRadius + OtherRadius;
		const float CombinedRadiusSq = CombinedRadius * CombinedRadius;



		FLine Line;
		FVector2D u;

		// ROV空间是一个被圆截断了的圆锥。由左leg和右leg以及圆弧组成

		if (DistSq > CombinedRadiusSq)
		{
			const FVector2D w = FVector2D(RelativeVelocity - InvTimeHorizon * RelativePosition);
			const float wLengthSq = FVector2D::DotProduct(w, w);
			const float dotProduct1 = FVector2D::DotProduct(w, FVector2D(RelativePosition));

			if (dotProduct1 < 0.0f && dotProduct1 * dotProduct1 > CombinedRadiusSq * wLengthSq)
			{
				// 最短距离从圆弧上出去
				const float wLength = FMath::Sqrt(wLengthSq);
				const FVector2D unitW = w.GetSafeNormal();

				Line.Direction = FVector2D(-unitW.Y, unitW.X);
				u = (CombinedRadius * InvTimeHorizon - wLength) * unitW;
			}
			else
			{
				// 最短距离从两条leg之一出去
				const float leg = FMath::Sqrt(DistSq - CombinedRadiusSq);

				if (FVector2D::CrossProduct(FVector2D(RelativePosition), w) < 0.0f)
				{
					// 左leg
					Line.Direction = FVector2D(RelativePosition.X * leg + RelativePosition.Y * CombinedRadius, -RelativePosition.X * CombinedRadius + RelativePosition.Y * leg) / DistSq;
				}
				else
				{
					// 右leg
					Line.Direction = -FVector2D(RelativePosition.X * leg - RelativePosition.Y * CombinedRadius, RelativePosition.X * CombinedRadius + RelativePosition.Y * leg) / DistSq;
				}
				const float dotProduct2 = FVector2D::DotProduct(FVector2D(RelativeVelocity), Line.Direction);

				u = dotProduct2 * Line.Direction - FVector2D(RelativeVelocity);
			}
		}
		else
		{
			/* Collision. Project on cut-off circle of time timeStep. */
			const float InvTimeStep = 1.0f / DeltaTime;

			/* Vector from cutoff center to relative velocity. */
			const FVector2D w = FVector2D(RelativeVelocity - InvTimeStep * RelativePosition);

			const float wLength = w.Length();
			const FVector2D unitW = w.GetSafeNormal();

			Line.Direction = FVector2D(-unitW.Y, unitW.X);
			u = (CombinedRadius * InvTimeStep - wLength) * unitW;
		}
		float factor = 0.5f;
		Line.Point = FVector2D(CurVelocity) + factor * u;
		OrcaLines.Add(Line);
	}

	bool bChoseOpt = true;
	bool bUsingSafe = true;
	FVector2D TmpPrevV = FVector2D(PrefVelocity);
	bool bFrontStopMove = false;


	FVector2D NewV = FVector2D::ZeroVector;

	bChoseOpt = false;

	FVector2D TempCurV = FVector2D(CurVelocity);
	int lineFail = linearProgram2(OrcaLines, MaxSpeed, TmpPrevV, TempCurV, bChoseOpt, NewV);
	NewVelocity = FVector(NewV, 0.f);

	// 无解
	if (lineFail < OrcaLines.Num())
	{
		linearProgram3(OrcaLines, lineFail, MaxSpeed, TempCurV, NewV);
		NewVelocity = FVector(NewV, 0.f);
		float TmpMaxSpeed = FMath::Max(MaxSpeedNoSolve, NewV.Length());
		NewVelocity = FVector(NewV, 0.f).GetSafeNormal() * TmpMaxSpeed;

	}
}

void UORCAComponent::UpdateAgentNeighbors()
{
	Neighbors.Empty();
	UORCAManagementSubsystem* ORCASystem = GetWorld()->GetSubsystem<UORCAManagementSubsystem>();
	if (ORCASystem == nullptr)
		return;
	AActor* Agent = Cast<AActor>(GetOwner());
	FVector AgentPos = Agent->GetActorLocation();
	//先暴力，有兴趣的可以用四叉树或者KDtree优化
	for (auto OtherAgentCom : *(ORCASystem->GetAgentComponents()))
	{
		if (OtherAgentCom == this)
			continue;

		AActor* OtherAgent = Cast<AActor>(OtherAgentCom->GetOwner());
		if (OtherAgent == nullptr)
			continue;

		FVector OtherAgentPos = OtherAgent->GetActorLocation();
		if (FVector::Dist2D(AgentPos, OtherAgentPos) < SearchRange)
		{
			Neighbors.Add(OtherAgentCom);
		}
	}

}

bool UORCAComponent::HasValidORCAInterface()
{
	return GetOwner()->Implements<UIORCAInterface>();
}

bool UORCAComponent::linearProgram1(const TArray<FLine>& lines, int lineNo, float radius, const FVector2D& optVelocity, const FVector2D& curVelocity, bool directionOpt, FVector2D& result)
{
	const float dotProduct = FVector2D::DotProduct(lines[lineNo].Point, lines[lineNo].Direction);
	// 圆和该直线相交的线段的一半距离。
	const float discriminant = dotProduct * dotProduct + radius * radius - FVector2D::DotProduct(lines[lineNo].Point, lines[lineNo].Point);

	if (discriminant < 0.0f)
	{
		/* Max speed circle fully invalidates line lineNo. */
		return false;
	}

	const float sqrtDiscriminant = FMath::Sqrt(discriminant);
	float tLeft = -dotProduct - sqrtDiscriminant;
	float tRight = -dotProduct + sqrtDiscriminant;
	for (int i = 0; i < lineNo; i++)
	{
		const float denominator = FVector2D::CrossProduct(lines[i].Direction, lines[lineNo].Direction);
		const float numerator = FVector2D::CrossProduct(lines[lineNo].Point - lines[i].Point, lines[i].Direction);

		if (FMath::Abs(denominator) <= RVO_EPSILON)
		{
			/* Lines lineNo and i are (almost) parallel. */
			if (numerator < 0.0f)
			{
				return false;
			}
			else
			{
				continue;
			}
		}

		const float t = numerator / denominator;

		if (denominator >= 0.0f)
		{
			/* Line i bounds line lineNo on the right. */
			tRight = FMath::Min(tRight, t);
		}
		else
		{
			/* Line i bounds line lineNo on the left. */
			tLeft = FMath::Max(tLeft, t);
		}

		if (tLeft > tRight)
		{
			return false;
		}
	}

	if (directionOpt)
	{
		// 求最优解
		FVector2D TempAns1 = lines[lineNo].Point + tRight * lines[lineNo].Direction;
		FVector2D TempAns2 = lines[lineNo].Point + tLeft * lines[lineNo].Direction;


		/* Optimize direction. */
		if (FVector2D::DotProduct(optVelocity, lines[lineNo].Direction) > 0.0f) {
			/* Take right extreme. */
			result = TempAns1;
		}
		else {
			/* Take left extreme. */
			result = TempAns2;
		}

	}
	else
	{
		/* Optimize closest point. */
		const float t = FVector2D::DotProduct(lines[lineNo].Direction, (optVelocity - lines[lineNo].Point));

		if (t < tLeft)
		{
			result = lines[lineNo].Point + tLeft * lines[lineNo].Direction;
		}
		else if (t > tRight)
		{
			result = lines[lineNo].Point + tRight * lines[lineNo].Direction;
		}
		else
		{
			result = lines[lineNo].Point + t * lines[lineNo].Direction;
		}

	}
	return true;
}

int UORCAComponent::linearProgram2(const TArray<FLine>& lines, float radius, const FVector2D& optVelocity, const FVector2D& curVelocity, bool directionOpt, FVector2D& result)
{
	// 如果为true则选最优速度
	if (directionOpt)
	{
		result = optVelocity.GetSafeNormal() * radius;
	}
	else if (FVector2D::DotProduct(optVelocity, optVelocity) > radius * radius)
	{
		result = optVelocity.GetSafeNormal() * radius;
	}
	else
	{
		result = optVelocity;
	}

	for (int i = 0; i < lines.Num(); i++)
	{
		if (FVector2D::CrossProduct(lines[i].Direction, lines[i].Point - result) < 0.0f)
		{
			/* Result does not satisfy constraint i. Compute new optimal result. */
			const FVector2D tempResult = result;

			if (!linearProgram1(lines, i, radius, optVelocity, curVelocity, directionOpt, result))
			{
				result = tempResult;

				return i;
			}
		}
	}


	return lines.Num();
}

void UORCAComponent::linearProgram3(const TArray<FLine>& lines, int beginLine, float radius, const FVector2D& curVelocity, FVector2D& result)
{
	float distance = 0.0f;

	for (int i = beginLine; i < lines.Num(); ++i)
	{
		if (FVector2D::CrossProduct(lines[i].Point - result, lines[i].Direction) > distance)
		{
			// CrossProduct(lines[i].direction, lines[i].point - result)相当于求result点到直线lines[i]距离的2倍
			/* Result does not satisfy constraint of line i. */
			TArray<FLine> projLines;
			// 根据邻居的速度空间计算一个最小影响邻居最优解的速度，表现出来的就是随邻居移动。
			for (int j = 0; j < i; ++j)
			{
				FLine line;

				float determinant = FVector2D::CrossProduct(lines[j].Direction, lines[i].Direction);

				if (FMath::Abs(determinant) <= RVO_EPSILON)
				{
					/* Line i and line j are parallel. */
					if (FVector2D::DotProduct(lines[i].Direction, lines[j].Direction) > 0.0f)
					{
						/* Line i and line j point in the same direction. */
						continue;
					}
					else
					{
						/* Line i and line j point in opposite direction. */
						line.Point = 0.5f * (lines[i].Point + lines[j].Point);
					}
				}
				else
				{
					line.Point = lines[i].Point + (FVector2D::CrossProduct(lines[i].Point - lines[j].Point, lines[j].Direction) / determinant) * lines[i].Direction;
				}

				line.Direction = (lines[j].Direction - lines[i].Direction).GetSafeNormal();
				projLines.Add(line);
			}

			const FVector2D tempResult = result;

			if (linearProgram2(projLines, radius, FVector2D(lines[i].Direction.Y, -lines[i].Direction.X), curVelocity, false, result) < projLines.Num())
			{
				/* This should in principle not happen.  The result is by definition already in the feasible region of this linear program. If it fails it is due to small floating point error, and the current result is kept.
				 * already in the feasible region of this linear program. If it fails,
				 * it is due to small floating point error, and the current result is
				 * kept.
				 */
				result = tempResult;
			}

			distance = FVector2D::CrossProduct(lines[i].Point - result, lines[i].Direction);
		}
	}
}

