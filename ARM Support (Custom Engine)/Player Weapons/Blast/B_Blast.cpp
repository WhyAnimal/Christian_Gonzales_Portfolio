//------------------------------------------------------------------------------
// File Name:	B_Blast.cpp
// Author(s):	Christian Gonzalse (christian.gonzales) Juno Insixiengmay
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "Behaviors/Behavior.h"
#include "B_Blast.h"

#include "B_PlayerController.h"//player movement direction
#include "B_LazerBullet.h" // LazerBullet
//input stuff
#include "Inputs/inputKeys.h"
#include "Inputs/InputSystem.h"
//entity components
#include "Entity/Entity.h"
#include"Transform/Transform.h"
#include "Physics/Physics.h"
#include "Graphics/Sprite.h"
#include <Collider/ColliderCircle.h>

#include "Messaging/Messaging.h"
#include "Scenes/SceneSystem.h"
#include "Core/Systems/Camera/CameraSystem.h"

void BlastSystem::Init()
{

}

void BlastSystem::Update(float dt)
{
	for (auto& comp : components)
	{
		if (Request<Weapon>(comp.first)->level > 0)
		{
			BlastIcon(comp.first, comp.second);
			switch (comp.second->type)
			{
			case BlastIdle: {
				TracePlayer(dt, comp.first, comp.second);
				ShootingDirection(dt, comp.first, comp.second);
				SpawnBlast(dt, comp.first, comp.second);
				break;
			}
			case Blastinvalid:
				comp.second->type = BlastIdle;
				break;
			case BlastCD:
				TracePlayer(dt, comp.first, comp.second);
				WeaponCoolDown(dt, comp.first, comp.second);
				ShootingDirection(dt, comp.first, comp.second);
				break;
			}
		}
		else
		{
			//make the particles not show up or dont set the particles to exists		
			//make the current damage zero
			auto BlastWeapon = Request<Weapon>(comp.first);
			BlastWeapon->currentDamage = 0;
		}
	}

}

void BlastSystem::Exit()
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

void BlastSystem::Deserialize()
{
	auto reader = Serializer::GetInstance();
	reader->ReadFile("Data/JSONS/Behaviors/B_Blast.json");
	auto items = reader->GetData("Names");
	for (auto& item : items)
	{
		Blast* data = new Blast{};
		std::string name = static_cast<std::string>(item);
		
		//data->scale_mult = reader->GetData(name + ".Scale Mult"); 
		data->lazarAmounts = reader->GetData(name + ".LazarAmounts");
		data->lazarSecondaryLevelUp = reader->GetData(name + ".LazarSecondaryLevelUp");
		data->reduceCoolDownBoost = reader->GetData(name + ".ReduceCoolDownBoost");
		data->lazarSpeed = reader->GetData(name + ".LazarSpeed");
		data->lazarGrowSpeed = reader->GetData(name + ".LazarGrowSpeed");
		data->lazarShrinkSpeed = reader->GetData(name + ".LazarShrinkSpeed");
		data->lazarLifeSpanTimer = reader->GetData(name + ".LazarLifeSpanTimer");
		data->lazarLength = reader->GetData(name + ".LazarLength");
		data->explosionRadius = reader->GetData(name + ".ExplosionRadius");
		data->explosionLifeSpan = reader->GetData(name + ".ExplosionLifeSpan");

		archetypes[name] = data;
	}
}

void BlastSystem::Serialize()
{
	auto ser = Serializer::GetInstance();
	ser->ReadFile("Data/JSONS/Behaviors/B_Blast.json");
	std::vector<std::string> names = {};
	for (const auto& pair : archetypes) {
		const auto data = pair.second;

		auto& name = pair.first;
		names.push_back(name);

		//ser->SetData(name + ".Scale Mult", data->scale_mult);
		//ser->SetData(name + ".BaseCD", data->attackBaseCD);
		//ser->SetData(name + ".CD", data->attackCooldown);
	}
	ser->SetData("Names", names);
	ser->Transcribe("Data/JSONS/Behaviors/B_Blast.json");
	ser->CleanData();
}

