// Written by EXPx2


#include "DataAssets/PlayerCharactersSet.h"

FPlayerCharacterInfo* UPlayerCharactersSet::FindByCharacterCode(FName CharacterCode)
{
	return Characters.FindByPredicate([CharacterCode](FPlayerCharacterInfo& Info)
	{
		return Info.CharacterCode == CharacterCode;
	});
}
