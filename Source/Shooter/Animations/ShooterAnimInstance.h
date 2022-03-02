// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Shooter/Library/OffsetStateEnumLibrary.h"
#include "ShooterAnimInstance.generated.h"

class AShooterCharacter;

/**
* 
*/
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UShooterAnimInstance();
	
	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	private:
	/**
	* Set the lateral speed of the character from the velocity.
	* @param CharacterSpeed passing by reference and setting the speed value.
	*/
	void SetCharacterSpeed(float& CharacterSpeed);

	/** Returns true if the character is in the air. */
	bool IsCharacterInTheAir();
	/** Returns true if the character is moving. */
	bool IsCharacterAccelerating();

protected:
	/** Handle turning in place variables. */
	void TurnInPlace();

	/** Handle calculation for leaning while running. */
	void Lean(float DeltaTime);
private:
	/** Set Offset state. */
	void SetOffsetState();
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	AShooterCharacter* ShooterCharacter;

	/** The speed of the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	float Speed;

	/** Whether or not the character is in the air. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	bool bIsInAir;
	/** Whether or not the character is moving. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	bool bIsAccelerating;

	/** Offset yaw user for strafing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(AllowPrivateAccess="true"))
	float MovementOffsetYaw;

	/** Offset yaw the frame before we stopped moving. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(AllowPrivateAccess="true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess="true"))
	bool bAiming;

	/** Yaw of the Character this frame; Only updated when standing still and not in air. */
	float TIPCharacterYaw;

	/** Yaw of the Character the previous frame; Only updated when standing still and not in air. */
	float TIPCharacterYawLastFrame;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn in Place", meta=(AllowPrivateAccess="true"))
	float RootYawOffset;

	/** Rotation curve value this frame. */
	float RotationCurve;
	
	/** Rotation curve value last frame. */
	float RotationCurveLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn in Place", meta=(AllowPrivateAccess="true"))
	float Pitch;

	/** True when reloading, used to prevent Aim offset when reloading. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn in Place", meta=(AllowPrivateAccess="true"))
	bool bReloading;

	/** Offset state; used to determine which Aim offset to use. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn in Place", meta=(AllowPrivateAccess="true"))
	EOffsetState OffsetState;

	/** Character Yaw this frame. */
	FRotator CharacterRotation;

	/** Character Yaw last frame. */
	FRotator CharacterRotationLastFrame;

	/** Used for leaning in the running blendspace. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Lean", meta=(AllowPrivateAccess="true"))
	float YawDelta;
};