void BlastSystem::CreateComponent(const int& id, const std::string& name)
{
	if (components.contains(id)) {
		std::string&& mssg = "Tried to create a Blast from archetype " + name + ", and associated with ID ";
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
	components[id] = new Blast{};

	//Create<Weapon>("Blast", id);

	*components[id] = *archetypes[name];
}

void BlastSystem::DestroyComponent(const int& id)
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

void BlastSystem::ClearComponents()
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

void BlastSystem::ActivateComponent(const int& id)
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

void BlastSystem::DeactivateComponent(const int& id)
{
	if (!components.contains(id)) {
		std::string&& mssg = "Tried to deactivate a PlayerController associated with ID " + id;
		Tracing::Trace(Tracing::WARNING, mssg.c_str());
	}
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = false;
	}
}

void BlastSystem::LevelUpHandler(const LevelUp* e)
{
	if (e->type == "Blast")
	{
		for (auto& comp : components)
		{
			auto BlastWeapon = Request<Weapon>(comp.first);

			//check if Blast level is max
			if (BlastWeapon->level < BlastWeapon->maxLevel)
			{
				//if not max then up level
				BlastWeapon->level++;
				//change the level damage if it is more than level 2
				if (BlastWeapon->level >= 2)
				{
					BlastWeapon->currentDamage = ((BlastWeapon->damageBoost * BlastWeapon->level) * BlastWeapon->baseDamage) + BlastWeapon->baseDamage;
				}
				else
				{
					//when weapon hits level 1
					
					//give the Blast current damage its base damage 
					BlastWeapon->currentDamage = BlastWeapon->baseDamage;
				}
				
				//Secondary effect
				if (BlastWeapon->level % comp.second->lazarSecondaryLevelUp == 0)
				{
					//reduce cooldown
					BlastWeapon->attackBaseCD = BlastWeapon->attackBaseCD * comp.second->reduceCoolDownBoost;
					//maybe increase bullet size
				}
			}
			else
			{
				//to stop you from cheesing item drops
				if (BlastWeapon->level == BlastWeapon->maxLevel)
				{
					//make it true so i can change color of bullets

					//ultimate upgrade maybe
					comp.second->isUltFlag = true;
					BlastWeapon->level++;
					//increase damage, and idk what else
					BlastWeapon->currentDamage = ((BlastWeapon->damageBoost * (BlastWeapon->level + 1)) * BlastWeapon->baseDamage) + BlastWeapon->baseDamage;

					//shoot the other way as well

				}
			}
		}
		NextLevelUpText(e);
	}
}

