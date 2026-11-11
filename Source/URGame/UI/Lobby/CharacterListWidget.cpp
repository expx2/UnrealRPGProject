// Written by EXPx2


#include "UI/Lobby/CharacterListWidget.h"

#include "Components/ListView.h"
#include "DataAssets/PlayerCharactersSet.h"
#include "Definitions/CharacterListItem.h"
#include "GameInstances/URGameInstance.h"

void UCharacterListWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UURGameInstance* URGI = GetGameInstance<UURGameInstance>())
	{
		SetListView(URGI->PlayerCharactersSet);
	}
}

void UCharacterListWidget::SetListView(UPlayerCharactersSet* PlayerCharactersSet)
{
	if (!PlayerCharactersSet)
	{
		return;
	}
	
	ListView->ClearListItems();
	ListView->OnItemClicked().RemoveAll(this);
	
	ListView->OnItemClicked().AddUObject(this, &ThisClass::HandleItemClicked);
	
	TArray<UCharacterListItem*> Items;
	for (FPlayerCharacterInfo& CharacterInfo : PlayerCharactersSet->Characters)
	{
		UCharacterListItem* Item = NewObject<UCharacterListItem>(this);
		Item->Info = CharacterInfo;
		Items.Add(Item);
	}
	ListView->SetListItems(Items);
}

void UCharacterListWidget::HandleItemClicked(UObject* Item)
{
	K2_HandleItemClicked(Item);
	OnClicked.Broadcast(Item);
}
