#include "stdafx.h"
#include <xtl.h>
#include <string>

#include "temperatureManager.h"
#include "context.h"
#include "drawing.h"
#include "Scenes\scene.h"
#include "Scenes\mainScene.h"
#include "Scenes\sceneManager.h"
#include "inputManager.h"
#include "xboxConfig.h"
#include "xboxinternals.h"
#include "meshUtility.h"
#include "utils.h"
#include "resources.h"
#include "stringUtility.h"
#include "json.h"
#include "theme.h"
#include "ssfn.h"
#include "ftpServer.h"
#include "httpServer.h"
#include "driveManager.h"
#include "XKUtils\XKEEPROM.h"
#include "timeUtility.h"
#include "network.h"

#include "stb_image_write.h"

#include <xgraphics.h>

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

enum apiActionEnum
{ 
	apiActionNone = 0,
	apiActionReboot = 1,
	apiActionShutdown = 2
}; 

namespace
{
	apiActionEnum mApiAction = apiActionNone;
	int mBankId = 0;
	int mCounter = 0;
}

utils::dataContainer* onGetCallback(const char* path, const char* query)
{
	utils::dataContainer* body = NULL;

	if (stringUtility::equals(path, "\\", true))
	{
		return httpServer::redirect();
	}
	else if (stringUtility::equals(path, "\\pico.css", true))
	{
		body = new utils::dataContainer((char*)&pico_css, sizeof(pico_css), sizeof(pico_css));
	}
	else if (stringUtility::equals(path, "\\favicon.ico", true))
	{
		body = new utils::dataContainer((char*)&favicon_ico, sizeof(favicon_ico), sizeof(favicon_ico));
	}
	else if (stringUtility::equals(path, "\\remoteview.html", true))
	{
		body = new utils::dataContainer((char*)&remoteview_html, sizeof(remoteview_html), sizeof(remoteview_html));
	}
	else if (stringUtility::equals(path, "\\remoteview.js", true))
	{
		body = new utils::dataContainer((char*)&remoteview_js, sizeof(remoteview_js), sizeof(remoteview_js));
	}
	else if (stringUtility::equals(path, "\\downloads.html", true))
	{
		body = new utils::dataContainer((char*)&downloads_tools_html, sizeof(downloads_tools_html), sizeof(downloads_tools_html));
	}
	else if (stringUtility::equals(path, "\\downloads.js", true))
	{
		body = new utils::dataContainer((char*)&downloads_tools_js, sizeof(downloads_tools_js), sizeof(downloads_tools_js));
	}
	else if (stringUtility::equals(path, "\\index.html", true))
	{
		body = new utils::dataContainer((char*)&index_tools_html, sizeof(index_tools_html), sizeof(index_tools_html));
	}
	else if (stringUtility::equals(path, "\\index.js", true))
	{
		body = new utils::dataContainer((char*)&index_tools_js, sizeof(index_tools_js), sizeof(index_tools_js));
	}
	else if (stringUtility::equals(path, "\\main.css", true))
	{
		body = new utils::dataContainer((char*)&main_css, sizeof(main_css), sizeof(main_css));
	}	
	else if (stringUtility::equals(path, "\\api\\shutdown", true))
	{
		mApiAction = apiActionShutdown;
		return httpServer::generateResponse(200, "OK");
	}
	else if (stringUtility::equals(path, "\\api\\reboot", true))
	{
		mApiAction = apiActionReboot;
		return httpServer::generateResponse(200, "OK");
	}
	else if (stringUtility::equals(path, "\\api\\downloadeeprom", true))
	{
		XKEEPROM* eeprom = new XKEEPROM();
		eeprom->ReadFromXBOX();
		body = new utils::dataContainer(EEPROM_SIZE);
		eeprom->GetEEPROMData((LPEEPROMDATA)body->data);
		delete(eeprom);
	}
	else if (stringUtility::equals(path, "\\api\\screenshot", true))
	{
		context::setScreenshot(NULL);
		context::setTakeScreenshot(true);
		while (context::getScreenshot() == NULL)
		{
			Sleep(100);
		}
		body = context::getScreenshot();
	}

	if (body == NULL)
	{
		return httpServer::generateResponse(404, "Not Found");
	}

	char* extension = fileSystem::getExtension(path);
	char* mimeType = httpServer::getMimeType(extension);
	char* contentLength = stringUtility::formatString("%lu", body->size);
	char* header = stringUtility::formatString("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nCache-Control: no-cache\r\nContent-Type: %s\r\nX-Content-Type-Options: nosniff\r\nContent-Length: %s\r\n\r\n", mimeType, contentLength);

	utils::debugPrint("Header len = %i\n", strlen(header));
	utils::debugPrint("Response = %s\n", header);

	utils::dataContainer* result = new utils::dataContainer(strlen(header) + body->size);
	memcpy(result->data, header, strlen(header));
	memcpy(result->data + strlen(header), body->data, body->size);

	delete(body);
	free(extension);
	free(mimeType);
	free(contentLength);
	free(header);

	return result;
}

