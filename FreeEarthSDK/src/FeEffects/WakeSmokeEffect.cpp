#include <osg/MatrixTransform>
#include <osg/Geometry>

#include <FeUtils/CoordConverter.h>

#include <FeEffects/WakeSmokeEffect.h>
#include <FeUtils/PathRegistry.h>

namespace FeEffect
{
	CWakeSmokeEffect::CWakeSmokeEffect()
		:CFreeEffect()
		,m_vecPos(osg::Vec3d(0.0,0.0,0.0))
		,m_rpPSU(NULL)
		,m_rpPS(NULL)
		,m_rpRShooter(NULL)
		,m_rpProgram(NULL)
		,m_rpPlacer(NULL)
		,m_rpEmmitter(NULL)
		,m_rpCounter(NULL)
		,m_rpGeode(NULL)
		,m_dTime(0.3)
		, m_dBegin(0.2)
		, m_dEnd(8.0)
		, m_vecBegin(1.0, 1.0, 1.0, 0.47843137)
		, m_vecEnd(0.0, 0.0, 0.0, 0.094117649)
	{

	}

	CWakeSmokeEffect::~CWakeSmokeEffect()
	{
		m_rpPSU = NULL;
		m_rpPS =NULL;
		m_rpRShooter = NULL;
		m_rpProgram=NULL;
		m_rpPlacer=NULL;
		m_rpEmmitter=NULL;
		m_rpCounter=NULL;
	}

	bool CWakeSmokeEffect::CreateEffect()
	{
		// 添加更新器，以实现每帧的粒子管理
		if(!m_rpPSU.valid())
		{
			m_rpPSU = new osgParticle::ParticleSystemUpdater;
		}
		
		// 将更新器节点添加到场景中
		if(!this->containsNode(m_rpPSU.get()))
		{
			addChild(m_rpPSU);
		}

		// 创建并初始化粒子系统
		if(!m_rpPS.valid())
		{
			m_rpPS = new osgParticle::ParticleSystem;
		}
		
		// 将更新器与粒子系统对象关联
		if(!m_rpPSU->containsParticleSystem(m_rpPS.get()))
		{
			m_rpPSU->addParticleSystem(m_rpPS);
		}

		// 设置材质，是否放射粒子，以及是否使用光照
		m_rpPS->setDefaultAttributes(FeFileReg->GetFullPath("texture/effects/smoke.rgb"), false, false);
		//由于粒子系统类继承自Drawable类，因此我们可以将其作为Geode的子节点加入场景
		m_rpGeode = new osg::Geode;
		m_rpGeode->addDrawable(m_rpPS);
		addChild(m_rpGeode.get());

		//创建粒子对象
		//osgParticle::Particle ptemplate;
		//将对象设为粒子系统缺省的粒子对象
		m_rpPS->setDefaultParticleTemplate(m_ParticleTemplate);
		//设置生命周期-秒
		m_ParticleTemplate.setLifeTime(m_dTime);
		//设置粒子大小变化范围-米
		m_ParticleTemplate.setSizeRange(osgParticle::rangef(m_dBegin,m_dEnd));
		//设置粒子颜色变化范围
		//m_ParticleTemplate.setColorRange(osgParticle::rangev4(osg::Vec4(1.0, 1.0, 1.0, 0.47843137), osg::Vec4(0.0, 0.0, 0.0, 0.094117649)));
		m_ParticleTemplate.setColorRange(osgParticle::rangev4(m_vecBegin, m_vecEnd));
		//设置半径
		m_ParticleTemplate.setRadius(0.1);
		//设置重量-千克
		m_ParticleTemplate.setMass(0.1);

		// 创建标准放射极对象
		if(!m_rpEmmitter.valid())
		{
			m_rpEmmitter = new osgParticle::ModularEmitter;
		}
		
		m_rpEmmitter->setReferenceFrame(osgParticle::ParticleProcessor::RELATIVE_RF);
		addChild(m_rpEmmitter);
		m_rpEmmitter->setParticleTemplate(m_ParticleTemplate);
		m_rpEmmitter->setParticleSystem(m_rpPS);

		//弧度发射器
		if(!m_rpRShooter.valid())
		{
			m_rpRShooter = new osgParticle::RadialShooter;
		}
		
		m_rpEmmitter->setShooter(m_rpRShooter.get());
		//发射速度
		m_rpRShooter->setInitialSpeedRange(-35.0, -30.0);

		//与z轴夹角范围
		m_rpRShooter->setThetaRange(osg::DegreesToRadians(90.0), osg::DegreesToRadians(90.0));
		//与xy的夹角范围
		m_rpRShooter->setPhiRange(osg::DegreesToRadians(90.0), osg::DegreesToRadians(90.0));
		//点放置器
		if(!m_rpPlacer.valid())
		{
			m_rpPlacer = new osgParticle::PointPlacer;
		}
		
		m_rpEmmitter->setPlacer(m_rpPlacer);
		m_rpPlacer->setCenter(m_vecPos);

		// 创建标准编程器对象并与粒子系统相关联，控制粒子在生命周期中的更新
		if(!m_rpProgram.valid())
		{
			m_rpProgram = new osgParticle::ModularProgram;
		}
		
		m_rpProgram->setParticleSystem(m_rpPS);

		// 获取放射极中缺省计数器的句柄，调整每帧增加的新粒子数目
		m_rpCounter = static_cast<osgParticle::RandomRateCounter*>(m_rpEmmitter->getCounter());
		// 每秒新生成新粒子范围
		m_rpCounter->setRateRange(200,250);
		//关联计数器
		m_rpEmmitter->setCounter(m_rpCounter.get());

		return true;
	}


