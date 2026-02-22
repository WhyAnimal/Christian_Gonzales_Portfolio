#pragma once

//------------------------------------------------------------------------------
// File Name:	B_YCircle360.h
// Author(s):	Christian Gonzalse (christian.gonzales)
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "Behaviors/BehaviorSystem.h"
#include "Behaviors/Behavior.h"
#include "GenericBehaviors/B_Weapon.h"

enum YCircle360State {
	YCircle360invalid = -1
	, YCircle360idle
	, YCircle360CD
};

//data inside the JSON FILE
struct YCircle360 {
	YCircle360State type = YCircle360invalid;
	//bool isActive;
	//float startingCooldown;
	//float cooldown;		
	int bulletAmounts;
	float bulletSpeed;
	std::string bulletType;
	int bulletsAmountMod;
	bool isUltFlag = false;
	float bulletLifeSpanTimer;

	float timeBetweenShots = 0.05f;
	float baseTimeBetweenShots = 0.05f;
	int bulletShot = 0;
};

class YCircle360System : public BehaviorSystem::Subsystem {
public:
	YCircle360System() = default;

	YCircle360System(const YCircle360System* other, const int id) = delete;

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
	
	void YCircle360Icon(int id, YCircle360* data);

	void NextLevelUpText(const LevelUp* e);

	//void MoveYCircle360(float dt, int id, Data* data);

	void TracePlayer(float dt, int id, YCircle360* data);

	void SpawnBullet(float dt, int id, YCircle360* data);

	void SpawnBulletSideShot(float dt, int id, YCircle360* data);

	void SpawnBulletSideShotDelayed(float dt, int id, YCircle360* data);

	void WeaponCoolDown(float dt, int id, YCircle360* data);

	void CollisionEnterHandler(const CollisionEnter* e);

	std::unordered_map<int, YCircle360*> components;
	std::unordered_map<std::string, YCircle360*> archetypes;

	static void Register();

	//make sure to add this to each behaviour
	REGISTER;
};