utils::dataContainer* onPostCallback(const char* path, const char* query, pointerVector<FormPart*>* formParts)
{
	utils::debugPrint("Post reciecved\n");

	return httpServer::generateResponse(404, "Not Found");
}

void onResponseSentCallback()
{
	if (mApiAction == apiActionShutdown)
	{
		utils::shutdown();
		Sleep(5000);
	}
	else if (mApiAction == apiActionReboot)
	{
		utils::reboot();
		Sleep(5000);
	}
}

typedef struct {
    DWORD dwWidth;
    DWORD dwHeight;
    BOOL  fProgressive;
    BOOL  fWideScreen;
	DWORD dwFreq;
} DISPLAY_MODE;

DISPLAY_MODE displayModes[] =
{
    //{   720,    480,    TRUE,   TRUE,  60 },         // 720x480 progressive 16x9
    //{   720,    480,    TRUE,   FALSE, 60 },         // 720x480 progressive 4x3
    //{   720,    480,    FALSE,  TRUE,  50 },         // 720x480 interlaced 16x9 50Hz
    //{   720,    480,    FALSE,  FALSE, 50 },         // 720x480 interlaced 4x3  50Hz
    //{   720,    480,    FALSE,  TRUE,  60 },         // 720x480 interlaced 16x9
    //{   720,    480,    FALSE,  FALSE, 60 },         // 720x480 interlaced 4x3


	// Width  Height Progressive Widescreen

	// HDTV Progressive Modes
    {  1280,    720,    TRUE,   TRUE,  60 },         // 1280x720 progressive 16x9

	// EDTV Progressive Modes
    {   720,    480,    TRUE,   TRUE,  60 },         // 720x480 progressive 16x9
    {   640,    480,    TRUE,   TRUE,  60 },         // 640x480 progressive 16x9
    {   720,    480,    TRUE,   FALSE, 60 },         // 720x480 progressive 4x3
    {   640,    480,    TRUE,   FALSE, 60 },         // 640x480 progressive 4x3

	// HDTV Interlaced Modes
	//    {  1920,   1080,    FALSE,  TRUE,  60 },         // 1920x1080 interlaced 16x9

	// SDTV PAL-50 Interlaced Modes
    {   720,    480,    FALSE,  TRUE,  50 },         // 720x480 interlaced 16x9 50Hz
    {   640,    480,    FALSE,  TRUE,  50 },         // 640x480 interlaced 16x9 50Hz
    {   720,    480,    FALSE,  FALSE, 50 },         // 720x480 interlaced 4x3  50Hz
    {   640,    480,    FALSE,  FALSE, 50 },         // 640x480 interlaced 4x3  50Hz

	// SDTV NTSC / PAL-60 Interlaced Modes
    {   720,    480,    FALSE,  TRUE,  60 },         // 720x480 interlaced 16x9
    {   640,    480,    FALSE,  TRUE,  60 },         // 640x480 interlaced 16x9
    {   720,    480,    FALSE,  FALSE, 60 },         // 720x480 interlaced 4x3
    {   640,    480,    FALSE,  FALSE, 60 },         // 640x480 interlaced 4x3
};

#define NUM_MODES (sizeof(displayModes) / sizeof(displayModes[0]))

bool supportsMode(DISPLAY_MODE mode, DWORD dwVideoStandard, DWORD dwVideoFlags)
{
    if (mode.dwFreq == 60 && !(dwVideoFlags & XC_VIDEO_FLAGS_PAL_60Hz) && (dwVideoStandard == XC_VIDEO_STANDARD_PAL_I))
	{
		return false;
	}    
    if (mode.dwFreq == 50 && (dwVideoStandard != XC_VIDEO_STANDARD_PAL_I))
	{
		return false;
	}
    if (mode.dwHeight == 480 && mode.fWideScreen && !(dwVideoFlags & XC_VIDEO_FLAGS_WIDESCREEN ))
	{
		return false;
	}
    if (mode.dwHeight == 480 && mode.fProgressive && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_480p))
	{
		return false;
	}
    if (mode.dwHeight == 720 && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_720p))
	{
		return false;
	}
    if (mode.dwHeight == 1080 && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_1080i))
	{
		return false;
	}
    return true;
}

