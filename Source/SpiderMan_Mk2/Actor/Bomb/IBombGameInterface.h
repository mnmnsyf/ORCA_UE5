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
#include "UObject/Interface.h"
#include "IBombGameInterface.generated.h"

// 定义委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBombGameStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBombGameCompleteDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBombGameFailDelegate);

// 这个类定义接口
UINTERFACE(MinimalAPI)
class UBombGameInterface : public UInterface
{
    GENERATED_BODY()
};

// 实际接口类，所有实现这个接口的类需要实现这些函数
class SPIDERMAN_MK2_API IBombGameInterface
{
    GENERATED_BODY()

public:
    // 虚函数 - 获取开始事件委托
    virtual FOnBombGameStartDelegate& GetOnBombGameStartDelegate() = 0;
    
    // 虚函数 - 获取完成事件委托
    virtual FOnBombGameCompleteDelegate& GetOnBombGameCompleteDelegate() = 0;
    
    // 虚函数 - 获取失败事件委托
    virtual FOnBombGameFailDelegate& GetOnBombGameFailDelegate() = 0;
}; 