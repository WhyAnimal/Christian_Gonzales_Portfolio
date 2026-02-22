//------------------------------------------------------------------------------
// File Name:	B_BinaryBomb.cpp
// Author(s):	Christian Gonzalse (christian.gonzales) Juno Insixiengmay
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <cmath> // pow

#include "Behaviors/Behavior.h"
#include "B_BinaryBomb.h"
#include "B_GameManager.h"
#include "B_Bullet.h" // bullet

#include "B_BinaryBombBullet.h"

//entity components
#include "Entity/Entity.h"
#include"Transform/Transform.h"
#include "Physics/Physics.h"
#include "Graphics/Sprite.h"
#include <Collider/ColliderCircle.h>

#include <Graphics/ParticleLogic/Particle.h>
#include <Messaging/Events/ParticleRadius.h>



void BinaryBombSystem::Init()
{

}

void BinaryBombSystem::Update(float dt)
{
	for (auto& comp : components)
	{
		if (Request<Weapon>(comp.first)->level > 0)
		{
			BinaryBombIcon(comp.first, comp.second);
			switch (comp.second->type)
			{
			case BinaryBombIdle: {
				TracePlayer(dt, comp.first, comp.second);
				SpawnBullet(dt, comp.first, comp.second);
				break;
			}
			case BinaryBombinvalid:
				comp.second->type = BinaryBombIdle;
				break;
			case BinaryBombCD:
				TracePlayer(dt, comp.first, comp.second);
				WeaponCoolDown(dt, comp.first, comp.second);
				break;
			}
		}
		else
		{
			//make the particles not show up or dont set the particles to exists
			Broadcast("SetRadius", new ParticleRadius(1000.f));
			//make the current damage zero
			auto BinaryBombWeapon = Request<Weapon>(comp.first);
			BinaryBombWeapon->currentDamage = 0;
		}
	}

}

void BinaryBombSystem::Exit()
{
	for (auto& comp : components)
	{
		delete comp.second;
	}
	components.clear();

	for (auto& arch : archetypes)
	{
		delete arch.second;
	}
	archetypes.clear();
}

void BinaryBombSystem::Deserialize()
{
	auto reader = Serializer::GetInstance();
	reader->ReadFile("Data/JSONS/Behaviors/B_BinaryBomb.json");
	auto items = reader->GetData("Names");
	for (auto& item : items)
	{
		BinaryBomb* data = new BinaryBomb{};
		std::string name = static_cast<std::string>(item);
		//data->attackBaseCD = reader->GetData(name + ".BaseCoolDown");
		//data->attackCooldown = reader->GetData(name + ".CoolDown");
		data->heightAmounts = reader->GetData(name + ".HeightAmounts");
		data->bulletType = reader->GetData(name + ".BulletType");
		data->bulletsAmountMod = reader->GetData(name + ".BulletsAmountMod");

		data->layerFuseDelay = reader->GetData(name + ".LayerFuseDelay");
		data->bombFuseTimer = reader->GetData(name + ".BombFuseTimer");
		data->explosionRadius = reader->GetData(name + ".ExplosionRadius");
		data->explosionGrowSpeed = reader->GetData(name + ".ExplosionGrowSpeed");
		data->explosionLifeSpan = reader->GetData(name + ".ExplosionLifeSpan");
		data->explosionShrinkSpeed = reader->GetData(name + ".ExplosionShrinkSpeed");


		archetypes[name] = data;
	}
}

void BinaryBombSystem::Serialize()
{
	auto ser = Serializer::GetInstance();
	ser->ReadFile("Data/JSONS/Behaviors/B_BinaryBomb.json");
	std::vector<std::string> names = {};
	for (const auto& pair : archetypes) {
		const auto data = pair.second;

		auto& name = pair.first;
		names.push_back(name);

		//ser->SetData(name + ".BaseCoolDown", data->startingCooldown);
		//ser->SetData(name + ".CoolDown", data->cooldown);
		//ser->SetData(name + ".BulletAmounts", data->bulletAmounts);
		//ser->SetData(name + ".BulletSpeed", data->bulletSpeed);
		ser->SetData(name + ".BulletType", data->bulletType);
	}
	ser->SetData("Names", names);
	ser->Transcribe("Data/JSONS/Behaviors/B_BinaryBomb.json");
	ser->CleanData();
}

