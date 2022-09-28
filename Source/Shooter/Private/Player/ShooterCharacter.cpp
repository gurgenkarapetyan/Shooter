// Fill out your copyright notice in the Description page of Project Settings.

#include "Shooter/Public/Player/ShooterCharacter.h"

#include "Shooter/Public/AI/Enemy.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Interfaces/BulletHitInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Shooter/Shooter.h"
#include "Shooter/Public/Items//Item.h"
#include "Shooter/Public/Items//Weapon.h"
#include "Shooter/Public/Items/Ammo.h"

AShooterCharacter::AShooterCharacter() :
	// Base Rates for turning/looking up
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	// Turn Rates field of view values
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	// Mouse look sensitivity scale factors
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),
	// Camera field of view values
	CameraDefaultFieldOfView(0.f),
	CameraZoomedFieldOfView(35.f),
	CameraCurrentFieldOfView(0.f),
	ZoomInterpolationSpeed(20.f),
	// Crosshair spread factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshariInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	// True when aiming weapon
	bAiming(false),
	// Bullet fire timer variables
	bFireButtonPressed(false),
	bShouldFire(true),
	// Automatic fire variables
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	// Item trace variables
	bShouldTraceForItems(false),
	CameraInterpolationDistance(250.f),
	CameraInterpolationElevation(65.f),
	// Starting ammo amounts 
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	// Combat variables
	CombatState(ECombatState::ECS_Unoccupied),
	bCrouching(false),
	BaseMovementSpeed(650.f),
	CrouchMovementSpeed(300.f),
	StandingCapsuleHalfHeight(88.f),
	CrouchingCapsuleHalfHeight(44.f),
	BaseGroundFriction(2.f),
	CrouchingGroundFriction(100.f),
	bAimingButtonPressed(false),
	// Icon animation property
	HighlightedSlot(-1),
	// Pickup sound timer properties
	bShouldPlayPickupSound(true),
	bShouldPlayEquipSound(true),
	PickupSoundResetTime(0.2f),
	EquipSoundResetTime(0.2f)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.0f;
	// Rotate the arm based on the controller
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Camera does not rotate relative to arm
	FollowCamera->bUsePawnControlRotation = false;

	SetCharacterMovementConfigurations();
	CreateInterpolationComponent();
}

void AShooterCharacter::SetCharacterMovementConfigurations()
{
	// Don't rotate when the controller rotates, Let the controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement 
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Hand Scene Component"));
}

