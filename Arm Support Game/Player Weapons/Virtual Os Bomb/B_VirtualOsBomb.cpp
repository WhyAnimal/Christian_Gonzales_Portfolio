//------------------------------------------------------------------------------
// File Name:	B_VirtualOsBomb.cpp
// Author(s):	Christian Gonzalse (christian.gonzales)
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "Behaviors/Behavior.h"
#include "B_VirtualOsBomb.h"

#include "B_VirtualOsBombBullet.h" // VirtualOsBombBullet 
#include "B_GameManager.h" // VobAmount
//entity components
#include "Entity/Entity.h"
#include"Transform/Transform.h"
#include "Physics/Physics.h"
#include "Graphics/Sprite.h"
#include "Graphics/SpriteSystem.h"
#include "Audio/AudioSystem.h"

#include "Messaging/Messaging.h"
#include "Core/Systems/Camera/CameraSystem.h"

void VirtualOsBombSystem::Init()
{

}

void VirtualOsBombSystem::Update(float dt)
{
	for (auto& comp : components)
	{
		if (Request<Weapon>(comp.first)->level > 0)
		{
			VirtualOsBombIcon(comp.first, comp.second);
			switch (comp.second->type)
			{
			case VirtualOsBombidle:
				TracePlayer(dt, comp.first, comp.second);
				SpawnBullet(dt, comp.first, comp.second);
				break;
			case VirtualOsBombinvalid:
				comp.second->type = VirtualOsBombidle;
				break;
			case VirtualOsBombCD:
				TracePlayer(dt, comp.first, comp.second);
				WeaponCoolDown(dt, comp.first, comp.second);
				break;
			}
		}
		else
		{
			//set damage to zero
			auto VirtualOsBombWeapon = Request<Weapon>(comp.first);
			VirtualOsBombWeapon->currentDamage = 0;
		}
	}
}

void VirtualOsBombSystem::Exit()
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

void VirtualOsBombSystem::Deserialize()
{
	auto reader = Serializer::GetInstance();
	reader->ReadFile("Data/JSONS/Behaviors/B_VirtualOsBomb.json");
	auto items = reader->GetData("Names");
	for (auto& item : items)
	{
		VirtualOsBomb* data = new VirtualOsBomb{};
		std::string name = static_cast<std::string>(item);
		//data->attackBaseCD = reader->GetData(name + ".BaseCoolDown");
		//data->attackCooldown = reader->GetData(name + ".CoolDown");
		data->bulletAmounts = reader->GetData(name + ".BulletAmounts");
		data->bulletSpeed = reader->GetData(name + ".BulletSpeed");
		data->bulletType = reader->GetData(name + ".BulletType");
		data->bulletsAmountMod = reader->GetData(name + ".BulletsAmountMod");
		data->bulletLifeSpanTimer = reader->GetData(name + ".BulletLifeSpan");
		data->bulletLifeSpanBoost = reader->GetData(name + ".BulletLifeSpanBoost");
		data->explosionRadius = reader->GetData(name + ".ExplosionRadius");
		data->explosionRadiusBase = data->explosionRadius;
		data->explosionTimer = reader->GetData(name + ".ExplosionTimer");
		data->slowingDownTimer = reader->GetData(name + ".SlowingDownTimer");

		data->explosionGrowSpeed = reader->GetData(name + ".ExplosionGrowSpeed");

		archetypes[name] = data;
	}
}

void VirtualOsBombSystem::Serialize()
{
	auto ser = Serializer::GetInstance();
	ser->ReadFile("Data/JSONS/Behaviors/B_VirtualOsBomb.json");
	std::vector<std::string> names = {};
	for (const auto& pair : archetypes) {
		const auto data = pair.second;

		auto& name = pair.first;
		names.push_back(name);

		//ser->SetData(name + ".BaseCoolDown", data->startingCooldown);
		//ser->SetData(name + ".CoolDown", data->cooldown);
		ser->SetData(name + ".BulletAmounts", data->bulletAmounts);
		ser->SetData(name + ".BulletSpeed", data->bulletSpeed);
		ser->SetData(name + ".BulletType", data->bulletType);
	}
	ser->SetData("Names", names);
	ser->Transcribe("Data/JSONS/Behaviors/B_VirtualOsBomb.json");
	ser->CleanData();
}

