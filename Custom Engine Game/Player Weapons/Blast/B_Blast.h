#pragma once
//------------------------------------------------------------------------------
// File Name:	B_Blast.h
// Author(s):	Christian Gonzalse (christian.gonzales)
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "Behaviors/BehaviorSystem.h"
#include "GenericBehaviors/B_Weapon.h"
#include "Behaviors/Behavior.h"

enum BlastState {
	Blastinvalid = -1
	, BlastIdle
	, BlastCD
};

//data inside the JSON FILE
struct Blast {
	BlastState type = Blastinvalid;
	bool isActive;
	bool isUltFlag = false;

	float shotAngle = 0;
	std::vector <bool> currentDirectionValue = {false, false, false, false};
	std::vector <bool> lastDirectionMoved = { false, true, false, false };

	int lazarAmounts;
	int lazarSecondaryLevelUp;
	float reduceCoolDownBoost;
	float lazarSpeed;
	float lazarGrowSpeed;
	float lazarShrinkSpeed;
	float lazarLifeSpanTimer;
	float lazarLength;
	float explosionRadius;
	float explosionLifeSpan;
};

class BlastSystem : public BehaviorSystem::Subsystem {
public:
	BlastSystem() = default;

	BlastSystem(const BlastSystem* other, const int id) = delete;

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

	void NextLevelUpText(const LevelUp* e);

	bool didDamageFlag = false;

	void ShootingDirection(float dt, int id, Blast* data);

	void SpawnBlast(float dt, int id, Blast* data);
	
	void WeaponCoolDown(float dt, int id, Blast* data);
	void TracePlayer(float dt, int id, Blast* data);

	void BlastIcon(int id, Blast* data);

	void CollisionEnterHandler(const CollisionEnter* e);

	void CollisionStayHandler(const CollisionStay* e);

	std::unordered_map<int, Blast*> components;
	std::unordered_map<std::string, Blast*> archetypes;

	static void Register();

	//make sure to add this to each behaviour
	REGISTER;
};