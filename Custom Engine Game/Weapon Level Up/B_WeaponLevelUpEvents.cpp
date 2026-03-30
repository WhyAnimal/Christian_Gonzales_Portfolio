//------------------------------------------------------------------------------
// File Name:	B_WeaponLevelUpEvents.cpp
// Author(s):	Christian Gonzalse (christian.gonzales) Juno Insixiengmay
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "Behaviors/Behavior.h"
#include "B_WeaponLevelUpEvents.h"

#include "B_PlayerController.h"
//input stuff
#include "Inputs/inputKeys.h"
#include "Inputs/InputSystem.h"
//entity components
#include "Entity/Entity.h"
#include"Transform/Transform.h"
#include "Physics/Physics.h"
#include <Collider/ColliderCircle.h>
#include "B_BotEnemy.h"
#include <Graphics/ParticleLogic/Particle.h>
#include "Scenes/SceneSystem.h"
#include "Graphics/Sprite.h"
#include "Buttons/B_Button.h"
#include "Core/Systems/Messaging/Events/LevelUp.h"

void WeaponLevelUpEventsSystem::Init()
{
	MessagingSystem::RegisterRequestFunc<WeaponLevelUpEvents>(
		[&](const int& id)->std::any {
			return std::any(components[id]);
		});

	MessagingSystem::RegisterEventFunc("BUTTON_CLICK", [&](const Event* e) {
			return ButtonPressHandler(dynamic_cast<const ButtonPress*>(e));
		});
}

void WeaponLevelUpEventsSystem::Update(float dt)
{
	for (auto& comp : components)
	{
		switch (comp.second->type)
		{
		case WeaponLevelUpEventsIdle:
			break;
		case WeaponLevelUpEventsinvalid:
			comp.second->type = WeaponLevelUpEventsStarting;
			break;
		case WeaponLevelUpEventsPicking:
			if (LevelUpChoosenWeapon(dt, comp.first, comp.second) || comp.second->WeaponUpgradeOptions.size() == 0)
			{
				//unpause time and mark all cards id for deletion
				comp.second->isLevelUp = false;
				MarkCards(dt, comp.first, comp.second);
				comp.second->type = WeaponlevelUpEventsPickedEffect;
			}
			break;
		case WeaponlevelUpEventsPickedEffect:
			//dealy before gameplay starts agian for particles
			PickedLevelUpEffects(comp.first, comp.second);
			break;
		case WeaponLevelUpEventsStarting:
			if (CheckAllWeaponLevelZero(dt, comp.first, comp.second))
			{
				MessagingSystem::Broadcast("LEVELUP", new LevelUp(std::string("LevelUpScreen")));
			}
			else
			{
				comp.second->type = WeaponLevelUpEventsIdle;
			}
			
			break;
		}
	}
}

void WeaponLevelUpEventsSystem::Exit()
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

void WeaponLevelUpEventsSystem::Deserialize()
{
	auto reader = Serializer::GetInstance();
	reader->ReadFile("Data/JSONS/Behaviors/B_WeaponLevelUpEvents.json");
	auto items = reader->GetData("Names");
	for (auto& item : items)
	{
		WeaponLevelUpEvents* data = new WeaponLevelUpEvents{};
		std::string name = static_cast<std::string>(item);

		data->levelUpToGameTimer = reader->GetData(name + ".LevelUpToGameTimer");
		data->levelUpToGameTimerBase = data->levelUpToGameTimer;
		data->maxWeaponAmount = reader->GetData(name + ".MaxWeaponAmount");

		//weapons names
		auto weaps = reader->GetData(name + ".Weapons");
		for (auto& weap : weaps)
		{
			data->Weapons.push_back(weap);
		}

		data->isLevelUp = false;
		 

		archetypes[name] = data;
	}
}

void WeaponLevelUpEventsSystem::Serialize()
{
	auto ser = Serializer::GetInstance();
	ser->ReadFile("Data/JSONS/Behaviors/B_WeaponLevelUpEvents.json");
	std::vector<std::string> names = {};
	for (const auto& pair : archetypes) {
		const auto data = pair.second;

		auto& name = pair.first;
		names.push_back(name);
	}
	ser->SetData("Names", names);
	ser->Transcribe("Data/JSONS/Behaviors/B_WeaponLevelUpEvents.json");
	ser->CleanData();
}

