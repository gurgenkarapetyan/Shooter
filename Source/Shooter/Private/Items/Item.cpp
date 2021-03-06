// Fill out your copyright notice in the Description page of Project Settings.

#include "Shooter/Public/Items/Item.h"

#include "FItemRarityTable.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Shooter/Public/Player/ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Curves/CurveVector.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AItem::AItem() :
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup),
	ZCurveTime(0.7f),
	// Item interpolation variables
	ItemInterpolationStartLocation(FVector(0.f)),
	CameraTargetLocation(FVector(0.f)),
	bInterpolating(false),
	ItemInterpolationX(0.f),
	ItemInterpolationY(0.f),
	InterpolationInitialYawOffset(0.f),
	ItemType(EItemType::EIT_MAX),
	InterpolationLocationIndex(0),
	MaterialIndex(0),
	bCanChangeCustomDepth(true),
	// Dynamic Material Parameters
	PulseCurveTime(5.f),
	GlowAmount(150.f),
	FresnelExponent(3.f),
	FresnelReflectFraction(4.f),
	SlotIndex(0),
	bCharacterInventoryFull(false)
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Item Mesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pick Up Widget"));
	PickUpWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Area Sphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
	}
	
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
	
	SetActiveStart();
	SetItemProperties(ItemState);

	// Set custom depth to disabled
	InitializeCustomDepth();

	StartPulseTimer();
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->UpdateOverlappedItemCountValue(1);
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->UpdateOverlappedItemCountValue(-1);
			ShooterCharacterRef->UnHighlightInventorySlot();
		}
	}
}

void AItem::SetActiveStart()
{
	for (int32 i = 0; i <= 5; ++i)
	{
		ActiveStars.Add(false);
	}

	switch (ItemRarity)
	{
	case EItemRarity::EIR_Damaged:
		ActiveStars[1] = true;
		break;;
	case EItemRarity::EIR_Common:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		break;
	case EItemRarity::EIR_Uncommon:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[4] = true;
		break;
	case EItemRarity::EIR_Rare:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		break;
	case EItemRarity::EIR_Legendary:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;
		break;
	default:
		break;
	}
}

void AItem::SetItemProperties(const EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:
		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EItemState::EIS_Equipped:
		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		// Set mesh properties
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_EquipInterping:
		// Set mesh properties
		PickUpWidget->SetVisibility(false);
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_PickedUp:
		// Set mesh properties
		PickUpWidget->SetVisibility(false);
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(false);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	default:
		break;
	}
}

void AItem::InitializeCustomDepth()
{
	CustomDepthEnabled(false);
}

void AItem::StartPulseTimer()
{
	if (ItemState == EItemState::EIS_Pickup)
	{
		GetWorldTimerManager().SetTimer(PulseTimer, this, &AItem::ResetPulseTimer, PulseCurveTime);
	}
}

void AItem::ResetPulseTimer()
{
	StartPulseTimer();
}

void AItem::CustomDepthEnabled(const bool bEnableCustomDepth) const 
{
	if (bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(bEnableCustomDepth);
	}
}

void AItem::OnConstruction(const FTransform& MovieSceneBlends)
{
	Super::OnConstruction(MovieSceneBlends);
	
	// Load the data in the Item Rarity Table
	// Path to the Item Rarity Data Table
	const FString RarityTablePath(TEXT("DataTable'/Game/DataTable/ItemRarity_DataTable.ItemRarity_DataTable'"));
	const UDataTable* RarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *RarityTablePath));
	if (RarityTableObject)
	{
		FItemRarityTable* RarityRow = nullptr;
		switch(ItemRarity)
		{
		case EItemRarity::EIR_Damaged:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Damaged"), TEXT(""));
		break;
		case EItemRarity::EIR_Common:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Common"), TEXT(""));
		break;
		case EItemRarity::EIR_Uncommon:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Uncommon"), TEXT(""));
		break;
		case EItemRarity::EIR_Rare:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Rare"), TEXT(""));
		break;
		case EItemRarity::EIR_Legendary:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Legendary"), TEXT(""));
		break;
		default:;
		}

		if (RarityRow)
		{
			GlowColor = RarityRow->GlowColor;
			LightColor = RarityRow->LightColor;
			DarkColor = RarityRow->DarkColor;
			NumberOfStars = RarityRow->NumberOfStars;
			IconBackground = RarityRow->IconBackground;
			if (GetItemMesh())
			{
				GetItemMesh()->SetCustomDepthStencilValue(RarityRow->CustomDepthStencil);
			}
		}
	}

	if (MaterialInstance)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		DynamicMaterialInstance->SetVectorParameterValue(TEXT("Fresnel Color"), GlowColor);
		ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);

		GlowMaterialEnabled(true);
	}
}

void AItem::GlowMaterialEnabled(const bool bEnableGlowMaterial) const
{
	if (DynamicMaterialInstance)
	{
		const int32 ColorValue = bEnableGlowMaterial ? 0.5 : 1;
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("Glow Blend Alpha"), ColorValue);
	}
}

