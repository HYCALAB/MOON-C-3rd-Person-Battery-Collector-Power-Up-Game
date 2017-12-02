// Fill out your copyright notice in the Description page of Project Settings.
#include "SpawnVolume.h"
#include "BatteryCollector.h"
#include "Kismet/KismetMathLibrary.h"
#include "Pickup.h"
#include "Components/BoxComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WhereToSpawn = CreateDefaultSubobject<UBoxComponent>(TEXT("WhereToSpawn"));

	//박스 컴포넌트를 루트로 설정하는 이유는 새 컴포넌트가 추가되면 이를 중심(wheretospawn)으로 삼게된다.
	RootComponent = WhereToSpawn;

	// Set the spawn delay range
	SpawnDelayRangeLow = 1.0f;
	SpawnDelayRangeHigh = 4.5f;

}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	// 아이템 샌성과정
	SpawnDelay = FMath::FRandRange(SpawnDelayRangeLow, SpawnDelayRangeHigh);
	// 핸들을가져다가 SpawnTimer를 위해 spawnpickup에 바인딩하고 spawndelay가 지나면 호출하고 반복은 하지 않음
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &ASpawnVolume::SpawnPickup, SpawnDelay, false);
}

// Called every frame
void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVector ASpawnVolume::GetRandomPointVolume()
{
	FVector SpawnOrigin = WhereToSpawn->Bounds.Origin;
	FVector SpawnExtent = WhereToSpawn->Bounds.BoxExtent;

	return UKismetMathLibrary::RandomPointInBoundingBox(SpawnOrigin, SpawnExtent);
}


void ASpawnVolume::SpawnPickup()
{
	if (WhatToSpawn != NULL)
	{
		UWorld* const world = GetWorld();
		if (world) {
			//보이드플레이트 코드?? + 누가 아이템을 만들었는지, 누가 만들게 하였는지를 명시하는 과정
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;

			FVector SpawnLocation = GetRandomPointVolume();

			FRotator SpawnRotation;
			SpawnRotation.Yaw = FMath::FRand() * 360.0f;
			SpawnRotation.Roll = FMath::FRand() * 360.0f;
			SpawnRotation.Pitch = FMath::FRand() * 360.0f;

			APickup* const SpawnedPickup = world->SpawnActor<APickup>(WhatToSpawn, SpawnLocation, SpawnRotation);

			// 아이템 샌성과정
			SpawnDelay = FMath::FRandRange(SpawnDelayRangeLow, SpawnDelayRangeHigh);
			// 핸들을가져다가 SpawnTimer를 위해 spawnpickup에 바인딩하고 spawndelay가 지나면 호출하고 반복은 하지 않음
			GetWorldTimerManager().SetTimer(SpawnTimer, this, &ASpawnVolume::SpawnPickup, SpawnDelay, false);
		}
	}
}