void WeaponLevelUpEventsSystem::CreateComponent(const int& id, const std::string& name)
{
	if (components.contains(id)) {
		std::string&& mssg = "Tried to create a WeaponLevelUpEvents from archetype " + name + ", and associated with ID ";
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
	components[id] = new WeaponLevelUpEvents{};

	//Create<Weapon>("WeaponLevelUpEvents", id);

	*components[id] = *archetypes[name];
}

void WeaponLevelUpEventsSystem::DestroyComponent(const int& id)
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

void WeaponLevelUpEventsSystem::ClearComponents()
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

void WeaponLevelUpEventsSystem::ActivateComponent(const int& id)
{
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = true;
	}
}

void WeaponLevelUpEventsSystem::DeactivateComponent(const int& id)
{
	if (!components.contains(id)) {
		std::string&& mssg = "Tried to deactivate a WeaponLevelUpEvents associated with ID " + id;
		Tracing::Trace(Tracing::WARNING, mssg.c_str());
	}
	if (Query<Weapon>(id)) {
		Request<Weapon>(id)->isActive = false;
	}
}

void WeaponLevelUpEventsSystem::LevelUpHandler(const LevelUp* e)
{
	if (e->type == "LevelUpScreen")
	{
		//SetUp before creating the Level Up Menu
		Time::TimePause(true);

		const int WeaponLevelUpEventsID = SpecRequest("NametoID", "WeaponLevelUpEvents").value();

		//pick the weapons to upgrade 
		auto& data = components[WeaponLevelUpEventsID];
		
		data->WeaponList = data->Weapons;
		data->choosenWeapon = -1;

		int timesTried = 0;
		
		//player weapon is at or above max dont give new weapons
		if (data->playerWeaponList.size() >= data->maxWeaponAmount)
		{
			//only do it if amount is higher than zero becuse RandomInt will crash with it being less than 0
			if (data->playerWeaponList.size() > 0)
			{
				//choose weapon list from Player Weapon list
				data->WeaponList = data->playerWeaponList;
				for (int i = 0; i < 3; ++i)
				{
					//choose 3 weapons to upgrade
					int canUpgradePlace = RandomInt(0, static_cast<int>(data->WeaponList.size() - 1));

					//if weapon is not max level add to card
					if (!CheckWeaponCurrentLevel(data->WeaponList.at(canUpgradePlace)))
					{
						//add to list of weapon on upgrdae cards
						data->WeaponUpgradeOptions.push_back(data->WeaponList.at(canUpgradePlace));

						//delete weapon that got added out of weaponList so no dups
						auto temp = data->WeaponList.at(data->WeaponList.size() - 1);
						data->WeaponList.at(data->WeaponList.size() - 1) = data->WeaponList.at(canUpgradePlace);
						data->WeaponList.at(canUpgradePlace) = temp;
						data->WeaponList.pop_back();

						timesTried = 0;
					}
					else
					{
						//Weapon chosen was max level so go back and pick new Weapon
						--i;
						++timesTried;
					}

					if (timesTried > 100)
					{
						break; // if Cant find weapon that fits break out
					}
				}
			}
		}
		else
		{
			//choose weapon list from all weapons
			data->WeaponList = data->Weapons;

			//only do it if amount is higher than zero becuse RandomInt will crash with it being less than 0
			if (data->WeaponList.size() > 0)
			{
				for (int i = 0; i < 3; ++i)
				{
					//choose 3 weapons to upgrade
					int canUpgradePlace = RandomInt(0, static_cast<int>(data->WeaponList.size() - 1));

					//if weapon is not max level add to card
					if (!CheckWeaponCurrentLevel(data->WeaponList.at(canUpgradePlace)))
					{
						//add to list of weapon on upgrdae cards
						data->WeaponUpgradeOptions.push_back(data->WeaponList.at(canUpgradePlace));

						//delete weapon that got added out of weaponList so no dups
						auto temp = data->WeaponList.at(data->WeaponList.size() - 1);
						data->WeaponList.at(data->WeaponList.size() - 1) = data->WeaponList.at(canUpgradePlace);
						data->WeaponList.at(canUpgradePlace) = temp;
						data->WeaponList.pop_back();

						timesTried = 0;
					}
					else
					{
						//Weapon chosen was max level so go back and pick new Weapon
						--i;
						++timesTried;
					}

					if (timesTried > 100)
					{
						break; // if Cant find weapon that fits break out
					}
				}
			}
		}

		data->isLevelUp = true;

		//pop up the ui to upgrade the weapons
		CreateUICards(data, WeaponLevelUpEventsID);

		//puase gameplay
		if (data->WeaponUpgradeOptions.size() != 0)
		{
			PauseGameplay();
		}
		
		data->type = WeaponLevelUpEventsPicking;
	}
}

