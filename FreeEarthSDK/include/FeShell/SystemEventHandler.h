/**************************************************************************************************
* @file SystemEventHandler.h
* @note 系统事件处理器
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef SCENE_SHELL_SYSTEM_HANDLER_H
#define SCENE_SHELL_SYSTEM_HANDLER_H

#include <osgViewer/View>
#include <osgEarth/MapNode>

#include <FeUtils/FreeViewPoint.h>
#include <FeUtils/EventListener.h>
#include <FeKits/manipulator/FreeEarthManipulator.h>

#include <FeShell/Export.h>
#include <FeUtils/SceneSysCall.h>
#include <FeShell/SystemSerializer.h>

#include <FeShell/EnvironmentSys.h>
#include <FeShell/SmartToolSys.h>
#include <FeShell/MeasureSys.h>

namespace FeShell
{
	using namespace FeUtil;

	typedef	enum 
	{
		E_State_FreeManp	= 1<<0,
		E_State_Editor		= 1<<1,
		E_State_Measure		= 1<<2
	}ESysEventState;


	class CFreeEarth;

	class CSystemHandler:public osgGA::GUIEventHandler
	{
	public:
		typedef std::list<CFeEventHandler*> FeEventHandlerList;

	public:
		CSystemHandler(CFreeEarth* pFreeEarth);
		~CSystemHandler();

	public:
		virtual bool handle(const GUIEventAdapter& ea, GUIActionAdapter& aa);

	public:
		void AttachUserEvent(CEventListener* pEvent);

		void PushFbEventHandler(CFeEventHandler* pHander);

		void RemoveFbEventHandler(CFeEventHandler* pHander);

	protected:
		CEventListener*			m_pEventUser;

		FeEventHandlerList		m_ltEventList;

		CFreeEarth*			m_opFreeEarth;
	};
}

#endif//SCENE_SHELL_SYSTEM_HANDLER_H
