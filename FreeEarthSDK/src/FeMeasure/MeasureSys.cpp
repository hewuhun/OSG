#include <FeMeasure/MeasureSys.h>

#include <FeUtils/CoordConverter.h>

#include <osgUtil/LineSegmentIntersector>

namespace FeMeasure
{
	bool CDefaultMeasureEventHandler::HandleMouseEventImplement( CMeasure* pMeasure, E_MOUSE_EVENT mouseEvent, osg::Vec3 vecMousePosLLH )
	{
		if(!pMeasure) return false;

		switch (mouseEvent)
		{
		case E_MOUSE_LBUTTON_CLICK:
			{
				if(pMeasure->BeginMeasure())
				{
					/// 开始一次新的测量
					pMeasure->ClearResult();
					pMeasure->AddCtrlPoint(vecMousePosLLH);
					pMeasure->UpdateResult();
				}
				else
				{
					/// 添加控制点
					pMeasure->AddCtrlPoint(vecMousePosLLH);
					pMeasure->UpdateResult();
				}
			
			}break;

		case E_MOUSE_RBUTTON_CLICK:
			{
				if(pMeasure->IsMeasuring())
				{
					/// 结束一次测量
					pMeasure->UpdateResult();
					pMeasure->EndMeasure();
				}

			}break;

		case E_MOUSE_MOVE:
			{
				if(pMeasure->IsMeasuring())
				{
					/// 测量中更新末尾控制点
					if(pMeasure->GetCtrlPoints().size() == 1)
					{
						pMeasure->AddCtrlPoint(vecMousePosLLH);
					}
					else if(pMeasure->GetCtrlPoints().size() > 1)
					{
						pMeasure->ReplaceCtrlPoint(vecMousePosLLH);
					}
					pMeasure->UpdateResult();
				}

			}break;
		}

		return true;
	}
}


namespace FeMeasure
{
#define HASMODKEY( W, V ) (( W & V ) == V )

	CMeasureSys::CMeasureSys()
		:FeUtil::CSceneSysCall()
		,m_fMouseDownX(0.0)
		,m_fMouseDownY(0.0)
		,m_opCurMeasure(NULL)
		,m_rpHandler(NULL)
	{
		SetSysKey(MEASURE_SYSTEM_CALL_DEFAULT_KEY);

		m_rpDefaultHandler = new CDefaultMeasureEventHandler();
	}

	CMeasureSys::~CMeasureSys()
	{
		UnInitiliazeImplement();
	}

	bool CMeasureSys::InitiliazeImplement()
	{
		m_rpHandler = new CMeasureHandler(this);

		return true;
	}

	bool CMeasureSys::UnInitiliazeImplement()
	{
		StopEventHandler();
		DeactiveMeasure();

		return true;
	}

	bool CMeasureSys::RunEventHandler()
	{
		if(m_rpHandler.valid() && m_opRenderContext.valid())
		{
			return m_opRenderContext->AddEventHandler(m_rpHandler.get());
		}

		return false;
	}

	bool CMeasureSys::StopEventHandler()
	{
		if(m_rpHandler.valid() && m_opRenderContext.valid())
		{
			return m_opRenderContext->RemoveEventHandler(m_rpHandler.get());
		}

		return false;
	}

	void CMeasureSys::ActiveMeasure( FeMeasure::CMeasure* pMeasure )
	{
		DeactiveMeasure();

		if(pMeasure)
		{
			if(!m_opSystemCallRoot->containsNode(pMeasure))
			{
				m_opSystemCallRoot->addChild(pMeasure);
				m_opCurMeasure = pMeasure;
				
				RunEventHandler();
			}
		}
	}

	void CMeasureSys::DeactiveMeasure()
	{
		if(m_opCurMeasure.valid() && m_opSystemCallRoot.valid())
		{
			m_opSystemCallRoot->removeChild(m_opCurMeasure.get());
			m_opCurMeasure = NULL;

			StopEventHandler();
		}
	}

	void CMeasureSys::ClearMesureResult()
	{
		DeactiveMeasure();
		m_opSystemCallRoot->removeChild(0, m_opSystemCallRoot->getNumChildren());
	}

	bool CMeasureSys::handle( const GUIEventAdapter& ea,GUIActionAdapter& aa, osg::Object* pO, osg::NodeVisitor* pV )
	{
		if ( ea.getHandled() )
		{
			return false;
		}

		if(!m_opCurMeasure.valid())
		{
			return false;
		}

		if (ea.getEventType() == GUIEventAdapter::PUSH)
		{        
			MouseDown(ea.getX(), ea.getY(), GUIEventAdapter::MouseButtonMask(ea.getButton()));
		}
		else if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
		{
			MouseUp(ea.getX(), ea.getY(), GUIEventAdapter::MouseButtonMask(ea.getButton()));
			aa.requestRedraw();
		}
		else if (ea.getEventType() == osgGA::GUIEventAdapter::DOUBLECLICK) 
		{        
			MouseDCheck(ea.getX(), ea.getY(), GUIEventAdapter::MouseButtonMask(ea.getButton()));
		}
		else if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE)
		{
			MouseMove(ea.getX(), ea.getY(), GUIEventAdapter::MouseButtonMask(ea.getButton()));
		}  