void WeaponLevelUpEventsSystem::ButtonPressHandler(const ButtonPress* e)
{
	if (e->command.find("WeaponLevelUp_") == std::string::npos)
		return;
	else
	{
		const int WeaponLevelUpEventsID = SpecRequest("NametoID", "WeaponLevelUpEvents").value();

		//if game is not paused then can be clicked
		if (!(components[WeaponLevelUpEventsID]->isPaused))
		{
			//pick the weapons to upgrade 
			auto& data = components[WeaponLevelUpEventsID];
			if (e->command == "WeaponLevelUp_1")
			{
				data->choosenWeapon = 0;
			}
			else if (e->command == "WeaponLevelUp_2")
			{
				data->choosenWeapon = 1;
			}
			else if (e->command == "WeaponLevelUp_3")
			{
				data->choosenWeapon = 2;
			}
		}
	}
}

void WeaponLevelUpEventsSystem::PauseGameplay()
{
	Time::TimePause(true);
}

void WeaponLevelUpEventsSystem::CreateUICards(WeaponLevelUpEvents* data, int id)
{
	if (data->WeaponUpgradeOptions.size() == 0)
	{
		return;
	}

	int cardsMade = 0;

	//create level up title text 
	auto val = SpecRequest("EntityCreation", "LevelUpTitle");
	if (val.has_value())
	{
		//level up text
		const int LevelUpTitleID = val.value();
		components[id]->AllCardsIDs.push_back(LevelUpTitleID);

		//create the level up cards
		for (auto& i : data->WeaponUpgradeOptions)
		{
			auto wUCB = SpecRequest("EntityCreation", "ButtonWeaponUpgradeCardBackground");

			if (wUCB.has_value())
			{
				const int cardBackgroundID = wUCB.value();

				//card background
				Transform* cardBackgroundTransform = Request<Transform>(cardBackgroundID);
				glm::vec3 cardBackgroundPos = { 0.0f, 0.0f, 0.0f };
				glm::vec3 cardBackgroundScale = cardBackgroundTransform->GetScale();

				Sprite* cardBackgroundSprite = Request<Sprite>(cardBackgroundID);
				Button* cardBackgroundButton = Request<Button>(cardBackgroundID);
				cardBackgroundButton->type = bButtonIdle;
				cardBackgroundButton->isActive = true;

				//Card Icon
				int cardIconID = CreateCardIcon(i, id, data);//System::RequestSceneAddition<SceneSystem>("FireWallCardIcon", id);
				if (cardIconID == -1)
				{
					return;
				}
				Transform* cardIconTransform = Request<Transform>(cardIconID);
				glm::vec3 cardIconPos = { 0.0f, 0.0f, 0.0f };
				Sprite* cardIconSprite = Request<Sprite>(cardIconID);
				TextBox* cardIconTB = cardIconSprite->GetTextBox();
				
				//Card LevelUP Description
				int levelDescriptionID = CreateWeaponDescription(i, id, data);
				Transform* levelDescriptionTransform = Request<Transform>(levelDescriptionID);
				glm::vec3 levelDescriptionTextPos = { 0.0f, 0.0f, 0.0f };
				Sprite* levelDescriptionSprite = Request<Sprite>(levelDescriptionID);

				switch (cardsMade)
				{
				case 0: // left most card
					//create the card background 
					cardBackgroundPos = glm::vec3(-250, 0, 1);
					cardBackgroundTransform->SetPos(cardBackgroundPos);
					components[id]->AllCardsIDs.push_back(cardBackgroundID);

					//create the left icon
					cardIconPos = glm::vec3(-250, 100, 1);
					cardIconTransform->SetPos(cardIconPos);
					components[id]->AllCardsIDs.push_back(cardIconID);

					//icon text
					cardIconSprite->SetTextPos(glm::vec3(cardBackgroundPos.x - 55.0f, cardBackgroundPos.y, cardBackgroundPos.z));

					//Level Description
					// new text
					levelDescriptionSprite->SetText(data->LevelDescriptionText);
					//text Pos
					levelDescriptionTextPos = cardBackgroundPos;
					levelDescriptionTextPos.x -= 75.0f;
					levelDescriptionTextPos.y -= 40.0f;
					levelDescriptionSprite->SetTextPos(levelDescriptionTextPos);

					components[id]->AllCardsIDs.push_back(levelDescriptionID);

					cardBackgroundButton->command = "WeaponLevelUp_1";

					break;
				case 1: //middle card
					//create the card background 
					cardBackgroundPos = glm::vec3(0, 0, 1);
					cardBackgroundTransform->SetPos(cardBackgroundPos);
					components[id]->AllCardsIDs.push_back(cardBackgroundID);

					//create the middle icon
					//create the left icon
					cardIconPos = glm::vec3(0, 100, 1);
					cardIconTransform->SetPos(cardIconPos);
					components[id]->AllCardsIDs.push_back(cardIconID);

					//icon text
					cardIconSprite->SetTextPos(glm::vec3(cardBackgroundPos.x - 55.0f, cardBackgroundPos.y, cardBackgroundPos.z));

					//Level Description
					// new text
					levelDescriptionSprite->SetText(data->LevelDescriptionText);
					//text Pos
					levelDescriptionTextPos = cardBackgroundPos;
					levelDescriptionTextPos.x -= 75.0f;
					levelDescriptionTextPos.y -= 40.0f;
					levelDescriptionSprite->SetTextPos(levelDescriptionTextPos);

					components[id]->AllCardsIDs.push_back(levelDescriptionID);

					cardBackgroundButton->command = "WeaponLevelUp_2";

					break;
				case 2://right most card
					//create the card background 
					cardBackgroundPos = glm::vec3(250, 0, 1);
					cardBackgroundTransform->SetPos(cardBackgroundPos);
					components[id]->AllCardsIDs.push_back(cardBackgroundID);

					//create the right icon
					//create the left icon
					cardIconPos = glm::vec3(250, 100, 1);
					cardIconTransform->SetPos(cardIconPos);
					components[id]->AllCardsIDs.push_back(cardIconID);

					//icon text
					cardIconSprite->SetTextPos(glm::vec3(cardBackgroundPos.x - 55.0f, cardBackgroundPos.y, cardBackgroundPos.z));

					//Level Description
					// new text
					levelDescriptionSprite->SetText(data->LevelDescriptionText);
					//text Pos
					levelDescriptionTextPos = cardBackgroundPos;
					levelDescriptionTextPos.x -= 75.0f;
					levelDescriptionTextPos.y -= 40.0f;
					levelDescriptionSprite->SetTextPos(levelDescriptionTextPos);

					components[id]->AllCardsIDs.push_back(levelDescriptionID);

					cardBackgroundButton->command = "WeaponLevelUp_3";

					break;
				default:
					break;
				}

				++cardsMade;

				cardBackgroundPos.x -= 45.0f;
				cardBackgroundPos.y = -(cardBackgroundScale.y / 2) + 60.f;
			}
		}
	}	
}

