#include <osg/MatrixTransform>
#include <osgSim/SphereSegment>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Depth>

#include <FeEffects/ScanRadarEffect.h>
#include <FeUtils/CoordConverter.h>
#include <FeEffects/SphereSegment.h>

namespace FeEffect
{
	CScanRadarEffect::CScanRadarEffect()
		:CFreeEffect()
		,m_dLoopTime(0.0)
		,m_dRadius(2000)
		,m_fAzMin(-10.0)
		,m_fAzMax(10.0)
		,m_fElevMin(0.0)
		,m_fElevMax(70)
		,m_vecColor(osg::Vec4d(0.5f,1.0f,1.0f,0.5f))
		,m_pMT(NULL)
		,m_opContext(NULL)
		,m_eScanType(FeEffect::ROTATION_SCANTYPE)
		,m_fElfinAzMax(2.0)
		,m_fElfinElevMax (2.0)
		,m_dOffsetYaw(0.0)
		,m_dOffsetRoll(0.0)
		,m_dOffsetPitch(0.0)
	{
		m_vecCenter.set(0.0, 0.0, 0.0);
		m_rpRadarWaveGroup = new osg::Group();
		CreateEffect();
	}

	CScanRadarEffect::CScanRadarEffect(FeUtil::CRenderContext* pContext, const osg::Vec3& center, double dRadius, 
		float  azMin,  
		float  azMax,  
		float  elevMin,  
		float  elevMax,
		osg::Vec4d color,
		ERadarScanType eScanType,
		double dLoopTime
		)
		:CFreeEffect()
		,m_dLoopTime(dLoopTime)
		,m_dRadius(dRadius)
		,m_vecCenter(center)
		,m_fAzMin(azMin)
		,m_fAzMax(azMax)
		,m_fElevMin(elevMin)
		,m_fElevMax(elevMax)
		,m_vecColor(color)
		,m_opContext(pContext)
		,m_pMT(NULL)
		,m_eScanType(eScanType)
		,m_fElfinAzMax(2.0)
		,m_fElfinElevMax (2.0)
		,m_vecElfinColor(m_vecColor)
		,m_dOffsetYaw(0.0)
		,m_dOffsetRoll(0.0)
		,m_dOffsetPitch(0.0)
	{	  
		m_rpRadarWaveGroup = new osg::Group();
		CreateEffect();
	}

	CScanRadarEffect::~CScanRadarEffect()
	{

	}

	osg::Vec3d CScanRadarEffect::GetCenter() const
	{
		return m_vecCenter;
	}

	void CScanRadarEffect::SetCenter( const osg::Vec3d& center )
	{
		m_vecCenter = center;
	}

	double CScanRadarEffect::GetRadius() const
	{
		return m_dRadius;
	}

	void CScanRadarEffect::SetRadius( double dRadius )
	{
		m_dRadius = dRadius;
	}

	double CScanRadarEffect::GetLoopTime() const
	{
		return m_dLoopTime;
	}

	void CScanRadarEffect::SetLoopTime( double dLoopTime )
	{
		m_dLoopTime = dLoopTime;
	}

	float CScanRadarEffect::GetAzMin() const
	{
		return m_fAzMin;
	}

	void CScanRadarEffect::SetAzMin( float fAzMin )
	{
		m_fAzMin = fAzMin;
	}

	float CScanRadarEffect::GetAzMax() const
	{
		return m_fAzMax;
	}

	void CScanRadarEffect::SetAzMax( float fAzMax )
	{
		m_fAzMax = fAzMax;
	}

	float CScanRadarEffect::GetElevMin() const
	{
		return m_fElevMin;
	}

	void CScanRadarEffect::SetElevMin( float fElevMin )
	{
		m_fElevMin = fElevMin;
	}

	float CScanRadarEffect::GetElevMax() const
	{
		return m_fElevMax;
	}

	void CScanRadarEffect::SetElevMax( float fElevMax )
	{
		m_fElevMax = fElevMax;
	}

	void CScanRadarEffect::SetColor( osg::Vec4d vecColor )
	{
		m_vecColor = vecColor;
	}

	osg::Vec4d CScanRadarEffect::GetColor() const
	{
		return m_vecColor;
	}

