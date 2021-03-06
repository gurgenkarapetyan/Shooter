// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Shooter/Library/OffsetStateEnumLibrary.h"
#include "Shooter/Library/WeaponTypeEnumLibrary.h"
#include "ShooterAnimInstance.generated.h"

class AShooterCharacter;

UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UShooterAnimInstance();
	
	virtual void NativeInitializeAnimation() override;

	/**
	 * Updating properties for animation states.
	 * Called every frame from the animation blueprint from event graph
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(const float DeltaTime);

protected:
	/** Handle turning in place variables. */
	void TurnInPlace();

	/** Handle calculation for leaning while running. */
	void Lean(float DeltaTime);
	
private:
	/** Returns true if the character is crouching. */
	bool IsCharacterCrouching() const;
	
	/** Returns true if the character is reloading. */
	bool IsCharacterReloading() const;

	/** Returns true if the character is equipping. */
	bool IsCharacterEquipping() const;
	
	/** Returns true if the character is in the air. */
	bool IsCharacterInTheAir() const;

	/** Returns true if the character is moving. */
	bool IsCharacterAccelerating() const;

	/** Returns true if the character is aiming. */
	bool IsCharacterAiming() const;

	bool ShouldUseFABRIK() const;

	/**
	* Set the lateral speed of the character from the velocity.
	* @param CharacterSpeed passing by reference and setting the speed value.
	*/
	void SetCharacterSpeed(float& CharacterSpeed) const;

	/** Calculate MovementOffsetYaw for running blendspace. */
	void CalculateMovementOffsetYaw();

	/** Set Offset state. */
	void SetOffsetState();

	/** Set Recoil Weight depending on combat conditions. */
	void SetRecoilWeight();
	
private:
	/** Reference to the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta=(AllowPrivateAccess = "true"))
	AShooterCharacter* ShooterCharacter;

	/** The speed of the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta=(AllowPrivateAccess = "true"))
	float Speed;

	/** Whether or not the character is in the air. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta=(AllowPrivateAccess = "true"))
	bool bIsInAir;

	/** Whether or not the character is moving. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta=(AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/** Offset yaw user for strafing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta=(AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	/** Offset yaw the frame before we stopped moving. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta=(AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta=(AllowPrivateAccess = "true"))
	bool bAiming;

	/** Yaw of the Character this frame; Only updated when standing still and not in air. */
	float TIPCharacterYaw;

	/** Yaw of the Character the previous frame; Only updated when standing still and not in air. */
	float TIPCharacterYawLastFrame;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta=(AllowPrivateAccess = "true"))
	float RootYawOffset;

	/** Rotation curve value this frame. */
	float RotationCurve;
	
	/** Rotation curve value last frame. */
	float RotationCurveLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta=(AllowPrivateAccess = "true"))
	float Pitch;

	/** True when reloading, used to prevent Aim offset when reloading. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta=(AllowPrivateAccess = "true"))
	bool bReloading;

	/** Offset state; used to determine which Aim offset to use. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta=(AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	/** Character Yaw this frame. */
	FRotator CharacterRotation;

	/** Character Yaw last frame. */
	FRotator CharacterRotationLastFrame;

	/** Used for leaning in the running blendspace. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta=(AllowPrivateAccess = "true"))
	float YawDelta;

	/** True when crouching. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crouching", meta=(AllowPrivateAccess = "true"))
	bool bCrouching;

	/** True when equipping. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crouching", meta=(AllowPrivateAccess = "true"))
	bool bEquipping;
	
	/** Change the recoil weight based on turning in place and aiming. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess = "true"))
	float RecoilWeight;

	/** True when turning in place. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess = "true"))
	bool bTurningInPlace;

	/** Weapon type for the currently equipped weapon. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess = "true"))
	EWeaponType EquippedWeaponType;

	/** True when not reloading or equipping. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess = "true"))
	bool bShouldUseFABRIK;
};