void AShooterCharacter::CreateInterpolationComponent()
{
	InterpWeaponComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Component"));
	InterpWeaponComp->SetupAttachment(GetFollowCamera());
	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 1"));
	InterpComp1->SetupAttachment(GetFollowCamera());
	
	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 2"));
	InterpComp2->SetupAttachment(GetFollowCamera());
	
	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 3"));
	InterpComp3->SetupAttachment(GetFollowCamera());
	
	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 4"));
	InterpComp4->SetupAttachment(GetFollowCamera());
	
	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 5"));
	InterpComp5->SetupAttachment(GetFollowCamera());
	
	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (FollowCamera)
	{
		CameraDefaultFieldOfView = GetFollowCamera()->FieldOfView;
		CameraCurrentFieldOfView = CameraDefaultFieldOfView;
	}

	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->CustomDepthEnabled(false);
	EquippedWeapon->GlowMaterialEnabled(false);
	EquippedWeapon->SetCharacter(this);
	
	InitializeAmmoMap();
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	InitializeInterpolationLocations();
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, const bool bSwapping)
{
	if (!WeaponToEquip)
	{
		return;
	}

	WeaponToEquip->GetAreaSphere()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WeaponToEquip->GetCollisionBox()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(WeaponToEquip, GetMesh());
	}

	if (EquippedWeapon == nullptr)
	{
		// -1 == no EquippedWeapon yet. No need to reverse the icon animation
		EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
	}
	else if (!bSwapping)
	{
		EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
	}
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon() const
{
	if (!DefaultWeaponClass)
	{	
		return nullptr;	
	}

	return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

void AShooterCharacter::InitializeInterpolationLocations()
{
	const FInterpLocation WeaponLocation{ InterpWeaponComp, 0 };
	InterpLocations.Add(WeaponLocation);

	const FInterpLocation InterpLocation1{ InterpComp1, 0 };
	InterpLocations.Add(InterpLocation1);
	const FInterpLocation InterpLocation2{ InterpComp2, 0 };
	InterpLocations.Add(InterpLocation2);
	const FInterpLocation InterpLocation3{ InterpComp3, 0 };
	InterpLocations.Add(InterpLocation3);
	const FInterpLocation InterpLocation4{ InterpComp4, 0 };
	InterpLocations.Add(InterpLocation4);
	const FInterpLocation InterpLocation5{ InterpComp5, 0 };
	InterpLocations.Add(InterpLocation5);
	const FInterpLocation InterpLocation6{ InterpComp6, 0 };
	InterpLocations.Add(InterpLocation6);
}


void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	InterpCapsuleHalfHeight(DeltaTime);
	CameraInterpolationZoom(DeltaTime);
	SetLookUpRates();
	CalculateCrosshairSpread(DeltaTime);
	TraceForItems();
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime) const
{
	float TargetCapsuleHalfHeight{};
	bCrouching ? TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight : TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;

	const float InterpHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f);

	// Negative value if crouching; Positive value if standing
	const float DeltaCapsuleHalfHeight = InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector MeshOffset{ 0.f, 0.f, -DeltaCapsuleHalfHeight };
	GetMesh()->AddLocalOffset(MeshOffset);	
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::CameraInterpolationZoom(float DeltaTime)
{
	if (bAiming)
	{
		CameraCurrentFieldOfView = FMath::FInterpTo(CameraCurrentFieldOfView, CameraZoomedFieldOfView, DeltaTime, ZoomInterpolationSpeed);
	}
	else
	{
		CameraCurrentFieldOfView = FMath::FInterpTo(CameraCurrentFieldOfView, CameraDefaultFieldOfView, DeltaTime, ZoomInterpolationSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFieldOfView);
}

void AShooterCharacter::SetLookUpRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(const float DeltaTime)
{
	const FVector2D WalkSpeedRange { 0.f, 600.f };
	const FVector2D VelocityMultiplierRange { 0.f, 1.0f };
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	CalculateCrosshairInAirFactor(DeltaTime, CrosshariInAirFactor);
	CalculateCrosshairAimFactor(DeltaTime, CrosshairAimFactor);
	CalculateCrosshairFiringFactor(DeltaTime, CrosshairShootingFactor);
	
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshariInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::CalculateCrosshairInAirFactor(const float DeltaTime, float& CrosshairInAir) const
{
	if (GetCharacterMovement()->IsFalling())
	{
		CrosshairInAir = FMath::FInterpTo(CrosshairInAir, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAir = FMath::FInterpTo(CrosshairInAir, 0.f, DeltaTime, 30.f);
	}
}

void AShooterCharacter::CalculateCrosshairAimFactor(const float DeltaTime, float& CrosshairInAir) const
{
	if (bAiming)
	{
		CrosshairInAir = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairInAir = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}
}

void AShooterCharacter::CalculateCrosshairFiringFactor(const float DeltaTime, float& CrosshairInAir) const
{
	if (bFiringBullet)
	{
		CrosshairInAir = FMath::FInterpTo(CrosshairShootingFactor,  0.3f, DeltaTime, 60.f);	
	}
	else
	{
		CrosshairInAir = FMath::FInterpTo(CrosshairShootingFactor,  0.f, DeltaTime, 60.f);	
	}
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		TraceUnderCrosshairs(ItemTraceResult);
		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);

			const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
			if (TraceHitWeapon)
			{
				if (HighlightedSlot == -1)
				{
					// Not currently highlighting a slot; highlight one
					HighlightInventorySlot();
				}
			}
			else
			{
				// Is a slot being highlighted?
				if (HighlightedSlot != -1)
				{
					// UnHighlight the slot
					UnHighlightInventorySlot();
				}
			}
			
			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}

			if (TraceHitItem && TraceHitItem->GetPickUpWidget())
			{
				TraceHitItem->GetPickUpWidget()->SetVisibility(true);
				TraceHitItem->CustomDepthEnabled(true);

				if (Inventory.Num() >= INVENTORY_CAPACITY)
				{
					// Inventory is full
					TraceHitItem->SetCharacterInventoryFull(true);
				}
				else
				{
					// Inventory has room
					TraceHitItem->SetCharacterInventoryFull(false);
				}
			}

			// We hit an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					// We are hitting a different AItem this frame from last frame
					// Or AItem is null this frame
					TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
					TraceHitItemLastFrame->CustomDepthEnabled(false);
				}
			}

			// Store a reference to HitItem for next frame
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// No longer overlapping any items,
		// Item last frame should not show widget
		TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
		TraceHitItemLastFrame->CustomDepthEnabled(false);
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult)
{
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	const bool bScreenToWorld = GetScreenSpaceLocationOfCrosshairs(CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		const FVector Start { CrosshairWorldPosition };
		const FVector End {  Start + CrosshairWorldDirection * 50000.f };
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			return true;
		}
	}
	
	return false;
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("TurnRate"), this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis(TEXT("LookUpRate"), this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	
	PlayerInputComponent->BindAction(TEXT("FireButton"), IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("FireButton"), IE_Released, this, &AShooterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	
	PlayerInputComponent->BindAction(TEXT("Select"), IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Select"), IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction(TEXT("ReloadButton"), IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction(TEXT("FKey"), IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction(TEXT("1Key"), IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction(TEXT("2Key"), IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction(TEXT("3Key"), IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction(TEXT("4Key"), IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction(TEXT("5Key"), IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);
}

void AShooterCharacter::MoveForward(const float Value)
{
	if (Controller == nullptr && Value == 0.0f)
	{
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation { 0, Rotation.Yaw, 0 };

	const FVector Direction { FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
	AddMovementInput(Direction, Value);
}

void AShooterCharacter::MoveRight(const float Value)
{
	if (Controller == nullptr && Value == 0.0f)
	{
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation { 0, Rotation.Yaw, 0 };

	const FVector Direction { FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
	AddMovementInput(Direction, Value);
}

void AShooterCharacter::TurnAtRate(const float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(const float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(const float Value)
{
	float TurnScaleFactor;
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}

	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(const float Value)
{
	float LookUpScaleFactor;
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}

	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::Jump()
{
	Super::Jump();
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

bool AShooterCharacter::WeaponHasAmmo() const
{
	if (!EquippedWeapon)
	{
		return false;	
	}

	return (EquippedWeapon->GetAmmo() > 0);
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	if (WeaponHasAmmo())
	{
	
		PlayFireSoundCue();
		SendBullet();
		PlayFireAnimMontage();
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();

		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			// Start moving slide timer
			EquippedWeapon->StartSlideTimer();
		}
	}
}

void AShooterCharacter::PlayFireSoundCue() const
{
	USoundCue* FireSound = EquippedWeapon->GetFireSound();
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		UParticleSystem* MuzzleFlash = EquippedWeapon->GetMuzzleFlash();
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);;
		}

		FHitResult BeamHitResult;
		const bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);
		if (bBeamEnd)
		{
			// Does hit Actor implement BulletHitInterface?
			if (BeamHitResult.Actor.IsValid())
			{
				IBulletHitInterface* const BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.Actor.Get());
				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult);
				}

				AEnemy* const HitEnemy = Cast<AEnemy>(BeamHitResult.Actor.Get());
				if (HitEnemy)
				{
					int32 Damage;
					if (BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						// Head shot
						Damage = EquippedWeapon->GetHeadShotDamage();
						UGameplayStatics::ApplyDamage(BeamHitResult.Actor.Get(), Damage, GetController(), this, UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, true);
					}
					else
					{
						// Body shot
						Damage = EquippedWeapon->GetDamage();
						UGameplayStatics::ApplyDamage(BeamHitResult.Actor.Get(), Damage, GetController(), this, UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, false);

					}
				}
			}
			else
			{
				// Spawn default particles
				if (ImpactParticle)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, BeamHitResult.Location);
				}
			}

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
			}
		}
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamLocation;
	// Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		// Tentative beam location - still need to trace from gun
		OutBeamLocation = CrosshairHitResult.Location;
	}
	
	// Perform a second trace, this time from the gun barrel
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - WeaponTraceStart };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };
	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	// object between barrel and BeamEndPoint?
	if (!OutHitResult.bBlockingHit) 
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}

	return true;
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// Get world position and direction of crosshairs
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	const bool bScreenToWorld = GetScreenSpaceLocationOfCrosshairs(CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		// Trace from Crosshair world location outward
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

bool AShooterCharacter::GetScreenSpaceLocationOfCrosshairs(FVector& CrosshairWorldPosition, FVector& CrosshairWorldDirection)
{
	FVector2D ViewportSize;
	GetCurrentSizeOfViewport(ViewportSize);

	const FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	return UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation,CrosshairWorldPosition,CrosshairWorldDirection);	
}

