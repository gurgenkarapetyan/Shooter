// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GruxAnimInstance.generated.h"

class AEnemy;

/**
 * 
 */
UCLASS()
class SHOOTER_API UGruxAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	/**
	 * Updating properties for animation states.
	 * Called every frame from the animation blueprint from event graph
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(const float DeltaTime);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AEnemy* Enemy;
	
	/** Lateral movement speed. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Speed;
};
