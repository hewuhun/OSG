#include <osg/MatrixTransform>
#include <osgSim/SphereSegment>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osgParticle/ExplosionEffect>

#include <FeEffects/FreeEffect.h>
#include <FeUtils/CoordConverter.h>


namespace FeEffect
{
	CFreeEffect::CFreeEffect()
		:osg::Group()
		,m_bVisible(false)
		,m_dRadius(2000.0)
		,m_vecCenter(0.0,0.0,0.0)
		,m_vecColor(0.0,1.0,1.0,1.0)
		,m_unType(0)
	{

	}

	CFreeEffect::~CFreeEffect()
	{

	}

	bool CFreeEffect::GetVisible() const
	{
		return m_bVisible;
	}

	void CFreeEffect::SetVisible( bool bVisible )
	{
		m_bVisible = bVisible;
		SetShowOrHide(bVisible);
	}

	std::string CFreeEffect::GetKey() const
	{
		return m_strKey;
	}

	void CFreeEffect::SetKey( const std::string& strKey )
	{
		m_strKey = strKey;
	}

	void CFreeEffect::SetShowOrHide(bool blShow)
	{
		if(blShow)
		{
			this->setNodeMask(0xffffffff);
		}
		else
		{
			this->setNodeMask(0x00000000);
		}
	}

	bool CFreeEffect::StartEffect()
	{
		return true;
	}

	void CFreeEffect::SetType(unsigned int unType)
	{
		m_unType = unType;
	}

	unsigned int CFreeEffect::GetType() const
	{
		return m_unType;
	}

	void CFreeEffect::SetRadius( double dRadius )
	{
		m_dRadius = dRadius;
	}

	double CFreeEffect::GetRadius() const
	{
		return m_dRadius;
	}

	void CFreeEffect::SetCenter( osg::Vec3d vecCenter )
	{
		m_vecCenter = vecCenter;
	}

	osg::Vec3d CFreeEffect::GetCenter() const
	{
		return m_vecCenter;
	}

	void CFreeEffect::SetColor( osg::Vec4d vecColor )
	{
		m_vecColor = vecColor;
	}

	osg::Vec4d CFreeEffect::GetColor() const
	{
		return m_vecColor;
	}

	bool CFreeEffect::UpdataEffect()
	{
		return false;
	}

	bool CFreeEffect::ClearEffect()
	{
		return false;
	}

}

namespace FeEffect
{

	CExplodeEffect::CExplodeEffect()
		:CFreeEffect()
		,m_vecPos(0,0,0)
	{
		
	}

	CExplodeEffect::~CExplodeEffect()
	{

	}

	bool CExplodeEffect::CreateEffect()
	{
		osg::ref_ptr<osgParticle::ExplosionEffect> pExplosion = new osgParticle::ExplosionEffect(m_vecPos, 100.0f);

		return addChild(pExplosion.get());
	}

	void CExplodeEffect::SetPos( osg::Vec3d vecPos )
	{
		m_vecPos = vecPos;
	}


}

namespace FeEffect
{

	CShootEffect::CShootEffect()
	{

	}

	CShootEffect::~CShootEffect()
	{

	}

	bool CShootEffect::CreateEffect()
	{
		return true;
	}


}

namespace FeEffect
{

	CShootRangeEffect::CShootRangeEffect()
	{

	}

	CShootRangeEffect::~CShootRangeEffect()
	{

	}