bool createDevice()
{
	uint32_t videoFlags = XGetVideoFlags();
	uint32_t videoStandard = XGetVideoStandard();
	uint32_t currentMode;
    for (currentMode = 0; currentMode < NUM_MODES-1; currentMode++)
    {
		if (supportsMode(displayModes[currentMode], videoStandard, videoFlags)) 
		{
			break;
		}
    } 

	LPDIRECT3D8 d3d = Direct3DCreate8(D3D_SDK_VERSION);
    if(d3d == NULL)
	{
		utils::debugPrint("Failed to create d3d\n");
        return false;
	}

	context::setBufferWidth(720);
	context::setBufferHeight(480);
	//context::setBufferWidth(displayModes[currentMode].dwWidth);
	//context::setBufferHeight(displayModes[currentMode].dwHeight);

	D3DPRESENT_PARAMETERS params; 
    ZeroMemory(&params, sizeof(params));
	params.BackBufferWidth = displayModes[currentMode].dwWidth;
    params.BackBufferHeight = displayModes[currentMode].dwHeight;
	params.Flags = displayModes[currentMode].fProgressive ? D3DPRESENTFLAG_PROGRESSIVE : D3DPRESENTFLAG_INTERLACED;
    params.Flags |= displayModes[currentMode].fWideScreen ? D3DPRESENTFLAG_WIDESCREEN : 0;
    params.FullScreen_RefreshRateInHz = displayModes[currentMode].dwFreq;
	params.BackBufferFormat = D3DFMT_X8R8G8B8;
    params.BackBufferCount = 1;
    params.EnableAutoDepthStencil = FALSE;
	params.SwapEffect = D3DSWAPEFFECT_COPY;
    params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	LPDIRECT3DDEVICE8 d3dDevice;
    if (FAILED(d3d->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &d3dDevice)))
	{
		utils::debugPrint("Failed to create device\n");
        return false;
	}
	context::setD3dDevice(d3dDevice);

	D3DXMATRIX matProjection;
	D3DXMatrixOrthoOffCenterLH(&matProjection, 0, (float)context::getBufferWidth(), 0, (float)context::getBufferHeight(), 1.0f, 100.0f);
	context::getD3dDevice()->SetTransform(D3DTS_PROJECTION, &matProjection);

	D3DXMATRIX  matView;
    D3DXMatrixIdentity(&matView);
    context::getD3dDevice()->SetTransform( D3DTS_VIEW, &matView);

	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	context::getD3dDevice()->SetTransform( D3DTS_WORLD, &matWorld);

	context::getD3dDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	context::getD3dDevice()->SetVertexShader(D3DFVF_CUSTOMVERTEX);
	context::getD3dDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	context::getD3dDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	context::getD3dDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

	context::getD3dDevice()->BeginScene();
	context::getD3dDevice()->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0xff000000, 1.0f, 0L);
	context::getD3dDevice()->EndScene();
	context::getD3dDevice()->Present(NULL, NULL, NULL, NULL);

	return true;
}

void refreshInfo()
{
	if (network::isReady() == true)
	{
		XNADDR addr;
		memset(&addr, 0, sizeof(addr));
		DWORD dwState = XNetGetTitleXnAddr(&addr);
		if (dwState != XNET_GET_XNADDR_PENDING)
		{
			char* ipAddress = (XNetGetEthernetLinkStatus() & XNET_ETHERNET_LINK_ACTIVE) ? stringUtility::formatString("%i.%i.%i.%i", 
				addr.ina.S_un.S_un_b.s_b1,
				addr.ina.S_un.S_un_b.s_b2,
				addr.ina.S_un.S_un_b.s_b3,
				addr.ina.S_un.S_un_b.s_b4) : strdup("0.0.0.0");
			char* currentIp = context::getCurrentIp();
			if (stringUtility::equals(ipAddress, currentIp, false) == false)
			{
				context::setCurrentIp(ipAddress);
				mCounter = 0;
			}
			free(ipAddress);
			free(currentIp);
		}
	}

	if (mCounter == 0)
	{
		context::setCurrentFreeMem(utils::getFreePhysicalMemory());
		context::setCurrentFanSpeed(temperatureManager::getFanSpeed() * 2);
		context::setCurrentCpuTemp(temperatureManager::getCpuTemp());
		context::setCurrentMbTemp(temperatureManager::getMbTemp());
		mCounter = 60;
	}
	else
	{
		mCounter--;
	}
}
#define WIFI_SSID_LENGTH 32 
#define WIFI_PASSWORD_LENGTH 63 

