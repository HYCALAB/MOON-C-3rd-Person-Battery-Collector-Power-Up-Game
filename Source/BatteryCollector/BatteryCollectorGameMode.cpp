// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "BatteryCollectorGameMode.h"
#include "BatteryCollectorCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "SpawnVolume.h"
#include <Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h>
#include <Runtime/Engine/Classes/Components/SkeletalMeshComponent.h>

ABatteryCollectorGameMode::ABatteryCollectorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	// base decay rate
	DecayRate = 0.02f;
}

void ABatteryCollectorGameMode::BeginPlay() {
	Super::BeginPlay();

	// Find all spawn volume actors
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundActors);

	for (auto Actor : FoundActors) {
		ASpawnVolume* SpawnVolumeActor = Cast<ASpawnVolume>(Actor);
		if (SpawnVolumeActor) {
			SpawnVolumeActors.AddUnique(SpawnVolumeActor);
		}
	}

	SetCurrentState(EBatteryPlayState::EPlaying);

	ABatteryCollectorCharacter* MyCharacter = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter) {
		PowerToWin = (MyCharacter->GetInitialPower()) * 1.25f;
	}

	if (HUDWidgetClass != nullptr) {
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (CurrentWidget != nullptr) {
			CurrentWidget->AddToViewport();
		}
	}

}

void ABatteryCollectorGameMode::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	ABatteryCollectorCharacter* MyCharacter = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter) {
		// If our power is greater than needed to win, set the game's state to Won
		if (MyCharacter->GetCurrentPower() > PowerToWin) {
			SetCurrentState(EBatteryPlayState::EWon);
		}
		else if (MyCharacter->GetCurrentPower() > 0) {
			MyCharacter->UpdatePower(-DeltaTime * DecayRate * (MyCharacter->GetInitialPower()));
		}
		else {
			SetCurrentState(EBatteryPlayState::EGameOver);
		}
	}
}

float ABatteryCollectorGameMode::GetPowerToWin() const {
	return PowerToWin;
}

EBatteryPlayState ABatteryCollectorGameMode::GetCurrentState() const {
	return CurrentState;
}

void ABatteryCollectorGameMode::SetCurrentState(EBatteryPlayState NewState) {
	CurrentState = NewState;
	HandleNewState(NewState);
}

void ABatteryCollectorGameMode::HandleNewState(EBatteryPlayState NewState) {
	switch (NewState) {

	case EBatteryPlayState::EPlaying: {
		for (ASpawnVolume* volume : SpawnVolumeActors) {
			volume->SetSpawningActive(true);
		}
	}
	break;
	case EBatteryPlayState::EWon: {
		for (ASpawnVolume* volume : SpawnVolumeActors) {
			volume->SetSpawningActive(false);
		}
	}
	break;
	case EBatteryPlayState::EGameOver: {
		for (ASpawnVolume* volume : SpawnVolumeActors) {
			volume->SetSpawningActive(false);
		}
		//block player input
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (PlayerController) {
			PlayerController->SetCinematicMode(true, false, false, true, true);
		}
		// ав╢б ╬ю - ragdoll Effect
		ACharacter* MyCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
		if (MyCharacter) {
			MyCharacter->GetMesh()->SetSimulatePhysics(true);
			MyCharacter->GetMovementComponent()->MovementState.bCanJump = false;
		}
	}
	break;

	default:
	case EBatteryPlayState::EUnknown:
		break;
	
	}
}