		return false;
	}

	bool CMeasureSys::MouseDown( float fMouseDownX, float fMouseDownY, GUIEventAdapter::MouseButtonMask mk )
	{
		m_fMouseDownX = fMouseDownX;
		m_fMouseDownY = fMouseDownY;
		
		return false;
	}

	bool CMeasureSys::MouseUp( float fMouseDownX, float fMouseDownY, GUIEventAdapter::MouseButtonMask mk )
	{
		if(abs(m_fMouseDownX - fMouseDownX) < 3 && abs(m_fMouseDownY - fMouseDownY) < 3)
		{
			osg::Vec3d vecLLH(0.0, 0.0, 0.0);

			if(!m_opCurMeasure.valid()) return false;
			if(!FeUtil::ScreenXY2DegreeLLH(m_opRenderContext.get(), fMouseDownX, fMouseDownY, vecLLH)) return false;

			switch(mk)
			{
			case GUIEventAdapter::LEFT_MOUSE_BUTTON:
				{
					HandleEvent(m_opCurMeasure.get(), CMeasureEventHandler::E_MOUSE_LBUTTON_CLICK, vecLLH);
				}break;

			case GUIEventAdapter::RIGHT_MOUSE_BUTTON:
				{
					HandleEvent(m_opCurMeasure.get(), CMeasureEventHandler::E_MOUSE_RBUTTON_CLICK, vecLLH);
				
				}break;

			default:
				break;
			}
		}

		return false;
	}

	bool CMeasureSys::MouseMove( float fMouseDownX, float fMouseDownY, GUIEventAdapter::MouseButtonMask mk )
	{
		if(!m_opCurMeasure.valid()) return false;

		if(m_opCurMeasure->IsMeasuring())
		{
			osg::Vec3d vecLLH(0.0, 0.0, 0.0);

			if(!m_opCurMeasure.valid()) 
				return false;

			if(!FeUtil::ScreenXY2DegreeLLH(m_opRenderContext.get(), fMouseDownX, fMouseDownY, vecLLH))
				return false;

			HandleEvent(m_opCurMeasure.get(), CMeasureEventHandler::E_MOUSE_MOVE, vecLLH);
		}
		
		return false;
	}

	bool CMeasureSys::MouseDCheck( float fMouseDownX, float fMouseDownY, GUIEventAdapter::MouseButtonMask mk )
	{
		osg::Vec3d vecLLH(0.0, 0.0, 0.0);

		if(!m_opCurMeasure.valid()) 
			return false;

		if(!FeUtil::ScreenXY2DegreeLLH(m_opRenderContext.get(), fMouseDownX, fMouseDownY, vecLLH))
			return false;

		if(GUIEventAdapter::LEFT_MOUSE_BUTTON == mk)
		{
			HandleEvent(m_opCurMeasure.get(), CMeasureEventHandler::E_MOUSE_LBUTTON_DBCLICK, vecLLH);
		}
		else if(GUIEventAdapter::RIGHT_MOUSE_BUTTON == mk)
		{
			HandleEvent(m_opCurMeasure.get(), CMeasureEventHandler::E_MOUSE_RBUTTON_DBCLICK, vecLLH);
		}
		
		return false;
	}

	void CMeasureSys::AddEventHandler( CMeasureEventHandler* eventHandler )
	{
		if(!eventHandler) return;

		MEASURE_EVENT_HANDLER_LIST::iterator iter = std::find(m_handlerList.begin(), m_handlerList.end(), eventHandler);
		if(iter == m_handlerList.end())
		{
			m_handlerList.push_back(eventHandler);
		}
	}

	bool CMeasureSys::RemoveEventHandler( CMeasureEventHandler* eventHandler )
	{
		if(!eventHandler) return false;

		MEASURE_EVENT_HANDLER_LIST::iterator iter = std::find(m_handlerList.begin(), m_handlerList.end(), eventHandler);
		if(iter != m_handlerList.end())
		{
			m_handlerList.erase(iter);
			return true;
		}
		return false;
	}

	void CMeasureSys::ClearEventHandler()
	{
		m_handlerList.clear();
	}

	bool CMeasureSys::HandleEvent( CMeasure* pMeasure, CMeasureEventHandler::E_MOUSE_EVENT mouseEvent, osg::Vec3 vecMousePosLLH )
	{
		MEASURE_EVENT_HANDLER_LIST::iterator iter = m_handlerList.begin();
		while(iter != m_handlerList.end())
		{
			if(iter->valid() && iter->get()->HandleMouseEvent(pMeasure, mouseEvent, vecMousePosLLH))
			{
				return true;
			}
			iter++;
		}
		return (m_rpDefaultHandler.valid())?
			m_rpDefaultHandler->HandleMouseEvent(pMeasure, mouseEvent, vecMousePosLLH) : false;
	}

}

namespace FeMeasure
{
	CMeasureSys::CMeasureHandler::CMeasureHandler(CMeasureSys* pMeasureSys)
		:GUIEventHandler()
		,m_opMeasureSys(pMeasureSys)
	{

	}

	CMeasureSys::CMeasureHandler::~CMeasureHandler()
	{

	}

	bool CMeasureSys::CMeasureHandler::
		handle( const GUIEventAdapter& ea, GUIActionAdapter& aa, osg::Object* pO, osg::NodeVisitor* pNV)
	{
		if(m_opMeasureSys.valid())
		{
			return m_opMeasureSys->handle(ea, aa, pO, pNV);
		}

		return false;
	}

}