	bool CShootRangeEffect::CreateEffect()
	{	
		osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
		osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
		double dRadius = 2000;
		osg::ref_ptr<osg::Vec3Array> pVecArray = new osg::Vec3Array;
		osg::Vec3 center = osg::Vec3(0.0,0.0,-sin(osg::DegreesToRadians(60.0))*2000.0);
		double alp, alp1, bet1;

		for (double n=10; n<=30; n+=10)
		{
			alp = osg::DegreesToRadians(n);
			alp1 = osg::DegreesToRadians(n+10);
			//for (double m=0; m<=360; m+=30)
			//{
			//	bet1 = osg::DegreesToRadians(m);
			//	pVecArray->push_back(center+osg::Vec3(dRadius*sinf(alp)*sinf(bet1), dRadius*sinf(alp)*cosf(bet1), dRadius*cosf(alp)));
			//}
			//for (double m=0; m<=360; m+=30)
			//{
			//	bet1 = osg::DegreesToRadians(m);
			//	pVecArray->push_back(center+osg::Vec3(dRadius*sinf(alp1)*sinf(bet1), dRadius*sinf(alp1)*cosf(bet1), dRadius*cosf(alp1)));
			//}
			for (double m=0; m<=360; m+=30)
			{
				bet1 = osg::DegreesToRadians(m);
				pVecArray->push_back(center+osg::Vec3(dRadius*sinf(alp)*sinf(bet1), dRadius*sinf(alp)*cosf(bet1), dRadius*cosf(alp)));
				pVecArray->push_back(center+osg::Vec3(dRadius*sinf(alp1)*sinf(bet1), dRadius*sinf(alp1)*cosf(bet1), dRadius*cosf(alp1)));
			}
		}

		pGeometry->setVertexArray(pVecArray.get());
		pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, pVecArray->size()));

		osg::Vec4Array *colorArray = new osg::Vec4Array;
		osg::Vec4 vecColor = osg::Vec4(0.0,1.0,0.0,0.3);
		colorArray->push_back(vecColor);
		pGeometry->setColorArray(colorArray);
		pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);


		pGeometry->setNormalArray(pVecArray);
		pGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

		pGeode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		pGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pGeode->addDrawable(pGeometry);

		addChild(pGeode.get());
	
		return true;
	}

}

namespace FeEffect
{
	CFreeRadarEffect::CFreeRadarEffect()
		:CFreeEffect()
		,m_dLoopTime(5.0)
		,m_dRadius(2000)
		,m_fAzMin(-10.0)
		,m_fAzMax(10.0)
		,m_fElevMin(0.0)
		,m_fElevMax(70)
	{
		m_vecCenter.set(0.0, 0.0, 0.0);
		m_rpRadarWaveGroup = new osg::Group();
	}

	CFreeRadarEffect::CFreeRadarEffect( const osg::Vec3& center, double dRadius, double dLoopTime ,
		float  azMin,  
		float  azMax,  
		float  elevMin,  
		float  elevMax
		)
		:CFreeEffect()
		,m_dLoopTime(dLoopTime)
		,m_dRadius(dRadius)
		,m_vecCenter(center)
		,m_fAzMin(azMin)
	    ,m_fAzMax(azMax)
	    ,m_fElevMin(elevMin)
		,m_fElevMax(elevMax)
	{	  
		m_rpRadarWaveGroup = new osg::Group();
	}

	CFreeRadarEffect::~CFreeRadarEffect()
	{

	}

	osg::Vec3d CFreeRadarEffect::GetCenter() const
	{
		return m_vecCenter;
	}

	void CFreeRadarEffect::SetCenter( const osg::Vec3d& center )
	{
		m_vecCenter = center;
	}

	double CFreeRadarEffect::GetRadius() const
	{
		return m_dRadius;
	}

	void CFreeRadarEffect::SetRadius( double dRadius )
	{
		m_dRadius = dRadius;
	}

	double CFreeRadarEffect::GetLoopTime() const
	{
		return m_dLoopTime;
	}

	void CFreeRadarEffect::SetLoopTime( double dLoopTime )
	{
		m_dLoopTime = dLoopTime;
	}

	////加入CallBack
	//bool CFreeRadarEffect::CreateEffect()
	//{
	//	FeUtil::AnimationPath* pAnimationPath = CreateRadarAnimationPath();
	//	if(!pAnimationPath) return false;

	//	osg::ref_ptr<osg::MatrixTransform> pRadarMT = new osg::MatrixTransform;
	//	addChild(pRadarMT.get());

	//	pRadarMT->setUpdateCallback(new FeUtil::AnimationPathCallback(pAnimationPath));
	//	pRadarMT->addChild(m_rpRadarWaveGroup);
	//	osg::ref_ptr<osgSim::SphereSegment> pSS = new osgSim::SphereSegment(
	//		m_vecCenter, 
	//		m_dRadius,
	//		osg::DegreesToRadians(m_fAzMin),
	//		osg::DegreesToRadians(m_fAzMax),
	//		osg::DegreesToRadians(m_fElevMin),
	//		osg::DegreesToRadians(m_fElevMax),
	//		60);