	void CWakeSmokeEffect::SetPos( osg::Vec3d vecPos )
	{
		SetCenter(vecPos);
		m_vecPos = vecPos;

	}

	osg::Vec3d CWakeSmokeEffect::GetPos( )
	{
		return m_vecPos;
	}

	void CWakeSmokeEffect::SetWakeAbsoluteCenter(osg::Vec3d vecPos)
	{
		if(m_rpPlacer.valid())
		{
			m_rpPlacer->setCenter(vecPos);
		}
	}

	void CWakeSmokeEffect::SetAngle(double dTheta,double dPhi)
	{
		if(m_rpRShooter.valid())
		{
			m_rpRShooter->setThetaRange(dTheta,dTheta);
			m_rpRShooter->setPhiRange(dPhi,dPhi); 
		}
		
	}

	void CWakeSmokeEffect::SetShooterSpeedRange(double dSpeed)
	{
		if(m_rpRShooter.valid())
		{
			m_rpRShooter->setInitialSpeedRange(dSpeed,dSpeed);
		}
		
	}

	void CWakeSmokeEffect::SetParticleNum(int nNum)
	{
		if(m_rpCounter.valid())
		{
			m_rpCounter->setRateRange(nNum,nNum);
		}
		
	}

	void CWakeSmokeEffect::SetLifeTime(double dTime)
	{
		m_dTime = dTime; 
		//m_ParticleTemplate.setLifeTime(dTime);
	}

	void CWakeSmokeEffect::UpdatePosAndAngle(osg::Matrix matrix,double dTheta,double dPhi)
	{
		osg::Vec3d vecpos = m_vecPos*matrix;
		SetWakeAbsoluteCenter( vecpos );
		SetAngle(dTheta,dPhi);
	}

	void CWakeSmokeEffect::SetSizeRange(double dBegin,double dEnd)
	{
		//设置粒子大小变化范围-米
		m_dBegin = dBegin;
		m_dEnd = dEnd;
	//	m_ParticleTemplate.setSizeRange(osgParticle::rangef(dBegin, dEnd));
	}
	
	void CWakeSmokeEffect::SetColorRange(osg::Vec4d VecBegin,osg::Vec4d VecEnd)
	{
		//m_ParticleTemplate.setColorRange((osgParticle::rangev4(VecBegin,VecEnd)));
		m_vecBegin = VecBegin;
		m_vecEnd = VecEnd;
	}

	void CWakeSmokeEffect::SetReferenceFrame(int nType)
	{
		if(m_rpEmmitter.valid())
		{
			m_rpEmmitter->setReferenceFrame((osgParticle::ParticleProcessor::ReferenceFrame)(nType));
		}
	}

	bool CWakeSmokeEffect::UpdataEffect()
	{
		ClearEffect();
		CreateEffect();
		return true;
	}

	bool  CWakeSmokeEffect::ClearEffect()
	{
		//m_rpEmmitter->setEnabled(false);
		//m_rpPSU->removeParticleSystem(m_rpPS.get());
		removeChildren(0,getNumChildren());

		m_rpEmmitter = NULL;
		m_rpPSU  = NULL;
		m_rpPS = NULL;

		 return false;
	}

	osg::Geode *CWakeSmokeEffect::GetSmokeGeode()
	{
		return m_rpGeode.get();
	}

	void  CWakeSmokeEffect::SetGeoTileScale(osg:: Vec3d v)
	{
		//m_mxScale.makeScale(v);
	}

	void  CWakeSmokeEffect::SetGeoTileRotate( double angle, const osg::Vec3f axis )
	{
/*		m_pRotateMT->setMatrix(osg::Matrix::rotate(angle,axis));*/
	}

	void  CWakeSmokeEffect::SetGeoTileTrans( osg::Vec3d v)
	{
/*		m_pTransMT->setMatrix(osg::Matrix::translate(v));*/
	}

}


//namespace FeEffect
//{
//
//	CWakeFlameCallback::CWakeFlameCallback()
//		:osg::NodeCallback()
//	{
//
//	}
//
//	CWakeFlameCallback::~CWakeFlameCallback()
//	{
//
//	}
//
//	void CWakeFlameCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
//	{
//		osg::NodeCallback::traverse(node,nv);
//	}
//
//}