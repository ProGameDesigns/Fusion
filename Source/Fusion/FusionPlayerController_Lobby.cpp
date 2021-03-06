// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"

#include "Online/FusionGame_Lobby.h"

#include "FusionGameInstance.h"
#include "FusionHUD.h"

#include "FusionGameViewportClient.h"

#include "Online/NetworkLobbyPlayerState.h"

#include "Widgets/Menus/ServerMenu_Widget.h" // TODO: is this needed?

#include "Widgets/Menus/Lobby/LobbyMenu_Widget.h"

#include "FusionPlayerController_Lobby.h"



void AFusionPlayerController_Lobby::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;



	if (IsLocalPlayerController())
	{
		GetFusionHUD()->CreateGameWidgets();

		UFusionGameInstance* FGI = GetWorld() != NULL ? Cast<UFusionGameInstance>(GetWorld()->GetGameInstance()) : NULL;
		FGI->GotoState(FusionGameInstanceState::Lobby);
	}


	// show the lobby and view it as soon as the player gets in
	// doing this inside the game instance


	// Timer handle to call the RequestServerPlayerListUpdate() later
	// the reason I use a timer is to give the PlayerState enough time to set up the player name
	FTimerHandle RequestServerPlayerListUpdateHanndle;

	//Call the server to request PlayerList Update after 0.01s
	GetWorld()->GetTimerManager().SetTimer(RequestServerPlayerListUpdateHanndle, this, &AFusionPlayerController_Lobby::RequestServerPlayerListUpdate, 0.05f, false);


	OnUpdateUMGPlayerList().AddUObject(this, &AFusionPlayerController_Lobby::OnUpdateUMGPlayerList);
	OnReceiveChatMessage().AddUObject(this, &AFusionPlayerController_Lobby::OnReceiveChatMessage);

}

AFusionHUD* AFusionPlayerController_Lobby::GetFusionHUD() const
{
	return Cast<AFusionHUD>(GetHUD());
}


void AFusionPlayerController_Lobby::SendChatMessage(const FText & ChatMessage)
{
	// if this is the server call the game mode to prodcast the Chat Message
	if (Role == ROLE_Authority)
	{
		AFusionGame_Lobby* GM = Cast<AFusionGame_Lobby>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			//Add the player's name to the Chat Message then send it to the server
			const FText OutChatMessage = FText::FromString(PlayerState->PlayerName + ": " + ChatMessage.ToString());
			GM->ProdcastChatMessage(OutChatMessage);
		}
	}
	else //else call the serverside function on this
		Server_SendChatMessage(ChatMessage);
}


void AFusionPlayerController_Lobby::Server_SendChatMessage_Implementation(const FText & ChatMessage)
{
	//if not the server call the server side function to 
	SendChatMessage(ChatMessage);
}

//called from the GameMode when it prodcast the chat message to all connected players
void AFusionPlayerController_Lobby::Client_ReceiveChatMessage_Implementation(const FText & ChatMessage)
{
	//call the Recive message function to show it on UMG
	OnReceiveChatMessage().Broadcast(ChatMessage);
}


void AFusionPlayerController_Lobby::KickPlayer(int32 PlayerIndex)
{
	//if the player is the host, get the game mode and send it to kick the player from the game
	if (Role == ROLE_Authority)
	{
		AFusionGame_Lobby* GM = Cast<AFusionGame_Lobby>(GetWorld()->GetAuthGameMode());
		if (GM)
			GM->KickPlayer(PlayerIndex);
	}
}

//called from the game mode when the player is kicked by the host to make the player destroy his session and leave game
void AFusionPlayerController_Lobby::Client_GotKicked_Implementation()
{
	//get the game Instance to make the player destroy his session and leave game

	UFusionGameInstance* NetworkedGameInstance = Cast<UFusionGameInstance>(GetGameInstance());
	

	if (NetworkedGameInstance)
	{
		//show the player that he got kicked in message in UMG
		NetworkedGameInstance->ShowErrorMessage(FText::FromString(TEXT("You got kicked from the server")));
		//make the player call the game Instance to destroy his session
		NetworkedGameInstance->DestroySessionAndLeaveGame();
	}
}

