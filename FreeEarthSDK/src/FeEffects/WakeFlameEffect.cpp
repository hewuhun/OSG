#include <osg/MatrixTransform>
#include <osg/Geometry>

#include <FeUtils/CoordConverter.h>

#include <FeEffects/WakeFlameEffect.h>
#include <FeUtils/PathRegistry.h>

namespace FeEffect
{
	CWakeFlameBase::CWakeFlameBase()
		:CFreeEffect()
		,m_rpMT(NULL)
		,m_rpWakeCallback(NULL)
	{
		m_rpMT = new osg::MatrixTransform;
		addChild(m_rpMT.get());
	}

	CWakeFlameBase::~CWakeFlameBase()
	{

	}

	bool CWakeFlameBase::CreateEffect()
	{
		return true;
	}

	void CWakeFlameBase::SetMT(osg::MatrixTransform *pMT)
	{
		//m_rpMT = new osg::MatrixTransform();
		if(!m_rpWakeCallback.valid())
		{
			m_rpWakeCallback = new CWakeFlameCallback(this,pMT);
		}

		
	}

	void CWakeFlameBase::AddWakeCallBack()
	{
		if(m_rpMT.valid() && m_rpWakeCallback.valid())
		{
			m_rpMT->addUpdateCallback(m_rpWakeCallback.get());
		}
	}

	void CWakeFlameBase::RemoveWakeCallBack()
	{
		if(m_rpMT.valid() && m_rpWakeCallback.valid())
		{
			m_rpMT->removeUpdateCallback(m_rpWakeCallback.get());
		}
	}

	bool CWakeFlameBase::StartEffect()
	{
		if(m_rpMT.valid() && m_rpWakeCallback.valid())
		{
			m_rpMT->setUpdateCallback(m_rpWakeCallback.get());
		}
		
		SetEffectEnable( true );

		return true;
	}

	bool CWakeFlameBase::ClearEffect()
	{
		if(m_rpMT.valid() )
		{
			m_rpMT->setUpdateCallback(NULL);
		}

		SetEffectEnable( false );

		return true;
	}

	void CWakeFlameBase::SetPos(osg::Vec3d vecPos)
	{

	}

	void CWakeFlameBase::SetEffectEnable( bool bEnable )
	{
		int nCount = getNumChildren();

		for(int n =0; n<nCount;n++)
		{
			CWakeFlameEffect*pWakeEffect = dynamic_cast<CWakeFlameEffect*>(getChild(n));
			if(pWakeEffect)
			{
				pWakeEffect->SetEmmitterEnable(bEnable);
			}
		}
	}


}

namespace FeEffect
{
	CWakeFlameEffect::CWakeFlameEffect()
		:CFreeEffect()
		,m_vecPos(osg::Vec3d(0.0,0.0,0.0))
		,m_rpPSU(NULL)
		,m_rpPS(NULL)
		,m_rpRShooter(NULL)
		,m_rpProgram(NULL)
		,m_rpPlacer(NULL)
		,m_rpEmmitter(NULL)
		,m_rpCounter(NULL)
		,m_dTime(0.3)
		, m_dBegin(1.3)
		, m_dEnd(0.8)
	{

	}

	CWakeFlameEffect::~CWakeFlameEffect()
	{
		m_rpPSU = NULL;
		m_rpPS =NULL;
		m_rpRShooter = NULL;
		m_rpProgram=NULL;
		m_rpPlacer=NULL;
		m_rpEmmitter=NULL;
		m_rpCounter=NULL;
	}

	bool CWakeFlameEffect::CreateEffect()
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
		osg::ref_ptr<osg::Geode> gnode = new osg::Geode;
		gnode->addDrawable(m_rpPS);
		addChild(gnode);

		//创建粒子对象
		//osgParticle::Particle ptemplate;
		//将对象设为粒子系统缺省的粒子对象
		m_rpPS->setDefaultParticleTemplate(m_ParticleTemplate);
		//设置生命周期-秒
		m_ParticleTemplate.setLifeTime(m_dTime);
		//设置粒子大小变化范围-米
		m_ParticleTemplate.setSizeRange(osgParticle::rangef(m_dBegin, m_dEnd));
		//设置粒子颜色变化范围
		m_ParticleTemplate.setColorRange(osgParticle::rangev4(osg::Vec4(1.0, 0.5f, 0.0f, 0.3f),osg::Vec4(0.9882f, 0.7490f, 0.5882f, 0.1f)));
		//设置半径
		m_ParticleTemplate.setRadius(0.1);
		//设置重量-千克
		m_ParticleTemplate.setMass(0.1);

		// 创建标准放射极对象
		if(!m_rpEmmitter.valid())
		{
			m_rpEmmitter = new osgParticle::ModularEmitter;
		}
		
		m_rpEmmitter->setReferenceFrame(osgParticle::ParticleProcessor::ABSOLUTE_RF);
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
		m_rpRShooter->setInitialSpeedRange(-50, -40.0);
		//与z轴夹角范围
		m_rpRShooter->setThetaRange(osg::DegreesToRadians(90.0),osg::DegreesToRadians(90.0));
		//与xy的夹角范围
		m_rpRShooter->setPhiRange(osg::DegreesToRadians(90.0),osg::DegreesToRadians(90.0)); 

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

	void CWakeFlameEffect::SetPos( osg::Vec3d vecPos )
	{
		m_vecPos = vecPos;
	}

