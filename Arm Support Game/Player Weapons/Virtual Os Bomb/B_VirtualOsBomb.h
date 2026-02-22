#pragma once

//------------------------------------------------------------------------------
// File Name:	B_VirtualOsBomb.h
// Author(s):	Christian Gonzalse (christian.gonzales)
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "Behaviors/BehaviorSystem.h"
#include "Behaviors/Behavior.h"
#include "GenericBehaviors/B_Weapon.h"

enum VirtualOsBombState {
	VirtualOsBombinvalid = -1
	, VirtualOsBombidle
	, VirtualOsBombCD
};

//data inside the JSON FILE
struct VirtualOsBomb {
	VirtualOsBombState type = VirtualOsBombinvalid;
	//bool isActive;
	//float startingCooldown;
	//float cooldown;		
	int bulletAmounts;
	float bulletSpeed;
	std::string bulletType;
	int bulletsAmountMod;
	bool isUltFlag = false;
	float bulletLifeSpanTimer;
	float bulletLifeSpanBoost;
	float explosionRadius;
	float explosionRadiusBase;
	float explosionTimer = 0.f;
	float slowingDownTimer = 0.f;

	float explosionGrowSpeed;
	float bombExplosionGrow;
};

class VirtualOsBombSystem : public BehaviorSystem::Subsystem {
public:
	VirtualOsBombSystem() = default;

	VirtualOsBombSystem(const VirtualOsBombSystem* other, const int id) = delete;

	// Initialize the current state of the behavior component.
	// Params:
	//	 behavior = Pointer to the behavior component.
	void Init() override;

	// Update the current state of the behavior component.
	// Params:
	//	 behavior = Pointer to the behavior component.
	//	 dt = Change in time (in seconds) since the last game loop.
	void Update(float dt) override;

	// Exit the current state of the behavior component.
	// Params:
	//	 behavior = Pointer to the behavior component.
	//	 dt = Change in time (in seconds) since the last game loop.
	void Exit() override;

	void Deserialize() override;
	void Serialize() override;

	void CreateComponent(const int& id, const std::string& name) override;
	void DestroyComponent(const int& id) override;
	void ClearComponents() override;

	void ActivateComponent(const int& id) override;
	void DeactivateComponent(const int& id) override;

	void LevelUpHandler(const LevelUp*);


private:
	//typedef 

	void VirtualOsBombIcon(int id, VirtualOsBomb* data);

	void NextLevelUpText(const LevelUp* e);

	//void MoveVirtualOsBomb(float dt, int id, Data* data);

	void TracePlayer(float dt, int id, VirtualOsBomb* data);

	void SpawnBullet(float dt, int id, VirtualOsBomb* data);

	void SpawnBulletSideShot(float dt, int id, VirtualOsBomb* data);

	void WeaponCoolDown(float dt, int id, VirtualOsBomb* data);

	void CollisionEnterHandler(const CollisionEnter* e);

	std::unordered_map<int, VirtualOsBomb*> components;
	std::unordered_map<std::string, VirtualOsBomb*> archetypes;

	static void Register();

	//make sure to add this to each behaviour
	REGISTER;
};