void VirtualOsBombSystem::CreateComponent(const int& id, const std::string& name)
{
	if (components.contains(id)) {
		std::string&& mssg = "Tried to create a PlayerController from archetype " + name + ", and associated with ID ";
		mssg += id;
		mssg += ". Old component will be overwritten";
		Tracing::Trace(Tracing::WARNING, mssg.c_str());
	}
	if (!archetypes.contains(name)) {
		/* TODO: This is commented out because there are problems associtated with attempting to create an entity that does not have a specific behavior
				Also writing to a file multiple times per second is not very good
		*/

		//std::string&& mssg = "Tried to create a PlayerController from archetype " + name + ", which does not exist";
		//Tracing::Trace(Tracing::ERROR, mssg.c_str());
		return;
	}
	components[id] = new VirtualOsBomb{};
	*components[id] = *archetypes[name];
}

void VirtualOsBombSystem::DestroyComponent(const int& id)
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

void VirtualOsBombSystem::ClearComponents()
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

void VirtualOsBombSystem::ActivateComponent(const int& id)
{
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = true;
	}
}

void VirtualOsBombSystem::DeactivateComponent(const int& id)
{
	if (!components.contains(id)) {
		std::string&& mssg = "Tried to deactivate a PlayerController associated with ID " + id;
		Tracing::Trace(Tracing::WARNING, mssg.c_str());
	}
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = false;
	}
}

void VirtualOsBombSystem::LevelUpHandler(const LevelUp* e)
{
	if (e->type == "VirtualOsBomb")
	{
		for (auto& comp : components)
		{
			auto VirtualOsBombWeapon = Request<Weapon>(comp.first);
			//check if VirtualOsBomb level is max
			if (VirtualOsBombWeapon->level < VirtualOsBombWeapon->maxLevel)
			{
				//if not max then up level
				VirtualOsBombWeapon->level++;
				//change the level damage if it is more than level 2
				if (VirtualOsBombWeapon->level >= 2)
				{
					VirtualOsBombWeapon->currentDamage = ((VirtualOsBombWeapon->damageBoost * VirtualOsBombWeapon->level) * VirtualOsBombWeapon->baseDamage) + VirtualOsBombWeapon->baseDamage;
				}
				else
				{
					//when the weapon first hits level 1
					//activate the VirtualOsBomb and give it to the player or something
					VirtualOsBombWeapon->currentDamage = VirtualOsBombWeapon->baseDamage;
				}

				//incrase lifeSpan Maybe add radius
				if (VirtualOsBombWeapon->level % comp.second->bulletsAmountMod == 0)
				{
					comp.second->bulletLifeSpanTimer += comp.second->bulletLifeSpanTimer * comp.second->bulletLifeSpanBoost;

					comp.second->explosionRadius = ((VirtualOsBombWeapon->level / comp.second->bulletsAmountMod) * comp.second->bulletLifeSpanBoost) * comp.second->explosionRadiusBase;
				}
			}
			else
			{
				//to stop you from cheesing item drops
				if (VirtualOsBombWeapon->level == VirtualOsBombWeapon->maxLevel)
				{
					//make it true so i can change color of bullets
					comp.second->isUltFlag = true;
					//ultimate upgrade maybe
					VirtualOsBombWeapon->level++;
					//increase damage, bullet amount, and bullet life span
					//comp.second->bulletLifeSpanTimer = comp.second->bulletLifeSpanTimer;
					comp.second->bulletLifeSpanTimer += comp.second->bulletLifeSpanTimer * comp.second->bulletLifeSpanBoost;
					VirtualOsBombWeapon->currentDamage = ((VirtualOsBombWeapon->damageBoost * (VirtualOsBombWeapon->level + 1)) * VirtualOsBombWeapon->baseDamage) + VirtualOsBombWeapon->baseDamage;
					//comp.second->bulletAmounts += 2;
				}

			}
		}
		NextLevelUpText(e);
	}
}