	osg::Vec3d CWakeFlameEffect::GetPos( )
	{
		return m_vecPos;
	}

	void CWakeFlameEffect::SetWakeAbsoluteCenter(osg::Vec3d vecPos)
	{
		if(m_rpPlacer.valid())
		{
			m_rpPlacer->setCenter(vecPos);
		}
	}

	void CWakeFlameEffect::SetAngle(double dTheta,double dPhi)
	{
		if(m_rpRShooter.valid())
		{
			m_rpRShooter->setThetaRange(dTheta,dTheta);
			m_rpRShooter->setPhiRange(dPhi,dPhi); 
		}
		
	}

	void CWakeFlameEffect::SetShooterSpeedRange(double dSpeed)
	{
		if(m_rpRShooter.valid())
		{
			m_rpRShooter->setInitialSpeedRange(dSpeed,dSpeed);
		}
		
	}

	void CWakeFlameEffect::SetParticleNum(int nNum)
	{
		if(m_rpCounter.valid())
		{
			m_rpCounter->setRateRange(nNum,nNum);
		}
		
	}

	void CWakeFlameEffect::SetLifeTime(double dTime)
	{
		m_dTime = dTime; 
	//	m_ParticleTemplate.setLifeTime(dTime);
	}

	void CWakeFlameEffect::UpdatePosAndAngle(osg::Matrix matrix,double dTheta,double dPhi)
	{
		osg::Vec3d vecpos = m_vecPos*matrix;
		SetWakeAbsoluteCenter( vecpos );
		SetAngle(dTheta,dPhi);
	}

	void CWakeFlameEffect::SetSizeRange(double dBegin,double dEnd)
	{
		//设置粒子大小变化范围-米
		m_dBegin = dBegin;
		m_dEnd = dEnd;
		//m_ParticleTemplate.setSizeRange(osgParticle::rangef(dBegin, dEnd));
	}
	
	void CWakeFlameEffect::SetColorRange(osg::Vec4d VecBegin,osg::Vec4d VecEnd)
	{
		m_ParticleTemplate.setColorRange((osgParticle::rangev4(VecBegin,VecEnd)));
	}

	void CWakeFlameEffect::SetReferenceFrame(int nType)
	{
		if(m_rpEmmitter.valid())
		{
			m_rpEmmitter->setReferenceFrame((osgParticle::ParticleProcessor::ReferenceFrame)(nType));
		}
	}

	void CWakeFlameEffect::SetEmmitterEnable( bool bEnable )
	{
		if(m_rpEmmitter.valid())
		{
			m_rpEmmitter->setEnabled(bEnable);
		}
	}

	void CWakeFlameEffect::UpdataFlameEffect()
	{
		ClearEffect();
		CreateEffect();
		return;
	}

	bool CWakeFlameEffect::ClearEffect()
	{
		removeChildren(0,getNumChildren());

		m_rpEmmitter = NULL;
		m_rpPSU  = NULL;
		m_rpPS = NULL;

		return false;
	}

}


namespace FeEffect
{

	CWakeFlameCallback::CWakeFlameCallback( CWakeFlameBase *pWakeBase,osg::MatrixTransform* pMT)
		:osg::NodeCallback()
		,m_pWakeBase(pWakeBase)
		,m_vecWorld(0.0,0.0,0.0)
		,m_pMT(pMT)
	{
		
	}

	CWakeFlameCallback::~CWakeFlameCallback()
	{

	}

	void CWakeFlameCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		osg::Vec3d vecpos(1.0,1.0,1.0);
		osg::Vec3d vecWorld(0.0,0.0,0.0);

		double dTheta = 0.0;
		double dPhi = 0.0;

		osg::Matrix matrix;

		osg::MatrixTransform *pMT = m_pMT/*dynamic_cast<osg::MatrixTransform *>(node)*/;

		if(pMT)
		{
			matrix = pMT->getMatrix();
			vecWorld =matrix.getTrans();
			vecpos = vecWorld-m_vecWorld;//前后两帧向量差
			m_vecWorld = vecWorld;

			vecpos.normalize();

			if((vecpos.x() > -0.00000001 && vecpos.x()<0.00000001) 
				&& (vecpos.x() > -0.00000001 && vecpos.x()<0.00000001)
				&&(vecpos.x() > -0.00000001 && vecpos.x()<0.00000001))
			{
				osg::NodeCallback::traverse(node,nv);
				return;
			}

			if(vecpos.x() > -0.00000001 && vecpos.x()<0.00000001)
			{
				dPhi = 0.0;
			}
			else
			{
				dPhi = atan(vecpos.y()/vecpos.x());
			}

			dTheta = acos(vecpos.z());

			if((vecpos.y() < -0.000001 && vecpos.x() <-0.0000001)||(vecpos.x() < -0.000001 && vecpos.y() >0.0000001))
			{
				dPhi -= osg::PI;
			}

		}

		int nCount = m_pWakeBase->getNumChildren();

		for(int n =0; n<nCount;n++)
		{
			CWakeFlameEffect*pWakeEffect = dynamic_cast<CWakeFlameEffect*>(m_pWakeBase->getChild(n));
			if(pWakeEffect)
			{

				pWakeEffect->UpdatePosAndAngle(matrix,dTheta,dPhi);
			}
		}

		osg::NodeCallback::traverse(node,nv);
	}

}