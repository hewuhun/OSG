#include <FeOcean/OceanNode.h>
#include <osgEarthDrivers/engine_mp/TileNode>
#include <osgViewer/CompositeViewer>

namespace FeOcean
{
	
	COceanContext::COceanContext()
		: m_bInitialized(false)
		, m_opSRS(0)
	{
	}

	void COceanContext::Initialize( unsigned int contextID )
	{
		if(!IsReady())
		{
			m_bInitialized = InitilizeImpl(contextID);
		}
	}

	void COceanContext::Update( double dSimTime )
	{
		UpdateImpl(dSimTime);
	}

}


namespace FeOcean
{
	COceanNode::COceanNode( FeUtil::CRenderContext* rc)
		: m_dMaxAltitude(100000)
		,m_bIsInVisibleRange(false)
		,m_bCanReflecting(false)
		,m_bEnableReflection(true)
	{
		if(rc)
		{
			m_opViewer = rc->GetView();
		}

		if(m_opViewer.valid())
		{
			osgViewer::CompositeViewer* vv = dynamic_cast<osgViewer::CompositeViewer*>(m_opViewer->getViewerBase());
			if(vv)
			{
				m_rpOceanUpdateCallback = new COceanUpdateCallback(this); 
				vv->addPreUpdateCallback(m_rpOceanUpdateCallback);
			}
		}
	}

	COceanNode::~COceanNode()
	{
	}

	const osgEarth::SpatialReference * COceanNode::GetSRS()
	{
		return m_rpOceanContext.valid() ? m_rpOceanContext->GetSRS() : 0;
	}

	void COceanNode::Show()
	{
		setNodeMask(0x11111111);
	}

	void COceanNode::Hide()
	{
		setNodeMask(0x00000000);
	}

	bool COceanNode::IsHide() const
	{
		return (getNodeMask() == 0x0);
	}

	void  COceanNode::SetOceanMaxVisibleDistance(double dValue)
	{
		m_dMaxAltitude = dValue;
	}

	double  COceanNode::GetOceanMaxVisibleDistance() const
	{
		return m_dMaxAltitude;
	}

	void COceanNode::Initialize( unsigned int contextID )
	{
		if(m_rpOceanContext.valid() && !m_rpOceanContext->IsReady())
		{
			m_rpOceanContext->Initialize(contextID);
			
			if(m_rpOceanContext->IsReady())
			{
				InitializeImpl();
			}
		}
	}

	void COceanNode::Update()
	{
		if(IsInitialized() && GetSRS() && m_opViewer.valid())
		{
			osg::ref_ptr<osg::Camera> cam = new osg::Camera();

			if(m_opViewer->getCameraManipulator())
			{
				m_opViewer->getCameraManipulator()->updateCamera(*cam.get());
			}

			osg::Vec3d eye,tmp, local;
			double dCurAltitude = 0;

			cam->getViewMatrix().getLookAt(eye,tmp,tmp);
			GetSRS()->transformFromWorld(eye, local, &dCurAltitude);

			if ( dCurAltitude > m_dMaxAltitude )
			{
				m_bIsInVisibleRange = false;
			}
			else
			{
				m_bIsInVisibleRange = true;

				m_bCanReflecting = (dCurAltitude > 0) ? true : false;

				if(m_rpOceanContext.valid())
				{
					m_rpOceanContext->Update(m_opViewer->getFrameStamp()->getSimulationTime());
				}
			}
		
			UpdateImpl();
		}
	}


}