void AShooterCharacter::GetCurrentSizeOfViewport(FVector2D& ViewportSize)
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
}

void AShooterCharacter::PlayFireAnimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	StartCrosshairBulletFire();
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed && EquippedWeapon->GetAutomatic())
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
	}
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;
	if (CombatState != ECombatState::ECS_Reloading && CombatState != ECombatState::ECS_Equipping)
	{
		Aim();
	}
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAiming();
}

void AShooterCharacter::StopAiming()
{
	
	bAiming = false;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}
	
	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this, true);
		TraceHitItem = nullptr;
	}
}

void AShooterCharacter::DropWeapon() const
{
	if (!EquippedWeapon)
	{
		return;
	}

	const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

	EquippedWeapon->SetItemState(EItemState::EIS_Falling);
	EquippedWeapon->ThrowWeapon();
}

void AShooterCharacter::SelectButtonReleased()
{
	TraceHitItem = nullptr;
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	// TODO: Fix for AR Weapon
	// TODO: Fix SMG, when 0 bullet is left
	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	if (EquippedWeapon == nullptr)
	{
		return;
	}

	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull())
	{
		if(bAiming)
		{
			StopAiming();
		}
		
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (ReloadMontage && AnimInstance)
		{

			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr)
	{
		return false;
	}

	const auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;	
	}
	
	return false;
}

