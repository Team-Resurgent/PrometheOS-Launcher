#include "launchScene.h"
#include "sceneManager.h"
#include "menuScene.h"

#include "..\context.h"
#include "..\drawing.h"
#include "..\component.h"
#include "..\ssfn.h"
#include "..\inputManager.h"
#include "..\stringUtility.h"
#include "..\socketUtility.h"
#include "..\driveManager.h"
#include "..\ftpServer.h"
#include "..\httpServer.h"
#include "..\temperatureManager.h"
#include "..\xboxConfig.h"
#include "..\theme.h"
#include "..\network.h"

launchScene::launchScene()
{
	mSelectedControl = 0;
}

void launchScene::update()
{
	// Exit Action

	if (inputManager::buttonPressed(ButtonB))
	{
		sceneManager::popScene();
		return;
	}

	// Select Actions

	if (inputManager::buttonPressed(ButtonA))
	{
		if (mSelectedControl == 0) 
		{
			xboxConfig::launchXbe("HDD0-E:\\UDATA\\58434154\\XCAT.xbe");
			return;
		}
		else if (mSelectedControl == 1) 
		{
			xboxConfig::launchXbe("HDD0-E:\\UDATA\\58434154\\Insignia.xbe");
			return;
		}
		else if (mSelectedControl == 2) 
		{
			xboxConfig::launchXbe("HDD0-E:\\UDATA\\58434154\\Gamepad.xbe");
			return;
		}
		else if (mSelectedControl == 3) 
		{
			xboxConfig::launchXbe("HDD0-E:\\UDATA\\58434154\\MemCheck.xbe");
			return;
		}
		else if (mSelectedControl == 4) 
		{
			xboxConfig::launchXbe("HDD0-E:\\UDATA\\58434154\\R5Softmod.xbe");
			return;
		}	
	}

	// Down Actions

	if (inputManager::buttonPressed(ButtonDpadDown))
	{
		mSelectedControl = mSelectedControl < 4 ? mSelectedControl + 1 : 0;
	}

	// Up Actions

	if (inputManager::buttonPressed(ButtonDpadUp))
	{
		mSelectedControl = mSelectedControl > 0 ? mSelectedControl - 1 : 4; 
	}

	network::init();
}

void launchScene::render()
{
	component::panel(theme::getPanelFillColor(), theme::getPanelStrokeColor(), 16, 16, 688, 448);
	for (int i = 0; i < 3; i++)
	{
		drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), "Launch Application", i == 2 ? theme::getHeaderTextColor() : 0xff000000, theme::getHeaderAlign(), 39 + (i == 0 ? 0 : i ^ 3), theme::getHeaderY(), 640);
	}

	int32_t yPos = (context::getBufferHeight() - (5 * 40) - 10) / 2;
	yPos += theme::getCenterOffset();

	component::button(mSelectedControl == 0, false, "Xbox Content Archival Tool", 193, yPos, 322, 30);

	yPos += 40;

	component::button(mSelectedControl == 1, false, "Insignia Setup Assistant", 193, yPos, 322, 30);

	yPos += 40;

	component::button(mSelectedControl == 2, false, "Gamepad Tester", 193, yPos, 322, 30);

	yPos += 40;

	component::button(mSelectedControl == 3, false, "MemCheck Memory Tester", 193, yPos, 322, 30);

	yPos += 40;

	component::button(mSelectedControl == 4, false, "Rocky5's Softmod v1.2.2", 193, yPos, 322, 30);

	drawing::drawBitmapStringAligned(context::getBitmapFontSmall(), "\xC2\xA2 Back", theme::getFooterTextColor(), horizAlignmentRight, 40, theme::getFooterY(), 640);
}
