// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class UBoxComponent;
class UWidgetComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "EIR_Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),
	EIR_Max UMETA(DisplayName = "DefaultMax"),
};

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE UWidgetComponent* GetPickUpWidget() const { return PickUpWidget; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Sets the ActiveStars array of bools based on rarity */
	void SetActiveStart();
	
	/** Called when overlapping AreaSphere. */
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** Called when end overlapping AreaSphere. */
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
private:
	/** Skeleton mesh for the item. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	USkeletalMeshComponent* ItemMesh;
	
	/** Line trace collides with box to show HUD widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	UBoxComponent* CollisionBox;

	/** Pop up widget for the player when looks at the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	UWidgetComponent* PickUpWidget;

	/** Enable item tracing when overlapped. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	USphereComponent* AreaShpere;

	/** The name which appears on Pickup Widget*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	FString ItemName;

	/** Item count (ammo, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	int32 ItemCount;

	/** Item rarity - determines number of starts in PickUp Widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	TArray<bool> ActiveStars;
};