void AShooterCharacter::FinishReloading()
{
	// Update the Combat State
	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}
	
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	// Update the AmmoMap
	const auto AmmoType = EquippedWeapon->GetAmmoType();
	if (AmmoMap.Contains(AmmoType))
	{
		// Amount of ammo the Character is carrying of the EquippedWeapon type
		int32 CarriedAmmo = AmmoMap[AmmoType];

		// Space left in the magazine of EquippedWeapon
		const int32 MagazineEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if (MagazineEmptySpace > CarriedAmmo)
		{
			// Reload the magazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// Fill the magazine
			EquippedWeapon->ReloadAmmo(MagazineEmptySpace);
			CarriedAmmo -= MagazineEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

void AShooterCharacter::FinishEquipping()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (bAimingButtonPressed)
	{
		Aim();
	}
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}

	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void AShooterCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0)
	{
		return;
	}
	
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 1)
	{
		return;
	}
	
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::TwoKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 2)
	{
		return;
	}
	
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 3)
	{
		return;
	}
	
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 4)
	{
		return;
	}
	
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 5)
	{
		return;
	}
	
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(const int32 CurrentItemIndex, const int32 NewItemIndex)
{
	const bool bCanExchangeItems = CurrentItemIndex != NewItemIndex && NewItemIndex < Inventory.Num() && (CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping);
	if (!bCanExchangeItems)
	{
		return; 
	}

	if (bAiming)
	{
		StopAiming();
	}
		
	const auto OldEquippedWeapon = EquippedWeapon;
	const auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
	EquipWeapon(NewWeapon);
	
	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	NewWeapon->SetItemState(EItemState::EIS_Equipped);

	CombatState = ECombatState::ECS_Equipping;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Equip"));
	}

	NewWeapon->PlayEquipSound(true);
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for (int32 i = 0; i < Inventory.Num(); ++i)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
	}

	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}

	// Inventory is Full
	return -1;
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot{ GetEmptyInventorySlot() };
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	// TODO FIX OVERLAP BUG
	//HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	//HighlightedSlot = -1;
}

FInterpLocation AShooterCharacter::GetInterpolationLocation(const int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];	
	}

	return FInterpLocation();
}

void AShooterCharacter::UpdateOverlappedItemCountValue(const int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound();

	const auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else
		{
			SwapWeapon(Weapon);
		}
	}

	const auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickUpAmmo(Ammo);
	}
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}
	
	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::PickUpAmmo(AAmmo* Ammo)
{
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount = AmmoMap[Ammo->GetAmmoType()];
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr || HandSceneComponent == nullptr)
	{
		return;
	}
	
	const int32 ClipBoneIndex = EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName());
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);
	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

int32 AShooterCharacter::GetInterpolationLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for(int32 i = 1; i < InterpLocations.Num(); ++i)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AShooterCharacter::IncrementInterpolationLocationItemCount(const int32 Index, const int32 Amount)
{
	if (Amount < -1 || Amount > 1)
	{
		return;
	}

	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer,this, &AShooterCharacter::ResetEquipSoundTimer,EquipSoundResetTime);
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

EPhysicalSurface AShooterCharacter::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start = GetActorLocation();
	const FVector End = Start + FVector(0.f, 0.f, -400.f);

	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);
	// TEnumAsByte<EPhysicalSurface> HitSurface = HitResult.PhysMaterial->SurfaceType;\
	
	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}
