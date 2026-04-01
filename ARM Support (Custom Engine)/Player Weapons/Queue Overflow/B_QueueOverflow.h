#pragma once

//------------------------------------------------------------------------------
// File Name:	B_QueueOverflow.h
// Author(s):	Christian Gonzalse (christian.gonzales)
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "Behaviors/BehaviorSystem.h"
#include "Behaviors/Behavior.h"
#include "GenericBehaviors/B_Weapon.h"

enum QueueOverflowState {
	QueueOverflowinvalid = -1
	, QueueOverflowidle
	, QueueOverflowCD
};

//data inside the JSON FILE
struct QueueOverflow {
	QueueOverflowState type = QueueOverflowinvalid;
	//bool isActive;
	//float startingCooldown;
	//float cooldown;		
	int bulletAmounts;
	int bulletShot = 0;
	float bulletLifeSpanTimer;
	float bulletSpeed;
	float xBulletMaxVel;
	float yBulletMaxVel;
	std::string bulletType;
	int bulletsAmountMod;
	bool isUltFlag = false;
	glm::vec3 bulletGravity = { 0.0f, 0.0f ,0.0f };

	float timeBetweenShots;
	float baseTimeBetweenShots;
};

class QueueOverflowSystem : public BehaviorSystem::Subsystem {
public:
	QueueOverflowSystem() = default;

	QueueOverflowSystem(const QueueOverflowSystem* other, const int id) = delete;

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

	void QueueOverflowIcon(int id, QueueOverflow* data);

	void NextLevelUpText(const LevelUp* e);

	//void MoveQueueOverflow(float dt, int id, Data* data);

	void TracePlayer(float dt, int id, QueueOverflow* data);

	void SpawnBullet(float dt, int id, QueueOverflow* data);

	void WeaponCoolDown(float dt, int id, QueueOverflow* data);

	void CollisionEnterHandler(const CollisionEnter* e);

	std::unordered_map<int, QueueOverflow*> components;
	std::unordered_map<std::string, QueueOverflow*> archetypes;

	static void Register();

	//make sure to add this to each behaviour
	REGISTER;
};