void BinaryBombSystem::CreateComponent(const int& id, const std::string& name)
{
	if (components.contains(id)) {
		std::string&& mssg = "Tried to create a PlayerController from archetype " + name + ", and associated with ID ";
		mssg += id;
		mssg += ". Old component will be overwritten";
		Tracing::Trace(Tracing::WARNING, mssg.c_str());
	}
	else if (!archetypes.contains(name)) {
		/* TODO: This is commented out because there are problems associtated with attempting to create an entity that does not have a specific behavior
				Also writing to a file multiple times per second is not very good
		*/

		//std::string&& mssg = "Tried to create a PlayerController from archetype " + name + ", which does not exist";
		//Tracing::Trace(Tracing::ERROR, mssg.c_str());
		return;
	}
	components[id] = new BinaryBomb{};

	//Create<Weapon>("BinaryBomb", id);

	*components[id] = *archetypes[name];
}

void BinaryBombSystem::DestroyComponent(const int& id)
{
	if (!components.contains(id)) {
		//std::string&& mssg = "Tried to destroy a PlayerController that did not exist associated with ID" + id;
		//Tracing::Trace(Tracing::ERROR, mssg.c_str());
		return;
	}
	else
	{
		auto cmp = components.extract(id);
		delete cmp.mapped();
	}
}

void BinaryBombSystem::ClearComponents()
{
	for (auto& comp : components) {
		delete comp.second;
	}
	for (auto& arch : archetypes) {
		delete arch.second;
	}
	components.clear();
	archetypes.clear();
}

void BinaryBombSystem::ActivateComponent(const int& id)
{
	//if (!components.contains(id)) {
	//	std::string&& mssg = "Tried to activate a PlayerController associated with ID " + id;
	//	Tracing::Trace(Tracing::WARNING, mssg.c_str());
	//}
	//else components[id]->isActive = true;
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = true;
	}
}

void BinaryBombSystem::DeactivateComponent(const int& id)
{
	if (!components.contains(id)) {
		std::string&& mssg = "Tried to deactivate a PlayerController associated with ID " + id;
		Tracing::Trace(Tracing::WARNING, mssg.c_str());
	}
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = false;
	}
}

void BinaryBombSystem::LevelUpHandler(const LevelUp* e)
{
	if (e->type == "BinaryBomb")
	{
		for (auto& comp : components)
		{
			auto BinaryBombWeapon = Request<Weapon>(comp.first);
			//check if BinaryBomb level is max
			if (BinaryBombWeapon->level < BinaryBombWeapon->maxLevel)
			{
				//if not max then up level
				BinaryBombWeapon->level++;
				//change the level damage if it is more than level 2
				if (BinaryBombWeapon->level >= 2)
				{
					BinaryBombWeapon->currentDamage = ((BinaryBombWeapon->damageBoost * BinaryBombWeapon->level) * BinaryBombWeapon->baseDamage) + BinaryBombWeapon->baseDamage;
				}
				else
				{
					//when the weapon first hits level 1
					//activate the BinaryBomb and give it to the player or something
					BinaryBombWeapon->currentDamage = BinaryBombWeapon->baseDamage;
				}

				//change amount to shoot here
				if (BinaryBombWeapon->level % comp.second->bulletsAmountMod == 0)
				{
					comp.second->heightAmounts++;
				}
			}
			else
			{
				//to stop you from cheesing item drops
				if (BinaryBombWeapon->level == BinaryBombWeapon->maxLevel)
				{
					//make it true so i can change color of bullets
					comp.second->isUltFlag = true;
					//ultimate upgrade maybe
					BinaryBombWeapon->level++;
					//increase damage, bullet amount, and bullet life span
					BinaryBombWeapon->currentDamage = ((BinaryBombWeapon->damageBoost * (BinaryBombWeapon->level + 1)) * BinaryBombWeapon->baseDamage) + BinaryBombWeapon->baseDamage;
				}

			}
		}
		NextLevelUpText(e);
	}
}

