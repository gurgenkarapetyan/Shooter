// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Shooter/Library/ItemEnumLibrary.h"
#include "Item.generated.h"

class UBoxComponent;
class UWidgetComponent;
class USphereComponent;
class AShooterCharacter;

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
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return  ItemMesh; }
	void SetItemState(EItemState State);

	/** Called from the AShooterCharacter class. */
	void StartItemCurve(AShooterCharacter* Character);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/** Called when overlapping AreaSphere. */
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** Called when end overlapping AreaSphere. */
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Sets the ActiveStars array of bools based on rarity */
	void SetActiveStart();

	/** Sets properties of the Item's components based on State. */
	void SetItemProperties(EItemState State);
	
	
	/** Called when ItemInterpTimer is Finished. */
	void FinishInterping();

	/** Handle item interpolation when in the EquipInterping state. */
	void ItemInterp(float DeltaTime);
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
	USphereComponent* AreaSphere;

	/** The name which appears on Pickup Widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	FString ItemName;

	/** Item count (ammo, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	int32 ItemCount;

	/** Item rarity - determines number of starts in PickUp Widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	TArray<bool> ActiveStars;

	/** State of the Item. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Propertis", meta = (AllowPrivateAccess="true"))
	EItemState ItemState;

	/** The curve asset to use for the item's Z location when interping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Propertis", meta = (AllowPrivateAccess="true"))
	UCurveFloat* ItemZCurve;
	
	/** Plays when we start interping. */
	FTimerHandle ItemInterpTimer;

	/** Duration of the curve and timer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Propertis", meta = (AllowPrivateAccess="true"))
	float ZCurveTime;
	
	/** Starting location when interping begins. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Propertis", meta = (AllowPrivateAccess="true"))
	FVector ItemInterpStartLocation;

	/** Target interp location in front of the camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Propertis", meta = (AllowPrivateAccess="true"))
	FVector CameraTargetLocation;

	/** true when interping. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Propertis", meta = (AllowPrivateAccess="true"))
	bool bInterping;

	/** X for the Item while interping in the EquipInterping state. */
	float ItemInterpX;
	/** Y for the Item while interping in the EquipInterping state. */
	float ItemInterpY;

	/** Initial Yaw offset between the camera and the interping item. */
	float InterpInitialYawOffset;

	/** Curve used to scale the item when interping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Propertis", meta = (AllowPrivateAccess="true"))
	UCurveFloat* ItemScaleCurve;
	
	/** Pointer to the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Propertis", meta = (AllowPrivateAccess="true"))
	AShooterCharacter* ShooterCharacterRef;
};