void VirtualOsBombSystem::NextLevelUpText(const LevelUp* e)
{
	for (auto& comp : components)
	{
		auto VirtualOsBombWeapon = Request<Weapon>(comp.first);

		int nextLevel = VirtualOsBombWeapon->level + 1;


		//check if VirtualOsBomb level is max
		if (nextLevel < VirtualOsBombWeapon->maxLevel)
		{
			//change the level damage if it is more than level 2
			if (nextLevel >= 2)
			{
				VirtualOsBombWeapon->levelUpgradeText = "Base Damage up by " + std::to_string(static_cast<int>((VirtualOsBombWeapon->damageBoost * 100))) + "%";
			}
			else
			{
				//activate the VirtualOsBomb and give it to the player or something
				VirtualOsBombWeapon->levelUpgradeText = "Base Damage up by " + std::to_string(static_cast<int>((VirtualOsBombWeapon->damageBoost * 100))) + "%";
			}

			//change amount to shoot here
			if (nextLevel % comp.second->bulletsAmountMod == 0)
			{
				//VirtualOsBombWeapon->levelUpgradeText = VirtualOsBombWeapon->levelUpgradeText + "~Increase Bullet Shoot";
				VirtualOsBombWeapon->levelUpgradeText = "Incrase Explosion Radius";
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

void VirtualOsBombSystem::VirtualOsBombIcon(int id, VirtualOsBomb* data)
{
	const auto& val = SpecRequest("NametoID", "GameplayHUDVirtualOsBombIcon");
	if (val.has_value())
	{
		//get player id and transform to get its postion
		const int VirtualOsBombIconID = val.value();
		Sprite* VirtualOsBombIconIDSprite = Request<Sprite>(VirtualOsBombIconID);

		if (VirtualOsBombIconIDSprite)
		{
			auto weapon_data = Request<Weapon>(id);
			//set the text component to Lvl: _ the _ is the current level
			std::string VirtualOsBombIconNewText = std::to_string(weapon_data->level);
			VirtualOsBombIconIDSprite->SetText(VirtualOsBombIconNewText);
		}
	}
}

//have the weapon follow the player
void VirtualOsBombSystem::TracePlayer(float dt, int id, VirtualOsBomb* data)
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

//spawn bullets
void VirtualOsBombSystem::SpawnBullet(float dt, int id, VirtualOsBomb* data)
{
	Transform* WeaponTransform = Request<Transform>(id);
	glm::vec3 WeaponPos = WeaponTransform->GetPos();

	float currentAngle = 0;
	float angleCalculation = 360.0f / data->bulletAmounts;
	glm::vec3 bulletVel = glm::vec3(1.0f, 0.0f, 0.0f);

	const auto& gameManagerVal = SpecRequest("NametoID", "GameManager");

	if (gameManagerVal.has_value())
	{
		const int gameManagerID = gameManagerVal.value();
		auto gameManager_data = Request<GameManager>(gameManagerID);
		

		for (int i = 0; i < data->bulletAmounts; ++i)
		{
			// getting the current angle based on the iteration by the offset angle addition + some randomness
			currentAngle = RandomFloat(0.0f, 360.f);

			std::optional<int> bullet;

			bullet = SpecRequest("EntityCreation", "VirtualOsBombBullet");

			if (bullet.has_value())
			{
				gameManager_data->vobAmount++;

				const int bulletID = bullet.value();

				Transform* bulletTransform = Request<Transform>(bulletID);
				Physics* bulletPhysics = Request<Physics>(bulletID);

				// the matrix to multiple the starting vector    bullet speed             rotate by this    around this axis
				glm::mat4 rotationValue = glm::rotate(glm::mat4(data->bulletSpeed), glm::radians(currentAngle), glm::vec3(0, 0, 1));

				// applying that rotation matrix to the original velocity (hint its just a normalized vec in the x axis) 
				glm::vec3 bulletRotationValue = glm::vec3(rotationValue * glm::vec4(bulletVel, 1.0f));

				// setting the stuff
				bulletTransform->SetPos(WeaponPos);
				bulletPhysics->velocity(bulletRotationValue);

				//set bullet damage
				Request<Weapon>(bulletID)->currentDamage = Request<Weapon>(id)->GetDamage();

				//change bullet life span
				auto bullet_data = Request<VirtualOsBombBullet>(bulletID);
				bullet_data->virtualOsBombBulletLifeSpan = data->bulletLifeSpanTimer;
				bullet_data->explosionRadius = data->explosionRadius;
				bullet_data->explosionTimer = data->explosionTimer;
				bullet_data->slowingDownTimer = data->slowingDownTimer;
				bullet_data->explosionGrowSpeed = data->explosionGrowSpeed;
			}
		}
	}

	data->type = VirtualOsBombCD;
}

//count down the cooldown to fire agian
void VirtualOsBombSystem::WeaponCoolDown(float dt, int id, VirtualOsBomb* data)
{
	auto weapon_data = Request<Weapon>(id);

	if (weapon_data->attackCooldown >= 0)
	{
		weapon_data->attackCooldown -= dt;

	}
	else
	{
		weapon_data->attackCooldown = weapon_data->attackBaseCD;
		data->type = VirtualOsBombidle;
	}
}

void VirtualOsBombSystem::CollisionEnterHandler(const CollisionEnter* e)
{

}

void VirtualOsBombSystem::Register()
{
	BehaviorSystem::RegisterSubsystem("VirtualOsBomb", new VirtualOsBombSystem());
}
