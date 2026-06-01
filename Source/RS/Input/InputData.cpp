// Fill out your copyright notice in the Description page of Project Settings.


#include "InputData.h"


UInputData::UInputData()
{

}

TObjectPtr<UInputAction> UInputData::FindAction(const FName& Name) const
{

	const TObjectPtr<UInputAction>* Action = Actions.Find(Name);

	if (Action != nullptr)
	{
		return *Action;
	}

	return nullptr;
}

// CDO가 기본으로 생성되며 입력 에셋들은 모두 로딩이 되어 있는 상태이므로
// 필요하면 CDO를 이용해서 이 에셋에 접근해서 사용한다.
UDefaultInputData::UDefaultInputData()
{
	
}




USelectInputData::USelectInputData()
{
	
}
