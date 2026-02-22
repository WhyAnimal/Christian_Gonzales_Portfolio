//------------------------------------------------------------------------------
// File Name:	B_FireWall.cpp
// Author(s):	Christian Gonzalse (christian.gonzales) Juno Insixiengmay
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "Behaviors/Behavior.h"
#include "B_FireWall.h"

#include "B_GameManager.h"
//input stuff
#include "Inputs/inputKeys.h"
#include "Inputs/InputSystem.h"
//entity components
#include "Entity/Entity.h"
#include"Transform/Transform.h"
#include "Physics/Physics.h"
#include "Graphics/Sprite.h"
#include <Collider/ColliderCircle.h>
#include "B_BotEnemy.h"
#include <Graphics/ParticleLogic/Particle.h>
#include <Messaging/Events/ParticleRadius.h>



void FireWallSystem::Init()
{

}

void FireWallSystem::Update(float dt)
{
	for (auto& comp : components)
	{
		if (Request<Weapon>(comp.first)->level > 0)
		{
			if (comp.second->isMadePartFlag == false)
			{
				Broadcast("EmitterChildObject", new ParticleEmitterBase("WallEffect", comp.first));
				comp.second->isMadePartFlag = true;
			}

			FireWallIcon(comp.first, comp.second);
			switch (comp.second->type)
			{
			case FireWallIdle: {
				TracePlayer(dt, comp.first, comp.second);
				if (didDamageFlag)
				{
					didDamageFlag = false;
					comp.second->type = FireWallCD;
				}
				auto transform = Request<Transform>(comp.first);
				auto scale = transform->GetScale();
				//auto rot = transform->GetRot();
				if (comp.second->time > 0.5f) {
					comp.second->scale_mult *= -1;
					comp.second->time = 0;
				}
				scale += sin(dt) * comp.second->scale_mult;
				//rot += dt * 15;
				comp.second->time += dt;
				transform->SetScale(scale);
				//transform->SetRot(rot);
				break;
			}
			case FireWallinvalid:
				comp.second->type = FireWallIdle;
				break;
			case FireWallCD:
				//TracePlayer(dt, comp.first, comp.second);
				WaitAttackCooldown(dt, comp.first, comp.second);
				break;
			}
		}
		else
		{
			//make the particles not show up or dont set the particles to exists
			
			//make the current damage zero
			auto FireWallWeapon = Request<Weapon>(comp.first);
			FireWallWeapon->currentDamage = 0;
		}
	}

}

void FireWallSystem::Exit()
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

void FireWallSystem::Deserialize()
{
	auto reader = Serializer::GetInstance();
	reader->ReadFile("Data/JSONS/Behaviors/B_FireWall.json");
	auto items = reader->GetData("Names");
	for (auto& item : items)
	{
		FireWall* data = new FireWall{};
		std::string name = static_cast<std::string>(item);
		data->scale_mult = reader->GetData(name + ".Scale Mult");
		data->radius = reader->GetData(name + ".Radius");
		data->baseRadius = data->radius;
		data->radiusBoost = reader->GetData(name + ".RadiusBoost");
		data->radiusUpgradeMod = reader->GetData(name + ".RadiusUpgradeMod");
		data->descriptionText = "urmom0";
		data->isMadePartFlag = false;

		data->maxRadius = reader->GetData(name + ".MaxRadius");
		

		archetypes[name] = data;
	}
}

void FireWallSystem::Serialize()
{
	auto ser = Serializer::GetInstance();
	ser->ReadFile("Data/JSONS/Behaviors/B_FireWall.json");
	std::vector<std::string> names = {};
	for (const auto& pair : archetypes) {
		const auto data = pair.second;

		auto& name = pair.first;
		names.push_back(name);

		ser->SetData(name + ".Scale Mult", data->scale_mult);
		//ser->SetData(name + ".BaseCD", data->attackBaseCD);
		//ser->SetData(name + ".CD", data->attackCooldown);
	}
	ser->SetData("Names", names);
	ser->Transcribe("Data/JSONS/Behaviors/B_FireWall.json");
	ser->CleanData();
}

void FireWallSystem::CreateComponent(const int& id, const std::string& name)
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
	components[id] = new FireWall{};

	//Broadcast("EmitterChildObject", new ParticleEmitterBase("WallEffect", SpecRequest("NametoID", "Player").value()));

	//Create<Weapon>("FireWall", id);

	*components[id] = *archetypes[name];
}

void FireWallSystem::DestroyComponent(const int& id)
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

