// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "Player/EquippedWeaponTypes.h"
#include "Weapons/WeaponTypes.h"

#include "MasterWeapon.generated.h"


UENUM()
enum class EWeaponState
{
	Idle,
	Firing,
	Equipping,
	Reloading
};

/**
*
*/
UCLASS(ABSTRACT)//, Blueprintable)
class FUSION_API AMasterWeapon : public AActor
{
	
	GENERATED_BODY()

	virtual void PostInitializeComponents() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	float GetEquipStartedTime() const;

	float GetEquipDuration() const;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	bool bIsEquipped;

	bool bPendingEquip;

	FTimerHandle TimerHandle_HandleFiring;

	FTimerHandle EquipFinishedTimerHandle;



protected:

	AMasterWeapon(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* The character socket to store this item at. (primary/seconday) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EEquippedWeaponTypes WeaponSlot;

	/* The character socket to store this item at. (primary/seconday) */
	EWeaponTypes WeaponType = EWeaponTypes::None;

	/** pawn owner */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn)
	class AFusionCharacter* MyPawn;

	/** weapon mesh: 3rd person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh;

	UFUNCTION()
	void OnRep_MyPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

	virtual void OnEquipFinished();

	bool IsEquipped() const;

	bool IsAttachedToPawn() const;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ShotsPerMinute;

public:

	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;

	virtual void OnUnEquip();

	void OnEquip(bool bPlayAnimation);

	/* Set the weapon's owning pawn */
	void SetOwningPawn(AFusionCharacter* NewOwner);

	/* Get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class AFusionCharacter* GetPawnOwner() const;

	virtual void OnEnterInventory(AFusionCharacter* NewOwner);

	virtual void OnLeaveInventory();

	FORCEINLINE EEquippedWeaponTypes GetWeaponSlot() { return WeaponSlot; }

	/* The class to spawn in the level when dropped */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AWeaponPickupActor> WeaponPickupClass;


public:

	/************************************************************************/
	/* Fire & Damage Handling                                               */
	/************************************************************************/


	void StartFire();

	void StopFire();

	EWeaponState GetCurrentState() const;

	/* You can assign default values to function parameters, these are then optional to specify/override when calling the function. */
	void AttachMeshToPawn(EEquippedWeaponTypes Slot = EEquippedWeaponTypes::EWT_Primary);


protected:

	bool CanFire() const;

	FVector GetAdjustedAim() const;

	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

	/* With PURE_VIRTUAL we skip implementing the function in SWeapon.cpp and can do this in SWeaponInstant.cpp / SFlashlight.cpp instead */
	virtual void FireWeapon() PURE_VIRTUAL(AMasterWeapon::FireWeapon, );

private:

	void SetWeaponState(EWeaponState NewState);

	void DetermineWeaponState();

	virtual void HandleFiring();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartFire();
	void ServerStartFire_Implementation();
	bool ServerStartFire_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopFire();
	void ServerStopFire_Implementation();
	bool ServerStopFire_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerHandleFiring();
	void ServerHandleFiring_Implementation();
	bool ServerHandleFiring_Validate();
		
	void OnBurstStarted();

	void OnBurstFinished();

	bool bWantsToFire;

	EWeaponState CurrentState;

	bool bRefiring;

	float LastFireTime;

	/* Time between shots for repeating fire */
	float TimeBetweenShots;

	/************************************************************************/
	/* Simulation & FX                                                      */
	/************************************************************************/

private:

	UFUNCTION()
	void OnRep_BurstCounter();

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* EquipSound;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* EquipAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FireAnim;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	UPROPERTY(EditDefaultsOnly)
	FName MuzzleAttachPoint;

	bool bPlayingFireAnim;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

protected:

	virtual void SimulateWeaponFire();

	virtual void StopSimulatingWeaponFire();
		
	FVector GetMuzzleLocation() const;

	FVector GetMuzzleDirection() const;

	UAudioComponent* PlayWeaponSound(USoundCue* SoundToPlay);

	float PlayWeaponAnimation(UAnimMontage* Animation, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	void StopWeaponAnimation(UAnimMontage* Animation);


	/************************************************************************/
	/* Ammo & Reloading                                                     */
	/************************************************************************/

private:

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* OutOfAmmoSound;

	FTimerHandle TimerHandle_ReloadWeapon;

	FTimerHandle TimerHandle_StopReload;


protected:

	/* Time to assign on reload when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoAnimReloadDuration;

	/* Time to assign on equip when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoEquipAnimDuration;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;

	void UseAmmo();

	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmo;

	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmoInClip;

	/* Weapon ammo on spawn */
	UPROPERTY(EditDefaultsOnly)
	int32 StartAmmo;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmoPerClip;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ReloadAnim;

	virtual void ReloadWeapon();

	/* Trigger reload from server */
	UFUNCTION(Reliable, Client)
	void ClientStartReload();
	void ClientStartReload_Implementation();

	/* Is weapon and character currently capable of starting a reload */
	bool CanReload();

	UFUNCTION()
	void OnRep_Reload();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartReload();
	void ServerStartReload_Implementation();
	bool ServerStartReload_Validate();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopReload();
	void ServerStopReload_Implementation();
	bool ServerStopReload_Validate();

public:

	virtual void StartReload(bool bFromReplication = false);

	virtual void StopSimulateReload();

	/* Give ammo to weapon and return the amount that was not 'consumed' beyond the max count */
	int32 GiveAmmo(int32 AddAmount);

	/* Set a new total amount of ammo of weapon */
	void SetAmmoCount(int32 NewTotalAmount);


	UFUNCTION(BlueprintCallable, Category = "Ammo")
	FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	FORCEINLINE int32 GetCurrentAmmoInClip() const { return CurrentAmmoInClip; }

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	FORCEINLINE int32 GetMaxAmmoPerClip() const { return MaxAmmoPerClip; }

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }
};