#define I2C_SLAVE_ADDRESS 0x60
#define I2C_DATA_LENGTH 256

#define I2C_READ_FLAG 0x00
#define I2C_WRITE_FLAG 0x80

#define I2C_COMMAND_EFFECT 0x00
#define I2C_COMMAND_EFFECT_PARAM (I2C_COMMAND_EFFECT + 1)
#define I2C_COMMAND_EFFECT_COLOR1_R (I2C_COMMAND_EFFECT_PARAM + 1)
#define I2C_COMMAND_EFFECT_COLOR1_G (I2C_COMMAND_EFFECT_COLOR1_R + 1)
#define I2C_COMMAND_EFFECT_COLOR1_B (I2C_COMMAND_EFFECT_COLOR1_G + 1)
#define I2C_COMMAND_EFFECT_COLOR2_R (I2C_COMMAND_EFFECT_COLOR1_B + 1)
#define I2C_COMMAND_EFFECT_COLOR2_G (I2C_COMMAND_EFFECT_COLOR2_R + 1)
#define I2C_COMMAND_EFFECT_COLOR2_B (I2C_COMMAND_EFFECT_COLOR2_G + 1)
#define I2C_COMMAND_COLOR1_R (I2C_COMMAND_EFFECT_COLOR2_B + 1)
#define I2C_COMMAND_COLOR1_G (I2C_COMMAND_COLOR1_R + 1)
#define I2C_COMMAND_COLOR1_B (I2C_COMMAND_COLOR1_G + 1)
#define I2C_COMMAND_COLOR2_R (I2C_COMMAND_COLOR1_B + 1)
#define I2C_COMMAND_COLOR2_G (I2C_COMMAND_COLOR2_R + 1)
#define I2C_COMMAND_COLOR2_B (I2C_COMMAND_COLOR2_G + 1)
#define I2C_COMMAND_COLOR3_R (I2C_COMMAND_COLOR2_B + 1)
#define I2C_COMMAND_COLOR3_G (I2C_COMMAND_COLOR3_R + 1)
#define I2C_COMMAND_COLOR3_B (I2C_COMMAND_COLOR3_G + 1)
#define I2C_COMMAND_COLOR4_R (I2C_COMMAND_COLOR3_B + 1)
#define I2C_COMMAND_COLOR4_G (I2C_COMMAND_COLOR4_R + 1)
#define I2C_COMMAND_COLOR4_B (I2C_COMMAND_COLOR4_G + 1)
#define I2C_COMMAND_WIFI_SSID_START (I2C_COMMAND_COLOR4_B + 1)
#define I2C_COMMAND_WIFI_SSID_END (I2C_COMMAND_WIFI_SSID_START + WIFI_SSID_LENGTH - 1)
#define I2C_COMMAND_WIFI_PASSWORD_START (I2C_COMMAND_WIFI_SSID_END + 1)
#define I2C_COMMAND_WIFI_PASSWORD_END (I2C_COMMAND_WIFI_PASSWORD_START + WIFI_PASSWORD_LENGTH - 1)

#define I2C_COMMAND_APPLY 0x7E
#define I2C_COMMAND_SET_COMMAND 0x7F

#define WIFI_SSID "EqUiNoX"
#define WIFI_PASSWORD "R1chm0nd"

void writeScratchSetting(uint8_t command, uint8_t value)
{
	HalWriteSMBusByte(I2C_SLAVE_ADDRESS << 1, command | I2C_WRITE_FLAG, value);
	Sleep(1);
}

void applySettings()
{
	HalWriteSMBusByte(I2C_SLAVE_ADDRESS << 1, I2C_COMMAND_APPLY | I2C_WRITE_FLAG, 0x00);
	Sleep(1);
}

void writeColor(uint8_t command, uint8_t r, uint8_t g, uint8_t b)
{
	writeScratchSetting(command, r);
	writeScratchSetting(command + 1, g);
	writeScratchSetting(command + 2, b);

	applySettings();
	Sleep(100);
}