	void CScanRadarEffect::CreateRadar()
	{
		if(!m_pMT.valid())
		{
			return;
		}

		osg::ref_ptr<FeEffect::CSphereSegment> pSS = new FeEffect::CSphereSegment(
			osg::Vec3d(0,0,0), 
			m_dRadius,
			osg::DegreesToRadians(m_fAzMin),
			osg::DegreesToRadians(m_fAzMax),
			osg::DegreesToRadians(m_fElevMin),
			osg::DegreesToRadians(m_fElevMax),
			15);

		pSS->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		pSS->setAllColors(m_vecColor);
		osg::Vec4d vecColor = m_vecColor;
		vecColor.a() = 0.1;
		pSS->setSideColor(vecColor);
		pSS->setSurfaceLineColor(osg::Vec4d(1.0,1.0,1.0,0.5));
		pSS->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		m_pMT->addChild(pSS.get());
	}

	void CScanRadarEffect::CreateScanElfin()
	{
		if(!m_pMT.valid())
		{
			return;
		}

		osg::ref_ptr<osg::MatrixTransform> rpScanElfinMT = new osg::MatrixTransform;
		osg::ref_ptr<osgSim::SphereSegment> pSS = new osgSim::SphereSegment(
			osg::Vec3d(0,0,0), 
			m_dRadius,
			osg::DegreesToRadians(0.0),
			osg::DegreesToRadians(m_fElfinAzMax),
			osg::DegreesToRadians(0.0),
			osg::DegreesToRadians(m_fElfinElevMax),
			5);

		pSS->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		pSS->setAllColors(m_vecElfinColor);
		pSS->setSideColor(m_vecElfinColor);

		rpScanElfinMT->addChild(pSS);
		if(m_pMT.valid())
		{
			m_pMT->addChild(rpScanElfinMT);
		}
		osg::ref_ptr<osg::Depth> dp = new osg::Depth();
		dp->setWriteMask(false);
		pSS->getOrCreateStateSet()->setAttribute(dp.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		rpScanElfinMT->setUpdateCallback(new FeUtil::AnimationPathCallback(CreateRadarElfinScanPath()));

	}

	void CScanRadarEffect::SetScanType( ERadarScanType eScanType )
	{
		m_eScanType = eScanType;
		//UpdateEffect();
	}


	bool CScanRadarEffect::CreateEffect()
	{  
		m_pMT = new osg::MatrixTransform();
		m_rpRadarWaveGroup->addChild(m_pMT);
		m_pMT->setMatrix(osg::Matrix::translate(m_vecCenter));
		addChild(m_rpRadarWaveGroup);

		UpdateEffect();

		return true;
	}


	bool CScanRadarEffect::UpdateEffect()
	{
		if(!m_pMT.valid())
		{
			return false;
		}
		m_pMT->setUpdateCallback(NULL);
		m_pMT->removeChild(0,m_pMT->getNumChildren());

		m_pMT->setMatrix(osg::Matrix::rotate(
			osg::DegreesToRadians(m_dOffsetPitch),osg::X_AXIS,
			osg::DegreesToRadians(m_dOffsetRoll),osg::Y_AXIS,
			osg::DegreesToRadians(m_dOffsetYaw),osg::Z_AXIS) *osg::Matrix::translate(m_vecCenter));

		switch(m_eScanType)
		{
		case ROTATION_SCANTYPE:
			{
				CreateRadar();
				m_pMT->setUpdateCallback(new FeUtil::AnimationPathCallback(CreateRadarAnimationPath()));
			}break;
		case HORIZONTAL_SCANTYPE:
		case VERTICAL_SCANTYPE:
			{
				CreateRadar();
				CreateScanElfin();
			}break;
		case STATIC_SCANTYPE:
			{
				CreateRadar();
			}
			break;
		default:
			break;
		}
		return true;
	}

	FeUtil::AnimationPath* CScanRadarEffect::CreateRadarAnimationPath()
	{
		FeUtil::AnimationPath* pAnimationPath = new FeUtil::AnimationPath;

		pAnimationPath->setLoopMode(FeUtil::AnimationPath::LOOP);

		int numSamples = 36;
		float yaw = 0.0f;
		float yaw_delta = 2.0f * osg::PI/((float)numSamples-1.0f);

		double time=0.0f;
		double time_delta = m_dLoopTime/(double)numSamples;

		for(int i=0;i< numSamples; ++i)
		{
			osg::Vec3 position(m_vecCenter);
			osg::Quat rotation(osg::Quat(-yaw, osg::Vec3(0.0,0.0,1.0)));

			pAnimationPath->insert(time,FeUtil::AnimationPath::ControlPoint(position,rotation));

			yaw += yaw_delta;
			time += time_delta;

		}
		return pAnimationPath;
	}

	FeUtil::AnimationPath* CScanRadarEffect::CreateRadarElfinScanPath()
	{
		FeUtil::AnimationPath* pAnimationPath = new FeUtil::AnimationPath;
		pAnimationPath->setLoopMode(FeUtil::AnimationPath::LOOP);

		double  dDisElfinAz =  m_fElfinAzMax;
		double  dDisElfinElev =  m_fElfinElevMax;

		int numSamplesX = (m_fAzMax-m_fAzMin)/(m_fElfinAzMax);
		int numSamplesY = (m_fElevMax-m_fElevMin)/(m_fElfinElevMax);

		float yaw = m_fAzMin;
		float yaw_delta = (m_fAzMax-m_fAzMin)/((float)numSamplesX-1);

		float pitching = m_fElevMin;
		float pitching_delta = (m_fElevMax-m_fElevMin)/((float)numSamplesY-1);

		switch(m_eScanType)
		{
		case HORIZONTAL_SCANTYPE:
			{
				pitching_delta = pitching_delta/((float)numSamplesX);
			}break;
		case VERTICAL_SCANTYPE:
			{
				yaw_delta = yaw_delta/((float)numSamplesY);
			}break;
		default:
			break;
		}
		

		double time=0.0f;
		double time_delta = m_dLoopTime/(double)numSamplesX;

		if(m_eScanType == VERTICAL_SCANTYPE)
		{
			int nTemp = numSamplesX;
			numSamplesX = numSamplesY ;
			numSamplesY = nTemp;
		}

		for(int n = 0;n < numSamplesY; n++)
		{
			for(int m=0;m< numSamplesX; ++m)
			{
				if(yaw >= (m_fAzMax - dDisElfinAz))
				{
					yaw = (m_fAzMax - dDisElfinAz);
				}
				else if(yaw <= m_fAzMin)
				{
					yaw = m_fAzMin;
				}

				if(pitching >= (m_fElevMax-dDisElfinElev))
				{
					pitching = m_fElevMax-dDisElfinElev;
				}
				else if(pitching <= (m_fElevMin))
				{
					pitching = m_fElevMin;
				}

				osg::Vec3 position(0.0,0.0,0.0);
				osg::Quat rotation;
				rotation.makeRotate(osg::DegreesToRadians(pitching),osg::X_AXIS,0.0,osg::Y_AXIS,-osg::DegreesToRadians(yaw),osg::Z_AXIS);

				pAnimationPath->insert(time,FeUtil::AnimationPath::ControlPoint(position,rotation));

				pitching += pitching_delta;
				yaw += yaw_delta;
				time += time_delta;

			}
			if(m_eScanType == HORIZONTAL_SCANTYPE)
			{
				yaw_delta = -yaw_delta;
				yaw += yaw_delta;
			}
			else if(m_eScanType == VERTICAL_SCANTYPE)
			{
				pitching_delta = -pitching_delta;
				pitching += pitching_delta;
			}
			
		}

		FeUtil::AnimationPath::TimeControlPointMap mapTimeCtrl,mapTimeCtrlTemp;
		mapTimeCtrl = pAnimationPath->getTimeControlPointMap();
		mapTimeCtrlTemp = mapTimeCtrl;
		if(mapTimeCtrlTemp.size() > 0)
		{
			FeUtil::AnimationPath::TimeControlPointMap::iterator itrTemp = mapTimeCtrlTemp.end();

			for(-- itrTemp;itrTemp!= mapTimeCtrlTemp.begin();--itrTemp )
			{
				pAnimationPath->getTimeControlPointMap()[time] = itrTemp->second;
				time += time_delta;
			}
			pAnimationPath->getTimeControlPointMap()[time] = itrTemp->second;
		}
		return pAnimationPath;
	}

	void CScanRadarEffect::SetScanElfinRange( float fElfinAzMax,float fElfinElevMax )
	{
		m_fElfinAzMax	= fElfinAzMax; 
		m_fElfinElevMax = fElfinElevMax;

		//UpdateEffect();
	}

	void CScanRadarEffect::SetScanElfinColor( osg::Vec4d vecElfinColor )
	{
		m_vecElfinColor = vecElfinColor;
		//UpdateEffect();
	}

	void CScanRadarEffect::SetRange( float fAzMin, float fAzMax,float fElevMin,float fElevMax )
	{
		m_fAzMin = fAzMin;
		m_fAzMax = fAzMax;
		m_fElevMax = fElevMax;
		m_fElevMin = fElevMin;
	}

	void CScanRadarEffect::SetRotateOffset( double dOffsetPitch,double dOffsetRoll,double dOffsetYaw )
	{
		m_dOffsetPitch = dOffsetPitch;
		m_dOffsetYaw = dOffsetYaw;
		m_dOffsetRoll = dOffsetRoll;
	}

}
