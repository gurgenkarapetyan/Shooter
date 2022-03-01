// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

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
	
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

protected:
	void StopFalling();
	
private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	/** Ammo count for this Weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	/** Type of weapon. */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;
};
