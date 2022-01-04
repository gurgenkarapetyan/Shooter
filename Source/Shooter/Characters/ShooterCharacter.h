// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();

protected:
	virtual void BeginPlay() override;

	/** Called for forward/backward input. */
	void MoveForward(float Value);
	
	/** Called for side to side input. */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at given rate.
	 * @param Rate this is a normalized rate, i.e. 1.0 means 100% desired turn rate.
	 */
	void TurnAtRate(float Rate);

	/**
	* Called via input to look up/down at given rate.
	* @param Rate this is a normalized rate, i.e. 1.0 means 100% desired rate.
	*/
	void LookUpAtRate(float Rate);
public:	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Returns CameraBoom subobject. */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject. */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
private:
	/** Camera boom positioning the camera behind the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Camera that follows the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Base turn rate, in degree/seconds. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** Base look up/down rate in degree/seconds. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	float BaseLookUpRate;
};
