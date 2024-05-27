#ifndef _WIN32_MONITOR_INFO_H_
#define _WIN32_MONITOR_INFO_H_

#ifndef WIN32
#error "Only for Win32 platform"
#endif

#include "Win32Headers.h"
#include <wingdi.h>
#include <string>

enum eMonitorIndex
{
	PRIMARY_MONITOR_INDEX = 0,
	SECONDARY_MONITOR_INDEX = 1
};

// Monitor info struct
struct Win32MonitorInfo
{
public:
	// Returns Win32MonitorInfo for input index. If returned instance
	// has bAttached set to false, the monitor index is invalid.
	static Win32MonitorInfo GetMonitorInfo(int iMonitorIndex);

	bool bAttached;
	std::string name;
	int x, y;
	int width, height;

private:
	// Hide constructor
	Win32MonitorInfo() : bAttached(false), name("Invalid") { }
};



inline Win32MonitorInfo Win32MonitorInfo::GetMonitorInfo(int iMonitorIndex)
{
	// g++ doesn't define this in its wingdi.h
	#ifndef DISPLAY_DEVICE_ATTACHED_TO_DESKTOP
	#define DISPLAY_DEVICE_ATTACHED_TO_DESKTOP 0x00000001
	#endif

	// Grab display device data on the selected monitor
	DISPLAY_DEVICE dispDevice;
	dispDevice.cb = sizeof(dispDevice);
	if ( !EnumDisplayDevices(nullptr, iMonitorIndex, &dispDevice, 0) )
		return Win32MonitorInfo();

	// Grab the device mode data for the selected monitor
	DEVMODE devMode;
	devMode.dmSize = sizeof(devMode);	
	if ( !EnumDisplaySettings(dispDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode) )
		return Win32MonitorInfo();

	// Create and return instance
	Win32MonitorInfo monInfo;
	monInfo.bAttached = dispDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP;
	monInfo.name = dispDevice.DeviceName;
	monInfo.x = devMode.dmPosition.x;
	monInfo.y = devMode.dmPosition.y;
	monInfo.width = devMode.dmPelsWidth;
	monInfo.height = devMode.dmPelsHeight;

	return monInfo;
}

#endif // _WIN32_MONITOR_INFO_H_
