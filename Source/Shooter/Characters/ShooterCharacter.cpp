// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Shooter/Actors/Item.h"
#include "Shooter/Actors/Weapon.h"

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
	CameraDefaultFOV(0.f),
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(20.f),
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
	AutomaticFireRate(0.1f),
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	// Item trace variables
	bShouldTraceForItems(false),
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
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
	bAimingButtonPressed(false)
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	SetCharacterMovementConfigurations();
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	EquipWeapon(SpawnDefaultWeapon());
	InitializeAmmoMap();
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	InterpCapsuleHalfHeight(DeltaTime);
	CameraInterpZoom(DeltaTime);
	SetLookUpRates();
	CalculateCrosshairSpread(DeltaTime);
	TraceForItems();
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

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!WeaponToEquip)
	{
		return;
	}

	const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(WeaponToEquip, GetMesh());
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if (!DefaultWeaponClass)
	{	
		return nullptr;	
	}

	return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
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

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
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

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange { 0.f, 600.f };
	FVector2D VelocityMultiplierRange { 0.f, 1.0f };
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	CalculateCrosshairInAirFactor(DeltaTime, CrosshariInAirFactor);
	CalculateCrosshairAimFactor(DeltaTime, CrosshairAimFactor);
	CalculateCrosshairFiringFactor(DeltaTime, CrosshairShootingFactor);
	
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshariInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::CalculateCrosshairInAirFactor(float DeltaTime, float& CrosshariInAir)
{
	if (GetCharacterMovement()->IsFalling())
	{
		CrosshariInAir = FMath::FInterpTo(CrosshariInAir, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshariInAir = FMath::FInterpTo(CrosshariInAir, 0.f, DeltaTime, 30.f);
	}
}

void AShooterCharacter::CalculateCrosshairAimFactor(float DeltaTime, float& CrosshariInAir)
{
	if (bAiming)
	{
		CrosshariInAir = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.f);
	}
	else
	{
		CrosshariInAir = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}
}

void AShooterCharacter::CalculateCrosshairFiringFactor(float DeltaTime, float& CrosshariInAir)
{
	if (bFiringBullet)
	{
		CrosshariInAir = FMath::FInterpTo(CrosshairShootingFactor,  0.3f, DeltaTime, 60.f);	
	}
	else
	{
		CrosshariInAir = FMath::FInterpTo(CrosshairShootingFactor,  0.f, DeltaTime, 60.f);	
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

			if (TraceHitItem && TraceHitItem->GetPickUpWidget())
			{
				TraceHitItem->GetPickUpWidget()->SetVisibility(true);
			}

			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
				}
			}
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult)
{
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = GetScreenSpaceLocationOfCrosshairs(CrosshairWorldPosition, CrosshairWorldDirection);
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
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AShooterCharacter::LookUpAtRate);

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
}

void AShooterCharacter::MoveForward(float Value)
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

void AShooterCharacter::MoveRight(float Value)
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

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
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

void AShooterCharacter::LookUp(float Value)
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

bool AShooterCharacter::WeaponHasAmmo()
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
		CreateFireMuzzleFlashParticle();
		PlayFireAnimMontage();
		StartCrosshairBulletFire();
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();
	}
}

void AShooterCharacter::PlayFireSoundCue()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);;
	}
}

void AShooterCharacter::CreateFireMuzzleFlashParticle()
{
	if (!EquippedWeapon)
	{
		return;
	}
	
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);;
		}

		SetBulletLineTrace(SocketTransform);
	}
}

void AShooterCharacter::SetBulletLineTrace(const FTransform Barrel)
{
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = GetScreenSpaceLocationOfCrosshairs(CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		FHitResult ScreenTraceHit;
		const FVector Start = CrosshairWorldPosition;
		const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * 50000.f;

		FVector BeamEndPoint = End;
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		if (ScreenTraceHit.bBlockingHit)
		{
			BeamEndPoint = ScreenTraceHit.Location;
			if (ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, ScreenTraceHit.Location);
			}
		}

		// perform second trace, this time from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart = Barrel.GetLocation();
		const FVector WeaponTraceEnd = BeamEndPoint;
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

		// object between barrel and BeamEndPoint
		if (WeaponTraceHit.bBlockingHit)
		{
			BeamEndPoint = WeaponTraceHit.Location;
		}
		
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, Barrel);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
			}
		}
	}
}

bool AShooterCharacter::GetScreenSpaceLocationOfCrosshairs(FVector& CrosshairWorldPosition, FVector& CrosshairWorldDirection)
{
	FVector2D ViewportSize;
	GetCurrentSizeOfViewport(ViewportSize);

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

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
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed)
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
	if (CombatState != ECombatState::ECS_Reloading)
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
	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this);
		if (TraceHitItem->GetPickUpSound())
		{
			UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickUpSound());
		}
	}
}

void AShooterCharacter::DropWeapon()
{
	if (!EquippedWeapon)
	{
		return;
	}

	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
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

	auto AmmoType = EquippedWeapon->GetAmmoType();

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

void AShooterCharacter::UpdateOverlappedItemCountValue(int8 Amount)
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

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };

	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	if (Item->GetEquipSound())
	{
		UGameplayStatics::PlaySound2D(this, Item->GetEquipSound());
	}
	
	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		SwapWeapon(Weapon);
	}
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
}

void AShooterCharacter::GrabClip()
{
	// TODO: Fix Clip issue 
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	
	if (HandSceneComponent == nullptr)
	{
		return;
	}
	
	int32 ClipBoneIndex = EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName());
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);
	
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);
	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}
