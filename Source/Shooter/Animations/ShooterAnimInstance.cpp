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
	TIPCharacterYaw(0.f),
	TIPCharacterYawLastFrame(0.f),
	RootYawOffset(0.f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip),
	CharacterRotation(FRotator(0.f)),
	CharacterRotationLastFrame(FRotator(0.f)),
	YawDelta(0.f),
	bCrouching(false)
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

	bCrouching = ShooterCharacter->GetCrouching();
	bReloading = (ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading);
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

	SetOffsetState();
	TurnInPlace();
	Lean(DeltaTime);
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

void UShooterAnimInstance::SetOffsetState()
{
	if (bReloading)
	{
		OffsetState = EOffsetState::EOS_Reloading;
	}
	else if (bIsInAir)
	{
		OffsetState = EOffsetState::EOS_InAir;
	}
	else if (ShooterCharacter->GetAiming())
	{
		OffsetState = EOffsetState::EOS_Aiming;
	}
	else
	{
		OffsetState = EOffsetState::EOS_Hip;
	}
}


void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr)
	{
		return;
	}

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;
	
	if (Speed > 0 || bIsInAir)
	{
		// Don't want to turn in place; Character is moving
		RootYawOffset = 0.f;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float TIPYawDelta{ TIPCharacterYaw - TIPCharacterYawLastFrame };

		// Root Yaw Offset, updated and clamped to [-180, 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

		// 1.0 if turning, 0.0 if not
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if (Turning > 0)
		{
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			// RootYawOffset > 0, -> Turning Left. RootYawOffset < 0, -> Turning Right.
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
			if (ABSRootYawOffset > 90.f)
			{
				const float YawExcess{ ABSRootYawOffset - 90.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
	}
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
	{
		return;
	}

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f);
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
}
