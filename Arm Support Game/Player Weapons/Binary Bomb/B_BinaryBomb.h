#pragma once
//------------------------------------------------------------------------------
// File Name:	B_BinaryBomb.h
// Author(s):	Christian Gonzalse (christian.gonzales) Juno Insixiengmay
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "Behaviors/BehaviorSystem.h"
#include "GenericBehaviors/B_Weapon.h"
#include "Behaviors/Behavior.h"

enum BinaryBombState {
	BinaryBombinvalid = -1
	, BinaryBombIdle
	, BinaryBombCD
};

//data inside the JSON FILE
struct BinaryBomb {
	BinaryBombState type = BinaryBombinvalid;
	
	glm::vec2 screenSize;

	int heightAmounts;
	int heightLayerNodes;
	std::string bulletType;
	int bulletsAmountMod;

	float layerFuseDelay;
	float bombFuseTimer = 0.f;
	float explosionRadius;
	float explosionGrowSpeed;
	float explosionLifeSpan;
	float explosionShrinkSpeed;

	bool isUltFlag = false;
};

class BinaryBombSystem : public BehaviorSystem::Subsystem {
public:
	BinaryBombSystem() = default;

	BinaryBombSystem(const BinaryBombSystem* other, const int id) = delete;

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




	void WeaponCoolDown(float dt, int id, BinaryBomb* data);

	void TracePlayer(float dt, int id, BinaryBomb* data);

	void SpawnBulletRecursive(float dt, int id, BinaryBomb* data, int currentHeight, int nodePosition, float leftBound, float rightBound);

	void SpawnBullet(float dt, int id, BinaryBomb* data);

	void BinaryBombIcon(int id, BinaryBomb* data);

	void CollisionEnterHandler(const CollisionEnter* e);

	void CollisionStayHandler(const CollisionStay* e);

	std::unordered_map<int, BinaryBomb*> components;
	std::unordered_map<std::string, BinaryBomb*> archetypes;

	static void Register();

	//make sure to add this to each behaviour
	REGISTER;
};