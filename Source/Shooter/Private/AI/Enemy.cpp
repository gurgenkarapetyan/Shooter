// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter/Public/AI/Enemy.h"

#include "DrawDebugHelpers.h"
#include "AI/EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/ShooterCharacter.h"
#include "Sound/SoundCue.h"

AEnemy::AEnemy() :
	Health(100.f),
	MaxHealth(100.f),
	HealthBarDisplayTime(4.f),
	HitReactTimeMin(0.5f),
	HitReactTimeMax(0.75f),
	bCanHitReact(true),
	HitNumberDestroyTime(1.5f),
	bStunned(false),
	StunChance(0.5f),
	bCanAttack(true),
	AttackWaitTime(1.f),
	AttackLFast(TEXT("AttackLFast")),
	AttackRFast(TEXT("AttackRFast")),
	AttackL(TEXT("AttackL")),
	AttackR(TEXT("AttackR")),
	LeftWeaponSocket(TEXT("FX_Trail_L_01")),
	RightWeaponSocket(TEXT("FX_Trail_R_01")),
	BaseDamage(20.f),
	bDying(false),
	DeathTime(4.f)
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
	// Create the Agro Sphere 
	AgrosSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Agro Sphere"));
	AgrosSphere->SetupAttachment(GetRootComponent());
	
	// Create the Combat Sphere
 	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Combat Range Sphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	// Construct left and right weapon collision boxes
	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Weapon Box"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));
	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Weapon Box"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	AgrosSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereBeginOverlap);
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeSphereBeginOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeSphereEndOverlap);

	// Bind functions to overlap events for weapon boxes
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponBeginOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponBeginOverlap);

	// Set collision presets for weapon boxes
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	EnemyAIController = Cast<AEnemyAIController>(GetController());
	
	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);
	
	DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), WorldPatrolPoint2, 25.f, 12, FColor::Red, true);

	if (EnemyAIController)
	{
		EnemyAIController->GetBlackBoardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
		EnemyAIController->GetBlackBoardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);
		EnemyAIController->GetBlackBoardComponent()->SetValueAsBool(TEXT("CanAttack"), true);
		EnemyAIController->RunBehaviorTree(BehaviorTree);
	}
}

void AEnemy::AgroSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr)
	{
		return;
	}
	
	AShooterCharacter* const ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		EnemyAIController->GetBlackBoardComponent()->SetValueAsObject(TEXT("Target"), ShooterCharacter);
	}
}

void AEnemy::CombatRangeSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	SetInAttackRangeProperties(true);
}

void AEnemy::CombatRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetInAttackRangeProperties(false);
}

void AEnemy::SetInAttackRangeProperties(const bool InAttackRange)
{
	bInAttackRange = InAttackRange;
	if (EnemyAIController)
	{
		EnemyAIController->GetBlackBoardComponent()->SetValueAsBool(TEXT("InAttackRange"), InAttackRange);
	}
}

void AEnemy::OnLeftWeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* const ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		DoDamage(ShooterCharacter);
		SpawnBlood(ShooterCharacter, LeftWeaponSocket);
		StunCharacter(ShooterCharacter);
	}
}

void AEnemy::OnRightWeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* const ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		DoDamage(ShooterCharacter);
		SpawnBlood(ShooterCharacter, RightWeaponSocket);
		StunCharacter(ShooterCharacter);
	}
}

void AEnemy::DoDamage(AShooterCharacter* const ShooterCharacter)
{
	if (ShooterCharacter == nullptr)
	{
		return;
	}
	
	UGameplayStatics::ApplyDamage(ShooterCharacter, BaseDamage, EnemyAIController, this, UDamageType::StaticClass());

	if (ShooterCharacter->GetMeleeImpactSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShooterCharacter->GetMeleeImpactSound(), GetActorLocation());	
	}
}