void FireWallSystem::ClearComponents()
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

void FireWallSystem::ActivateComponent(const int& id)
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

void FireWallSystem::DeactivateComponent(const int& id)
{
	if (!components.contains(id)) {
		std::string&& mssg = "Tried to deactivate a PlayerController associated with ID " + id;
		Tracing::Trace(Tracing::WARNING, mssg.c_str());
	}
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = false;
	}
}

void FireWallSystem::LevelUpHandler(const LevelUp* e)
{
	if (e->type == "FireWall")
	{
		for (auto& comp : components)
		{
			auto FireWallWeapon = Request<Weapon>(comp.first);

			
			//check if FireWall level is max
			if (FireWallWeapon->level < FireWallWeapon->maxLevel)
			{
				//if not max then up level
				FireWallWeapon->level++;

				ColliderCircle* radiusCollider = static_cast<ColliderCircle*>(Request<Collider>(comp.first));

				//change the level damage if it is more than level 2
				if (FireWallWeapon->level >= 2)
				{
					FireWallWeapon->currentDamage = ((FireWallWeapon->damageBoost * (FireWallWeapon->level)) * FireWallWeapon->baseDamage) + FireWallWeapon->baseDamage;
				}
				else
				{
					//when weapon hits level 1
					//activate the firewall particles effect 
					//Broadcast("EmitterChildObject", new ParticleEmitterBase("P", comp.first));
					float NewRadius = radiusCollider->GetRadius();
					Broadcast("SetRadius", new ParticleRadius(NewRadius));
					//give the firewall current damage its base damage 
					FireWallWeapon->currentDamage = FireWallWeapon->baseDamage;
				}

				//change radius size here
				if (FireWallWeapon->level % comp.second->radiusUpgradeMod == 0)
				{
					int timesSizeIncrease = FireWallWeapon->level / comp.second->radiusUpgradeMod;
					float NewRadius = comp.second->baseRadius;
					NewRadius += (comp.second->radiusBoost * timesSizeIncrease) * comp.second->baseRadius;

					if (NewRadius >= comp.second->maxRadius)
					{
						NewRadius = comp.second->maxRadius;
					}

					//set radius of collider and particle
					radiusCollider->SetRadius(NewRadius);
					Broadcast("SetRadius", new ParticleRadius(NewRadius));
				}
			}
			else
			{
				//to stop you from cheesing item drops
				if (FireWallWeapon->level == FireWallWeapon->maxLevel)
				{
					//ultimate upgrade maybe
					comp.second->isUltFlag = true;
					FireWallWeapon->level++;
					//increase damage
					FireWallWeapon->currentDamage = ((FireWallWeapon->damageBoost * (FireWallWeapon->level + 1)) * FireWallWeapon->baseDamage) + FireWallWeapon->baseDamage;
					//change radius to max
					ColliderCircle* radiusCollider = static_cast<ColliderCircle*>(Request<Collider>(comp.first));
					float NewRadius = radiusCollider->GetRadius();
					NewRadius = comp.second->maxRadius;
					//set radius of collider and particle
					radiusCollider->SetRadius(NewRadius);
					Broadcast("SetRadius", new ParticleRadius(NewRadius));
				}
			}
		}
		NextLevelUpText(e);
	}
}

