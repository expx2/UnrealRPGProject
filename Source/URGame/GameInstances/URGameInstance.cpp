// Written by EXPx2


#include "GameInstances/URGameInstance.h"

#include "Character/URPlayerCharacter.h"
#include "DataAssets/PlayerCharactersSet.h"

UClass* UURGameInstance::GetCharacterClass(const FName& CharacterCode) const
{
	if (PlayerCharactersSet)
	{
		if (FPlayerCharacterInfo* Info = PlayerCharactersSet->FindByCharacterCode(CharacterCode))
		{
			if (UClass* CharacterClass = Info->CharacterClass.Get())
			{
				return CharacterClass;
			}
		}
	}
	
	return nullptr;
}
