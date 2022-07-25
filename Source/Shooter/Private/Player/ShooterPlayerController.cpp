// Fill out your copyright notice in the Description page of Project Settings.

#include "Shooter/Public/Player/ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"

AShooterPlayerController::AShooterPlayerController()
{
	
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayWidget)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayWidget);

		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
