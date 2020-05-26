#include <FeKits/sky/FreeSky.h>

#include <OpenThreads/ScopedLock>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/TimeUtil.h>

namespace FeKit
{
	const double G_dTimeFlyRate = 1440.0; //默认的流逝速率

	CFreeSky::CFreeSky(FeUtil::CRenderContext* pContext)
		:CKitsCtrl(pContext)
		,m_rpAnimation(NULL)
		,m_rpSychCallback(NULL)
		,m_opSkyNode(NULL)
		,m_opNebulaNode(NULL)
		,m_bAnimationSwitch(false)
		,m_bSychLocalTime(false)
	{

	}

	CFreeSky::~CFreeSky( void )
	{
		m_opSkyNode = NULL;
		m_opNebulaNode = NULL;
		m_rpAnimation = NULL;
	}

	void CFreeSky::Show()
	{
		if(!m_bShow)
		{
			SetSunVisible(true);
			m_bShow = true;
		}
	}

	void CFreeSky::Hide()
	{
		if(m_bShow)
		{
			m_bShow = false;
			SetSunVisible(false);
		}
	}

	bool CFreeSky::Initialize()
	{
		return Initialize(osgEarth::Util::SkyOptions());
	}

	bool CFreeSky::Initialize( const osgEarth::Util::SkyOptions& options)
	{
		if(m_opRenderContext.valid())
		{
			///创建默认的太阳系光照系统以及星空系统
			m_opSkyNode = osgEarth::Util::SkyNode::create(options, m_opRenderContext->GetMapNode());
			if(m_opSkyNode.valid())
			{
				SetSunVisible(false);

				if(!m_opRenderContext->GetMapNode()->containsNode(m_opSkyNode.get()))
				{
					osgEarth::insertGroup(m_opSkyNode.get(), m_opRenderContext->GetMapNode());
				}

				// z00013 添加地球自转MT
				//if(!m_opRenderContext->GetRoot()->containsNode(m_opSkyNode.get()))
				//{
				//	osg::observer_ptr<osg::MatrixTransform>	opEarthRotationMT = m_opRenderContext->GetEarthRotationMT();

				//	osg::Group *pGroup = m_opRenderContext->GetRoot();
				//	m_opSkyNode->addChild(opEarthRotationMT.get());
				//	pGroup->removeChild(opEarthRotationMT.get());

				//	pGroup->addChild(m_opSkyNode.get());
				//}
			}

			//添加Nebula（星空背景），自定义附加的节点
			std::string strNubela = FeFileReg->GetFullPath("texture/sky/nebula.jpg");
			m_opNebulaNode = new CHgNebula(m_opRenderContext->GetMapNode(), strNubela);
			addChild(m_opNebulaNode.get());
			///创建时光流逝的callback
			m_rpAnimation = new CAnimateSkyCallback();
			m_rpSychCallback = new CSychLocalTimeCallback();

			m_bShow ? Show() : Hide();
			return true;
		}
		return false;
	}

	void CFreeSky::SetTimeFlyVisible( bool bState )
	{
		if (bState && m_bSychLocalTime)
		{
			SetSyncLocalTime(false);
		}

		if(m_opSkyNode.valid() && m_rpAnimation.valid())
		{
			if(bState)
			{
				addUpdateCallback(m_rpAnimation.get());
			}
			else
			{
				removeUpdateCallback(m_rpAnimation.get());
				m_rpAnimation->Reset();
			}
			m_bAnimationSwitch = bState;
		}
	}

	bool CFreeSky::GetTimeFlyVisible() const
	{
		return m_bAnimationSwitch;
	}

	void CFreeSky::SetAnimationRate( double dRate )
	{
		if(m_rpAnimation.valid())
		{
			m_rpAnimation->SetUpdateRate(dRate);
		}
	}

	double CFreeSky::GetAnimationRate() const
	{
		if(m_rpAnimation.valid())
		{
			return m_rpAnimation->GetUpdateRate();
		}

		return G_dTimeFlyRate;
	}

	void CFreeSky::SetDateTime( const osgEarth::DateTime& time )
	{
		if(m_opSkyNode.valid())
		{
			m_opSkyNode->setDateTime(time);
		}

		if(m_opNebulaNode.valid())
		{
			m_opNebulaNode->SetDateTime(time);
		}
	}

	osgEarth::DateTime CFreeSky::GetDateTime() const
	{
		if(m_opSkyNode.valid())
		{
			return m_opSkyNode->getDateTime();
		}

		return osgEarth::DateTime();
	}

	void CFreeSky::Attach( osgViewer::View* pView, int nLightNum )
	{
		if(m_opSkyNode.valid())
		{
			return m_opSkyNode->attach(pView, nLightNum);
		}
	}

	void CFreeSky::SetAmbient( const osg::Vec4& vecAmbient )
	{
		if(m_opSkyNode.valid())
		{
			osg::Light* pLight = m_opSkyNode->getSunLight();
			if(pLight)
			{
				pLight->setAmbient(vecAmbient);
			}
		}
	}

	osg::Vec4 CFreeSky::GetAmbient() const
	{
		if(m_opSkyNode.valid())
		{
			osg::Light* pLight = m_opSkyNode->getSunLight();
			if(pLight)
			{
				return pLight->getAmbient();
			}
		}

		return osg::Vec4();
	}

	void CFreeSky::SetSunVisible( bool bValue )
	{
		if(m_opSkyNode.valid())
		{
			m_opSkyNode->setLighting(bValue);
			m_opSkyNode->setSunVisible(bValue);
		}
	}

