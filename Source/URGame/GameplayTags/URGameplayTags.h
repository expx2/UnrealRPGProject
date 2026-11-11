// Written by EXPx2

#pragma once

#include "NativeGameplayTags.h"

namespace URGameplayTags
{
	// ========== Character Movement State ==========
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_MovementMode_Walking);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_MovementMode_NavWalking);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_MovementMode_Falling);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_MovementMode_Swimming);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_MovementMode_Flying);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_MovementMode_Custom);
	
	// Unreal Movement Modes
	URGAME_API extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
	
	// ========== Character State ==========
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dead); // 죽음
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_InCombat); // 전투 상태
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Untargetable); // 타겟팅 불가
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_CantTakeDamage); // 대미지 받을 수 없음
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Blocked_PhysicsRotation); // PhysicsRotation 제한
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Invincible); // 무적
	
	// ========== Player State ==========
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_InputBlocked_Movement);	// 이동 입력 제한

	// ========== SetByCaller DataTag ==========
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_DamageMultiplier);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_ManaCost);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Cooldown);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_ChainAbility);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_GrantTag);
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Duration);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Value);
	
	// ========== TargetData Tags ==========
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TargetData_Time);
	
	// ========== TAGS ==========
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Test);
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_CanActivateAbilityTag);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_Cooldown);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_Cost);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_Networking);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_Undefined); // 알 수 없는 에러
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_IsDead); // 사망함
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_NoTarget); // 타겟 없음
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_InvalidTarget); // 타겟이 유효하지 않음
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_MoveFailed); // 자동 이동 실패
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_PathFollowingFailed); // 자동 이동 실패
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_Condition); // 조건 미충족 (Blocked, Missing)
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Fail_BadRequest); // 비정상 경로
	
	// ========== Gameplay Events ==========
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);

	// ========== Gameplay Cues ==========
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_DamageTaken);
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Buff_Invincible);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Buff_Defense);
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Burst);
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Greystone_Primary);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Greystone_ClearAPath);
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Twinblast_Primary);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Twinblast_Primary_Left);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Twinblast_Primary_Right);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Twinblast_Ultimate);
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Zako_Ability1);
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Zako_Shooter_Ability1);
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Boss_Ability1);
	
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Particle_Burst);
	
	// ========== Cheat ==========
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_InfiniteHealth);
	
	// ========== (De)Buff ==========
	URGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Buff_Bleed);
	
}