	//	pSS->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
	//	pSS->setAllColors(osg::Vec4(0.5f,1.0f,1.0f,0.5f));
	//	pSS->setSideColor(osg::Vec4(0.5f,1.0f,1.0f,0.1f));

	//	m_rpRadarWaveGroup->addChild(pSS.get());

	//	return true;
	//}

	//不加CallBack，如果需要动画，请用上面被注释掉的同名函数
	bool CFreeRadarEffect::CreateEffect()
	{
		addChild(m_rpRadarWaveGroup);
		osg::ref_ptr<osgSim::SphereSegment> pSS = new osgSim::SphereSegment(
			m_vecCenter, 
			m_dRadius,
			osg::DegreesToRadians(m_fAzMin),
			osg::DegreesToRadians(m_fAzMax),
			osg::DegreesToRadians(m_fElevMin),
			osg::DegreesToRadians(m_fElevMax),
			60);

		pSS->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		pSS->setAllColors(osg::Vec4(0.5f,1.0f,1.0f,0.5f));
		pSS->setSideColor(osg::Vec4(0.5f,1.0f,1.0f,0.1f));

		m_rpRadarWaveGroup->addChild(pSS.get());

		return true;
	}

	FeUtil::AnimationPath* CFreeRadarEffect::CreateRadarAnimationPath()
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
			osg::Vec3 position(m_vecCenter + osg::Vec3(sinf(yaw) ,cosf(yaw) , 0.0f));
			osg::Quat rotation(osg::Quat(-(yaw+osg::inDegrees(90.0f)), osg::Vec3(0.0,0.0,1.0)));

			pAnimationPath->insert(time,FeUtil::AnimationPath::ControlPoint(position,rotation));

			yaw += yaw_delta;
			time += time_delta;

		}
		return pAnimationPath;
	}

	float CFreeRadarEffect::GetAzMin() const
	{
		return m_fAzMin;
	}

	void CFreeRadarEffect::SetAzMin( float fAzMin )
	{
		m_fAzMin = fAzMin;
	}

	float CFreeRadarEffect::GetAzMax() const
	{
		return m_fAzMax;
	}

	void CFreeRadarEffect::SetAzMax( float fAzMax )
	{
		m_fAzMax = fAzMax;
	}

	float CFreeRadarEffect::GetElevMin() const
	{
		return m_fElevMin;
	}

	void CFreeRadarEffect::SetElevMin( float fElevMin )
	{
		m_fElevMin = fElevMin;
	}

	float CFreeRadarEffect::GetElevMax() const
	{
		return m_fElevMax;
	}

	void CFreeRadarEffect::SetElevMax( float fElevMax )
	{
		m_fElevMax = fElevMax;
	}

	bool CFreeRadarEffect::UpdataEffect()
	{
		m_rpRadarWaveGroup->removeChild(0,m_rpRadarWaveGroup->getNumChildren());
		osg::ref_ptr<osgSim::SphereSegment> pSS = new osgSim::SphereSegment(
			m_vecCenter, 
			m_dRadius,
			osg::DegreesToRadians(m_fAzMin),
			osg::DegreesToRadians(m_fAzMax),
			osg::DegreesToRadians(m_fElevMin),
			osg::DegreesToRadians(m_fElevMax),
			60);

		pSS->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		pSS->setAllColors(osg::Vec4(0.5f,1.0f,1.0f,0.5f));
		pSS->setSideColor(osg::Vec4(0.5f,1.0f,1.0f,0.1f));
		m_rpRadarWaveGroup->addChild(pSS.get());

		return true;
	}

}


namespace FeEffect
{

	CSelementEffect::CSelementEffect()
		:CFreeEffect()
	{

	}

	CSelementEffect::~CSelementEffect()
	{

	}

