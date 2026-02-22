//------------------------------------------------------------------------------
// File Name:	B_ForceQuitProgram.cpp
// Author(s):	Christian Gonzalse (christian.gonzales) Juno Insixiengmay
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "Behaviors/Behavior.h"
#include "B_ForceQuitProgram.h"
//input stuff
#include "Inputs/inputKeys.h"
#include "Inputs/InputSystem.h"
//entity components
#include "Entity/Entity.h"
#include"Transform/Transform.h"
#include "Collider/Collider.h"
#include "Physics/Physics.h"
#include <Collider/ColliderCircle.h>
#include "B_BotEnemy.h"
#include "Graphics/Sprite.h"
#include "Graphics/SpriteSystem.h"



void ForceQuitProgramSystem::Init()
{
	
}

void ForceQuitProgramSystem::Update(float dt)
{
	for (auto& comp : components)
	{
		if (Request<Weapon>(comp.first)->level > 0)
		{
			ForceQuitProgramIcon(comp.first, comp.second);
			switch (comp.second->type)
			{
			case ForceQuitProgramIdle:
			{
				TracePlayer(dt, comp.first, comp.second);

				break;
			}
			case ForceQuitPrograminvalid:
				comp.second->type = ForceQuitProgramCD;
				break;
			case ForceQuitProgramCD:
				TracePlayer(dt, comp.first, comp.second);
				WeaponCoolDown(dt, comp.first, comp.second);
				break;
			case ForceQuitProgramAttack:
				TracePlayer(dt, comp.first, comp.second);
				FQPAttackEnemy(dt, comp.first, comp.second);
				break;
			case ForceQuitProgramKillEffect:
				TracePlayer(dt, comp.first, comp.second);
				FQPKillEffectTimer(dt, comp.first, comp.second);
				break;
			}
		}
	}

}

void ForceQuitProgramSystem::Exit()
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

void ForceQuitProgramSystem::Deserialize()
{
	auto reader = Serializer::GetInstance();
	reader->ReadFile("Data/JSONS/Behaviors/B_ForceQuitProgram.json");
	auto items = reader->GetData("Names");
	for (auto& item : items)
	{
		ForceQuitProgram* data = new ForceQuitProgram{};
		std::string name = static_cast<std::string>(item);
		data->amountToKill = reader->GetData(name + ".AmountToKill");
		data->isBig = reader->GetData(name + ".IsBig");
		data->radius = reader->GetData(name + ".Radius");
		data->killEffectTimer = reader->GetData(name + ".KillEffectTimer");
		data->baseKillEffectTimer = reader->GetData(name + ".KillEffectTimer");
		data->cooldownUpgradeMod = reader->GetData(name + ".CooldownUpgradeMod");

		archetypes[name] = data;
	}
}

void ForceQuitProgramSystem::Serialize()
{
}

void ForceQuitProgramSystem::CreateComponent(const int& id, const std::string& name)
{
	if (components.contains(id)) {
		std::string&& mssg = "Tried to create a ForceQuitProgram from archetype " + name + ", and associated with ID ";
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
	components[id] = new ForceQuitProgram{};

	//Create<Weapon>("ForceQuitProgram", id);

	*components[id] = *archetypes[name];
}

void ForceQuitProgramSystem::DestroyComponent(const int& id)
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

void ForceQuitProgramSystem::ClearComponents()
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

void ForceQuitProgramSystem::ActivateComponent(const int& id)
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

void ForceQuitProgramSystem::DeactivateComponent(const int& id)
{
	if (!components.contains(id)) {
		std::string&& mssg = "Tried to deactivate a ForceQuitProgram associated with ID " + id;
		Tracing::Trace(Tracing::WARNING, mssg.c_str());
	}
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = false;
	}
}

void ForceQuitProgramSystem::LevelUpHandler(const LevelUp* e)
{
	if (e->type == "ForceQuitProgram")
	{
		for (auto& comp : components)
		{
			auto FQPWeapon = Request<Weapon>(comp.first);
			//check if FQP level is max
			if (FQPWeapon->level < FQPWeapon->maxLevel)
			{
				//if not max then up level
				FQPWeapon->level++;
				//change the level damage if it is more than level 2
				if (FQPWeapon->level >= 2)
				{
					comp.second->amountToKill++;
				}
				else
				{
					comp.second->amountToKill++;
				}

				//change cooldown here
				if (FQPWeapon->level % comp.second->cooldownUpgradeMod == 0)
				{
					FQPWeapon->attackCooldown = FQPWeapon->attackCooldown * FQPWeapon->damageBoost;
				}

			}
			else
			{
				//to stop you from cheesing item drops
				if (FQPWeapon->level == FQPWeapon->maxLevel)
				{
					//to stop you from cheesing item drops
					if (FQPWeapon->level == FQPWeapon->maxLevel)
					{
						comp.second->isUltFlag = true;
						//ultimate upgrade maybe
						FQPWeapon->level++;
						//ult stuff idk
					}
				}
			}
		}
		NextLevelUpText(e);
	}
}

