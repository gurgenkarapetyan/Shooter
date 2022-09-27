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

	UFUNCTION()
	/** Called when something overlaps with the agro shpere. */
	void AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
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
	
	/** Overlap sphere for when the enemy becomes hostile. */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
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