	bool CSelementEffect::CreateEffect()
	{	
		osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
		osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry;
		double dRadius = 10;
		osg::ref_ptr<osg::Vec3Array> pVecArray = new osg::Vec3Array;
		osg::Vec3 center = osg::Vec3(0.0,0.0,0.0);
		double bet1;


		for (double m=0; m<=360; m+=10)
		{
			bet1 = osg::DegreesToRadians(m);
			pVecArray->push_back(center+osg::Vec3(dRadius*sinf(bet1), dRadius*cosf(bet1), 1.0));
		}

		pGeometry->setVertexArray(pVecArray.get());
		pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, pVecArray->size()));

		osg::Vec4Array *colorArray = new osg::Vec4Array;
		osg::Vec4 vecColor = osg::Vec4(0.0,1.0,0.0,0.3);
		colorArray->push_back(vecColor);
		pGeometry->setColorArray(colorArray);
		pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);


		pGeometry->setNormalArray(pVecArray);
		pGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

		pGeode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		pGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

		osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth();
		pLineWidth->setWidth(10.0);
		pGeode->getOrCreateStateSet()->setAttributeAndModes(pLineWidth.get(), osg::StateAttribute::ON);

		pGeode->addDrawable(pGeometry);

		addChild(pGeode.get());

		return true;
	}

}

namespace FeEffect
{
	CFreeRadarTEffect::CFreeRadarTEffect()
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
	{
		m_vecCenter.set(0.0, 0.0, 0.0);
		m_rpRadarWaveGroup = new osg::Group();
	}

