//------------------------------------------------------------------------------
// File Name:	B_YCircle360.cpp
// Author(s):	Christian Gonzalse (christian.gonzales)
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "Behaviors/Behavior.h"
#include "B_YCircle360.h"

#include "B_Bullet.h" // bullet 
#include "B_RicochetBullet.h" // ricochet bullet
//input stuff
#include "Inputs/inputKeys.h"
#include "Inputs/InputSystem.h"
//entity components
#include "Entity/EntitySystem.h"
#include "Entity/Entity.h"
#include"Transform/Transform.h"
#include "Physics/Physics.h"
#include "Graphics/Sprite.h"
#include "Graphics/SpriteSystem.h"
#include "Audio/AudioSystem.h"

#include "Audio/AudioSystem.h"

//IMGUI
#include "Editor/EditorSystem.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Scenes/SceneSystem.h"

#include "Messaging/Messaging.h"

#include "Core/Systems/Camera/CameraSystem.h"

void YCircle360System::Init()
{

}

void YCircle360System::Update(float dt)
{
	for (auto& comp : components)
	{
		if (Request<Weapon>(comp.first)->level > 0)
		{
			YCircle360Icon(comp.first, comp.second);
			switch (comp.second->type)
			{
			case YCircle360idle:
				TracePlayer(dt, comp.first, comp.second);
				//SpawnBullet(dt, comp.first, comp.second);
				//SpawnBulletSideShot(dt, comp.first, comp.second);
				SpawnBulletSideShotDelayed(dt, comp.first, comp.second);
				break;
			case YCircle360invalid:
				comp.second->type = YCircle360idle;
				break;
			case YCircle360CD:
				TracePlayer(dt, comp.first, comp.second);
				WeaponCoolDown(dt, comp.first, comp.second);
				break;
			}
		}
		else
		{
			//set damage to zero
			auto YCircle360Weapon = Request<Weapon>(comp.first);
			YCircle360Weapon->currentDamage = 0;
		}
	}
}

void YCircle360System::Exit()
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

void YCircle360System::Deserialize()
{
	auto reader = Serializer::GetInstance();
	reader->ReadFile("Data/JSONS/Behaviors/B_YCircle360.json");
	auto items = reader->GetData("Names");
	for (auto& item : items)
	{
		YCircle360* data = new YCircle360{};
		std::string name = static_cast<std::string>(item);
		//data->attackBaseCD = reader->GetData(name + ".BaseCoolDown");
		//data->attackCooldown = reader->GetData(name + ".CoolDown");
		data->bulletAmounts = reader->GetData(name + ".BulletAmounts");
		data->bulletSpeed = reader->GetData(name + ".BulletSpeed");
		data->bulletType = reader->GetData(name + ".BulletType");
		data->bulletsAmountMod = reader->GetData(name + ".BulletsAmountMod");
		data->bulletLifeSpanTimer = reader->GetData(name + ".BulletLifeSpan");
		data->timeBetweenShots = reader->GetData(name + ".TimeBetweenShots");
		data->baseTimeBetweenShots = data->timeBetweenShots;

		archetypes[name] = data;
	}
}

void YCircle360System::Serialize()
{
	auto ser = Serializer::GetInstance();
	ser->ReadFile("Data/JSONS/Behaviors/B_YCircle360.json");
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
	ser->Transcribe("Data/JSONS/Behaviors/B_YCircle360.json");
	ser->CleanData();
}

void YCircle360System::CreateComponent(const int& id, const std::string& name)
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
	components[id] = new YCircle360{};
	*components[id] = *archetypes[name];
}

void YCircle360System::DestroyComponent(const int& id)
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

void YCircle360System::ClearComponents()
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

void YCircle360System::ActivateComponent(const int& id)
{
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = true;
	}
}

void YCircle360System::DeactivateComponent(const int& id)
{
	if (!components.contains(id)) {
		std::string&& mssg = "Tried to deactivate a PlayerController associated with ID " + id;
		Tracing::Trace(Tracing::WARNING, mssg.c_str());
	}
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = false;
	}
}

