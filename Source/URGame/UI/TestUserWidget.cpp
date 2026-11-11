// Written by EXPx2


#include "UI/TestUserWidget.h"

#include "URGame.h"

void UTestUserWidget::ReceiveActor_Implementation(AActor* Actor)
{
	UR_LOG(LogUR, Log, TEXT("Script Set Actor %s"), *Actor->GetName());
}

void UTestUserWidget::ClearActor_Implementation()
{
	UR_LOG(LogUR, Log, TEXT("Script Clear"));
}
