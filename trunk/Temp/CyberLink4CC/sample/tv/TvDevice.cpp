/******************************************************************
*
*	CyberUPnP for Java
*
*	Copyright (C) Satoshi Konno 2002
*
*	File : TVDevice.java
*
******************************************************************/

#include <string.h>
#include <iostream>
#include <sstream>

#include "TvDevice.h"

using namespace std;
using namespace CyberLink;

////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////

TVDevice::TVDevice()
{
	//////////////////////////////////////
	// Control Ponit
	//////////////////////////////////////
    clockTime = "";
    airconTemp = "";

	m_ctrlPoint = new ControlPoint();
		
	m_ctrlPoint->addNotifyListener(this);
	m_ctrlPoint->addSearchResponseListener(this);
	m_ctrlPoint->addEventListener(this);
		
	//////////////////////////////////////
	// Device
	//////////////////////////////////////

	//try {
		m_pTVDev = new Device(DESCRIPTION_FILE_NAME);
		Action *getPowerAction = m_pTVDev->getAction("GetPower");
		getPowerAction->setActionListener(this);
	
		Action *setPowerAction = m_pTVDev->getAction("SetPower");
		setPowerAction->setActionListener(this);
		
		ServiceList *serviceList = m_pTVDev->getServiceList();
		Service *service = serviceList->getService(0);
		service->setQueryListener(this);
	//}
	//catch (InvalidDescriptionException e) {
	//	cerr << e.getMessage() << endl;
	//}
}

////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////

TVDevice::~TVDevice()
{
	delete m_pTVDev;
	//delete m_ctrlPoint;
}

////////////////////////////////////////////////
//	start/stop
////////////////////////////////////////////////
	
void TVDevice::start()
{
	m_ctrlPoint->start();
	m_pTVDev->start();
}

void TVDevice::stop()
{
	m_pTVDev->stop();
	m_ctrlPoint->stop();
}

////////////////////////////////////////////////
//	Device (Common)
////////////////////////////////////////////////
	
bool TVDevice::isDevice(SSDPPacket *packet, const char * deviceType)
{
	string usnBuf;
	const char * usn = packet->getUSN(usnBuf);
	int usnLen = strlen(usn);
	int devLen = strlen(deviceType);
	if (usnLen < devLen)
		return false;
	int ret = strncmp(usn + (usnLen - devLen), deviceType, devLen);
	if (ret == 0)
		return true;
	return false;
}
	
Service *TVDevice::getDeviceService(const char * deviceType, const char * serviceType)
{
	Device *dev = m_ctrlPoint->getDevice(deviceType);
	if (dev == NULL)
		return NULL;
	Service *service = dev->getService(serviceType);
	if (service == NULL)
		return NULL;
	return service;
}

bool TVDevice::subscribeService(SSDPPacket *packet, const char * deviceType, const char * serviceType)
{
	Service *service = getDeviceService(deviceType, serviceType);
	if (service == NULL)
		return false;
	bool ret = m_ctrlPoint->subscribe(service);
	if (ret == false)
		cerr << "Couldn't subscribe a service (" << deviceType << "," << serviceType << ")" << endl;	
	return ret;
}

////////////////////////////////////////////////
//	SSDP Listener
////////////////////////////////////////////////
	
void TVDevice::checkNewDevices(SSDPPacket *packet)
{
	subscribeService(packet, CLOCK_DEVICE_TYPE, CLOCK_SERVICE_TYPE);
	subscribeService(packet, AIRCON_DEVICE_TYPE, AIRCON_SERVICE_TYPE);
	subscribeService(packet, LIGHT_DEVICE_TYPE, LIGHT_SERVICE_TYPE);
	subscribeService(packet, WASHER_DEVICE_TYPE, WASHER_SERVICE_TYPE);
}
	
void TVDevice::checkRemoveDevices(SSDPPacket *packet)
{
	if (isDevice(packet, CLOCK_DEVICE_TYPE) == true)
		clockTime = "";
	if (isDevice(packet, AIRCON_DEVICE_TYPE) == true)
		airconTemp = "";
}

////////////////////////////////////////////////
// ActionListener
////////////////////////////////////////////////

bool TVDevice::actionControlReceived(Action *action)
{
	return false;
}

////////////////////////////////////////////////
// QueryListener
////////////////////////////////////////////////

bool TVDevice::queryControlReceived(StateVariable *stateVar)
{
	return true;
}

////////////////////////////////////////////////
// deviceNotifyReceived
////////////////////////////////////////////////

void OutputSSDPPacket(SSDPPacket *packet, bool isDeviceSearch = false)
{
	if (isDeviceSearch == true) {
		string usnBuf;
		string stBuf;
		string locationBuf;
		const char *uuid = packet->getUSN(usnBuf);
		const char *st = packet->getST(stBuf);
		const char *url = packet->getLocation(locationBuf);
		cout << "device search res : uuid = " << uuid << ", ST = " << st << ", location = " << url << endl; 
		return;
	}
	
	if (packet->isDiscover() == true) {
		string stBuf;
		const char *st = packet->getST(stBuf);
		cout << "ssdp:discover : ST = " << st << endl; 
	}
	else if (packet->isAlive() == true) {
		string usnBuf;
		string ntBuf;
		string locationBuf;
		const char *usn = packet->getUSN(usnBuf);
		const char *nt = packet->getNT(ntBuf);
		const char *url = packet->getLocation(locationBuf);
		cout << "ssdp:alive : uuid = " << usn << ", NT = " << nt << ", location = " << url << endl; 
	}
	else if (packet->isByeBye() == true) {
		string usnBuf;
		string ntBuf;
		const char *usn = packet->getUSN(usnBuf);
		const char *nt = packet->getNT(ntBuf);
		cout << "ssdp:byebye : uuid = " << usn << ", NT = " << nt << endl; 
	}
}

////////////////////////////////////////////////
// deviceNotifyReceived
////////////////////////////////////////////////

void TVDevice::deviceNotifyReceived(SSDPPacket *packet)
{
	OutputSSDPPacket(packet);
	
	if (packet->isAlive() == true)
		checkNewDevices(packet);
	if (packet->isByeBye() == true)
		checkRemoveDevices(packet);
}
	
////////////////////////////////////////////////
// deviceSearchResponseReceived
////////////////////////////////////////////////

void TVDevice::deviceSearchResponseReceived(SSDPPacket *packet)
{
	OutputSSDPPacket(packet);
	
	checkNewDevices(packet);
}
	
////////////////////////////////////////////////
// eventNotifyReceived
////////////////////////////////////////////////

//void TVDevice::eventNotifyReceived(const char *uuid, long seq, const char *name, const char *value)
void TVDevice::eventNotifyReceived(const std::string &uuid, long seq, const std::string &varName, const std::string &value)
{
		cout << "eventNotifyReceived : " << uuid << ", " << seq << ", " << varName << ", " << value << endl;
		
		Service *service = m_ctrlPoint->getSubscriberService(uuid);
		if (service == NULL)
			return;
		if (service->isService(CLOCK_SERVICE_TYPE) == true)
			clockTime = value;
		else if (service->isService(AIRCON_SERVICE_TYPE) == true)
			airconTemp = value;
		else {
			/*
			if (value != null && 0 < value.length()) {
				Device dev = service.getDevice();
				String fname = dev.getFriendlyName();
				message = fname + ":" + value;
			}
			*/
		}
}

////////////////////////////////////////////////
// update
////////////////////////////////////////////////

void TVDevice::update()
{
}			


