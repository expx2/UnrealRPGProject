// Written by EXPx2


#include "UI/Lobby/CharacterListEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "DataAssets/PlayerCharactersSet.h"
#include "Definitions/CharacterListItem.h"

void UCharacterListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	UCharacterListItem* ListItem = Cast<UCharacterListItem>(ListItemObject);
	
	if (!ListItem)
	{
		return;
	}
	
	ImgPortrait->SetBrushFromTexture(ListItem->Info.Portrait.LoadSynchronous());
	TxtName->SetText(FText::FromName(ListItem->Info.CharacterCode));
}
