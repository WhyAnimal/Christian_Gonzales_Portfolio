//------------------------------------------------------------------------------
// File Name:	B_QueueOverflow.c
// Author(s):	Christian Gonzalse (christian.gonzales)
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "Behaviors/Behavior.h"
#include "B_QueueOverflow.h"

#include "B_QueueOverflowBullet.h" // bullet 
//entity components
#include "Entity/Entity.h"
#include"Transform/Transform.h"
#include "Physics/Physics.h"
#include "Graphics/Sprite.h"
#include "Graphics/SpriteSystem.h"
#include "Audio/AudioSystem.h"

#include "Messaging/Messaging.h"
#include "Core/Systems/Camera/CameraSystem.h"

void QueueOverflowSystem::Init()
{

}

void QueueOverflowSystem::Update(float dt)
{
	for (auto& comp : components)
	{
		if (Request<Weapon>(comp.first)->level > 0)
		{
			QueueOverflowIcon(comp.first, comp.second);
			switch (comp.second->type)
			{
			case QueueOverflowidle:
				TracePlayer(dt, comp.first, comp.second);
				SpawnBullet(dt, comp.first, comp.second);
				break;
			case QueueOverflowinvalid:
				comp.second->type = QueueOverflowidle;
				break;
			case QueueOverflowCD:
				TracePlayer(dt, comp.first, comp.second);
				WeaponCoolDown(dt, comp.first, comp.second);
				break;
			}
		}
		else
		{
			//set damage to zero
			auto QueueOverflowWeapon = Request<Weapon>(comp.first);
			QueueOverflowWeapon->currentDamage = 0;
		}
	}
}

void QueueOverflowSystem::Exit()
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

void QueueOverflowSystem::Deserialize()
{
	auto reader = Serializer::GetInstance();
	reader->ReadFile("Data/JSONS/Behaviors/B_QueueOverflow.json");
	auto items = reader->GetData("Names");
	for (auto& item : items)
	{
		QueueOverflow* data = new QueueOverflow{};
		std::string name = static_cast<std::string>(item);
		//data->attackBaseCD = reader->GetData(name + ".BaseCoolDown");
		//data->attackCooldown = reader->GetData(name + ".CoolDown");
		data->bulletAmounts = reader->GetData(name + ".BulletAmounts");
		data->bulletSpeed = reader->GetData(name + ".BulletSpeed");
		data->bulletType = reader->GetData(name + ".BulletType");
		data->bulletsAmountMod = reader->GetData(name + ".BulletsAmountMod");
		data->bulletLifeSpanTimer = reader->GetData(name + ".BulletLifeSpan");

		data->bulletGravity.x = reader->GetData(name + ".XBulletGravity");
		data->bulletGravity.y = reader->GetData(name + ".YBulletGravity");

		data->xBulletMaxVel = reader->GetData(name + ".XBulletMaxVel");
		data->yBulletMaxVel = reader->GetData(name + ".YBulletMaxVel");

		data->timeBetweenShots = reader->GetData(name + ".TimeBetweenShots");
		data-> baseTimeBetweenShots = data->timeBetweenShots;


		archetypes[name] = data;
	}
}

void QueueOverflowSystem::Serialize()
{
	auto ser = Serializer::GetInstance();
	ser->ReadFile("Data/JSONS/Behaviors/B_QueueOverflow.json");
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
	ser->Transcribe("Data/JSONS/Behaviors/B_QueueOverflow.json");
	ser->CleanData();
}

void QueueOverflowSystem::CreateComponent(const int& id, const std::string& name)
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
	components[id] = new QueueOverflow{};
	*components[id] = *archetypes[name];
}

void QueueOverflowSystem::DestroyComponent(const int& id)
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

void QueueOverflowSystem::ClearComponents()
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

void QueueOverflowSystem::ActivateComponent(const int& id)
{
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = true;
	}
}

void QueueOverflowSystem::DeactivateComponent(const int& id)
{
	if (!components.contains(id)) {
		std::string&& mssg = "Tried to deactivate a PlayerController associated with ID " + id;
		Tracing::Trace(Tracing::WARNING, mssg.c_str());
	}
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = false;
	}
}

void QueueOverflowSystem::LevelUpHandler(const LevelUp* e)
{
	if (e->type == "QueueOverflow")
	{
		for (auto& comp : components)
		{
			auto QueueOverflowWeapon = Request<Weapon>(comp.first);
			//check if QueueOverflow level is max
			if (QueueOverflowWeapon->level < QueueOverflowWeapon->maxLevel)
			{
				//if not max then up level
				QueueOverflowWeapon->level++;
				//change the level damage if it is more than level 2
				if (QueueOverflowWeapon->level >= 2)
				{
					QueueOverflowWeapon->currentDamage = ((QueueOverflowWeapon->damageBoost * QueueOverflowWeapon->level) * QueueOverflowWeapon->baseDamage) + QueueOverflowWeapon->baseDamage;
				}
				else
				{
					//when the weapon first hits level 1
					//activate the QueueOverflow and give it to the player or something
					QueueOverflowWeapon->currentDamage = QueueOverflowWeapon->baseDamage;
				}

				//change amount to shoot here
				if (QueueOverflowWeapon->level % comp.second->bulletsAmountMod == 0)
				{
					comp.second->bulletAmounts++;
				}
			}
			else
			{
				//to stop you from cheesing item drops
				if (QueueOverflowWeapon->level == QueueOverflowWeapon->maxLevel)
				{
					//make it true so i can change color of bullets
					comp.second->isUltFlag = true;
					//ultimate upgrade maybe
					QueueOverflowWeapon->level++;
					//increase damage, bullet amount
					QueueOverflowWeapon->currentDamage = ((QueueOverflowWeapon->damageBoost * (QueueOverflowWeapon->level + 1)) * QueueOverflowWeapon->baseDamage) + QueueOverflowWeapon->baseDamage;
					comp.second->bulletAmounts += 2;
				}

			}
		}
		NextLevelUpText(e);
	}
}

