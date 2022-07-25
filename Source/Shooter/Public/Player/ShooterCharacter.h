// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Shooter/Library/AmmoTypeEnumLibrary.h"
#include "Shooter/Library/CombatStateEnumLibrary.h"
#include "ShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USoundCue;
class UParticleSystem;
class AItem;
class AWeapon;

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	/** Scene component used for its location for interping. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	/** Number of items interping to/at this scene comp location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Returns CameraBoom subobject. */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject. */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Returns aiming true/false. */
	FORCEINLINE bool GetAiming() const { return bAiming; }
	/** Returns crouching true/false. */
	FORCEINLINE bool GetCrouching() const { return bCrouching; }

	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
	FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }
	
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }

	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
	
	FInterpLocation GetInterpolationLocation(const int32 Index);

	/** Return the index in InterpolationLocations array with the lowest ItemCount. */
	int32 GetInterpolationLocationIndex();

	void IncrementInterpolationLocationItemCount(const int32 Index, const int32 Amount);
	
	/** Adds/Subtracts to/from OverlappedItemCount and updates bShouldTraceForItems. */
	void UpdateOverlappedItemCountValue(const int8 Amount);
	
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;
	
	void GetPickupItem(AItem* Item);

	void StartPickupSoundTimer();
	void StartEquipSoundTimer();

	void UnHighlightInventorySlot();
	
protected:
	virtual void BeginPlay() override;

	/** Called for forward/backward input. */
	void MoveForward(const float Value);
	
	/** Called for side to side input. */
	void MoveRight(const float Value);

	/**
	 * Called via input to turn at given rate.
	 * @param Rate this is a normalized rate, i.e. 1.0 means 100% desired turn rate.
	 */
	void TurnAtRate(const float Rate);

	/**
	* Called via input to look up/down at given rate.
	* @param Rate this is a normalized rate, i.e. 1.0 means 100% desired rate.
	*/
	void LookUpAtRate(const float Rate);
	
	/**
	* Rotate controller based on mouse X movement.
	* @param Value The input value from mouse movement.
	*/
	void Turn(const float Value);
	
	/**
	* Rotate controller based on mouse Y movement.
	* @param Value The input value from mouse movement.
	*/
	void LookUp(const float Value);

	virtual void Jump() override;
	
	/** Called when the fire button is pressed. */
	void FireWeapon();
	
	/** Calculate crosshair spread amount value. */
	void CalculateCrosshairSpread(const float DeltaTime);
	
	void StartCrosshairBulletFire();
	
	UFUNCTION()
	void FinishCrosshairBulletFire();
	
	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer();
	
	UFUNCTION()
	void AutoFireReset();

	/** Bound to the R key. */
	void ReloadButtonPressed();

	/** Handle reloading of the weapon. */
	void ReloadWeapon();

	/** Checks to see if we have ammo of the EquippedWeapon's ammo type. */
	bool CarryingAmmo();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();
	
	void CrouchButtonPressed();
	
	/** Line Trace for items under the crosshairs. */
	bool TraceUnderCrosshairs(FHitResult& OutHitResult);

	/** Trace for items if OverlappedItemCount > 0. */
	void TraceForItems();

	/** Spawns a default weapon and equips it. */
	AWeapon* SpawnDefaultWeapon() const;

	/** Takes a weapon and attaches it to the mesh. */
	void EquipWeapon(AWeapon* WeaponToEquip, const bool bSwapping = false);

	/** Detach weapon and let it fall to the ground. */
	void DropWeapon() const;

	void SelectButtonPressed();
	void SelectButtonReleased();

	/** Set bAiming to true or false with button press. */
	void AimingButtonPressed();
	void AimingButtonReleased();

	void Aim();
	void StopAiming();
	
	/** Drops currently equipped Weapon and Equips TraceHitItem. */
	void SwapWeapon(AWeapon* WeaponToSwap);

	/** Initialize the Ammo Map with ammo values. */
	void InitializeAmmoMap();

	/** Check to make sure out weapon has ammo. */
	bool WeaponHasAmmo() const;

	/** Called from animation Blueprint with Grab Clip notify. */
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	/** Called from animation Blueprint with Release Clip notify. */
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	/** Interps capsule half height when crouching/standing. */
	void InterpCapsuleHalfHeight(float DeltaTime) const;

	void PickUpAmmo(class AAmmo* Ammo);

