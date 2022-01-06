// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Shooter/Characters/ShooterCharacter.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (!ShooterCharacter)
	{
		return;
	}

	SetCharacterSpeed(Speed);

	bIsInAir = IsCharacterInTheAir();
	bIsAccelerating = IsCharacterAccelerating();

	FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
	MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation,AimRotation).Yaw;

	if (ShooterCharacter->GetVelocity().Size() > 0.f)
	{
		LastMovementOffsetYaw = MovementOffsetYaw;
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
