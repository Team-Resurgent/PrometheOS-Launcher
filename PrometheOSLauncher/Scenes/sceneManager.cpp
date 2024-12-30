#include "scenemanager.h"
#include "scene.h"
#include "mainScene.h"
#include "systemInfoScene.h"
#include "backupEepromScene.h"
#include "menuScene.h"
#include "launchScene.h"
#include "snakeScene.h"
#include "invadersScene.h"
#include "dlcSignerScene.h"
#include "videoSettingsScene.h"
#include "audioSettingsScene.h"
#include "regionSettingsScene.h"

#include "..\xboxConfig.h"
#include "..\context.h"
#include "..\stringUtility.h"
#include "..\pointerVector.h"

#include <xtl.h>

namespace 
{
	pointerVector<sceneContainer*>* mScenes = new pointerVector<sceneContainer*>(true);
}

scene* sceneManager::getScene()
{
	sceneContainer* result = mScenes->get(mScenes->count() - 1);
	return result->scene;
}

void sceneManager::pushScene(sceneItemEnum sceneItem)
{
	if (sceneItem == sceneItemMainScene)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new mainScene(), "PrometheOS");
		addScene(container);
	}
	if (sceneItem == sceneItemSystemScene)
	{
		pointerVector<utils::intContainer*>* sceneItems = new pointerVector<utils::intContainer*>(true);
		sceneItems->add(new utils::intContainer(sceneItemSystemInfoScene));
		sceneItems->add(new utils::intContainer(sceneItemVideoSettingsScene));
		sceneItems->add(new utils::intContainer(sceneItemAudioSettingsScene));
		sceneItems->add(new utils::intContainer(sceneItemRegionSettingsScene));
		sceneItems->add(new utils::intContainer(sceneItemUtilitiesScene));
		sceneItems->add(new utils::intContainer(sceneItemGamesScene));
		sceneContainer* container = new sceneContainer(sceneItem, new menuScene("Select System option...", "", sceneItems), "System Settings");
		addScene(container);
	}
	else if (sceneItem == sceneItemGamesScene)
	{
		pointerVector<utils::intContainer*>* sceneItems = new pointerVector<utils::intContainer*>(true);
		sceneItems->add(new utils::intContainer(sceneItemSnakeScene));
		sceneItems->add(new utils::intContainer(sceneItemInvadersScene));
		sceneContainer* container = new sceneContainer(sceneItem, new menuScene("Select Game option...", "", sceneItems), "Games");
		addScene(container);
	}
	else if (sceneItem == sceneItemUtilitiesScene)
	{
		pointerVector<utils::intContainer*>* sceneItems = new pointerVector<utils::intContainer*>(true);
		sceneItems->add(new utils::intContainer(sceneItemEepromToolsScene));
		sceneItems->add(new utils::intContainer(sceneItemDlcSignerScene));
		sceneContainer* container = new sceneContainer(sceneItem, new menuScene("Select Utility option...", "", sceneItems), "Utilities");
		addScene(container);
	}
	else if (sceneItem == sceneItemEepromToolsScene)
	{
		pointerVector<utils::intContainer*>* sceneItems = new pointerVector<utils::intContainer*>(true);
		sceneItems->add(new utils::intContainer(sceneItemBackupEepromScene));
		sceneContainer* container = new sceneContainer(sceneItem, new menuScene("Select EEPROM option...", "", sceneItems), "EEPROM Selection");
		addScene(container);
	}
	else if (sceneItem == sceneItemSystemInfoScene)
	{
		pointerVector<utils::intContainer*>* sceneItems = new pointerVector<utils::intContainer*>(true);
		sceneItems->add(new utils::intContainer(sceneItemSystemInfoSceneConsole));
		sceneItems->add(new utils::intContainer(sceneItemSystemInfoSceneStorage));
		sceneItems->add(new utils::intContainer(sceneItemSystemInfoSceneAudio));
		sceneItems->add(new utils::intContainer(sceneItemSystemInfoSceneVideo));
		sceneItems->add(new utils::intContainer(sceneItemSystemInfoSceneAbout));
		sceneContainer* container = new sceneContainer(sceneItem, new menuScene("Select System Info option...", "", sceneItems), "System Info");
		addScene(container);
	}
	else if (sceneItem == sceneItemLaunchScene)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new launchScene(), "Launch Application");
		addScene(container);
	}
	else if (sceneItem == sceneItemSystemInfoSceneConsole)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new systemInfoScene(systemInfoCategoryConsole), "Console Info");
		addScene(container);
	}
	else if (sceneItem == sceneItemSystemInfoSceneStorage)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new systemInfoScene(systemInfoCategoryStorage), "Storage Info");
		addScene(container);
	}
	else if (sceneItem == sceneItemSystemInfoSceneAudio)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new systemInfoScene(systemInfoCategoryAudio), "Audio Info");
		addScene(container);
	}
	else if (sceneItem == sceneItemSystemInfoSceneVideo)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new systemInfoScene(systemInfoCategoryVideo), "Video Info");
		addScene(container);
	}
	else if (sceneItem == sceneItemSystemInfoSceneAbout)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new systemInfoScene(systemInfoCategoryAbout), "About Info");
		addScene(container);
	}
	else if (sceneItem == sceneItemBackupEepromScene)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new backupEepromScene(), "Backup EEPROM");
		addScene(container);
	}
	else if (sceneItem == sceneItemSnakeScene)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new snakeScene(), "Snake");
		addScene(container);
	}
	else if (sceneItem == sceneItemInvadersScene)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new invadersScene(), "Invaders");
		addScene(container);
	}
	else if (sceneItem == sceneItemDlcSignerScene)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new dlcSignerScene(), "DLC / Update Signer");
		addScene(container);
	}
	else if (sceneItem == sceneItemVideoSettingsScene)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new videoSettingsScene(), "Video Settings");
		addScene(container);
	}
	else if (sceneItem == sceneItemAudioSettingsScene)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new audioSettingsScene(), "Audio Settings");
		addScene(container);
	}
	else if (sceneItem == sceneItemRegionSettingsScene)
	{
		sceneContainer* container = new sceneContainer(sceneItem, new regionSettingsScene(), "Region Settings");
		addScene(container);
	}
}

void sceneManager::pushScene(sceneContainer* container)
{
	addScene(container);
}

void sceneManager::popScene(sceneResult result)
{
	sceneContainer* popContainer = mScenes->get(mScenes->count() - 1);
	if (popContainer->onSceneClosingCallback != NULL)
	{
		popContainer->onSceneClosingCallback(result, popContainer->context, popContainer->scene);
	}

	mScenes->remove(mScenes->count() - 1);

	sceneContainer* container = mScenes->get(mScenes->count() - 1);
	context::setCurrentTitle(container->description);
}

// Private

void sceneManager::addScene(sceneContainer* sceneContainer)
{
	mScenes->add(sceneContainer);
	context::setCurrentTitle(sceneContainer->description);
}