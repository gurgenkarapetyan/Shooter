// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/BulletHitInterface.h"
#include "Enemy.generated.h"

class USoundCue;
class UBehaviorTree;
class AEnemyAIController;
class USphereComponent;
class UBoxComponent;
class AShooterCharacter;

UCLASS()
class SHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void BulletHit_Implementation(FHitResult HitResult) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	FORCEINLINE FString GetHeadBone() const { return HeadBone; }

	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	/** Display amount of damage applied to. */
	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitNumber(const int32 Damage, const FVector HitLocation, bool bHeadShot);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called when something overlaps with the agro sphere. */
	UFUNCTION()
	void AgroSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** Called when something overlaps with the combat sphere and is in attack range. */
	UFUNCTION()
	void CombatRangeSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** Called when out of overlapping with combat sphere and not anymore in attack range. */
	UFUNCTION()
	void CombatRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Called when left weapon overlap with character during attack. */
	UFUNCTION()
	void OnLeftWeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** Called when right weapon overlap with character during attack. */
	UFUNCTION()
	void OnRightWeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** Apply damage to the actor that enemy attacked.
	 *	@param ShooterCharacter character that will receive an attack.
	 */
	void DoDamage(AShooterCharacter* const ShooterCharacter);


	/** Apply damage to the actor that enemy attacked.
	 *	@param ShooterCharacter character that will receive an attack and spawn a blood effect.
	 *	@param SocketName weapon socket name 
	 */
	void SpawnBlood(const AShooterCharacter* const ShooterCharacter, const FName SocketName) const;

	/** Attempt to stun character
	 * @param ShooterCharacter character that enemy will try to stun.
	 */
	void StunCharacter(AShooterCharacter* const ShooterCharacter);
	
	/** Activate/Deactivate collision for weapon box. */
	UFUNCTION(BlueprintCallable)
	void ActivateLeftWeapon();
	UFUNCTION(BlueprintCallable)
	void DeactivateLeftWeapon();
	UFUNCTION(BlueprintCallable)
	void ActivateRightWeapon();
	UFUNCTION(BlueprintCallable)
	void DeactivateRightWeapon();

	void ResetCanAttack();

	void ToggleEnemyCanAttack(const bool bCanAttackCharacter);

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	UFUNCTION()
	void DestroyEnemy();
	
	/** Toggle InAttackRange and Blackboard values for checking attack.
	 *	@param InAttackRange
	 */
	void SetInAttackRangeProperties(const bool InAttackRange);
	
	/** Display health bar once get shot. */
	UFUNCTION(BlueprintNativeEvent)
	void ShowHeathBar();
	void ShowHeathBar_Implementation();

	/** Hide health bar after some time. */
	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();
	
	/** Death functionality. */
	void Die();

	/**
	* Play montage section when get a hit at specific rate .
	* @param Section name to play montage.
	* @param PlayRate which rate should montage play.
	*/
	void PlayHitMontage(const FName Section, const float PlayRate = 1.f);
	
	void ResetHitReactTimer();

	/** Store Widget/Location in HitNumbers map when hit applied.
	* @param HitNumber display widget reference.
	* @param Location widget location.
	*/
	UFUNCTION(BlueprintCallable)
	void StoreHitNumber(UUserWidget* HitNumber, FVector Location);

	/** Remove HitNumber from HitNumbers Map and ViewPort after specific time.
	 * @param HitNumber to be removed. 
	 */
	UFUNCTION()
	void DestroyHitNumber(UUserWidget* HitNumber);
	
	/** Update HitNumbers screen space location according to the hit location of the bullet. */
	void UpdateHitNumbers() const;

	UFUNCTION(BlueprintCallable)
	void SetStunned(const bool Stunned);

	/**
	* Play montage section when enemy attacks.
	* @param Section name to play montage.
	* @param PlayRate which rate should montage play.
	*/
	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(const FName Section, const float PlayRate = 1.f);

	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName() const;
	
private:
	/** Particles to spawn when hit by bullet. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	/** Sound to play when hit by bullet. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USoundCue* ImpactSound;

	/** Current health. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float Health;

	/** Maximum health. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	/** Name of the head bone. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FString HeadBone;

	FTimerHandle HealthBarTimer;

	/** Time to display health bar once shot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float HealthBarDisplayTime;

	/** Montage containing hit and death animation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	FTimerHandle HitReactTimer;

	/** Range values to generate random number for smoother hit animation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMax;
	
	bool bCanHitReact;

	/** Map to store HitNumber widgets and their hit location. */
	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TMap<UUserWidget*, FVector> HitNumbers;

	/** Time before a HitNumber is removed from the screen. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float HitNumberDestroyTime;

	/** True when playing the get hit animation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bStunned;

	/** Chance of being stunned.
	 *	0: no stun chance
	 *  1: 100% stun chance
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float StunChance;

	/** True when in attack range time to attack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;

	/** True when enemy can attack. */
	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bCanAttack;
	
	FTimerHandle AttackWaitTimer;

	/** Minimum wait time between attacks. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float AttackWaitTime;
	
	/** Montage containing different attacks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	/** The four attack montage section names. */
	FName AttackLFast;
	FName AttackRFast;
	FName AttackL;
	FName AttackR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName LeftWeaponSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName RightWeaponSocket;
	
	/** Collision volume for left weapon. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* LeftWeaponCollision;
	
	/** Collision volume for right weapon. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* RightWeaponCollision;

	/** Base damage for the enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float BaseDamage;

	/** Death anim montage for the enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	bool bDying;

	FTimerHandle DeathTimer;

	/** Time after death until destroy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float DeathTime;
	
	/** Overlap sphere for attack range. */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CombatRangeSphere;
	
	/** Overlap sphere for when the enemy becomes hostile. */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AgrosSphere;
	
	/** Behavior tree for the AI Character. */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;
	
	/** Point for the enemy to move to. */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;

	/** Second Point for the enemy to move to. */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint2;
	
	AEnemyAIController* EnemyAIController;
};
