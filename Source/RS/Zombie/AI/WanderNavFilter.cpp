// Fill out your copyright notice in the Description page of Project Settings.


#include "WanderNavFilter.h"
#include "NavAreas/NavArea_Default.h"

UWanderNavFilter::UWanderNavFilter()
{
	FNavigationFilterArea LinkArea;
	LinkArea.AreaClass = UNavArea_Default::StaticClass();
	LinkArea.TravelCostOverride = 100000.f;
	LinkArea.EnteringCostOverride = 100000.f;

	LinkArea.bOverrideEnteringCost = true;
	LinkArea.bOverrideTravelCost = true;

	Areas.Add(LinkArea);
}
