#include "mainScene.h"
#include "sceneManager.h"
#include "menuScene.h"

#include "..\context.h"
#include "..\drawing.h"
#include "..\component.h"
#include "..\ssfn.h"
#include "..\inputManager.h"
#include "..\stringUtility.h"
#include "..\socketUtility.h"
#include "..\ftpServer.h"
#include "..\httpServer.h"
#include "..\temperatureManager.h"
#include "..\xboxConfig.h"
#include "..\theme.h"
#include "..\network.h"

mainScene::mainScene()
{
	mSelectedControl = 0;
}

void mainScene::update()
{
	// Select Actions

	if (inputManager::buttonPressed(ButtonA))
	{
		if (mSelectedControl == 0) 
		{
			sceneManager::pushScene(sceneItemLaunchScene);
			return;
		}
		else if (mSelectedControl == 1) 
		{
			sceneManager::pushScene(sceneItemSystemScene);
			return;
		}
		else if (mSelectedControl == 2) 
		{
			utils::reboot();
			return;
		}
		else if (mSelectedControl == 3) 
		{
			utils::shutdown();
			return;
		}
	}

	// Down Actions

	if (inputManager::buttonPressed(ButtonDpadDown))
	{
		mSelectedControl = mSelectedControl < 3 ? mSelectedControl + 1 : 0;
	}

	// Up Actions

	if (inputManager::buttonPressed(ButtonDpadUp))
	{
		mSelectedControl = mSelectedControl > 0 ? mSelectedControl - 1 : 3; 
	}

	network::init();
}

void mainScene::render()
{
	component::panel(theme::getPanelFillColor(), theme::getPanelStrokeColor(), 16, 16, 688, 448);

	//drawing::drawBitmapStringAligned(context::getBitmapFontLarge(), "PROMETHEOS LAUNCHER", theme::getPrometheosColor(), theme::getHeaderAlign(), 40, theme::getHeaderY(), 640);

	//drawing::drawBitmapStringAligned(context::getBitmapFontLarge(), "\xC2\xA7\xC2\xA8\xC2\xA9\xC2\xAA\xC2\xAB\xC2\xAC\xC2\xAD\xC2\xAB\xC2\xA9\xC2\xAE \xC2\xAC\xC2\xA9\xC2\xA9\xC2\xB4\xC2\xAE", theme::getPrometheosColor(), theme::getPrometheosAlign(), 40, theme::getPrometheosY(), 640);

	int32_t yPos = (context::getBufferHeight() - (4 * 40) - 10) / 2;
	yPos += theme::getCenterOffset();

	component::button(mSelectedControl == 0, false, "Launch Application", 193, yPos, 322, 30);

	yPos += 40;

	component::button(mSelectedControl == 1, false, "System", 193, yPos, 322, 30);

	yPos += 40;

	component::button(mSelectedControl == 2, false, "Reboot", 193, yPos, 322, 30);

	yPos += 40;

	component::button(mSelectedControl == 3, false, "Shutdown", 193, yPos, 322, 30);

	char* currentIp = context::getCurrentIp();
	char* ip = stringUtility::formatString("IP %s", currentIp);
	drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), ip, theme::getFooterTextColor(), horizAlignmentCenter, 60 + (150 * 0), theme::getFooterY(), 150);
	free(ip);
	free(currentIp);

	char* cpuTemp = stringUtility::formatString("CPU Temp %i\xC2\xB1" "C", context::getCurrentCpuTemp());
	drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), cpuTemp, theme::getFooterTextColor(), horizAlignmentCenter, 60 + (150 * 1), theme::getFooterY(), 150);
	free(cpuTemp);

	char* mbTemp = stringUtility::formatString("M/B Temp %i\xC2\xB1" "C", context::getCurrentMbTemp());
	drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), mbTemp, theme::getFooterTextColor(), horizAlignmentCenter, 60 + (150 * 2), theme::getFooterY(), 150);
	free(mbTemp);

	char* memoryFormat = stringUtility::formatSize(context::getCurrentFreeMem());
	char* memory = stringUtility::formatString("Free Mem %s", memoryFormat);
	drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), memory, theme::getFooterTextColor(), horizAlignmentCenter, 60 + (150 * 3), theme::getFooterY(), 150);
	free(memory);
	free(memoryFormat); 
}
