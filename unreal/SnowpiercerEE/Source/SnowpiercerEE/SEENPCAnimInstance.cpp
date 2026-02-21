#include "SEENPCAnimInstance.h"
#include "TrainGame/AI/SEENPCCharacter.h"

void USEENPCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ASEENPCCharacter* NPC = Cast<ASEENPCCharacter>(OwnerCharacter);
	if (!NPC) return;

	NPCState = NPC->GetCurrentState();
	NPCClass = NPC->GetNPCClass();
	DetectionLevel = NPC->GetDetectionLevel();

	bIsPatrolling = (NPCState == ESEENPCState::Patrolling);
	bIsInCombat = (NPCState == ESEENPCState::Combat);
	bIsAlerted = (NPCState == ESEENPCState::Alerted || NPCState == ESEENPCState::Suspicious);
}