void BinaryBombSystem::NextLevelUpText(const LevelUp* e)
{
	for (auto& comp : components)
	{
		auto BinaryBombWeapon = Request<Weapon>(comp.first);

		int nextLevel = BinaryBombWeapon->level + 1;


		//check if BinaryBomb level is max
		if (nextLevel < BinaryBombWeapon->maxLevel)
		{
			//change the level damage if it is more than level 2
			if (nextLevel >= 2)
			{
				BinaryBombWeapon->levelUpgradeText = "Base Damage up by " + std::to_string(static_cast<int>((BinaryBombWeapon->damageBoost * 100))) + "%";
			}
			else
			{
				//activate the BinaryBomb and give it to the player or something
				BinaryBombWeapon->levelUpgradeText = "Base Damage up by " + std::to_string(static_cast<int>((BinaryBombWeapon->damageBoost * 100))) + "%";
			}

			//change amount to shoot here
			if (nextLevel % comp.second->bulletsAmountMod == 0)
			{
				//BinaryBombWeapon->levelUpgradeText = BinaryBombWeapon->levelUpgradeText + "~Increase Bullet Shoot";
				BinaryBombWeapon->levelUpgradeText = "Spawns An Addtional Layer";
			}
		}
		else
		{
			//ultimate upgrade maybe
		}
	}
}

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------

//count down the cooldown to fire agian
void BinaryBombSystem::WeaponCoolDown(float dt, int id, BinaryBomb* data)
{
	auto weapon_data = Request<Weapon>(id);

	if (weapon_data->attackCooldown >= 0)
	{
		weapon_data->attackCooldown -= dt;

	}
	else
	{
		weapon_data->attackCooldown = weapon_data->attackBaseCD;
		data->type = BinaryBombIdle;
	}
}

void BinaryBombSystem::TracePlayer(float dt, int id, BinaryBomb* data)
{
	const auto& val = SpecRequest("NametoID", "Player");
	if (val.has_value())
	{
		//get blast id and position
		const int playerID = val.value();

		Transform* playerTransform = Request<Transform>(playerID);

		Transform* myID = Request<Transform>(id);

		myID->SetPos(playerTransform->GetPos());
	}
}

void BinaryBombSystem::BinaryBombIcon(int id, BinaryBomb* data)
{
	const auto& val = SpecRequest("NametoID", "GameplayHUDBinaryBombIcon");
	if (val.has_value())
	{
		//get player id and transform to get its postion
		const int BinaryBombIconID = val.value();
		Sprite* BinaryBombIconIDSprite = Request<Sprite>(BinaryBombIconID);

		if (BinaryBombIconIDSprite)
		{
			auto weapon_data = Request<Weapon>(id);
			//set the text component to Lvl: _ the _ is the current level
			std::string BinaryBombIconNewText = std::to_string(weapon_data->level);
			BinaryBombIconIDSprite->SetText(BinaryBombIconNewText);
		}
	}
}

