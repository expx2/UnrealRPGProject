// Written by EXPx2


#include "GameplayTags/URGameplayTags.h"

struct FGameplayTag;

namespace URGameplayTags
{
	// ========== Character Movement State ==========
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_MovementMode_Walking, "Character.MovementMode.Walking", "Character is in walking state")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_MovementMode_NavWalking, "Character.MovementMode.NavWalking", "Character is in navigation walking state")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_MovementMode_Falling, "Character.MovementMode.Falling", "Character is in falling state")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_MovementMode_Swimming, "Character.MovementMode.Swimming", "Character is in swimming state")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_MovementMode_Flying, "Character.MovementMode.Flying", "Character is in flying state")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_MovementMode_Custom, "Character.MovementMode.Custom", "Character is in custom movement state")
	
	// Unreal Movement Modes
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ EMovementMode::MOVE_Walking, Character_MovementMode_Walking },
		{ EMovementMode::MOVE_NavWalking, Character_MovementMode_NavWalking },
		{ EMovementMode::MOVE_Falling, Character_MovementMode_Falling },
		{ EMovementMode::MOVE_Swimming, Character_MovementMode_Swimming },
		{ EMovementMode::MOVE_Flying, Character_MovementMode_Flying },
		{ EMovementMode::MOVE_Custom, Character_MovementMode_Custom }
	};
	
	// ========== Character State ==========
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Dead, "Character.State.Dead", "Character is dead")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_InCombat, "Character.State.InCombat", "Character is in combat")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Untargetable, "Character.State.Untargetable", "Character can't be target")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_CantTakeDamage, "Character.State.CantTakeDamage", "Character can't take damage")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Blocked_PhysicsRotation, "Character.State.Blocked.PhysicsRotation", "")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Invincible, "Character.State.Invincible", "")
	
	// ========== Player State ==========
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player_State_InputBlocked_Movement, "Player.State.InputBlocked.Movement", "Player input is blocked for movement")
	
	// ========== SetByCaller DataTag ==========
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_DamageMultiplier, "SetByCaller.DamageMultiplier", "Attack damage multiplier for damage execution")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_ManaCost, "SetByCaller.ManaCost", "SetByCaller DataTag for Mana Cost")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Cooldown, "SetByCaller.Cooldown", "SetByCaller DataTag for Cooldown")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_ChainAbility, "SetByCaller.ChainAbility", "SetByCaller DataTag for Chain Ability")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_GrantTag, "SetByCaller.GrantTag", "SetByCaller DataTag for Grant Tag")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Duration, "SetByCaller.Duration", "SetByCaller DataTag for Duration")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Value, "SetByCaller.Value", "SetByCaller DataTag for Value")
	
	// ========== TargetData Tags ==========
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TargetData_Time, "TargetData.Time", "")
	
	// ========== TAGS ==========
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Test, "TAG.Test", "")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_CanActivateAbilityTag, "TAG.Ability.Fail.CanActivateAbilityTag", "Ability failed because the ability can't be activated")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_Cooldown, "TAG.Ability.Fail.Cooldown", "Ability failed because it is on cooldown")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_Cost, "TAG.Ability.Fail.Cost", "Ability failed because it doesn't have enough cost")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_Networking, "TAG.Ability.Fail.Networking", "Ability failed because of networking issues")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_Undefined, "TAG.Ability.Fail.Undefined", "Ability failed because of undefined error")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_IsDead, "TAG.Ability.Fail.IsDead", "Ability failed because the character is dead")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_NoTarget, "TAG.Ability.Fail.NoTarget", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_InvalidTarget, "TAG.Ability.Fail.InvalidTarget", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_MoveFailed, "TAG.Ability.Fail.MoveFailed", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_PathFollowingFailed, "TAG.Ability.Fail.PathFollowingFailed", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_Condition, "TAG.Ability.Fail.Condition", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Fail_BadRequest, "TAG.Ability.Fail.BadRequest", "")

	// ========== Gameplay Events ==========
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death, "GameplayEvent.Death", "")

	// ========== Gameplay Cues ==========
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_DamageTaken, "GameplayCue.DamageTaken", "Character takes damage")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_Buff_Invincible, "GameplayCue.Buff.Invincible", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_Buff_Defense, "GameplayCue.Buff.Defense", "")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_HitImpact_Burst, "GameplayCue.HitImpact.Burst", "")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_HitImpact_Greystone_Primary, "GameplayCue.HitImpact.Greystone.Primary", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_HitImpact_Greystone_ClearAPath, "GameplayCue.HitImpact.Greystone.ClearAPath", "")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_HitImpact_Twinblast_Primary, "GameplayCue.HitImpact.Twinblast.Primary", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_HitImpact_Twinblast_Primary_Left, "GameplayCue.HitImpact.Twinblast.Primary.Left", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_HitImpact_Twinblast_Primary_Right, "GameplayCue.HitImpact.Twinblast.Primary.Right", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_HitImpact_Twinblast_Ultimate, "GameplayCue.HitImpact.Twinblast.Ultimate", "")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_HitImpact_Zako_Ability1, "GameplayCue.HitImpact.Zako.Ability1", "")
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_HitImpact_Zako_Shooter_Ability1, "GameplayCue.HitImpact.Zako_Shooter.Ability1", "")
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_HitImpact_Boss_Ability1, "GameplayCue.HitImpact.Boss.Ability1", "")
	
	
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_Particle_Burst, "GameplayCue.Particle.Burst", "")
	
	// ========== Cheat ==========
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_InfiniteHealth, "Cheat.InfiniteHealth", "")
	
	// ========== (De)Buff ==========
	URGAME_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(Buff_Bleed, "Buff.Bleed", "")
}
