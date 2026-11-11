// Written by EXPx2


#include "AbilityActionTask.h"

bool UAbilityActionTask::FinishTask(const bool bIsSuccess)
{
	if (bIsFinished)
	{
		return false;
	}
	
	bIsFinished = true;
	bTickingTask = false;
	
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (bIsSuccess)
		{
			OnCompleted.Broadcast(this);
		}
		else
		{
			OnFailed.Broadcast(this);
		}
	}
	
	EndTask();
	return true;
}
