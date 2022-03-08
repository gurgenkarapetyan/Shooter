#pragma once

#include "CoreMinimal.h"
#include "ItemTypeEnumLibrary.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapn UMETA(DisplayName = "Weapon"),
	EIT_MAX UMETA(DisplayName = "DefaultMax")
};