	bool CFreeSky::GetSunVisible()
	{
		if(m_opSkyNode.valid())
		{
			return m_opSkyNode->getSunVisible();
		}

		return false;
	}

	void CFreeSky::SetMoonVisible( bool bValue )
	{
		if(m_opSkyNode.valid())
		{
			m_opSkyNode->setMoonVisible(bValue);
		}
	}

	bool CFreeSky::GetMoonVisible()
	{
		if(m_opSkyNode.valid())
		{
			return m_opSkyNode->getMoonVisible();
		}

		return false;
	}

	void CFreeSky::SetStarsVisible( bool bValue )
	{
		if(m_opSkyNode.valid())
		{
			m_opSkyNode->setStarsVisible(bValue);
		}
	}

	bool CFreeSky::GetStarsVisible()
	{
		if(m_opSkyNode.valid())
		{
			return m_opSkyNode->getStarsVisible();
		}

		return false;
	}

	void CFreeSky::SetNebulaVisible(bool bValue)
	{
		if(m_opNebulaNode.valid())
		{
			m_opNebulaNode->SetNebulaVisible(bValue);
		}
	}

	bool CFreeSky::GetNebulaVisible()
	{
		if(m_opNebulaNode.valid())
		{
			return m_opNebulaNode->GetNebulaVisible();
		}

		return false;
	}

	void CFreeSky::SetAtmosphereVisible(bool bState)
	{
		if(m_opSkyNode.valid())
		{
			m_opSkyNode->getAtmosphere()->setNodeMask(bState);
		}
	}

	bool CFreeSky::GetAtmosphereVisible() const
	{
		if(m_opSkyNode.valid())
		{
			return (m_opSkyNode->getAtmosphere()->getNodeMask() != 0x0);
		}

		return false;
	}

	osg::Node* CFreeSky::GetAtmosphereNode()
	{
		return m_opSkyNode->getAtmosphere();
	}

	osg::Node* CFreeSky::GetSunNode()
	{
		return m_opSkyNode->getSun();
	}
	osg::Node* CFreeSky::GetMoonNode()
	{
		return m_opSkyNode->getMoon();
	}

	osg::Node* CFreeSky::GetNebulaNode()
	{
		return m_opNebulaNode.get();
	}

	osg::Node* CFreeSky::GetStarsNode()
	{
		return m_opSkyNode->getStars();
	}

	osg::Light* CFreeSky::GetLight()
	{
		return m_opSkyNode->getSunLight();
	}

	osg::Uniform* CFreeSky::GetUniformFogEnabled() 
	{
		return m_opSkyNode->GetUniformFogEnabled(); 
	}
	osg::Uniform* CFreeSky::GetUniformFogColor()   
	{
		return m_opSkyNode->GetUniformFogColor(); 
	}

	void CFreeSky::SetSyncLocalTime( bool bState )
	{
		if (bState && m_bAnimationSwitch)
		{
			SetTimeFlyVisible(false);
		}

		if(m_opSkyNode.valid() && m_rpSychCallback.valid())
		{
			if(bState)
			{
				addUpdateCallback(m_rpSychCallback.get());
			}
			else
			{
				removeUpdateCallback(m_rpSychCallback.get());
			}

			m_bSychLocalTime = bState;
		}
	}
	 
	bool CFreeSky::GetSyncLocalTime() const
	{
		return m_bSychLocalTime;
	}
}

namespace FeKit
{
	typedef ::time_t TimeStamp;

	CAnimateSkyCallback::CAnimateSkyCallback( double rate /*= 1440 */ )
		:m_dRate( rate )
		,m_dPrevTime( -1 )
		,m_dAccumTime( 0.0)
	{

	}

	CAnimateSkyCallback::~CAnimateSkyCallback()
	{

	}

	void CAnimateSkyCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if (nv->getVisitorType() == osg::NodeVisitor::NODE_VISITOR)
		{
			CFreeSky* pSky = dynamic_cast< CFreeSky* >( node );
			if (pSky)
			{            
				double time = nv->getFrameStamp()->getSimulationTime();            
				if (m_dPrevTime > 0)
				{                
					TimeStamp t = pSky->GetDateTime().asTimeStamp();                  
					double delta = ceil((time - m_dPrevTime) * m_dRate);
					m_dAccumTime += delta;

					if (m_dAccumTime > 1.0)
					{
						double deltaS = floor(m_dAccumTime );                    
						m_dAccumTime -= deltaS;
						t += deltaS;
						pSky->SetDateTime( t );                        
					}                
				}            
				m_dPrevTime = time;
			}
		}

		traverse( node, nv );
	}

	void CAnimateSkyCallback::SetUpdateRate( double dRate )
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_rateMutex);
		m_dRate = dRate;
	}

	double CAnimateSkyCallback::GetUpdateRate()
	{
		return m_dRate;
	}

	void CAnimateSkyCallback::Reset()
	{
		m_dPrevTime = -1;
		m_dAccumTime = 0.0;
	}

}

namespace FeKit
{
	CSychLocalTimeCallback::CSychLocalTimeCallback()
	{

	}

	CSychLocalTimeCallback::~CSychLocalTimeCallback()
	{

	}

	void CSychLocalTimeCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if(nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
		{
			CFreeSky* pSky = dynamic_cast< CFreeSky* >( node );
			if (pSky)
			{            
				osgEarth::DateTime dateTime = FeUtil::GetLocalTime();
				pSky->SetDateTime(dateTime);
			}
		}

		traverse( node, nv );
	}
}