private:
	/** Setting some configuration for character movement. */
	void SetCharacterMovementConfigurations();

	/** Create interpolation component for pickable items. */
	void CreateInterpolationComponent();
	
	/** Playing fire sound cue when character starts firing. */
	void PlayFireSoundCue() const;

	/** Create particle effect when character starts firing. */
	void CreateFireMuzzleFlashParticle();
	
	/** Play fire weapon animation montage when character starts firing. */
	void PlayFireAnimMontage() const;

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
	
	/** Handle interpolation for zoom when aiming. */
	void CameraInterpolationZoom(float DeltaTime);
	
	/** Set BaseTurnRate and BaseLookUpRate based on aiming. */
	void SetLookUpRates();
	
	/**
	* Returns true if deprojection was successful.
	* @param CrosshairWorldPosition corresponding 3D position in world space.
	* @param CrosshairWorldDirection world space direction vector away from the camera at the given 2D point.
	*/
	bool GetScreenSpaceLocationOfCrosshairs(FVector& CrosshairWorldPosition, FVector& CrosshairWorldDirection);

	/**
	*  Calculate crosshair in air factor
	* @param DeltaTime .
	* @param CrosshairInAir for calculating value of air factor.
	*/
	void CalculateCrosshairInAirFactor(const float DeltaTime, float& CrosshairInAir) const;

	/**
	* Calculate crosshair aim factor.
	* @param DeltaTime .
	* @param CrosshairAim for calculating value of air factor.
	*/
	void CalculateCrosshairAimFactor(const float DeltaTime, float& CrosshairAim) const;

	/**
	* Calculate crosshair firing factor.
	* @param DeltaTime .
	* @param CrosshairShooting for calculating value of shooting factor.
	*/
	void CalculateCrosshairFiringFactor(const float DeltaTime, float& CrosshairShooting) const;

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();
	
	/** Create FInterLocation structs for each interpolation location. Add to the array. */
	void InitializeInterpolationLocations();

	void FKeyPressed();
	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();
	void FourKeyPressed();
	void FiveKeyPressed();
	
	void ExchangeInventoryItems(const int32 CurrentItemIndex, const int32 NewItemIndex);

	int32 GetEmptyInventorySlot();

	void HighlightInventorySlot();
	
private:
	/** Camera boom positioning the camera behind the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Camera that follows the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Randomized gunshot sound cue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess = "true"))
	USoundCue* FireSound;

	/** Flash spawned at barrel socket. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess = "true"))
	UParticleSystem* MuzzleFlash;

	/** Particles spawned upon bullet impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess = "true"))
    UParticleSystem* ImpactParticle;

	/** Smoke trail for bullets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;
	
	/** Montage for firing the weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess = "true"))
	UAnimMontage* HipFireMontage;
	
	/** Base turn rate, in degree/seconds. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** Base look up/down rate in degree/seconds. Other scaling may affect final look up rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/** Turn rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	float HipTurnRate;

	/** Look up rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/** Turn rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	float AimingTurnRate;

	/** Look up rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/** Scale factor for mouse look sensitivity. Turn rate when not aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"), meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	/** Scale factor for mouse look sensitivity. Look up rate when not aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"), meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	/** Scale factor for mouse look sensitivity. Turn rate when aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"), meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	/** Scale factor for mouse look sensitivity. Look up rate when aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"), meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;
	
	/** Default camera field of view value. */
	float CameraDefaultFieldOfView;
	
	/** Field of view value for when zoomed in. */
	float CameraZoomedFieldOfView;

	/** Current Field of view this view. */
	float CameraCurrentFieldOfView;

	/** Interp speed when zooming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat",  meta=(AllowPrivateAccess = "true"))
	float ZoomInterpolationSpeed;

	/** Determines the spread of the crosshairs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta=(AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	/** Velocity component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta=(AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	/** In air component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta=(AllowPrivateAccess = "true"))
	float CrosshariInAirFactor;

	/** Aim component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta=(AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	/** Shooting component for crosshairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs", meta=(AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;
	
	/** True when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta=(AllowPrivateAccess = "true"))
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

	/** The AItem we hit last frame. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItemLastFrame;

	/** Currently equipped Weapon. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	/** Set this in Blueprints for the default Weapon class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;
	
	/** The item currently hit by our trace in TraceForItems (could be null) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;
	
	/** Distance outward from the camera for the interpolation destination. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	float CameraInterpolationDistance;
	
	/** Distance upward from the camera for the interpolation destination. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	float CameraInterpolationElevation;

	/** Map to keep track of ammo of the different ammo types. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	/** Starting amount of 9mm ammo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items", meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;

	/** Starting amount of Assault Rifle ammo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items", meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	/** Combat State, can only fire or reload if Unoccupied. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	/** Montage for reload animations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	/** Montage for Equip animations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EquipMontage;
	
	/** Transform of the clip when we first grab the clip during reloading. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	/** Scene component to attach to the Character's hand during reloading. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	/** True when crouching. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	/** Regular movement speed. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	/** Crouch movement speed. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	/** Current half height of the capsule. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CurrentCapsuleHalfHeight;

	/** Half height of the capsule when not crouching. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;

	/** Half height of the capsule when crouching. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	/** Ground friction while not crouching. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;

	/** Ground friction while crouching. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

	/** Used for knowing when the aiming button is pressed. */
	bool bAimingButtonPressed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpWeaponComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp4;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp5;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp6;
	
	/** Array of interp location structs. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	/** Array of AItems for our Inventory. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;

	const int32 INVENTORY_CAPACITY = 6;

	/** Delegate for sending slot information to InventoryBar when equipping. */
	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;

	/** Delegate for sending slot information for playing the icon animation. */
	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate HighlightIconDelegate;

	/** The index for the currently highlighted slot. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 HighlightedSlot;
	
	bool bShouldPlayPickupSound;
	bool bShouldPlayEquipSound;

	/** Time to wait before we can play another Pickup Sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float PickupSoundResetTime;

	/** Time to wait before we can play another Equip Sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;
};

