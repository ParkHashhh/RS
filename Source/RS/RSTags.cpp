#include "RSTags.h"

namespace RSTags
{
	namespace Player
	{
		UE_DEFINE_GAMEPLAY_TAG(Aiming, "Player.Combat.Aiming");
		UE_DEFINE_GAMEPLAY_TAG(Attacking, "Player.Combat.Attacking");
		UE_DEFINE_GAMEPLAY_TAG(Reloading, "Player.Combat.Reloading");
		UE_DEFINE_GAMEPLAY_TAG(SubSkilling, "Player.Combat.SubSkilling");
		UE_DEFINE_GAMEPLAY_TAG(Dashing, "Player.Movement.Dashing");
		UE_DEFINE_GAMEPLAY_TAG(Jumping, "Player.Movement.Jumping");
	}

	namespace Zombie
	{
	}

	namespace Common
	{
		/*
+GameplayTagList=(Tag="SetByCaller.Stat.BurnChance",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.BurnDamage",DevComment="")
		 */
		UE_DEFINE_GAMEPLAY_TAG(AttackPower, "SetByCaller.Stat.AttackPower");
		UE_DEFINE_GAMEPLAY_TAG(AttackSpeed, "SetByCaller.Stat.AttackSpeed");
		UE_DEFINE_GAMEPLAY_TAG(BulletRecycle, "SetByCaller.Stat.BulletRecycle");
		UE_DEFINE_GAMEPLAY_TAG(CriticalChance, "SetByCaller.Stat.CriticalChance");
		UE_DEFINE_GAMEPLAY_TAG(CriticalDamage, "SetByCaller.Stat.CriticalDamage");
		UE_DEFINE_GAMEPLAY_TAG(CurrentHP, "SetByCaller.Stat.CurrentHP");
		UE_DEFINE_GAMEPLAY_TAG(DashSpeed, "SetByCaller.Stat.DashSpeed");
		UE_DEFINE_GAMEPLAY_TAG(DefensePower, "SetByCaller.Stat.DefensePower");
		UE_DEFINE_GAMEPLAY_TAG(MaxHP, "SetByCaller.Stat.MaxHP");
		UE_DEFINE_GAMEPLAY_TAG(MaxRange, "SetByCaller.Stat.MaxRange");
		UE_DEFINE_GAMEPLAY_TAG(WalkSpeed, "SetByCaller.Stat.WalkSpeed");
		UE_DEFINE_GAMEPLAY_TAG(CurrentAmmo, "SetByCaller.Stat.CurrentAmmo");
		UE_DEFINE_GAMEPLAY_TAG(MaxAmmo, "SetByCaller.Stat.MaxAmmo");
		UE_DEFINE_GAMEPLAY_TAG(ShotSpread, "SetByCaller.Stat.ShotSpread");
	}
}