void YCircle360System::LevelUpHandler(const LevelUp* e)
{
	if (e->type == "YCircle360")
	{
		for (auto& comp : components)
		{
			auto YCircle360Weapon = Request<Weapon>(comp.first);
			//check if YCircle360 level is max
			if (YCircle360Weapon->level < YCircle360Weapon->maxLevel)
			{
				//if not max then up level
				YCircle360Weapon->level++;
				//change the level damage if it is more than level 2
				if (YCircle360Weapon->level >= 2)
				{
					YCircle360Weapon->currentDamage = ((YCircle360Weapon->damageBoost * YCircle360Weapon->level) * YCircle360Weapon->baseDamage) + YCircle360Weapon->baseDamage;
				}
				else
				{
					//when the weapon first hits level 1
					//activate the YCircle360 and give it to the player or something
					YCircle360Weapon->currentDamage = YCircle360Weapon->baseDamage;
				}

				//change amount to shoot here
				if (YCircle360Weapon->level % comp.second->bulletsAmountMod == 0)
				{
					comp.second->bulletAmounts += 2;
				}
			}
			else
			{
				//to stop you from cheesing item drops
				if (YCircle360Weapon->level == YCircle360Weapon->maxLevel)
				{
					//make it true so i can change color of bullets
					comp.second->isUltFlag = true;
					//ultimate upgrade maybe
					YCircle360Weapon->level++;
					//increase damage, bullet amount, and bullet life span
					comp.second->bulletLifeSpanTimer += comp.second->bulletLifeSpanTimer;
					YCircle360Weapon->currentDamage = ((YCircle360Weapon->damageBoost * (YCircle360Weapon->level + 1)) * YCircle360Weapon->baseDamage) + YCircle360Weapon->baseDamage;
					comp.second->bulletAmounts += 2;
				}
				
			}
		}
		NextLevelUpText(e);
	}
}

