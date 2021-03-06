// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Shooter/Library/ItemEnumLibrary.h"
#include "Shooter/Library/ItemTypeEnumLibrary.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

class UBoxComponent;
class UWidgetComponent;
class USphereComponent;
class AShooterCharacter;
class USoundCue;
class UCurveVector;



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
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return  ItemMesh; }

	FORCEINLINE EItemState GetItemState() const { return ItemState; }

	FORCEINLINE USoundCue* GetPickUpSound() const { return PickUpSound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	FORCEINLINE void SetPickUpSound(USoundCue* Sound) { PickUpSound = Sound; }
	FORCEINLINE void SetEquipSound(USoundCue* Sound) { EquipSound = Sound; }

	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(const int32 Index) { SlotIndex = Index; }

	FORCEINLINE void SetItemName(FString Name) { ItemName = Name; }

	/** Set item icon for the inventory */
	FORCEINLINE void SetIconItem(UTexture2D* Icon) { IconItem = Icon; }
	/** Set ammo icon for the inventory */
	FORCEINLINE void SetAmmoIcon(UTexture2D* Icon) { AmmoIcon = Icon; }

	FORCEINLINE UMaterialInstance* GetMaterialInstance() const { return MaterialInstance; }
	FORCEINLINE void SetMaterialInstance(UMaterialInstance* Instance) { MaterialInstance = Instance; }
	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMaterialInstance() const { return DynamicMaterialInstance; }
	FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic* Instance) { DynamicMaterialInstance = Instance; }

	FORCEINLINE FLinearColor GetGlowColor() const { return GlowColor; }
	FORCEINLINE int32 GetMaterialIndex() const { return MaterialIndex; }
	FORCEINLINE void SetMaterialIndex(const int32 Index) { MaterialIndex = Index; }
	
	FORCEINLINE void SetCharacter(AShooterCharacter* ShooterCharacter) { ShooterCharacterRef = ShooterCharacter;} 

	FORCEINLINE void SetCharacterInventoryFull(const bool bFull) { bCharacterInventoryFull = bFull; }
	
	void SetItemState(EItemState State);

	/** Called from the AShooterCharacter class. */
	void StartItemCurve(AShooterCharacter* Character, bool bForcePlaySound = false);

	void PlayEquipSound(const bool bForcePlaySound = false) const;
	
	virtual void CustomDepthEnabled(const bool bEnableCustomDepth) const;
	virtual void GlowMaterialEnabled(const bool bEnableGlowMaterial) const;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	
	/** Called when overlapping AreaSphere. */
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** Called when end overlapping AreaSphere. */
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Sets the ActiveStars array of booleans based on rarity */
	void SetActiveStart();

	/** Sets properties of the Item's components based on State. */
	virtual void SetItemProperties(EItemState State);

	void PlayPickupSound(const bool bForcePlaySound = false) const;
	
	/** Called when ItemInterpolationTimer is Finished. */
	void FinishInterpolating();

	/** Handle item interpolation when in the EquipInterping state. */
	void ItemInterpolation(const float DeltaTime);

	/** Get interpolation location base on the item type. */
	FVector GetInterpolationLocation() const;
	
	virtual void InitializeCustomDepth();

	void StartPulseTimer();
	void ResetPulseTimer();
	void UpdatePulse() const;

private:
	/** Skeleton mesh for the item. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;
	
	/** Line trace collides with box to show HUD widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* CollisionBox;

	/** Pop up widget for the player when looks at the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* PickUpWidget;

	/** Enable item tracing when overlapped. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AreaSphere;

	/** The name which appears on Pickup Widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	/** Item count (ammo, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	/** Item rarity - determines number of starts in PickUp Widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	/** State of the Item. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Propertis", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	/** The curve asset to use for the item's Z location when interpolating. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Propertis", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemZCurve;
	
	/** Plays when we start interpolating. */
	FTimerHandle ItemInterpolationTimer;

	/** Duration of the curve and timer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Propertis", meta = (AllowPrivateAccess = "true"))
	float ZCurveTime;
	
	/** Starting location when interpolating begins. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Propertis", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpolationStartLocation;

	/** Target interpolation location in front of the camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Propertis", meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;

	/** true when interpolating. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Propertis", meta = (AllowPrivateAccess = "true"))
	bool bInterpolating;

	/** X for the Item while interpolating in the EquipInterping state. */
	float ItemInterpolationX;
	/** Y for the Item while interpolating in the EquipInterping state. */
	float ItemInterpolationY;

	/** Initial Yaw offset between the camera and the interping item. */
	float InterpolationInitialYawOffset;

	/** Curve used to scale the item when interpolating. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Propertis", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;

	/** Sound played when Item is picked up. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Propertis", meta = (AllowPrivateAccess = "true"))
	USoundCue* PickUpSound;

	/** Sound played when the Item is equipped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Propertis", meta = (AllowPrivateAccess = "true"))
	USoundCue* EquipSound;
	
	/** Pointer to the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Propertis", meta = (AllowPrivateAccess = "true"))
	AShooterCharacter* ShooterCharacterRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;
	
	/** Index of the interpolation location this item is interpolating to. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 InterpolationLocationIndex;

	/** Index for the material we would like to change at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	/** Dynamic instance that we can change runtime. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	/** Material instance used with the Dynamic Material Instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;
	
	bool bCanChangeCustomDepth;

	/** Curve to drive the dynamic material parameters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* PulseCurve;

	/** Interpolation Curve to drive the dynamic material parameters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* InterpolationPulseCurve;

	FTimerHandle PulseTimer;

	/** Time for the PulseTimer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float GlowAmount;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelExponent;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction;
	
	/** Icon for this item in the inventory. */	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconItem;

	/** Ammo Icon for this item in the inventory. */	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIcon;

	/** Slot in the inventory array. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	/** True when the Character inventory is full. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	bool bCharacterInventoryFull;

	/** Item rarity DataTable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* ItemRarityDataTable;

	/** Color in the glow material. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;

	/** Light color in the pickup widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;

	/** Dark color in the pickup widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;

	/** Number of stars in pickup widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;

	/** Background icon for the inventory. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;
};