int WeaponLevelUpEventsSystem::CreateCardIcon(std::string WeaponName, int id, WeaponLevelUpEvents* data)
{
	//creates the card icon and name below it
	std::string cardIcon = WeaponName + "CardIcon";

	auto wUCB = SpecRequest("EntityCreation", cardIcon);

	if (wUCB.has_value())
	{
		return wUCB.value();
	}

	return -1;
}

int WeaponLevelUpEventsSystem::CreateWeaponDescription(std::string WeaponName, int id, WeaponLevelUpEvents* data)
{
	//create the desciption of the weapon level up and returns the id
	const auto& val = SpecRequest("NametoID", WeaponName);
	if (val.has_value())
	{
		//grab weapon ID
		const int weaponID = val.value();
		auto weaponData = Request<Weapon>(weaponID);

		//grab FireWall levelUpgradeText
		data->LevelDescriptionText = weaponData->levelUpgradeText;

		//create the description text entity
		auto desText = SpecRequest("EntityCreation", "CardLevelUpDescription");
		if (desText.has_value())
		{
			//grab FireWall IDx
			return desText.value();
		}
	}	
	//return -1 if doesnt work
	return -1;
}

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------

bool WeaponLevelUpEventsSystem::CheckAllWeaponLevelZero(float dt, int id, WeaponLevelUpEvents* data)
{
	//if all weapons level is 0 reutrn true else return false
	for (auto& i : data->Weapons)
	{
		const auto& val = SpecRequest("NametoID", i);
		if (val.has_value())
		{
			//grab weapon ID
			const int weaponID = val.value();
			auto weaponData = Request<Weapon>(weaponID);

			if (weaponData->level != 0)
			{
				return false;
			}
		}
	}

	return true;

}

