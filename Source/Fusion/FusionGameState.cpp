// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "FusionGameMode.h"
#include "FusionGameInstance.h"

#include "FusionPlayerController.h"

#include "FusionGameState.h"

AFusionGameState::AFusionGameState(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NumTeams = 0;
	RemainingTime = 0;
	bTimerPaused = false;
}

void AFusionGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFusionGameState, NumTeams);
	DOREPLIFETIME(AFusionGameState, RemainingTime);
	DOREPLIFETIME(AFusionGameState, bTimerPaused);
	DOREPLIFETIME(AFusionGameState, TeamScores);

}

void AFusionGameState::GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const
{
	OutRankedMap.Empty();

	//first, we need to go over all the PlayerStates, grab their score, and rank them
	TMultiMap<int32, AFusionPlayerState*> SortedMap;
	for (int32 i = 0; i < PlayerArray.Num(); ++i)
	{
		int32 Score = 0;
		AFusionPlayerState* CurPlayerState = Cast<AFusionPlayerState>(PlayerArray[i]);
		if (CurPlayerState && (CurPlayerState->GetTeamNum() == TeamIndex))
		{
			SortedMap.Add(FMath::TruncToInt(CurPlayerState->Score), CurPlayerState);
		}
	}

	//sort by the keys
	SortedMap.KeySort(TGreater<int32>());

	//now, add them back to the ranked map
	OutRankedMap.Empty();

	int32 Rank = 0;
	for (TMultiMap<int32, AFusionPlayerState*>::TIterator It(SortedMap); It; ++It)
	{
		OutRankedMap.Add(Rank++, It.Value());
	}

}

void AFusionGameState::RequestFinishAndExitToMainMenu()
{
	if (AuthorityGameMode)
	{
		// we are server, tell the gamemode
		AFusionGameMode* const GameMode = Cast<AFusionGameMode>(AuthorityGameMode);
		if (GameMode)
		{
			GameMode->RequestFinishAndExitToMainMenu();
		}
	}
	else
	{
		// we are client, handle our own business
		AFusionPlayerController* const PrimaryPC = Cast<AFusionPlayerController>(GetGameInstance()->GetFirstLocalPlayerController());

		if (PrimaryPC)
		{
			check(PrimaryPC->GetNetMode() == ENetMode::NM_Client);
			PrimaryPC->HandleReturnToMainMenu();
		}
	}

}
