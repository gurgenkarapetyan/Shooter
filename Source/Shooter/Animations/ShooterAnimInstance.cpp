// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Shooter/Characters/ShooterCharacter.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() :
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.f),
	LastMovementOffsetYaw(0.f),
	bAiming(false),
	CharacterYaw(0.f),
	CharacterYawLastFrame(0.f),
	RootYawOffset(0.f)
{
	
}

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

	bAiming = ShooterCharacter->GetAiming();
	TurnInPlace();
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

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr)
	{
		return;
	}

	if (Speed > 0)
	{
		
	}
	else
	{
		CharacterYawLastFrame = CharacterYaw;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float YawDelta { CharacterYaw - CharacterYawLastFrame };

		RootYawOffset -= YawDelta;
		if (GEngine) GEngine->AddOnScreenDebugMessage(1, -1, FColor::Blue, FString::Printf(TEXT("CharacterYaw: %f"), CharacterYaw));
		if (GEngine) GEngine->AddOnScreenDebugMessage(2, -1, FColor::Red, FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset));
	}
}