void writeWifiDetails()
{
	uint8_t ssid_len = strlen(WIFI_SSID);
	for (uint8_t i = 0; i < ssid_len; i++)
	{
		writeScratchSetting(I2C_COMMAND_WIFI_SSID_START + i, WIFI_SSID[i]);
	}

	uint8_t password_len = strlen(WIFI_PASSWORD);
	for (uint8_t i = 0; i < password_len; i++)
	{
		writeScratchSetting(I2C_COMMAND_WIFI_PASSWORD_START + i, WIFI_PASSWORD[i]);
	}

	applySettings();
}

void readSetting(uint8_t command, uint8_t* value)
{
	HalWriteSMBusByte(I2C_SLAVE_ADDRESS << 1, I2C_COMMAND_SET_COMMAND | I2C_WRITE_FLAG, command);
	Sleep(1);

	uint32_t temp = 0;
	HalReadSMBusByte(I2C_SLAVE_ADDRESS << 1, I2C_READ_FLAG, &temp);
	Sleep(1);

	*value = (uint8_t)temp;
}

void __cdecl main()
{
	utils::debugPrint("Welcome to PrometheOS Launcher\n");

	utils::setLedStates(SMC_LED_STATES_GREEN_STATE0 | SMC_LED_STATES_GREEN_STATE1 | SMC_LED_STATES_GREEN_STATE2 | SMC_LED_STATES_GREEN_STATE3);

	bool deviceCreated = createDevice();

	temperatureManager::init();

	context::setNetworkInitialized(false);

	driveManager::mountDrive("HDD0-C");
	driveManager::mountDrive("HDD0-E");
	
	context::setImageMap(new pointerMap<image*>(true));
	xboxConfig::init();
	theme::loadSkin();

	httpServer::registerOnGetCallback(onGetCallback);
	httpServer::registerOnPostCallback(onPostCallback);
	httpServer::registerOnResponseSentCallback(onResponseSentCallback);

	if (deviceCreated == false)
	{
		network::init();
		while (true)
		{
			temperatureManager::refresh();
			Sleep(100);
		}
	}


	drawing::loadFont(&font_sfn[0]);

	//\xC2\xA1 = A
	//\xC2\xA2 = B
	//\xC2\xA3 = X
	//\xC2\xA4 = Y
	//\xC2\xA5 = Folder
	//\xC2\xA6 = File
	//\xC2\xA7 = P
	//\xC2\xA8 = R
	//\xC2\xA9 = O
	//\xC2\xAA = M
	//\xC2\xAB = E
	//\xC2\xAC = T
	//\xC2\xAD = H
	//\xC2\xAE = S
	//\xC2\xAF = Cursor On
	//\xC2\xB0 = Cursor Off
	//\xC2\xB1 = Deg
	//\xC2\xB2 = LT
	//\xC2\xB3 = RT
	//\xC2\xB4 = L
	//\xC2\xB5 = White
	//\xC2\xB6 = Black

	bitmapFont* fontSmall = drawing::generateBitmapFont("FreeSans", SSFN_STYLE_REGULAR, 18, 18, 0, 256);
	context::setBitmapFontSmall(fontSmall);
	bitmapFont* fontMedium = drawing::generateBitmapFont("FreeSans", SSFN_STYLE_REGULAR, 25, 25, 0, 256);
	context::setBitmapFontMedium(fontMedium);
	bitmapFont* fontLarge = drawing::generateBitmapFont("FreeSans", SSFN_STYLE_REGULAR, 32, 32, 0, 512);
	context::setBitmapFontLarge(fontLarge);

	drawing::renderRoundedRect("panel-fill", 24, 24, 6, 0xffffffff, 0x00000000, 0);
	drawing::renderRoundedRect("panel-stroke", 24, 24, 6, 0x01010100, 0xffffffff, 2);

	sceneManager::pushScene(sceneItemMainScene);

    while (TRUE)
    {
		context::getD3dDevice()->BeginScene();

		temperatureManager::refresh();
		inputManager::processController();
		drawing::clearBackground((uint32_t)0);

		if (context::getTakeScreenshot() == true)
		{
			context::setScreenshot(drawing::takeScreenshot());
			context::setTakeScreenshot(false);
		}

		refreshInfo();

		sceneManager::getScene()->update();
		sceneManager::getScene()->render();

		context::getD3dDevice()->EndScene();
		context::getD3dDevice()->Present(NULL, NULL, NULL, NULL);
    }
}