#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include <osgGA/EventVisitor>

#include <FeKits/airview/AirView.h>
#include <FeUtils/CoordConverter.h>
#include <FeKits/manipulator/ManipulatorUtil.h>

namespace FeKit
{

	CAirView::CAirView(
		FeUtil::CRenderContext* pContext,
		int nXOffset, 
		int nYOffset, 
		int nWidth, 
		int nHeight, 
		CPlaceStrategy::KIT_PLACE ePlace, 
		const std::string& strDataPath)
		:CScreenKits(pContext, nXOffset, nYOffset, nWidth, nHeight, ePlace)
		,m_opFocus(NULL)
		,m_opGround(NULL)
		,m_dMinDistance(300000)
		,m_fLocateTime(4.0f)
	{
		setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal() + 1);
		CreateAirView(strDataPath);
	}


	CAirView::~CAirView(void)
	{
	}


	void CAirView::traverse( osg::NodeVisitor& nv )
	{
		if (nv.getVisitorType()==osg::NodeVisitor::EVENT_VISITOR)
		{
			osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(&nv);
			if (ev)
			{
				for(osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
					itr != ev->getEvents().end();
					++itr)
				{
					osgGA::GUIEventAdapter* ea = itr->get()->asGUIEventAdapter();
					if (handle(*ea, *(ev->getActionAdapter()))) 
					{
						ea->setHandled(true);
					}
				}
			}
		}

		CScreenKits::traverse(nv);
	}

	bool CAirView::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		bool bHandled = false;
		if(!ea.getHandled())
		{
			osgViewer::View* pView = dynamic_cast<osgViewer::View*>(&aa);
			if (pView)
			{
				switch(ea.getEventType())
				{
				case osgGA::GUIEventAdapter::DOUBLECLICK:
					{
						bHandled = DoubleClick(pView, ea.getX(), ea.getY());
					}
					break;
				}

				if(m_opFocus.valid() && m_opGround.valid())
				{
					osg::Vec3d vecEye(0.0, 0.0, 0.0);
					osg::Vec3d vecUp(0.0, 0.0, 0.0);
					osg::Vec3d vecCenter(0.0, 0.0, 0.0);
					osg::Vec3d vecLLH(0.0, 0.0, 0.0);
					pView->getCamera()->getViewMatrixAsLookAt(vecEye, vecUp, vecCenter);
					FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(), vecEye, vecLLH);

					m_opFocus->UpdateFocus(vecLLH);
				}
			}
		}

		return bHandled;
	}

	bool CAirView::DoubleClick( osgViewer::View* pView, unsigned unX, unsigned unY )
	{
		//是否在鸟瞰图内部，如果是根据屏幕坐标系计算世界坐标系然后通知地球

		if( pView && WithMe(unX, unY))
		{
			double dLocalX = (unX - m_nX) * 1.0;
			double dLocalY = (unY - m_nY) * 1.0;

			double dLon = 0.5 * AVGlobal::LONGITUDE * ((dLocalX / m_nWidth) * 2 - 1);
			double dLat = 0.5 * AVGlobal::LATITUDE * ((dLocalY / m_nHeight) * 2 - 1);

			FeKit::CManipulatorManager* pMg = dynamic_cast<FeKit::CManipulatorManager*>(pView->getCameraManipulator());
			if(NULL == pMg)
			{
				return false;
			}

			//判断当前操作器是否为地球操作器
			if (pMg->IsActiveCameraManipulator(FREE_EARTH_MANIPULATOR_KEY))
			{
				osgEarth::Viewpoint flyTo(osgEarth::Viewpoint("", dLon, dLat, 0.0, 0.0, -89.0, 10000.0));
				if (pMg->GetViewPoint().focalPoint()->vec3d().z() > 0)
				{
					flyTo.focalPoint()->vec3d().z() = pMg->GetViewPoint().focalPoint()->vec3d().z();
				}

				pMg->Locate(flyTo, 2.0);

				return true;
			}

		}

		return false;
	}

	void CAirView::CreateAirView( const std::string& strDataPath )
	{
		osg::Vec4d vecBColor(0.8, 0.8, 0.8, 1.0);
		double dWidth = m_dRightRate - m_dLeftRate;
		double dHeight = m_dTopRate - m_dBottomRate;

		CAirViewBoeder* pBorder = 
			new CAirViewBoeder(m_dLeftRate, m_dBottomRate, dWidth, dHeight, vecBColor);
		addChild(pBorder);

		osg::Vec4d vecFColor(1.0, 0.0, 0.0, 0.9);
		m_opFocus = new CAirViewFocus(m_dLeftRate, m_dBottomRate, dWidth, dHeight, vecFColor);
		addChild(m_opFocus.get());

		m_opGround = new CAirViewGround(m_dLeftRate, m_dBottomRate, dWidth, dHeight, strDataPath);
		addChild(m_opGround.get());

		m_opMarkGroup = new osg::Group;
		addChild(m_opMarkGroup.get());
	}

	bool CAirView::AddAirViewMark(CAirViewMark* pMark)
	{
		if(pMark && m_opMarkGroup.valid())
		{
			pMark->AttachAirViewSize(m_dWidthRate, m_dHeightRate);
			return m_opMarkGroup->addChild(pMark);
		}

		return false;
	}

	bool CAirView::ClearAirViewMarks()
	{
		if(m_opMarkGroup.valid())
		{
			return m_opMarkGroup->removeChildren(0, m_opMarkGroup->getNumChildren());
		}
		return false;
	}

	bool CAirView::RemoveAirViewMark( CAirViewMark* pMark )
	{
		if(m_opMarkGroup.valid() && m_opMarkGroup->containsNode(pMark))
		{
			return m_opMarkGroup->removeChild(pMark);
		}

		return false;
	}

}
