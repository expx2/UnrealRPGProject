// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/Package.h"

// 호출 함수 정보
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GetWorld()->GetPackage()->GetPIEInstanceID()) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_NETMODEINFO_ASC ((GetOwnerActor()->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GetOwnerActor()->GetWorld()->GetPackage()->GetPIEInstanceID()) : ((GetAvatarActor()->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_NETMODEINFO_GAMEPLAYABILITY ((GetOwningActorFromActorInfo()->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GetOwningActorFromActorInfo()->GetWorld()->GetPackage()->GetPIEInstanceID()) : ((GetAvatarActorFromActorInfo()->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_NETMODEINFO_ABILITYTASK ((Ability->GetAvatarActorFromActorInfo()->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), Ability->GetAvatarActorFromActorInfo()->GetWorld()->GetPackage()->GetPIEInstanceID()) : ((Ability->GetAvatarActorFromActorInfo()->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_NETMODEINFO_COMP ((GetOwner()->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GetOwner()->GetWorld()->GetPackage()->GetPIEInstanceID()) : ((GetOwner()->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))

//3_04
// 넷모드의 Local 및 Remote의 Role을 가져오는 매크로
#define LOG_LOCALROLEINFO *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetLocalRole()))
#define LOG_LOCALROLEINFO_ASC *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetOwnerActor()->GetLocalRole()))
#define LOG_LOCALROLEINFO_GAMEPLAYABILITY *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetOwningActorFromActorInfo()->GetLocalRole()))
#define LOG_LOCALROLEINFO_ABILITYTASK *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), Ability->GetAvatarActorFromActorInfo()->GetLocalRole()))
#define LOG_LOCALROLEINFO_COMP *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetOwner()->GetLocalRole()))

#define LOG_REMOTEROLEINFO *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetRemoteRole()))
#define LOG_REMOTEROLEINFO_ASC *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetOwnerActor()->GetRemoteRole()))
#define LOG_REMOTEROLEINFO_GAMEPLAYABILITY *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetOwningActorFromActorInfo()->GetRemoteRole()))
#define LOG_REMOTEROLEINFO_ABILITYTASK *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), Ability->GetAvatarActorFromActorInfo()->GetRemoteRole()))
#define LOG_REMOTEROLEINFO_COMP *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetOwner()->GetRemoteRole()))

#define UR_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("%s %s"), LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define UR_SERVERLOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s|%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define UR_SERVERLOG_ASC(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s|%s] %s %s"), LOG_NETMODEINFO_ASC, LOG_LOCALROLEINFO_ASC, LOG_REMOTEROLEINFO_ASC, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define UR_SERVERLOG_GA(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s|%s] %s %s"), LOG_NETMODEINFO_GAMEPLAYABILITY, LOG_LOCALROLEINFO_GAMEPLAYABILITY, LOG_REMOTEROLEINFO_GAMEPLAYABILITY, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define UR_SERVERLOG_ABILITYTASK(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s|%s] %s %s"), LOG_NETMODEINFO_ABILITYTASK, LOG_LOCALROLEINFO_ABILITYTASK, LOG_REMOTEROLEINFO_ABILITYTASK, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define UR_SERVERLOG_COMP(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s|%s] %s %s"), LOG_NETMODEINFO_COMP, LOG_LOCALROLEINFO_COMP, LOG_REMOTEROLEINFO_COMP, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

#define UR_SIMPLE_SERVERLOG UR_SERVERLOG(LogUR, Log, TEXT(""))

DECLARE_LOG_CATEGORY_EXTERN(LogUR, Log, All);

