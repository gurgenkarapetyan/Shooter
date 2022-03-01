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

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagazineCapacity() { return MagazineCapacity; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }

protected:
	void StopFalling();
	
private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	/** Ammo count for this Weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	/** Maximum ammo that our weapon can hold. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon properties", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;
	
	/** Type of weapon. */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	/** Type of ammo for this weapon. */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	/** FName for the reload montage section. */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;
};