void FireWallSystem::NextLevelUpText(const LevelUp* e)
{
	for (auto& comp : components)
	{
		auto FireWallWeapon = Request<Weapon>(comp.first);

		int nextLevel = FireWallWeapon->level;
		++nextLevel;


		//check if FireWall level is max
		if (nextLevel < FireWallWeapon->maxLevel)
		{
			//change the level damage if it is more than level 2
			if (nextLevel >= 2)
			{
				FireWallWeapon->levelUpgradeText = "Base Damage up by " + std::to_string( static_cast<int>((FireWallWeapon->damageBoost * 100)) ) + "%";
			}
			else
			{
				//activate the firewall and give it to the player or something
				FireWallWeapon->levelUpgradeText = "Base Damage up by " + std::to_string( static_cast<int>((FireWallWeapon->damageBoost * 100)) ) + "%";
			}

			//Change Radius and size
			if (nextLevel % comp.second->radiusUpgradeMod == 0)
			{
				ColliderCircle* radiusCollider = static_cast<ColliderCircle*>(Request<Collider>(comp.first));
				float NewRadius = radiusCollider->GetRadius();
				if (NewRadius == comp.second->maxRadius)
				{
					return;
				}
				//FireWallWeapon->levelUpgradeText = FireWallWeapon->levelUpgradeText + "~Radius Change by " + std::to_string(static_cast<int>((comp.second->radiusBoost * 100))) + "%";

				FireWallWeapon->levelUpgradeText = "Increase Radius by " + std::to_string(static_cast<int>((comp.second->radiusBoost * 100))) + "%";
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

void FireWallSystem::WaitAttackCooldown(float dt, int id, FireWall* data)
{
	auto weapon_data = Request<Weapon>(id);
	if (weapon_data->attackCooldown >= 0)
	{
		weapon_data->attackCooldown -= dt;
		TracePlayer(dt, id, data);
	}
	else
	{
		TracePlayer(dt, id, data);
		weapon_data->attackCooldown = weapon_data->attackBaseCD;
	}
}

void FireWallSystem::TracePlayer(float dt, int id, FireWall* data)
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

void FireWallSystem::FireWallIcon( int id, FireWall* data)
{
	const auto& val = SpecRequest("NametoID", "GameplayHUDFireWallIcon");
	if (val.has_value())
	{
		//get player id and transform to get its postion
		const int FireWallIconID = val.value();
		Sprite* FireWallIconIDSprite = Request<Sprite>(FireWallIconID);

		if (FireWallIconIDSprite)
		{
			auto weapon_data = Request<Weapon>(id);
			//set the text component to Lvl: _ the _ is the current level
			std::string FireWallIconNewText = std::to_string(weapon_data->level);
			FireWallIconIDSprite->SetText(FireWallIconNewText);
		}
	}
}

void FireWallSystem::CollisionEnterHandler(const CollisionEnter* e)
{
	////checks other layer if on enemy collider layer
	//if (e->layer_2 == 2)
	//{
	//	if (!components.contains(e->id_1)) return;

	////	//checks if the firewall on cooldown
	//	if (components[e->id_1]->type != FireWallCD)
	//	{
			


	//		/*
	//		didDamageFlag = true;
	//		DmgBroadcast("TakenDamage", e->id_2, 25.0f);
	//		*/

	//		//if not on cooldown then delete 
	//		int enemyID = e->id_2;
	//		//check enemy dropExp flag if true then call to drop exp if false dont
	//		//if()
	//		//{
	//			int DropChance = RandomInt(0, 100);
	//			if (DropChance <= 35)
	//			{
	//				Transform* enemyTransform = Request<Transform>(enemyID);
	//				//create an XP to spawn on the enemy death location
	//				int XpID = System::RequestSceneAddition<SceneSystem>("DeathEXP", enemyID);
	//				Transform* XpTransform = Request<Transform>(XpID);
	//				XpTransform->SetPos(enemyTransform->GetPos());
	//			}
	//			
	//		//}

	//		SpecBroadcast("MarkEntity", enemyID);
		//}

	//	
	//}



	////if (e->layer_1 == 1)
	////{
	////	if (!(components[e->id_1]->type == FireWallCD))
	////	{
	////		//components[e->id_1]->type = FireWallCD;
	////		//_Deactivate<Collider>(e->id_1);
	////		//DeactivateComponent(e->id_1);
	////	}
	////	
	////}
	//


	////	ColliderCircle* trans2 = static_cast<ColliderCircle*>(Request<Collider>(e->id_1));
	//	//trans2->Deactivate();
	//	//_Deactivate<Behavior>(e->id_1);
	//	//System::Deact<EntitySystem>(id());
}

void FireWallSystem::CollisionStayHandler(const CollisionStay* e)
{
	//checks the enemy layer
	if (e->layer_2 == 2)
	{
		if (!components.contains(e->id_1)) return;

		if (Request<Weapon>(e->id_1)->level > 0)
		{
			//get damage boost
			const auto& val = SpecRequest("NametoID", "GameManager");
			if (val.has_value())
			{
				//get Blast id and rotation
				const int gameManagerID = val.value();

				auto gameManager_data = Request<GameManager>(gameManagerID);

				float damage = Request<Weapon>(e->id_1)->GetDamage();

				damage += damage * gameManager_data->damageBoost;

				auto enemyHealth = Request<Enemy>(e->id_2)->health;

				Request<Enemy>(e->id_2)->health -= damage * Time::GetDt();
				Request<Enemy>(e->id_2)->takenDamageFlag = true;
			}
		}
	}
}

void FireWallSystem::Register()
{
	BehaviorSystem::RegisterSubsystem("FireWall", new FireWallSystem());
}