void ForceQuitProgramSystem::NextLevelUpText(const LevelUp* e)
{
	for (auto& comp : components)
	{
		auto FQPWeapon = Request<Weapon>(comp.first);

		int nextLevel = FQPWeapon->level + 1;


		//check if ForceQuitProgram level is max
		if (nextLevel < FQPWeapon->maxLevel)
		{
			//change the level damage if it is more than level 2
			if (nextLevel >= 2)
			{
				FQPWeapon->levelUpgradeText = "Targets Additional Enemies";
			}
			else
			{
				FQPWeapon->levelUpgradeText = "Targets Additional Enemies";
			}

			//change cooldown here
			if (FQPWeapon->level % comp.second->cooldownUpgradeMod == 0)
			{
				//FQPWeapon->levelUpgradeText = FQPWeapon->levelUpgradeText + "~Reduce cooldown by " + std::to_string(static_cast<int>((FQPWeapon->attackCooldown * 100))) + "%";
				
				FQPWeapon->levelUpgradeText = "Reduce cooldown by " + std::to_string((100 - static_cast<int>((FQPWeapon->damageBoost * 100)))) + "%";
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

void ForceQuitProgramSystem::ForceQuitProgramIcon(int id, ForceQuitProgram* data)
{
	const auto& val = SpecRequest("NametoID", "GameplayHUDForceQuitProgramIcon");
	if (val.has_value())
	{
		//get player id and transform to get its postion
		const int ForceQuitProgramIconID = val.value();
		Sprite* ForceQuitProgramIconIDSprite = Request<Sprite>(ForceQuitProgramIconID);

		if (ForceQuitProgramIconIDSprite)
		{
			auto weapon_data = Request<Weapon>(id);
			//set the text component to Lvl: _ the _ is the current level
			std::string ForceQuitProgramIconNewText = std::to_string(weapon_data->level);
			ForceQuitProgramIconIDSprite->SetText(ForceQuitProgramIconNewText);
		}
	}
}

void ForceQuitProgramSystem::TracePlayer(float dt, int id, ForceQuitProgram* data)
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

void ForceQuitProgramSystem::ResizeFQPCollider(float dt, int id, ForceQuitProgram* data)
{
	Collider* FQPCollider = Request<Collider>(id);
	ColliderCircle* colliderCircle = static_cast<ColliderCircle*>(FQPCollider);

	if (data->isBig)
	{
		//colliderCircle->SetRadius(0);
		//data->isBig = !data->isBig;
	}
	else
	{
		colliderCircle->SetRadius(data->radius);
		data->isBig = !data->isBig;
	}
}

void ForceQuitProgramSystem::WeaponCoolDown(float dt, int id, ForceQuitProgram* data)
{
	auto weapon_data = Request<Weapon>(id);

	if (weapon_data->attackCooldown >= 0)
	{
		weapon_data->attackCooldown -= dt;

	}
	else
	{
		weapon_data->attackCooldown = weapon_data->attackBaseCD;
		data->type = ForceQuitProgramAttack;
	}
}

void ForceQuitProgramSystem::FQPAttackEnemy(float dt, int id, ForceQuitProgram* data)
{
	ResizeFQPCollider(dt, id, data);

	for (int i = 0; i < data->amountToKill; ++i)
	{
		if (data->EnemyID.size() != 0)
		{
			//grab enemy id thats in the vector and sets its health to 0
			int EnemyIDDelete = RandomInt(0, static_cast<int>(data->EnemyID.size() - 1));
			//ToDo Bug here

			if (Query<Enemy>(data->EnemyID[EnemyIDDelete]))
			{

				data->TargetID.push_back(data->EnemyID[EnemyIDDelete]);

				Broadcast("EmitterChildObject", new ParticleEmitterBase("FQPKillEffect", data->EnemyID[EnemyIDDelete]));
				Broadcast("EmitterChildObject", new ParticleEmitterBase("FQPCrossHair", data->EnemyID[EnemyIDDelete]));
				Broadcast("AUDIOEVENT", new AudioEvent(AUDIO_PLAY, TYPE_SFX, "FQP_Charge"));

				data->type = ForceQuitProgramKillEffect;
				

				//get rid of enemy that set to 0 to not call it agian
				if (data->EnemyID.size() > 1)
				{
					auto temp = data->EnemyID[data->EnemyID.size() - 1];

					data->EnemyID[data->EnemyID.size() - 1] = data->EnemyID[EnemyIDDelete];
					data->EnemyID[EnemyIDDelete] = temp;
				}

				data->EnemyID.pop_back();
			}
			else
			{
				data->EnemyID.erase(data->EnemyID.begin() + EnemyIDDelete);
				--i;
			}
		}
		else
		{
			break;
		}
	}
	data->EnemyID.clear();
}

void ForceQuitProgramSystem::FQPKillEffectTimer(float dt, int id, ForceQuitProgram* data)
{
	if (data->killEffectTimer > 0)
	{
		data->killEffectTimer -= dt;
	}
	else
	{
		data->type = ForceQuitProgramCD;
		//kill all target enemies
		for (int i = 0; i < data->TargetID.size(); ++i)
		{
			if (Query<Enemy>(data->TargetID[i]))
			{
				Request<Enemy>(data->TargetID[i])->health = 0;
				Broadcast("AUDIOEVENT", new AudioEvent(AUDIO_PLAY, TYPE_SFX, "FQP_Kill"));
			}			
		}
		data->TargetID.clear();
		data->killEffectTimer = data->baseKillEffectTimer;
	}
}


void ForceQuitProgramSystem::CollisionEnterHandler(const CollisionEnter* e)
{
}

void ForceQuitProgramSystem::CollisionStayHandler(const CollisionStay* e)
{
	//checks the enemy layer
	if (e->layer_2 == 2)
	{
		if (!components.contains(e->id_1)) return;

		if (Request<Weapon>(e->id_1)->level > 0)
		{
			if (components[e->id_1]->type == ForceQuitProgramAttack)
			{
				//if enemy is an Item box then dont target it
				std::string entityName = Request<Entity>(e->id_2)->GetName();
				if (entityName != "ItemBox")
				{
					components[e->id_1]->EnemyID.push_back(e->id_2);
				}
				
			}
		}
		
	}
}

void ForceQuitProgramSystem::Register()
{
	BehaviorSystem::RegisterSubsystem("ForceQuitProgram", new ForceQuitProgramSystem());
}