void BlastSystem::NextLevelUpText(const LevelUp* e)
{
	for (auto& comp : components)
	{
		auto BlastWeapon = Request<Weapon>(comp.first);

		int nextLevel = BlastWeapon->level + 1;

		//check if Blast level is max
		if (nextLevel < BlastWeapon->maxLevel)
		{
			//change the level damage if it is more than level 2
			if (nextLevel >= 2)
			{
				BlastWeapon->levelUpgradeText = "Base Damage up by " + std::to_string(static_cast<int>((BlastWeapon->damageBoost * 100))) + "%";
			}
			else
			{
				//activate the Blast and give it to the player or something
				BlastWeapon->levelUpgradeText = "Base Damage up by " + std::to_string(static_cast<int>((BlastWeapon->damageBoost * 100))) + "%";
			}

			//change amount to shoot here
			if (nextLevel % comp.second->lazarSecondaryLevelUp == 0)
			{
				BlastWeapon->levelUpgradeText = "reduce Weapon Cooldown";
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

void BlastSystem::ShootingDirection(float dt, int id, Blast* data)
{
	auto player = SpecRequest("NametoID", "Player");

	if (player.has_value())
	{
		const int playerID = player.value();
		auto player_data = Request<PlayerController>(playerID);

		data->currentDirectionValue.clear();

		data->currentDirectionValue.push_back(player_data->moveUpwardFlag);
		data->currentDirectionValue.push_back(player_data->MoveRightFlag);
		data->currentDirectionValue.push_back(player_data->MoveDownwardFlag);
		data->currentDirectionValue.push_back(player_data->MoveLeftFlag);

		int directionAmountMoved = 0;

		for (int i = 0; i < data->currentDirectionValue.size(); ++i)
		{
			if (data->currentDirectionValue[i] == true)
			{
				++directionAmountMoved;
			}
		}

		if (directionAmountMoved > 0)
		{
			data->lastDirectionMoved = data->currentDirectionValue;
		}

		//getting angle for the shot

		if (data->lastDirectionMoved[1] == true)//right
		{
			data->shotAngle = 0.f;
		}
		else if (data->lastDirectionMoved[3] == true) //left
		{
			data->shotAngle = 180.f;
		}

		//up and down
		if (data->lastDirectionMoved[0] == true) // up
		{
			if (data->lastDirectionMoved[1] == true)//up and right
			{
				data->shotAngle = 45.f;
			}
			else if (data->lastDirectionMoved[3] == true) //up and left
			{
				data->shotAngle = 135.f;
			}
			else
			{
				data->shotAngle = 90.f; //just up
			}
		}
		else if (data->lastDirectionMoved[2] == true) //down
		{
			if (data->lastDirectionMoved[1] == true)//down and right
			{
				data->shotAngle = 315.f;
			}
			else if (data->lastDirectionMoved[3] == true) //down and left
			{
				data->shotAngle = 225.f;
			}
			else
			{
				data->shotAngle = 270.f; //just down
			}
		}
	}
}

void BlastSystem::SpawnBlast(float dt, int id, Blast* data)
{
	Transform* WeaponTransform = Request<Transform>(id);
	glm::vec3 WeaponPos = WeaponTransform->GetPos();

	glm::vec3 bulletVel = glm::vec3(1.0f, 0.0f, 0.0f);

	for (int i = 0; i < data->lazarAmounts; ++i)
	{
		//spawn bullet/lazar
		auto bullet = SpecRequest("EntityCreation", "LazerBullet");

		if (bullet.has_value())
		{
			const int bulletID = bullet.value();

			Transform* bulletTransform = Request<Transform>(bulletID);
			Physics* bulletPhysics = Request<Physics>(bulletID);

			// the matrix to multiple the starting vector    bullet speed             rotate by this    around this axis
			glm::mat4 rotationValue = glm::rotate(glm::mat4(data->lazarSpeed), glm::radians(data->shotAngle), glm::vec3(0, 0, 1));

			// applying that rotation matrix to the original velocity (hint its just a normalized vec in the x axis) 
			glm::vec3 bulletRotationValue = glm::vec3(rotationValue * glm::vec4(bulletVel, 1.0f));

			// setting the stuff
			bulletTransform->SetPos(WeaponPos);

			const auto& val = SpecRequest("NametoID", "Player");
			if (val.has_value())
			{
				//get player id and physics to get its postion
				const int playerID = val.value();

				Physics* playerPhysics = Request<Physics>(playerID);

				glm::vec3 playerSpeed = playerPhysics->acceleration();

				//bulletPhysics->velocity(bulletRotationValue + playerSpeed);			

				//set bullet damage
				Request<Weapon>(bulletID)->currentDamage = Request<Weapon>(id)->GetDamage();

				//set lazer variables
				auto bullet_data = Request<LazerBullet>(bulletID);
				bullet_data->lazarGrowSpeed = data->lazarGrowSpeed; // speed of growth 
				bullet_data->lazarShrinkSpeed = data->lazarShrinkSpeed; // speed of lazar shrinking
				bullet_data->bulletVelocity = bulletRotationValue;// + playerSpeed; // bullet velocity
				bullet_data->bulletLifeSpan = data->lazarLifeSpanTimer; // lifespan 
				bullet_data->rotateAngle = data->shotAngle; // angle to rotate bullet

				if (data->isUltFlag)
				{
					bullet_data->isExplodeMode = true; //is ult is on turn on the explosion
					bullet_data->explosionRadius = data->explosionRadius; // radius of explosion
					bullet_data->explosionLifeSpan = data->explosionLifeSpan; // time explosion last for
					
					//spawn extra in opposite way
					auto ultBullet = SpecRequest("EntityCreation", "LazerBullet");

					if (ultBullet.has_value())
					{
						const int ultBulletID = ultBullet.value();

						Transform* ultBulletTransform = Request<Transform>(ultBulletID);
						Physics* ultBulletPhysics = Request<Physics>(ultBulletID);

						// the matrix to multiple the starting vector    bullet speed             rotate by this    around this axis
						glm::mat4 rotationValue = glm::rotate(glm::mat4(data->lazarSpeed), glm::radians((data->shotAngle + 180.f)), glm::vec3(0, 0, 1));

						// applying that rotation matrix to the original velocity (hint its just a normalized vec in the x axis) 
						glm::vec3 ultBulletRotationValue = glm::vec3(rotationValue * glm::vec4(bulletVel, 1.0f));

						Request<Weapon>(ultBulletID)->currentDamage = Request<Weapon>(id)->GetDamage();

						// setting the stuff
						ultBulletTransform->SetPos(WeaponPos);

						//set Opposite lazer variables
						auto ultBullet_data = Request<LazerBullet>(ultBulletID);
						ultBullet_data->lazarGrowSpeed = data->lazarGrowSpeed; // speed of growth 
						ultBullet_data->lazarShrinkSpeed = data->lazarShrinkSpeed; // speed of lazar shrinking
						ultBullet_data->bulletVelocity = ultBulletRotationValue;// + playerSpeed; // bullet velocity
						ultBullet_data->bulletLifeSpan = data->lazarLifeSpanTimer; // lifespan 
						ultBullet_data->rotateAngle = (data->shotAngle + 180.f); // angle to rotate bullet

						ultBullet_data->isExplodeMode = true; //is ult is on turn on the explosion
						ultBullet_data->explosionRadius = data->explosionRadius; // radius of explosion
						ultBullet_data->explosionLifeSpan = data->explosionLifeSpan; // time explosion last for

						//ultBullet_data->isOppositeFlag = true;
					}
				}


			}
		}
	}

	data->type = BlastCD;
}

//count down the cooldown to fire agian
void BlastSystem::WeaponCoolDown(float dt, int id, Blast* data)
{
	auto weapon_data = Request<Weapon>(id);

	if (weapon_data->attackCooldown >= 0)
	{
		weapon_data->attackCooldown -= dt;

	}
	else
	{
		//set manager roate back to zero
		//Transform* WeaponTransform = Request<Transform>(id);
		//WeaponTransform->SetRot(0.0f);

		weapon_data->attackCooldown = weapon_data->attackBaseCD;
		data->type = BlastIdle;
	}
}

void BlastSystem::TracePlayer(float dt, int id, Blast* data)
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

void BlastSystem::BlastIcon(int id, Blast* data)
{
	const auto& val = SpecRequest("NametoID", "GameplayHUDBlastIcon");
	if (val.has_value())
	{
		//get player id and transform to get its postion
		const int BlastIconID = val.value();
		Sprite* BlastIconIDSprite = Request<Sprite>(BlastIconID);

		if (BlastIconIDSprite)
		{
			auto weapon_data = Request<Weapon>(id);
			//set the text component to Lvl: _ the _ is the current level
			std::string BlastIconNewText = std::to_string(weapon_data->level);
			BlastIconIDSprite->SetText(BlastIconNewText);
		}
	}
}

void BlastSystem::CollisionEnterHandler(const CollisionEnter* e)
{

}

void BlastSystem::CollisionStayHandler(const CollisionStay* e)
{

}

void BlastSystem::Register()
{
	BehaviorSystem::RegisterSubsystem("Blast", new BlastSystem());
}
