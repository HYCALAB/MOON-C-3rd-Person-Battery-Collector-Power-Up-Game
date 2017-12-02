// Fill out your copyright notice in the Description page of Project Settings.

#include "BatteryPickup.h"
#include "BatteryCollector.h"

ABatteryPickup::ABatteryPickup()
{
	GetMesh()->SetSimulatePhysics(true);

}

void ABatteryPickup::WasCollected_Implementation() {
	Super::WasCollected_Implementation();
	Destroy();

}