//called from server and passes in an array of player infos
void AFusionPlayerController_Lobby::Client_UpdatePlayerList_Implementation(const TArray<FLobbyPlayerInfo>& PlayerInfoArray)
{
	OnUpdateUMGPlayerList().Broadcast(PlayerInfoArray);
}

//called fro mthe palyer on begin play to request the player info array fro mthe server
void AFusionPlayerController_Lobby::RequestServerPlayerListUpdate()
{
	// if this is the server call the game mode to request info
	if (Role == ROLE_Authority)
	{
		AFusionGame_Lobby* GM = Cast<AFusionGame_Lobby>(GetWorld()->GetAuthGameMode());

		if (GM)
			GM->PlayerRequestUpdate();
	}
	else //else call the serverside function on this
		Server_RequestServerPlayerListUpdate();
}


void AFusionPlayerController_Lobby::Server_RequestServerPlayerListUpdate_Implementation()
{
	//if not the server call the server side function
	RequestServerPlayerListUpdate();
}

void AFusionPlayerController_Lobby::SetIsReadyState(bool NewReadyState)
{
	if (Role == ROLE_Authority)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("RoleAuthority/ server called"));
		ANetworkLobbyPlayerState* NetworkedPlayerState = Cast<ANetworkLobbyPlayerState>(PlayerState);
		if (NetworkedPlayerState)
			NetworkedPlayerState->bIsReady = NewReadyState;
		RequestServerPlayerListUpdate();
	}
	else
		Server_SetIsReadyState(NewReadyState);

}

void AFusionPlayerController_Lobby::Server_SetIsReadyState_Implementation(bool NewReadyState)
{
	//bool PassedNewState = NewReadyState;
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Role not Authority call server side"));
	SetIsReadyState(NewReadyState);
}

bool AFusionPlayerController_Lobby::CanGameStart() const
{
	if (Role == ROLE_Authority)
	{
		AFusionGame_Lobby* GM = Cast<AFusionGame_Lobby>(GetWorld()->GetAuthGameMode());
		if (GM)
			return GM->IsAllPlayerReady();
	}
	return false;
}

void AFusionPlayerController_Lobby::StartGame()
{
	//if the player is the host, get the game mode and send it to start the game
	if (Role == ROLE_Authority)
	{
		AFusionGame_Lobby* GM = Cast<AFusionGame_Lobby>(GetWorld()->GetAuthGameMode());
		if (GM)
			GM->StartGameFromLobby();
	}
}

void AFusionPlayerController_Lobby::OnReceiveChatMessage(const FText& ChatMessage)
{
	// pass the incoming chat message to UMG
	GetFusionHUD()->GetLobbyMenuWidget()->OnReceiveChatMessageComplete().Broadcast(ChatMessage);
}

void AFusionPlayerController_Lobby::OnUpdateUMGPlayerList(const TArray<struct FLobbyPlayerInfo>& PlayerInfoArray)
{
	// passes in the array that the server sent to UMG so the player can see it
	GetFusionHUD()->GetLobbyMenuWidget()->OnUpdatePlayerList().Broadcast(PlayerInfoArray);
}


void AFusionPlayerController_Lobby::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);

	if (GetWorld() != NULL)
	{
		UFusionGameViewportClient* FusionViewport = Cast<UFusionGameViewportClient>(GetWorld()->GetGameViewport());

		if (FusionViewport != NULL)
		{
			FusionViewport->ShowLoadingScreen();
		}

		AFusionHUD* FusionHUD = Cast<AFusionHUD>(GetHUD());
		if (FusionHUD != nullptr)
		{
			// Passing true to bFocus here ensures that focus is returned to the game viewport.
			//FusionHUD->HideLobbyMenu();
		}
	}
}
