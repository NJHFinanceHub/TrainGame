#include "SEENPCAnimInstance.h"
#include "TrainGame/AI/SEENPCCharacter.h"

void USEENPCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ASEENPCCharacter* NPC = Cast<ASEENPCCharacter>(OwnerCharacter);
	if (!NPC) return;

	NPCState = NPC->GetCurrentState();
	NPCClassType = NPC->GetNPCClass();
	DetectionLevel = NPC->GetDetectionLevel();

	bIsPatrolling = (NPCState == ENPCAIState::Patrolling);
	bIsInCombat = (NPCState == ENPCAIState::Combat);
	bIsAlerted = (NPCState == ENPCAIState::Chasing || NPCState == ENPCAIState::Investigating);
}