void QueueOverflowSystem::NextLevelUpText(const LevelUp* e)
{
	for (auto& comp : components)
	{
		auto QueueOverflowWeapon = Request<Weapon>(comp.first);

		int nextLevel = QueueOverflowWeapon->level + 1;


		//check if QueueOverflow level is max
		if (nextLevel < QueueOverflowWeapon->maxLevel)
		{
			//change the level damage if it is more than level 2
			if (nextLevel >= 2)
			{
				QueueOverflowWeapon->levelUpgradeText = "Base Damage up by " + std::to_string(static_cast<int>((QueueOverflowWeapon->damageBoost * 100))) + "%";
			}
			else
			{
				//activate the QueueOverflow and give it to the player or something
				QueueOverflowWeapon->levelUpgradeText = "Base Damage up by " + std::to_string(static_cast<int>((QueueOverflowWeapon->damageBoost * 100))) + "%";
			}

			//change amount to shoot here
			if (nextLevel % comp.second->bulletsAmountMod == 0)
			{
				//QueueOverflowWeapon->levelUpgradeText = QueueOverflowWeapon->levelUpgradeText + "~Increase Bullet Shoot";
				QueueOverflowWeapon->levelUpgradeText = "Shoots An Addtional Bullet";
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

void QueueOverflowSystem::QueueOverflowIcon(int id, QueueOverflow* data)
{
	const auto& val = SpecRequest("NametoID", "GameplayHUDQueueOverflowIcon");
	if (val.has_value())
	{
		//get player id and transform to get its postion
		const int QueueOverflowIconID = val.value();
		Sprite* QueueOverflowIconIDSprite = Request<Sprite>(QueueOverflowIconID);

		if (QueueOverflowIconIDSprite)
		{
			auto weapon_data = Request<Weapon>(id);
			//set the text component to Lvl: _ the _ is the current level
			std::string QueueOverflowIconNewText = std::to_string(weapon_data->level);
			QueueOverflowIconIDSprite->SetText(QueueOverflowIconNewText);
		}
	}
}

//have the weapon follow the player
void QueueOverflowSystem::TracePlayer(float dt, int id, QueueOverflow* data)
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
void QueueOverflowSystem::SpawnBullet(float dt, int id, QueueOverflow* data)
{
	if (data->timeBetweenShots <= 0.0f)
	{
		Transform* WeaponTransform = Request<Transform>(id);
		glm::vec3 WeaponPos = WeaponTransform->GetPos();

		float spreadAngle = 45.f;
		float currentAngle = 0;
		glm::vec3 bulletVel = glm::vec3(1.0f, 0.0f, 0.0f);

		if (data->bulletShot == 0)
		{
			Broadcast("AUDIOEVENT", new AudioEvent(AUDIO_PLAY, TYPE_SFX, "BulletHit_" + std::to_string(RandomInt(1, 6))));
		}

		//get the bullet vector		
		glm::vec3 bulletVector = glm::vec3{ 0.0f, data->bulletSpeed, 0.0f };
		bulletVector.x = RandomFloat(-data->xBulletMaxVel, data->xBulletMaxVel);
		bulletVector.y = RandomFloat(data->bulletSpeed, (data->yBulletMaxVel + data->bulletSpeed));

		const auto& bullet = SpecRequest("EntityCreation", "QueueOverflowBullet");
		if (bullet.has_value())
		{
			const int bulletID = bullet.value();

			Transform* bulletTransform = Request<Transform>(bulletID);
			Physics* bulletPhysics = Request<Physics>(bulletID);

			// setting the stuff
			bulletTransform->SetPos(WeaponPos);
			bulletPhysics->velocity(bulletVector);
			bulletPhysics->rotationalVelocity(glm::radians(180.0f));

			//set bullet values
			Request<Weapon>(bulletID)->currentDamage = Request<Weapon>(id)->GetDamage();
			auto bullet_data = Request<QueueOverflowBullet>(bulletID);
			bullet_data->bulletLifeSpan = data->bulletLifeSpanTimer;
			bullet_data->baseBulletLifeSpan = data->bulletLifeSpanTimer;
			bullet_data->bulletGravity = data->bulletGravity;
			bullet_data->maxSpeed = bulletVector.y + 1.f;
			bullet_data->isUlt = data->isUltFlag;
		}

		++data->bulletShot;
		data->timeBetweenShots = data->baseTimeBetweenShots;

		if (data->bulletShot == data->bulletAmounts)
		{
			data->type = QueueOverflowCD;
			data->bulletShot = 0;
		}
	}
	else
	{
		data->timeBetweenShots -= dt;
	}		
}

//count down the cooldown to fire agian
void QueueOverflowSystem::WeaponCoolDown(float dt, int id, QueueOverflow* data)
{
	auto weapon_data = Request<Weapon>(id);

	if (weapon_data->attackCooldown >= 0)
	{
		weapon_data->attackCooldown -= dt;

	}
	else
	{
		weapon_data->attackCooldown = weapon_data->attackBaseCD;
		data->type = QueueOverflowidle;
	}
}

void QueueOverflowSystem::CollisionEnterHandler(const CollisionEnter* e)
{

}

void QueueOverflowSystem::Register()
{
	BehaviorSystem::RegisterSubsystem("QueueOverflow", new QueueOverflowSystem());
}
