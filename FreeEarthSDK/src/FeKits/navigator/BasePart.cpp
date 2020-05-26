#include <osgViewer/View>
#include <osgGA/EventVisitor>

#include <FeKits/navigator/BasePart.h>
#include <FeUtils/UtilityGeom.h>

namespace
{
	/**
	*@note: 修改Geometry的透明度
	*/
	void ChangeGeometryTran(osg::Geode* pGeode, double dTran)
	{
		if(!pGeode) return;

		osg::Geometry* pGeom = dynamic_cast<osg::Geometry*>(pGeode->getDrawable(0));
		if(pGeom)
		{
			osg::Vec4dArray* pColor = dynamic_cast<osg::Vec4dArray*>(pGeom->getColorArray());
			if(pColor)
			{
				(*pColor)[0].w() = dTran;
				pColor->dirty();
			}
		}
	}
}

namespace FeKit
{
	const float RADIUS_RATE = 0.5;

	CBasePart::CBasePart
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:osg::MatrixTransform()
		,m_nPosX(0)
		,m_nPosY(0)
		,m_nWidth(0)
		,m_nHeight(0)
		,m_nCenterX(0)
		,m_nCenterY(0)
		,m_dRadius(0.0)
		,m_dLeft(dLeft)
		,m_dRight(dRight)
		,m_dBottom(dBottom)
		,m_dTop(dTop)

		,m_bActive(false)
		,m_bSelect(false)
	{
		m_dWidth = m_dRight - m_dLeft;
		m_dHeight = m_dTop - m_dBottom;

		setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal() + 1);
	}


	CBasePart::~CBasePart(void)
	{
	}

	void CBasePart::traverse( osg::NodeVisitor& nv )
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
						ea->setHandled(true);
				}
			}
		}

		osg::MatrixTransform::traverse(nv);
	}

	bool CBasePart::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		if(ea.getHandled()) return false;

		osgViewer::View* pView = dynamic_cast<osgViewer::View*>(&aa);
		if (!pView) return false;

		bool bHandled = false;

		switch(ea.getEventType())
		{
		case osgGA::GUIEventAdapter::PUSH:
			{
				bHandled = PushHandle(ea.getX(), ea.getY());
			}
			break;
		case  osgGA::GUIEventAdapter::DRAG:
			{
				bHandled = DragHandle(ea.getX(), ea.getY());
			}
			break;
		case osgGA::GUIEventAdapter::RELEASE:
			{
				bHandled = ReleaseHandle(ea.getX(), ea.getY());
			}
			break;
		case osgGA::GUIEventAdapter::MOVE:
			{
				bHandled = MoveHandle(ea.getX(), ea.getY());
			}
			break;
		case osgGA::GUIEventAdapter::DOUBLECLICK:
			{
				bHandled = DoubleClickHandle(ea.getX(), ea.getY());
			}
			break;
		}

		return bHandled;
	}

	void CBasePart::UpdatePos( int nX, int nY, int nWidth, int nHeight, double dWR, double dHR)
	{
		m_nPosX = nX + (m_dLeft + dWR * 0.5) / dWR * nWidth;
		m_nPosY = nY + (m_dBottom + dHR * 0.5) / dHR * nHeight;
		m_nWidth = m_dWidth / dWR * nWidth;
		m_nHeight = m_dHeight / dHR * nHeight;

		m_dRadius = (m_nWidth < m_nHeight ? m_nWidth : m_nHeight) * 0.5;
		m_nCenterX = m_nPosX + m_dRadius;
		m_nCenterY = m_nPosY + m_dRadius;

	}

	//////////////////////////////////////////////////////////////////////////
	void CCompositPart::UpdatePos( int nX, int nY, int nWidth, int nHeight, double dWR, double dHR )
	{
		CBasePart::UpdatePos(nX, nY, nWidth, nHeight, dWR, dHR);

		ChildPart::iterator itr = m_vecParts.begin();
		for(; itr != m_vecParts.end(); ++itr)
		{
			if(itr->valid())
			{
				itr->get()->UpdatePos(nX, nY, nWidth, nHeight, dWR, dHR);
			}
		}
	}

	bool CCompositPart::WithinMe( int nX, int nY )
	{
		return false;
	}

	void CCompositPart::SetCtrl( CNavigatorCtrl* pCtrl )
	{
		ChildPart::iterator itr = m_vecParts.begin();
		for(; itr != m_vecParts.end(); ++itr)
		{
			if(itr->valid())
			{
				itr->get()->SetCtrl(pCtrl);
			}
		}
	}

	CCompositPart::CCompositPart
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CBasePart(dLeft, dRight, dBottom, dTop, strRes)
	{
		m_vecParts.clear();
	}

	void CCompositPart::Active( bool bActive )
	{
		ChildPart::iterator itr = m_vecParts.begin();
		for(itr; itr != m_vecParts.end(); ++itr)
		{
			if(itr->valid())
			{
				itr->get()->Active(bActive);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool CLeafPart::WithinMe( int nX, int nY )
	{
		if((nX - m_nPosX <= m_nWidth) && 
			(nX - m_nPosX > 0) && 
			(nY - m_nPosY <= m_nHeight) &&
			(nY - m_nPosY > 0))
		{
			return true;
		}

		return false;
	}

	void CLeafPart::SetCtrl( CNavigatorCtrl* pCtrl )
	{
		m_opCtrl = pCtrl;
	}

	void CLeafPart::Create(bool bAspectRatio)
	{
		osg::ref_ptr<osg::Geode> pGeode = NULL;

		osg::Vec3d vecCorner(m_dLeft, m_dBottom, 0.0);

		if(bAspectRatio)
		{
			double dTemp = m_dWidth < m_dHeight ? m_dWidth : m_dHeight;

			vecCorner.x() += (m_dWidth - dTemp) * 0.5; 
			m_dWidth = dTemp;
			m_dHeight = dTemp;
		}

		pGeode = FeUtil::CreateBasePlate( m_strImageName, vecCorner, m_dWidth, m_dHeight, osg::Vec4d(1.0, 1.0, 1.0, m_dNormalTran));
		addChild(pGeode.get());
		m_opGeodeNode = pGeode.get();
	}

	CLeafPart::CLeafPart
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CBasePart(dLeft, dRight, dBottom, dTop, strRes)
		,m_opGeodeNode(NULL)
		,m_opCtrl(NULL)
		,m_strImageName("")
		,m_dNormalTran(0.0) 
		,m_dActiveTran(0.65)
		,m_dSelectTran(1.0)
	{
	}

	void CLeafPart::Active( bool bActive )
	{
		if(bActive)
		{
			ChangeGeometryTran(m_opGeodeNode.get(), m_dActiveTran);
		}
		else
		{
			ChangeGeometryTran(m_opGeodeNode.get(), m_dNormalTran);
		}

		m_bActive = bActive;
	}

	void CLeafPart::Select( bool bSelect )
	{
		if(bSelect)
		{
			ChangeGeometryTran(m_opGeodeNode.get(), m_dSelectTran);
		}
		else
		{
			ChangeGeometryTran(m_opGeodeNode.get(), m_dActiveTran);
		}

		m_bSelect = bSelect;
	}
}

