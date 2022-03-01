#pragma once

#include "CoreMinimal.h"
#include "WeaponTypeEnumLibrary.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
