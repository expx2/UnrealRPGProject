// Written by EXPx2


#include "DataAssets/URPopupWidgetMapping.h"

#include "UI/URPopupWidget.h"

UClass* UURPopupWidgetMapping::GetWidgetClass(const EPopupType Type)
{
	if (auto* PopupClass = BindingMap.Find(Type))
	{
		return PopupClass->Get();
	}
	
	return nullptr;
}
