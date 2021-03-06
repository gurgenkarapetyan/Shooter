// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Shooter/Library/WeaponTypeEnumLibrary.h"
#include "Shooter/Library/AmmoTypeEnumLibrary.h"
#include "Weapon.generated.h"

UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();

	virtual void Tick(float DeltaSeconds) override;

	/** Adds an impulse to the weapon. */
	void ThrowWeapon();

	/** Called from Character class when firing weapon. */
	void DecrementAmmo();
	
	void ReloadAmmo(int32 Amount);

	void StartSlideTimer();
	
	FORCEINLINE void SetMovingClip(const bool Move) { bMovingClip = Move; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

	FORCEINLINE void SetReloadMontageSection(const FName Name) { ReloadMontageSection = Name; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
	FORCEINLINE void SetClipBoneName(const FName Name) { ClipBoneName = Name; }
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }

	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }
	
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }

	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }
	
	FORCEINLINE bool GetAutomatic() const { return bAutomatic; }
	
	bool ClipIsFull() const;
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	void FinishMovingSlide();

	void UpdateSlideDisplacement();
	
	void StopFalling();
	
private:
	FTimerHandle ThrowWeaponTimer;

	float ThrowWeaponTime;

	bool bFalling;

	/** Ammo count for this Weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	/** Maximum ammo that our weapon can hold. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;
	
	/** Type of weapon. */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	/** Type of ammo for this weapon. */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	/** FName for the reload montage section. */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;

	/** Name for the clip bone for reload animation. */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName;
	
	/** True when moving the clip when reloading. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip;

	/** DataTable for weapon properties. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;

	int32 PreviousMaterialIndex;

	/** Textures for the weapon crosshairs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsMiddle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsLeft;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsRight;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsBottom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsTop;

	/** The speed at which automatic fire happens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	float AutoFireRate;

	/** Particle system spawned at the BarrelSocket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* MuzzleFlash;

	/** Sound played when the weapons fires. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	USoundCue* FireSound;

	/** Name of the bone to hide on the weapon mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	FName BoneToHide;

	/** Amount that the slide is pushed backed during pistol fire. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	float SlideDisplacement;

	/** Curve for the slide displacement. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* SlideDisplacementCurve;

	/** Timer handle for updating SlideDisplacement. */
	FTimerHandle SlideTimer;

	/** Time for displacing the slide during pistol fire. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	float SlideDisplacementTime;

	/** Max distance for the slide on the pistol. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	float MaxSlideDisplacement;

	/** Amount that the pistol will rotate during pistol fire. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	float RecoilRotation;
	
	/** Max rotation for pistol recoil. */
	float MaxRecoilRotation;
	
	/** True when moving pistol slide. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	bool bMovingSlide;

	/** True for auto gun fire. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	bool bAutomatic;
};
