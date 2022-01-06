// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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
};
