#pragma once
//------------------------------------------------------------------------------
// File Name:	B_FireWall.h
// Author(s):	Christian Gonzalse (christian.gonzales) Juno Insixiengmay
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "Behaviors/BehaviorSystem.h"
#include "GenericBehaviors/B_Weapon.h"
#include "Behaviors/Behavior.h"

enum FireWallState {
	FireWallinvalid = -1
	, FireWallIdle
	, FireWallCD
};

//data inside the JSON FILE
struct FireWall {
	FireWallState type = FireWallinvalid;
	//bool isActive;
	//float damage;
	//float attackCooldown;
	//float attackBaseCD;
	float time = 0.f;
	int scale_mult = 8;
	float radius;
	float baseRadius;
	float maxRadius;
	float radiusBoost;
	int radiusUpgradeMod;
	std::string descriptionText;
	bool isMadePartFlag = false;
	bool isUltFlag = false;
};

class FireWallSystem : public BehaviorSystem::Subsystem {
public:
	FireWallSystem() = default;

	FireWallSystem(const FireWallSystem* other, const int id) = delete;

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

	void NextLevelUpText(const LevelUp* e);

	bool didDamageFlag = false;	

	void WaitAttackCooldown(float dt, int id, FireWall* data);
	
	void TracePlayer(float dt, int id, FireWall* data);

	void FireWallIcon(int id, FireWall* data);

	void CollisionEnterHandler(const CollisionEnter* e);

	void CollisionStayHandler(const CollisionStay* e);

	std::unordered_map<int, FireWall*> components;
	std::unordered_map<std::string, FireWall*> archetypes;

	static void Register();

	//make sure to add this to each behaviour
	REGISTER;
};