void YCircle360System::NextLevelUpText(const LevelUp* e)
{
	for (auto& comp : components)
	{
		auto YCircle360Weapon = Request<Weapon>(comp.first);

		int nextLevel = YCircle360Weapon->level + 1;


		//check if YCircle360 level is max
		if (nextLevel < YCircle360Weapon->maxLevel)
		{
			//change the level damage if it is more than level 2
			if (nextLevel >= 2)
			{
				YCircle360Weapon->levelUpgradeText = "Base Damage up by " + std::to_string(static_cast<int>((YCircle360Weapon->damageBoost * 100)) ) + "%";
			}
			else
			{
				//activate the YCircle360 and give it to the player or something
				YCircle360Weapon->levelUpgradeText = "Base Damage up by " + std::to_string(static_cast<int>((YCircle360Weapon->damageBoost * 100)) ) + "%";
			}

			//change amount to shoot here
			if (nextLevel % comp.second->bulletsAmountMod == 0)
			{
				//YCircle360Weapon->levelUpgradeText = YCircle360Weapon->levelUpgradeText + "~Increase Bullet Shoot";
				YCircle360Weapon->levelUpgradeText = "Shoots Two Addtional Bullets";
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

void YCircle360System::YCircle360Icon(int id, YCircle360* data)
{
	const auto& val = SpecRequest("NametoID", "GameplayHUDYCircle360Icon");
	if (val.has_value())
	{
		//get player id and transform to get its postion
		const int YCircle360IconID = val.value();
		Sprite* YCircle360IconIDSprite = Request<Sprite>(YCircle360IconID);

		if (YCircle360IconIDSprite)
		{
			auto weapon_data = Request<Weapon>(id);
			//set the text component to Lvl: _ the _ is the current level
			std::string YCircle360IconNewText = std::to_string(weapon_data->level);
			YCircle360IconIDSprite->SetText(YCircle360IconNewText);
		}
	}
}

//have the weapon follow the player
void YCircle360System::TracePlayer(float dt, int id, YCircle360* data)
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
void YCircle360System::SpawnBullet(float dt, int id, YCircle360* data)
{
	Transform* WeaponTransform = Request<Transform>(id);
	glm::vec3 WeaponPos = WeaponTransform->GetPos();

	float currentAngle = 0;
	float angleCalculation = 360.0f / data->bulletAmounts;
	glm::vec3 bulletVel = glm::vec3(1.0f, 0.0f, 0.0f);

	for (int i = 0; i < data->bulletAmounts; ++i)
	{
		// getting the current angle based on the iteration by the offset angle addition + some randomness
		currentAngle = (angleCalculation * i) + 90;

		std::optional<int> bullet;

		if (data->isUltFlag)
		{
			//ToDo refactor ricochet bullet
			bullet = SpecRequest("EntityCreation", "RicoBullet");
		}
		else
		{
			//spawn normal bullets
			bullet = SpecRequest("EntityCreation", "Bullet");
		}

		if (bullet.has_value())
		{
			const int bulletID = bullet.value();

			Transform* bulletTransform = Request<Transform>(bulletID);
			Physics* bulletPhysics = Request<Physics>(bulletID);

			// the matrix to multiple the starting vector    bullet speed             rotate by this    around this axis
			glm::mat4 rotationValue = glm::rotate(glm::mat4(data->bulletSpeed), glm::radians(currentAngle), glm::vec3(0, 0, 1));

			// applying that rotation matrix to the original velocity (hint its just a normalized vec in the x axis) 
			glm::vec3 bulletRotationValue = glm::vec3(rotationValue * glm::vec4(bulletVel, 1.0f));

			// setting the stuff
			bulletTransform->SetPos(WeaponPos);
			//bulletTransform->SetRot(currentAngle);
			bulletPhysics->rotationalVelocity(glm::radians(180.0f));
			bulletPhysics->velocity(bulletRotationValue);

			//set bullet damage
			Request<Weapon>(bulletID)->currentDamage = Request<Weapon>(id)->GetDamage();

			//change bullet life span
			if (data->isUltFlag)
			{
				auto bullet_data = Request<RicochetBullet>(bulletID);
				bullet_data->bulletLifeSpan = data->bulletLifeSpanTimer;
			}
			else
			{
				auto bullet_data = Request<Bullet>(bulletID);
				bullet_data->bulletLifeSpan = data->bulletLifeSpanTimer;
			}
		}
	}
	
	data->type = YCircle360CD;
}

//test new way to spawn bullets
void YCircle360System::SpawnBulletSideShot(float dt, int id, YCircle360* data)
{
	Transform* WeaponTransform = Request<Transform>(id);
	glm::vec3 WeaponPos = WeaponTransform->GetPos();

	float spreadAngle = 45.f;
	float currentAngle = 0;
	glm::vec3 bulletVel = glm::vec3(1.0f, 0.0f, 0.0f);

	Broadcast("AUDIOEVENT", new AudioEvent(AUDIO_PLAY, TYPE_SFX, "BulletHit_" + std::to_string(RandomInt(1, 6))));

	//shoot both left and right of the player
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < data->bulletAmounts; ++j)
		{
			currentAngle = (180.0f * i) + (j / (data->bulletAmounts - 1.f) - 0.5f) * spreadAngle;

			std::optional<int> bullet;

			if (data->isUltFlag)
			{
				//ToDo refactor ricochet bullet
				bullet = SpecRequest("EntityCreation", "RicoBullet");
			}
			else
			{
				//spawn normal bullets
				bullet = SpecRequest("EntityCreation", "Bullet");
			}

			if (bullet.has_value())
			{
				const int bulletID = bullet.value();

				Transform* bulletTransform = Request<Transform>(bulletID);
				Physics* bulletPhysics = Request<Physics>(bulletID);

				// the matrix to multiple the starting vector    bullet speed             rotate by this    around this axis
				glm::mat4 rotationValue = glm::rotate(glm::mat4(data->bulletSpeed), glm::radians(currentAngle), glm::vec3(0, 0, 1));

				// applying that rotation matrix to the original velocity (hint its just a normalized vec in the x axis) 
				glm::vec3 bulletRotationValue = glm::vec3(rotationValue * glm::vec4(bulletVel, 1.0f));

				// setting the stuff
				bulletTransform->SetPos(WeaponPos);
				//bulletTransform->SetRot(currentAngle);
				bulletPhysics->rotationalVelocity(glm::radians(180.0f));
				bulletPhysics->velocity(bulletRotationValue);

				//set bullet damage
				Request<Weapon>(bulletID)->currentDamage = Request<Weapon>(id)->GetDamage();

				//change bullet life span
				if (data->isUltFlag)
				{
					auto bullet_data = Request<RicochetBullet>(bulletID);
					bullet_data->bulletLifeSpan = data->bulletLifeSpanTimer;
				}
				else
				{
					auto bullet_data = Request<Bullet>(bulletID);
					bullet_data->bulletLifeSpan = data->bulletLifeSpanTimer;
				}
			}
		}
	}

	data->type = YCircle360CD;
}

//test new way to spawn bullets
void YCircle360System::SpawnBulletSideShotDelayed(float dt, int id, YCircle360* data)
{
	//Broadcast("AUDIOEVENT", new AudioEvent(AUDIO_PLAY, TYPE_SFX, "BulletHit_" + std::to_string(RandomInt(1, 6))));
	if (data->timeBetweenShots <= 0.0f)
	{
		Transform* WeaponTransform = Request<Transform>(id);
		glm::vec3 WeaponPos = WeaponTransform->GetPos();

		float spreadAngle = 45.f;
		float currentAngle = 0;
		glm::vec3 bulletVel = glm::vec3(1.0f, 0.0f, 0.0f);

		//shoot both left and right of the player
		for (int i = 0; i < 2; ++i)
		{
			currentAngle = (180.0f * i) + (data->bulletShot / (data->bulletAmounts - 1.f) - 0.5f)* spreadAngle;;

			std::optional<int> bullet;

			if (data->isUltFlag)
			{
				//ToDo refactor ricochet bullet
				bullet = SpecRequest("EntityCreation", "RicoBullet");
			}
			else
			{
				//spawn normal bullets
				bullet = SpecRequest("EntityCreation", "Bullet");
			}

			if (bullet.has_value())
			{
				const int bulletID = bullet.value();

				Transform* bulletTransform = Request<Transform>(bulletID);
				Physics* bulletPhysics = Request<Physics>(bulletID);

				// the matrix to multiple the starting vector    bullet speed             rotate by this    around this axis
				glm::mat4 rotationValue = glm::rotate(glm::mat4(data->bulletSpeed), glm::radians(currentAngle), glm::vec3(0, 0, 1));

				// applying that rotation matrix to the original velocity (hint its just a normalized vec in the x axis) 
				glm::vec3 bulletRotationValue = glm::vec3(rotationValue * glm::vec4(bulletVel, 1.0f));

				// setting the stuff
				bulletTransform->SetPos(WeaponPos);
				//bulletTransform->SetRot(currentAngle);
				bulletPhysics->rotationalVelocity(glm::radians(180.0f));
				bulletPhysics->velocity(bulletRotationValue);

				//set bullet damage
				Request<Weapon>(bulletID)->currentDamage = Request<Weapon>(id)->GetDamage();

				//change bullet life span
				if (data->isUltFlag)
				{
					auto bullet_data = Request<RicochetBullet>(bulletID);
					bullet_data->bulletLifeSpan = data->bulletLifeSpanTimer;
				}
				else
				{
					auto bullet_data = Request<Bullet>(bulletID);
					bullet_data->bulletLifeSpan = data->bulletLifeSpanTimer;
				}
			}
		}

		++data->bulletShot;
		data->timeBetweenShots = data->baseTimeBetweenShots;

		if (data->bulletShot >= data->bulletAmounts)
		{
			data->bulletShot = 0;
			data->type = YCircle360CD;
		}
	}
	else
	{
		data->timeBetweenShots -= dt;
	}
}

//count down the cooldown to fire agian
void YCircle360System::WeaponCoolDown(float dt, int id, YCircle360* data)
{
	auto weapon_data = Request<Weapon>(id);

	if (weapon_data->attackCooldown >= 0)
	{
		weapon_data->attackCooldown -= dt;

	}
	else
	{
		weapon_data->attackCooldown = weapon_data->attackBaseCD;
		data->type = YCircle360idle;
	}
}

void YCircle360System::CollisionEnterHandler(const CollisionEnter* e)
{

}

void YCircle360System::Register()
{
	BehaviorSystem::RegisterSubsystem("YCircle360", new YCircle360System());
}
