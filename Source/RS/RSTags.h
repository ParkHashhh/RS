#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace RSTags
{
	namespace Player
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Aiming);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attacking);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dashing);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Reloading);
	}

	namespace Zombie
	{

	}

	namespace Common
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttackPower);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(CurrentHP);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(DashSpeed);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(DefensePower);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxHP);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(MaxRange);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(WalkSpeed);
	}
}