void AEnemy::SpawnBlood(const AShooterCharacter* const ShooterCharacter, const FName SocketName) const
{
	if (ShooterCharacter->GetBloodParticles() == nullptr)
	{
		return;
	}
	
	const USkeletalMeshSocket* const TipSocket = GetMesh()->GetSocketByName(SocketName);
	if (TipSocket == nullptr)
	{
		return;
	}
	
	const FTransform SocketTransform = TipSocket->GetSocketTransform(GetMesh());
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShooterCharacter->GetBloodParticles(), SocketTransform);
}

void AEnemy::StunCharacter(AShooterCharacter* const ShooterCharacter)
{
	if (ShooterCharacter)
	{
		const float Stun = FMath::FRandRange(0.f, 1.f);
		if (Stun <= ShooterCharacter->GetStunChance())
		{
			ShooterCharacter->Stun();
		}
	}
}

void AEnemy::ActivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ShowHeathBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHitNumbers();
}

void AEnemy::UpdateHitNumbers() const 
{
	for (const auto& HitPair : HitNumbers)
	{
		UUserWidget* HitNumber = HitPair.Key;
		const FVector Location = HitPair.Value;
		FVector2D ScreenPosition;
		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), Location, ScreenPosition);
		
		HitNumber->SetPositionInViewport(ScreenPosition);
	}
}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
	IBulletHitInterface::BulletHit_Implementation(HitResult);

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
	}

	if (bDying)
	{
		return;
	}
	
	ShowHeathBar();

	// Determine whether bullet hit stuns
	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
		// Stun the enemy
		PlayHitMontage(FName("HitReactFront"));
		SetStunned(true);
	}
}

void AEnemy::PlayHitMontage(const FName Section, const float PlayRate)
{
	UAnimInstance* const AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(HitMontage, PlayRate);
		AnimInstance->Montage_JumpToSection(Section, HitMontage);
	}

	bCanHitReact = false;
	const float HitReactTime = FMath::FRandRange(HitReactTimeMin, HitReactTimeMax);
	GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactTime);
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::SetStunned(const bool Stunned)
{
	bStunned = Stunned;
	if (EnemyAIController)
	{
		EnemyAIController->GetBlackBoardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
	}
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, const FVector Location)
{
	HitNumbers.Add(HitNumber, Location);
	
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);
	FTimerHandle HitNumberTimer;
	GetWorld()->GetTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
	HitNumbers.Remove(HitNumber);
	HitNumber->RemoveFromParent();
}

void AEnemy::PlayAttackMontage(const FName Section, const float PlayRate)
{
	UAnimInstance* const AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage, PlayRate);
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}

	ToggleEnemyCanAttack(false);
	GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackWaitTime);
}

void AEnemy::ResetCanAttack()
{
	ToggleEnemyCanAttack(true);
}

void AEnemy::ToggleEnemyCanAttack(const bool bCanAttackCharacter) 
{
	bCanAttack = bCanAttackCharacter;
	if (EnemyAIController)
	{
		EnemyAIController->GetBlackBoardComponent()->SetValueAsBool(TEXT("CanAttack"), bCanAttackCharacter);
	}
}

void AEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy, DeathTime);
}

void AEnemy::DestroyEnemy()
{
	Destroy();
}

FName AEnemy::GetAttackSectionName() const
{
	FName SectionName; 

	const int32 Section = FMath::RandRange(1, 4);
	switch (Section)
	{
	case 1:
		SectionName = AttackLFast;
	break;
	case 2:
		SectionName = AttackRFast;
	break;
	case 3:
		SectionName = AttackL;
	break;
	case 4:
		SectionName = AttackR;
	break;
	}

	return SectionName;
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Set the Target Blackboard Key to agro the Character
	if (EnemyAIController)
	{
		EnemyAIController->GetBlackBoardComponent()->SetValueAsObject(TEXT("Target"), DamageCauser);
	}
	
	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();
	}
	else
	{
		Health -= DamageAmount;
	}
	
	return DamageAmount;
}

void AEnemy::Die()
{
	if (bDying)
	{
		return;
	}
	bDying = true;
	HideHealthBar();
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}

	if (EnemyAIController)
	{
		EnemyAIController->GetBlackBoardComponent()->SetValueAsBool(TEXT("Dead"), true);
		EnemyAIController->StopMovement();
	}
}
