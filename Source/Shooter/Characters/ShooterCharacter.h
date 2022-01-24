// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USoundCue;
class UParticleSystem;
class AItem;

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
	
	/**
	* Rotate controller base on mouse X movement.
	* @param Value The input value from mouse movement.
	*/
	void Turn(float Value);
	
	/**
	* Rotate controller base on mouse Y movement.
	* @param Value The input value from mouse movement.
	*/
	void LookUp(float Value);
	
	/** Called when the fire button is pressed. */
	void FireWeapon();
	
	/** Calculate crosshair spread amount value */
	void CalculateCrosshairSpread(float DeltaTime);
	
	void StartCrosshairBulletFire();
	
	UFUNCTION()
	void FinishCrosshairBulletFire();
	
	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer();
	
	UFUNCTION()
	void AutoFireReset();

	/** Line Trace for items under the crosshairs */
	bool TraceUnderCrosshairs(FHitResult& OutHitResult);

	/** Trace for items if OverlappedItemCount > 0 */
	void TraceForItems();
private:
	/** Setting some configuration for character movement. */
	void SetCharacterMovementConfigurations();
	
	/** Playing fire sound cue when character starts firing. */
	void PlayFireSoundCue();

	/** Create particle effect when character starts firing. */
	void CreateFireMuzzleFlashParticle();
	
	/** Play fire weapon animation montage when character starts firing. */
	void PlayFireAnimMontage();

	/**
	 * Create LineTrace when character starts firing.
	 * @param Barrel is Weapon's BarrelSocket Transform information.
	 */
	void SetBulletLineTrace(const FTransform Barrel);

	/**
	* Returns the size of the current viewport.
	* @param ViewportSize for storing current viewport location.
	*/
	void GetCurrentSizeOfViewport(FVector2D& ViewportSize);

	/** Set bAiming to true or false with button press. */
	void AimingButtonPressed();
	void AimingButtonReleased();

	/** Handle interpolation for zoom when aiming */
	void CameraInterpZoom(float DeltaTime);
	
	/** Set BaseTurnRate and BaseLookUpRate based on aiming. */
	void SetLookUpRates();
	
	/**
	* Returns true if deprojection was successful.
	* @param CurrentViewportSize current viewport location.
	* @param CrosshairWorldPosition corresponding 3D position in world space.
	* @param CrosshairWorldDirection world space direction vector away from the camera at the given 2d point.
	*/
	bool GetScreenSpaceLocationOfCrosshairs(FVector& CrosshairWorldPosition, FVector& CrosshairWorldDirection);

	/**
	*  Calculate crosshair in air factor
	* @param DeltaTime .
	* @param CrosshariInAir for calculating value of air factor.
	*/
	void CalculateCrosshairInAirFactor(float DeltaTime, float &CrosshariInAir);

	/**
	* Calculate crosshair aim factor.
	* @param DeltaTime .
	* @param CrosshairAim for calculating value of air factor.
	*/
	void CalculateCrosshairAimFactor(float DeltaTime, float &CrosshairAim);

	/**
	* Calculate crosshair firing factor.
	* @param DeltaTime
	* @param CrosshairShooting for calculating value of shooting factor.
	*/
	void CalculateCrosshairFiringFactor(float DeltaTime, float &CrosshairShooting);
	
public:	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Returns CameraBoom subobject. */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject. */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Returns aiming true/false. */
	FORCEINLINE bool GetAiming() const { return bAiming; }
	
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	/** Adds/Subtracts to/from OverlappedItemCount and updates bShouldTraceForItems*/
	void UpdateOverlappedItemCountValue(int8 Amount);
	
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;
	
private:
	/** Camera boom positioning the camera behind the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Camera that follows the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Randomized gunshot sound cue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess = "true"))
	USoundCue* FireSound;

	/** Flash spawned at barrel socket. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess = "true"))
	UParticleSystem* MuzzleFlash;

	/** Particles spawned upon bullet impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess = "true"))
    UParticleSystem* ImpactParticle;

	/** Smoke trail for bullets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;
	
	/** Montage for firing the weapon . */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess = "true"))
	UAnimMontage* HipFireMontage;
	
	/** Base turn rate, in degree/seconds. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** Base look up/down rate in degree/seconds. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/** Turn rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	float HipTurnRate;

	/** Look up rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/** Turn rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	float AimingTurnRate;

	/** Look up rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/** Scale factor for mouse look sensitivity. Turn rate when not aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"), meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	/** Scale factor for mouse look sensitivity. Look up rate when not aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"), meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	/** Scale factor for mouse look sensitivity. Turn rate when aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"), meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	/** Scale factor for mouse look sensitivity. Look up rate when aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess = "true"), meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;
	
	/** Default camera field of view value. */
	float CameraDefaultFOV;
	
	/** Field of view value for when zoomed in. */
	float CameraZoomedFOV;

	/** Current Field of view this view. */
	float CameraCurrentFOV;

	/** Interp speed when zooming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat",  meta=(AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	/** Determines the spread of the crosshairs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crosshairs", meta=(AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	/** Velocity component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crosshairs", meta=(AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	/** In air component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crosshairs", meta=(AllowPrivateAccess = "true"))
	float CrosshariInAirFactor;

	/** Aim component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crosshairs", meta=(AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	/** Shooting component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crosshairs", meta=(AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;
	
	/** True when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta=(AllowPrivateAccess = "true"))
	bool bAiming;

	/** Left mouse button or right console trigger pressed. */
	bool bFireButtonPressed;
	
	/** True when we can fire. False waiting for the timer. */
	bool bShouldFire;
	
	/** Rate of automatic gun fire. */
	float AutomaticFireRate;

	/** Sets a timer between gunshots. */
	FTimerHandle AutoFireTimer;
	
	float ShootTimeDuration;

	bool bFiringBullet;

	FTimerHandle  CrosshairShootTimer;

	/** true if we should trace every frame for item.*/
	bool bShouldTraceForItems;

	/** Number of overlapped AItems. */
	int8 OverlappedItemCount;

	/** The AItem we hit last frame */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Items", meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItemLastFrame;
};
