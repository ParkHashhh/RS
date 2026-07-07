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
+GameplayTagList=(Tag="SetByCaller.Stat.AttackPower",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.AttackSpeed",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.BulletRecycle",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.BurnChance",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.BurnDamage",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.CriticalDamage",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.CurrentHP",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.DashSpeed",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.DefensePower",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.MaxHP",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.MaxRange",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.ShotSpread",DevComment="")
+GameplayTagList=(Tag="SetByCaller.Stat.WalkSpeed",DevComment="")
		 */
		UE_DEFINE_GAMEPLAY_TAG(AttackPower, "SetByCaller.Stat.AttackPower");
		UE_DEFINE_GAMEPLAY_TAG(CurrentHP, "SetByCaller.Stat.CurrentHP");
		UE_DEFINE_GAMEPLAY_TAG(DashSpeed, "SetByCaller.Stat.DashSpeed");
		UE_DEFINE_GAMEPLAY_TAG(DefensePower, "SetByCaller.Stat.DefensePower");
		UE_DEFINE_GAMEPLAY_TAG(MaxHP, "SetByCaller.Stat.MaxHP");
		UE_DEFINE_GAMEPLAY_TAG(MaxRange, "SetByCaller.Stat.MaxRange");
		UE_DEFINE_GAMEPLAY_TAG(WalkSpeed, "SetByCaller.Stat.WalkSpeed");
		UE_DEFINE_GAMEPLAY_TAG(CurrentAmmo, "SetByCaller.Stat.CurrentAmmo");
		UE_DEFINE_GAMEPLAY_TAG(MaxAmmo, "SetByCaller.Stat.MaxAmmo");

	}
}
