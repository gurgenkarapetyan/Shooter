// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Shooter/Characters/ShooterCharacter.h"

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if (ShooterCharacter)
	{
		SetCharacterSpeed(Speed);
		
		bIsInAir = IsCharacterInTheAir();
		bIsAccelerating = IsCharacterAccelerating();
	}
}

void UShooterAnimInstance::SetCharacterSpeed(float& CharacterSpeed)
{
	FVector Velocity = ShooterCharacter->GetVelocity();
	Velocity.Z = 0;
	CharacterSpeed = Velocity.Size();
}

bool UShooterAnimInstance::IsCharacterInTheAir()
{
	return ShooterCharacter->GetCharacterMovement()->IsFalling();
}

bool UShooterAnimInstance::IsCharacterAccelerating()
{
	return (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f);
}
