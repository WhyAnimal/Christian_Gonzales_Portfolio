#pragma once
//------------------------------------------------------------------------------
// File Name:	B_WeaponLevelUpEvents.h
// Author(s):	Christian Gonzalse (christian.gonzales) Juno Insixiengmay
// Project:		Arm Support
// Course:		GAM 200
//------------------------------------------------------------------------------

#include "Behaviors/BehaviorSystem.h"
#include "GenericBehaviors/B_Weapon.h"
#include "Behaviors/Behavior.h"

enum WeaponLevelUpEventsState {
	WeaponLevelUpEventsinvalid = -1
	, WeaponLevelUpEventsIdle
	, WeaponLevelUpEventsPicking
	, WeaponlevelUpEventsPickedEffect
	, WeaponLevelUpEventsStarting
};

//data inside the JSON FILE
struct WeaponLevelUpEvents {
	WeaponLevelUpEventsState type = WeaponLevelUpEventsinvalid;
	bool isActive;

	bool isLevelUp;
	bool isPaused = false;
	float levelUpToGameTimer;
	float levelUpToGameTimerBase;
	int maxWeaponAmount;

	std::vector<std::string> Weapons; // weapons list from json
	std::vector<std::string> WeaponList; // weapon list to mess with
	std::vector<std::string> WeaponUpgradeOptions; // current upgrades
	std::vector<std::string> playerWeaponList; // List of all weapon the player currently have
	std::vector<int> AllCardsIDs;
	int choosenWeapon;

	std::string LevelDescriptionText;
};

class WeaponLevelUpEventsSystem : public BehaviorSystem::Subsystem {
public:
	WeaponLevelUpEventsSystem() = default;

	WeaponLevelUpEventsSystem(const WeaponLevelUpEventsSystem* other, const int id) = delete;

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
	void ButtonPressHandler(const ButtonPress*);

private:

	// Check if all weapon is zero when games start if so bring up choose level pop up 
	// Params:
	//	 dt = Change in time (in seconds) since the last game loop.
	//	 id = id of the entity.
	//	 data = Pointer to the behavior component data.
	bool CheckAllWeaponLevelZero(float dt, int id, WeaponLevelUpEvents* data);

	// Checks the level of the current weapon choosen
	// Params:
	//	 WeaponName = String that is the weapon Name
	bool CheckWeaponCurrentLevel(std::string WeaponName);

	// Pause the Gameplay when the Level Up Menu is on.
	void PauseGameplay();

	// Create the Ui cards for the level up Menu
	// Params:
	//	 id = id of the entity.
	//	 data = Pointer to the behavior component data.
	void CreateUICards(WeaponLevelUpEvents* data, int id);

	// Create the icon for the cards for the level up Menu
	// Params:
	//	 WeaponName = name of the Weapon.
	//	 id = id of the entity.
	//	 data = Pointer to the behavior component data.
	int CreateCardIcon(std::string WeaponName, int id, WeaponLevelUpEvents* data);

	// Create the Weapon level up Description for the cards for the level up Menu
	// Params:
	//	 WeaponName = name of the Weapon.
	//	 id = id of the entity.
	//	 data = Pointer to the behavior component data.
	int CreateWeaponDescription(std::string WeaponName, int id, WeaponLevelUpEvents* data);

	// Level up Chossen So send message to the weapons to level up
	// Params:
	//	 dt = Change in time (in seconds) since the last game loop.
	//	 id = id of the entity.
	//	 data = Pointer to the behavior component data.
	bool LevelUpChoosenWeapon(float dt, int id, WeaponLevelUpEvents* data);

	// Mark Each card for deletion
	// Params:
	//	 dt = Change in time (in seconds) since the last game loop.
	//	 id = id of the entity.
	//	 data = Pointer to the behavior component data.
	void MarkCards(float dt, int id, WeaponLevelUpEvents* data);
	
	// Effect after the player choose level up
	// Params:
	//	 id = id of the entity.
	//	 data = Pointer to the behavior component data.
	void PickedLevelUpEffects(int id, WeaponLevelUpEvents* data);

	std::unordered_map<int, WeaponLevelUpEvents*> components;
	std::unordered_map<std::string, WeaponLevelUpEvents*> archetypes;

	static void Register();

	REGISTER;
};