	CFreeRadarTEffect::CFreeRadarTEffect(FeUtil::CRenderContext* pContext, const osg::Vec3& center, double dRadius, double dLoopTime ,
		float  azMin,  
		float  azMax,  
		float  elevMin,  
		float  elevMax,
		osg::Vec4d color
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
	{	  
		m_rpRadarWaveGroup = new osg::Group();
	}

	CFreeRadarTEffect::~CFreeRadarTEffect()
	{

	}

	osg::Vec3d CFreeRadarTEffect::GetCenter() const
	{
		return m_vecCenter;
	}

	void CFreeRadarTEffect::SetCenter( const osg::Vec3d& center )
	{
		m_vecCenter = center;
	}

	double CFreeRadarTEffect::GetRadius() const
	{
		return m_dRadius;
	}

	void CFreeRadarTEffect::SetRadius( double dRadius )
	{
		m_dRadius = dRadius;
	}

	double CFreeRadarTEffect::GetLoopTime() const
	{
		return m_dLoopTime;
	}

	void CFreeRadarTEffect::SetLoopTime( double dLoopTime )
	{
		m_dLoopTime = dLoopTime;
	}

	////加入CallBack
	//bool CFreeRadarEffect::CreateEffect()
	//{
	//	FeUtil::AnimationPath* pAnimationPath = CreateRadarAnimationPath();
	//	if(!pAnimationPath) return false;

	//	osg::ref_ptr<osg::MatrixTransform> pRadarMT = new osg::MatrixTransform;
	//	addChild(pRadarMT.get());

	//	pRadarMT->setUpdateCallback(new FeUtil::AnimationPathCallback(pAnimationPath));
	//	pRadarMT->addChild(m_rpRadarWaveGroup);
	//	osg::ref_ptr<osgSim::SphereSegment> pSS = new osgSim::SphereSegment(
	//		m_vecCenter, 
	//		m_dRadius,
	//		osg::DegreesToRadians(m_fAzMin),
	//		osg::DegreesToRadians(m_fAzMax),
	//		osg::DegreesToRadians(m_fElevMin),
	//		osg::DegreesToRadians(m_fElevMax),
	//		60);

	//	pSS->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
	//	pSS->setAllColors(osg::Vec4(0.5f,1.0f,1.0f,0.5f));
	//	pSS->setSideColor(osg::Vec4(0.5f,1.0f,1.0f,0.1f));

	//	m_rpRadarWaveGroup->addChild(pSS.get());

	//	return true;
	//}

	//不加CallBack，如果需要动画，请用上面被注释掉的同名函数
	bool CFreeRadarTEffect::CreateEffect()
	{  
		m_pMT = new osg::MatrixTransform();
	    m_rpRadarWaveGroup->addChild(m_pMT);
	    osg::Matrix matrix;
	    FeUtil::DegreeLLH2Matrix(m_opContext.get(), m_vecCenter,matrix);
	    m_pMT->setMatrix(matrix);
	    addChild(m_rpRadarWaveGroup);
	    osg::ref_ptr<osgSim::SphereSegment> pSS = new osgSim::SphereSegment(
		osg::Vec3d(0,0,0), 
		m_dRadius,
		osg::DegreesToRadians(m_fAzMin),
		osg::DegreesToRadians(m_fAzMax),
		osg::DegreesToRadians(m_fElevMin),
		osg::DegreesToRadians(m_fElevMax),
		60);

		pSS->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		pSS->setAllColors(m_vecColor);
		pSS->setSideColor(m_vecColor);

		m_pMT->addChild(pSS.get());

		return true;
	}

	FeUtil::AnimationPath* CFreeRadarTEffect::CreateRadarAnimationPath()
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
			osg::Vec3 position(m_vecCenter + osg::Vec3(sinf(yaw) ,cosf(yaw) , 0.0f));
			osg::Quat rotation(osg::Quat(-(yaw+osg::inDegrees(90.0f)), osg::Vec3(0.0,0.0,1.0)));

			pAnimationPath->insert(time,FeUtil::AnimationPath::ControlPoint(position,rotation));

			yaw += yaw_delta;
			time += time_delta;

		}
		return pAnimationPath;
	}

	float CFreeRadarTEffect::GetAzMin() const
	{
		return m_fAzMin;
	}

	void CFreeRadarTEffect::SetAzMin( float fAzMin )
	{
		m_fAzMin = fAzMin;
	}

	float CFreeRadarTEffect::GetAzMax() const
	{
		return m_fAzMax;
	}

	void CFreeRadarTEffect::SetAzMax( float fAzMax )
	{
		m_fAzMax = fAzMax;
	}

	float CFreeRadarTEffect::GetElevMin() const
	{
		return m_fElevMin;
	}

	void CFreeRadarTEffect::SetElevMin( float fElevMin )
	{
		m_fElevMin = fElevMin;
	}

	float CFreeRadarTEffect::GetElevMax() const
	{
		return m_fElevMax;
	}

	void CFreeRadarTEffect::SetElevMax( float fElevMax )
	{
		m_fElevMax = fElevMax;
	}

	bool CFreeRadarTEffect::UpdataEffect()
	{
		/*m_rpRadarWaveGroup->removeChild(0,m_rpRadarWaveGroup->getNumChildren());
		osg::ref_ptr<osgSim::SphereSegment> pSS = new osgSim::SphereSegment(
		m_vecCenter, 
		m_dRadius,
		osg::DegreesToRadians(m_fAzMin),
		osg::DegreesToRadians(m_fAzMax),
		osg::DegreesToRadians(m_fElevMin),
		osg::DegreesToRadians(m_fElevMax),
		60);

		pSS->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		pSS->setAllColors(osg::Vec4(0.5f,1.0f,1.0f,0.5f));
		pSS->setSideColor(osg::Vec4(0.5f,1.0f,1.0f,0.1f));
		m_rpRadarWaveGroup->addChild(pSS.get());

		return true;*/


		m_pMT->removeChild(0,m_pMT->getNumChildren());
		osg::Matrix matrix;
		FeUtil::DegreeLLH2Matrix(m_opContext.get(), m_vecCenter,matrix);
		m_pMT->setMatrix(matrix);
		osg::ref_ptr<osgSim::SphereSegment> pSS = new osgSim::SphereSegment(
			osg::Vec3d(0,0,0), 
			m_dRadius,
			osg::DegreesToRadians(m_fAzMin),
			osg::DegreesToRadians(m_fAzMax),
			osg::DegreesToRadians(m_fElevMin),
			osg::DegreesToRadians(m_fElevMax),
			60);

		pSS->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		pSS->setAllColors(m_vecColor);
		pSS->setSideColor(m_vecColor);

		m_pMT->addChild(pSS.get());
		return true;
	}

	void CFreeRadarTEffect::SetColor( osg::Vec4d vecColor )
	{
		m_vecColor = vecColor;
	}

	osg::Vec4d CFreeRadarTEffect::GetColor() const
	{
		return m_vecColor;
	}

}
