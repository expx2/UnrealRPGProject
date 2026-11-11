// Written by EXPx2


#include "Input/URInputComponent.h"

void UURInputComponent::ResetBindings()
{
	for (const uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
