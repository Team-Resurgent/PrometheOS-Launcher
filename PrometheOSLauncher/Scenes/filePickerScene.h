#pragma once

#include "scene.h"

#include "..\pointerVector.h"
#include "..\fileSystem.h"

enum filePickerType
{ 
	filePickerTypeEeprom = 0,
	filePickerTypeXbe = 1
}; 

class filePickerScene : public scene
{
public:
	filePickerScene(filePickerType filePickerType, bool useDevPath = false, bool hideMMU = false);
	~filePickerScene();
	void update();
	void render();
	char* getFilePath();
private:
	pointerVector<fileSystem::FileInfoDetail*>* getFileInfoDetails();
private:
	bool mInitialized;
	bool mUseDevPath;
	bool mHideMMU;
	filePickerType mFilePickerType;
	int mSelectedControl;
	int mScrollPosition;
	pointerVector<char*>* mMountedDrives;
	char* mCurrentPath;
	pointerVector<fileSystem::FileInfoDetail*>* mFileInfoDetails;
	char* mFilePath;
};