void AItem::SetItemState(const EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

void AItem::StartItemCurve(AShooterCharacter* Character, bool bForcePlaySound)
{
	ShooterCharacterRef = Character;

	InterpolationLocationIndex = Character->GetInterpolationLocationIndex();
	Character->IncrementInterpolationLocationItemCount(InterpolationLocationIndex, 1);
	
	PlayPickupSound(bForcePlaySound);
	
	ItemInterpolationStartLocation = GetActorLocation();
	bInterpolating = true;
	SetItemState(EItemState::EIS_EquipInterping);

	GetWorldTimerManager().ClearTimer(PulseTimer);
	GetWorldTimerManager().SetTimer(ItemInterpolationTimer, this, &AItem::FinishInterpolating, ZCurveTime);
	
	// Get initial Yaw of the Camera
	const float CameraRotationYaw{ Character->GetFollowCamera()->GetComponentRotation().Yaw };
	// Get initial Yaw of the Item
	const float ItemRotationYaw{ GetActorRotation().Yaw };
	// Initial Yaw offset between Camera and Item
	InterpolationInitialYawOffset = ItemRotationYaw - CameraRotationYaw;
	
	bCanChangeCustomDepth = false;
}

void AItem::PlayPickupSound(const bool bForcePlaySound) const
{
	if (ShooterCharacterRef == nullptr || PickUpSound == nullptr)
	{
		return;
	}
	
	if (bForcePlaySound)
	{
		UGameplayStatics::PlaySound2D(this, PickUpSound);
	}
	else if (ShooterCharacterRef->ShouldPlayPickupSound())
	{
		ShooterCharacterRef->StartPickupSoundTimer();
		UGameplayStatics::PlaySound2D(this, PickUpSound);
	}
	
}

void AItem::FinishInterpolating()
{
	bInterpolating = false;
	if (ShooterCharacterRef)
	{
		ShooterCharacterRef->IncrementInterpolationLocationItemCount(InterpolationLocationIndex, -1);
		ShooterCharacterRef->GetPickupItem(this);
		ShooterCharacterRef->UnHighlightInventorySlot();
	}

	SetActorScale3D(FVector(1.f));

	GlowMaterialEnabled(false);

	bCanChangeCustomDepth = true;
	CustomDepthEnabled(false);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle Item Interpolating when in the EquipInterping state
	ItemInterpolation(DeltaTime);

	// Get curve values from PulseCurve and set dynamic material parameters
	UpdatePulse();
}

void AItem::ItemInterpolation(const float DeltaTime)
{
	if (!bInterpolating)
	{
		return;
	}

	if (ShooterCharacterRef && ItemZCurve)
	{
		// Elapsed time since we started ItemInterpolationTimer
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpolationTimer);
		// Get curve value corresponding to ElapsedTime
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

		// Get the item's initial location when the curve started
		FVector ItemLocation = ItemInterpolationStartLocation;
		// Get location in front of the camera
		const FVector CameraInterpolationLocation{ GetInterpolationLocation() };

		// Vector from Item to Camera Interpolation Location, X and Y are zeroed out
		const FVector ItemToCamera{ FVector(0.f, 0.f, (CameraInterpolationLocation - ItemLocation).Z) };
		// Scale factor to multiply with CurveValue
		const float DeltaZ = ItemToCamera.Size();

		const FVector CurrentLocation{ GetActorLocation() };
		// Interpolated X value
		const float InterpolationXValue = FMath::FInterpTo(CurrentLocation.X,CameraInterpolationLocation.X, DeltaTime,30.0f);
		// Interpolated Y value
		const float InterpolationYValue = FMath::FInterpTo(CurrentLocation.Y,CameraInterpolationLocation.Y,DeltaTime,30.f);

		// Set X and Y of ItemLocation to Interpolated values
		ItemLocation.X = InterpolationXValue;
		ItemLocation.Y = InterpolationYValue;

		// Adding curve value to the Z component of the Initial Location (scaled by DeltaZ)
		ItemLocation.Z += CurveValue * DeltaZ;
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);
		
		// Camera rotation this frame
		const FRotator CameraRotation{ ShooterCharacterRef->GetFollowCamera()->GetComponentRotation() };
		// Camera rotation plus inital Yaw Offset
		const FRotator ItemRotation{ 0.f, CameraRotation.Yaw + InterpolationInitialYawOffset, 0.f };
		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
	}
}

FVector AItem::GetInterpolationLocation() const
{
	if (ShooterCharacterRef == nullptr)
	{
		return FVector(0.f);
	}

	switch (ItemType)
	{
	case EItemType::EIT_Ammo:
		return ShooterCharacterRef->GetInterpolationLocation(InterpolationLocationIndex).SceneComponent->GetComponentLocation();
	case EItemType::EIT_Weapn:
		return ShooterCharacterRef->GetInterpolationLocation(0).SceneComponent->GetComponentLocation();
	default:
		break;
	}
	
	return FVector();
}

void AItem::UpdatePulse() const
{
	float ElapsedTime;
	FVector CurveValue { };
	
	switch (ItemState)
	{
		case EItemState::EIS_Pickup:
			if (PulseCurve)
			{
				ElapsedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimer);
				CurveValue = PulseCurve->GetVectorValue(ElapsedTime);
			}
		break;
		case EItemState::EIS_EquipInterping:
			if (InterpolationPulseCurve)
			{
				ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpolationTimer);
				CurveValue = InterpolationPulseCurve->GetVectorValue(ElapsedTime);
			}
		break;
		default: break;
	}

	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("Glow Amount"), CurveValue.X * GlowAmount);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("Fresnel Exponenth"), CurveValue.Y * FresnelExponent);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("Fresnel Reflect Fraction"), CurveValue.Z * FresnelReflectFraction);
	}
}

void AItem::PlayEquipSound(const bool bForcePlaySound) const
{
	if (ShooterCharacterRef == nullptr || EquipSound == nullptr)
	{
		return;
	}
	
	if (bForcePlaySound)
	{
		UGameplayStatics::PlaySound2D(this, EquipSound);
	}
	else if (ShooterCharacterRef->ShouldPlayEquipSound())
	{
		ShooterCharacterRef->StartEquipSoundTimer();
		UGameplayStatics::PlaySound2D(this, EquipSound);
	}
}