bool WeaponLevelUpEventsSystem::CheckWeaponCurrentLevel(std::string WeaponName)
{
	//if weapon level is max level or higher return true
	const auto& val = SpecRequest("NametoID", WeaponName);
	if (val.has_value())
	{
		//grab weapon ID
		const int weaponID = val.value();
		auto weaponData = Request<Weapon>(weaponID);

		if (weaponData->level >= weaponData->maxLevel)
		{
			return true; // weapon is max level 
		}
		else
		{
			return false; // weapon is not max level
		}
	}

	return false;
}

bool WeaponLevelUpEventsSystem::LevelUpChoosenWeapon(float dt, int id, WeaponLevelUpEvents* data)
{
	//autoplay
	const auto& val = SpecRequest("NametoID", "Player");
	if (val.has_value())
	{
		//grab ForceQuitProgram ID
		const int PlayerID = val.value();
		if (Request<PlayerController>(PlayerID)->state == bPlayerControllerRandomAutoplay)
		{
			data->choosenWeapon = RandomInt(0, 2);
		}
	}

	//make sure the upgeeade exists
	if (data->choosenWeapon > data->WeaponUpgradeOptions.size() - 1 || data->WeaponUpgradeOptions.size() == 0)
	{
		data->choosenWeapon = -1;
	}
	//weapon to upgrade choosen
	if (data->choosenWeapon != -1)
	{
		MessagingSystem::Broadcast("LEVELUP", new LevelUp(data->WeaponUpgradeOptions.at(data->choosenWeapon)));

		

		return true;
	}

	//if all weapon level is max 
	if (data->WeaponUpgradeOptions.size() == 0)
	{
		return true;
	}

	return false;
}

void WeaponLevelUpEventsSystem::MarkCards(float dt, int id, WeaponLevelUpEvents* data)
{
	//mark cards for deletion
	for (auto& i : data->AllCardsIDs)
	{
		SpecBroadcast("MarkEntity", i);
	}

	data->AllCardsIDs.clear();
	data->WeaponUpgradeOptions.clear();
}

void WeaponLevelUpEventsSystem::PickedLevelUpEffects(int id, WeaponLevelUpEvents* data)
{
	float dt = Time::GetUnscaledDt();

	if (data->levelUpToGameTimer <= 0.0f)
	{
		//unpause time and resume gameplay
		Time::TimePause(false);

		//reset the type and timer for next level up
		data->levelUpToGameTimer = data->levelUpToGameTimerBase;
		data->type = WeaponLevelUpEventsIdle;
	}
	else
	{
		data->levelUpToGameTimer -= dt;
		//do effect here particles here
	}
}


void WeaponLevelUpEventsSystem::Register()
{
	BehaviorSystem::RegisterSubsystem("WeaponLevelUpEvents", new WeaponLevelUpEventsSystem());
}
