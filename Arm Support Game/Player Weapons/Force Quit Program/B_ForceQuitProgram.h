#pragma once
//------------------------------------------------------------------------------
// File Name:	#pragma once
//------------------------------------------------------------------------------
// File Name:	B_ForceQuitProgram.h
// Author(s):	Christian Gonzalse (christian.gonzales) Juno Insixiengmay
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "Behaviors/BehaviorSystem.h"
#include "GenericBehaviors/B_Weapon.h"
#include "Behaviors/Behavior.h"

enum ForceQuitProgramState {
	ForceQuitPrograminvalid = -1
	, ForceQuitProgramIdle
	, ForceQuitProgramCD
	, ForceQuitProgramAttack
	, ForceQuitProgramKillEffect
};

//data inside the JSON FILE
struct ForceQuitProgram {
	ForceQuitProgramState type = ForceQuitPrograminvalid;
	float killEffectTimer = 0.f;
	float baseKillEffectTimer;
	int amountToKill;
	bool isBig;
	float radius;
	int cooldownUpgradeMod;
	std::vector<int> EnemyID;
	std::vector<int> TargetID;
	bool isUltFlag = false;
};

class ForceQuitProgramSystem : public BehaviorSystem::Subsystem {
public:
	ForceQuitProgramSystem() = default;

	ForceQuitProgramSystem(const ForceQuitProgramSystem* other, const int id) = delete;

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
	void ForceQuitProgramIcon(int id, ForceQuitProgram* data);
	
	void NextLevelUpText(const LevelUp* e);

	bool didDamageFlag = false;
	
	void TracePlayer(float dt, int id, ForceQuitProgram* data);

	void ResizeFQPCollider(float dt, int id, ForceQuitProgram* data);

	void WeaponCoolDown(float dt, int id, ForceQuitProgram* data);

	void FQPAttackEnemy(float dt, int id, ForceQuitProgram* data);

	void FQPKillEffectTimer(float dt, int id, ForceQuitProgram* data);

	void CollisionEnterHandler(const CollisionEnter* e);
	void CollisionStayHandler(const CollisionStay* e);

	std::unordered_map<int, ForceQuitProgram*> components;
	std::unordered_map<std::string, ForceQuitProgram*> archetypes;

	static void Register();

	//make sure to add this to each behaviour
	REGISTER;
};