void BinaryBombSystem::SpawnBulletRecursive(float dt, int id, BinaryBomb* data, int currentHeight, int nodePosition, float leftBound, float rightBound)
{
	if (currentHeight > data->heightAmounts) return;

	Transform* WeaponTransform = Request<Transform>(id);
	glm::vec3 WeaponPos = WeaponTransform->GetPos();

	// Calculate screen dimensions
	glm::vec2 HalfScreenDist = CameraSystem::GetInstance()->GetWorldSpace();
	float yScreenRatio = CameraSystem::GetInstance()->GetRatio();
	HalfScreenDist.x *= -1;
	HalfScreenDist.y = yScreenRatio * HalfScreenDist.x;

	// Calculate position only if not root 
	if (currentHeight >= 0) 
	{
		glm::vec3 bombPos;
		bombPos.x = leftBound + (rightBound - leftBound) / 2.0f;
		bombPos.y = WeaponPos.y + (HalfScreenDist.y / (data->heightAmounts + 1)) * currentHeight;
		bombPos.z = 1.0f;

		// Spawn bomb at this position
		auto bombVal = SpecRequest("EntityCreation", data->bulletType);
		if (bombVal.has_value())
		{
			//get bomb id and transform to set its postion
			const int BombID = bombVal.value();
			Transform* bombTransform = Request<Transform>(BombID);
			bombTransform->SetPos(bombPos);

			//set bomb info
			Request<Weapon>(BombID)->currentDamage = Request<Weapon>(id)->GetDamage();

			auto bomb_data = Request<BinaryBombBullet>(BombID);
			bomb_data->bombFuseTimer = data->bombFuseTimer + (static_cast<float>(currentHeight) * data->layerFuseDelay);
			bomb_data->explosionRadius = data->explosionRadius;
			bomb_data->explosionGrowSpeed = data->explosionGrowSpeed;
			bomb_data->explosionLifeSpan = data->explosionLifeSpan;
			bomb_data->explosionShrinkSpeed = data->explosionShrinkSpeed;
		}

		if (data->isUltFlag)
		{
			//spawn bomb across the y axis
			auto ausBombVal = SpecRequest("EntityCreation", data->bulletType);
			if (ausBombVal.has_value())
			{
				//get bomb id and transform to set its postion
				const int BombID = ausBombVal.value();
				Transform* bombTransform = Request<Transform>(BombID);

				float newPosDiff = bombPos.y - WeaponPos.y;

				bombPos.y -= (newPosDiff * 2);
				bombTransform->SetPos(bombPos);

				Request<Weapon>(BombID)->currentDamage = Request<Weapon>(id)->GetDamage();

				//set bomb info
				Request<Weapon>(BombID)->currentDamage = Request<Weapon>(id)->GetDamage();

				auto bomb_data = Request<BinaryBombBullet>(BombID);
				bomb_data->bombFuseTimer = data->bombFuseTimer + (static_cast<float>(currentHeight) * data->layerFuseDelay);
				bomb_data->explosionRadius = data->explosionRadius;
				bomb_data->explosionGrowSpeed = data->explosionGrowSpeed;
				bomb_data->explosionLifeSpan = data->explosionLifeSpan;
				bomb_data->explosionShrinkSpeed = data->explosionShrinkSpeed;
			}
		}		
	}

	// Recurse to left and right children
	SpawnBulletRecursive(dt, id, data, currentHeight + 1, nodePosition * 2 - 1, leftBound, 
						(currentHeight == 0) ? WeaponPos.x : (leftBound + rightBound) / 2.0f);

	SpawnBulletRecursive(dt, id, data, currentHeight + 1, nodePosition * 2,
						(currentHeight == 0) ? WeaponPos.x : (leftBound + rightBound) / 2.0f, rightBound);
}

//spawn bullets
void BinaryBombSystem::SpawnBullet(float dt, int id, BinaryBomb* data)
{
	Transform* WeaponTransform = Request<Transform>(id);
	glm::vec3 WeaponPos = WeaponTransform->GetPos();

	// Calculate screen dimensions
	glm::vec2 HalfScreenDist = CameraSystem::GetInstance()->GetWorldSpace();
	float yScreenRatio = CameraSystem::GetInstance()->GetRatio();
	HalfScreenDist.x *= -1;
	HalfScreenDist.y = yScreenRatio * HalfScreenDist.x;

	// Start recursion from root (height 0, position 1)
	SpawnBulletRecursive(dt, id, data, 1, 1,
						WeaponPos.x - HalfScreenDist.x,
						WeaponPos.x + HalfScreenDist.x);

	data->type = BinaryBombCD;
}

void BinaryBombSystem::CollisionEnterHandler(const CollisionEnter* e)
{
	
}

void BinaryBombSystem::CollisionStayHandler(const CollisionStay* e)
{

}

void BinaryBombSystem::Register()
{
	BehaviorSystem::RegisterSubsystem("BinaryBomb", new BinaryBombSystem());
}
