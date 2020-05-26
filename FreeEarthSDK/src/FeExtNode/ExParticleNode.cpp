#include <FeExtNode/ExParticleNode.h>

#include <osgEarth/XmlUtils>
#include <osgDB/Options>
#include <osgEarth/Registry>
#include <osgGA/EventVisitor>
#include <osgViewer/Viewer>
#include <osgEarth/Picker>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/StrUtil.h>

//粒子
#include <osg/Geode>
#include <osgParticle/Particle>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/SectorPlacer>
#include <osgParticle/RadialShooter>
#include <osgParticle/AccelOperator>
#include <osgParticle/FluidFrictionOperator>
#include <osgParticle/AngularAccelOperator>
#include <osgParticle/AngularDampingOperator>
#include <osgParticle/BounceOperator>
#include <osgParticle/DampingOperator>
#include <osgParticle/ExplosionOperator>
#include <osgParticle/ForceOperator>
#include <osgParticle/OrbitOperator>
#include <osgParticle/SinkOperator>

#include <iostream>
#include <fstream>

namespace FeExtNode
{
	ParticleNodeEffect::ParticleNodeEffect( ST_PARTICLE_INFO sParticleInfo,bool automaticSetup/*=true*/)
		:osgParticle::ParticleEffect(automaticSetup)
	{
		memcpy( (void*)&m_sParticleInfo,&sParticleInfo,sizeof(ST_PARTICLE_INFO));

		//设置默认
		setDefaults();

		if (_automaticSetup)
		{
			buildEffect();
		}
	}

	void ParticleNodeEffect::AddDomianOpertor(osgParticle::DomainOperator* op,ST_EFFECT_OPITION_DOMIAN sDomian,unsigned int uiCount)
	{
		for ( unsigned int i = 1; i <= uiCount; ++i )
		{
			if ( sDomian.bPoint )
			{   
				if ( sDomian.sPoint.unNO == i )
				{
					op->addPointDomain(osg::Vec3(sDomian.sPoint.s3F.fX,sDomian.sPoint.s3F.fY,sDomian.sPoint.s3F.fZ));
					continue;
				}
			}

			if ( sDomian.bLine )
			{
				if ( sDomian.sLine.unNO == i )
				{
					op->addLineSegmentDomain( osg::Vec3(sDomian.sLine.s3FV1.fX,sDomian.sLine.s3FV1.fY,sDomian.sLine.s3FV1.fZ),
						osg::Vec3(sDomian.sLine.s3FV1.fX,sDomian.sLine.s3FV1.fY,sDomian.sLine.s3FV1.fZ));
					continue;
				}

			}

			if ( sDomian.bTri )
			{
				if ( sDomian.sTri.unNO == i )
				{
					op->addTriangleDomain( osg::Vec3(sDomian.sTri.s3FV1.fX,sDomian.sTri.s3FV1.fY,sDomian.sTri.s3FV1.fZ),
						osg::Vec3(sDomian.sTri.s3FV2.fX,sDomian.sTri.s3FV2.fY,sDomian.sTri.s3FV2.fZ),
						osg::Vec3(sDomian.sTri.s3FV3.fX,sDomian.sTri.s3FV3.fY,sDomian.sTri.s3FV3.fZ)
						);
					continue;
				}

			}

			if ( sDomian.bRect )
			{
				if ( sDomian.sRect.unNO == i )
				{
					op->addRectangleDomain( osg::Vec3(sDomian.sRect.s3FV1.fX,sDomian.sRect.s3FV1.fY,sDomian.sRect.s3FV1.fZ),
						osg::Vec3(sDomian.sRect.s3FV2.fX,sDomian.sRect.s3FV2.fY,sDomian.sRect.s3FV2.fZ),
						osg::Vec3(sDomian.sRect.s3FV3.fX,sDomian.sRect.s3FV3.fY,sDomian.sRect.s3FV3.fZ)
						);
					continue;
				}

			}

			if ( sDomian.bPlane )
			{
				if ( sDomian.sPlane.unNO == i )
				{
					op->addPlaneDomain(osg::Plane( osg::Vec3( sDomian.sPlane.s3FV1.fX,sDomian.sPlane.s3FV1.fY,sDomian.sPlane.s3FV1.fZ),sDomian.sPlane.fRadus));
					continue;
				}

			}

			if ( sDomian.bSphere )
			{
				if ( sDomian.sSphere.unNO == i )
				{
					op->addSphereDomain( osg::Vec3(sDomian.sSphere.s3FV1.fX,sDomian.sSphere.s3FV1.fY,sDomian.sSphere.s3FV1.fZ),sDomian.sSphere.fRadus );
					continue;
				}

			}

			if ( sDomian.bBox )
			{
				if ( sDomian.stBox.unNO == i )
				{
					op->addBoxDomain( osg::Vec3(sDomian.stBox.s3FV1.fX,sDomian.stBox.s3FV1.fY,sDomian.stBox.s3FV1.fZ)
						,osg::Vec3(sDomian.stBox.s3FV2.fX,sDomian.stBox.s3FV2.fY,sDomian.stBox.s3FV2.fZ));
					continue;
				}

			}

			if ( sDomian.bDisk )
			{
				if ( sDomian.sDisk.unNO == i )
				{
					op->addDiskDomain( osg::Vec3(sDomian.sDisk.s3FV1.fX,sDomian.sDisk.s3FV1.fY,sDomian.sDisk.s3FV1.fZ)
						,osg::Vec3(sDomian.sDisk.s3FV2.fX,sDomian.sDisk.s3FV2.fY,sDomian.sDisk.s3FV2.fZ)
						,sDomian.sDisk.fMin,sDomian.sDisk.fMax );
					continue;
				}

			}
		}
	}

	void ParticleNodeEffect::setDefaults()
	{
		osgParticle::ParticleEffect::setDefaults();
		
		//默认使用相对帧
		_useLocalParticleSystem = false;
		_textureFileName = m_sParticleInfo.szTexture;
		_emitterDuration = m_sParticleInfo.sShootOpt.fLife;

		// set up unit particle.
		_defaultParticleTemplate.setLifeTime(m_sParticleInfo.sParticleOpt.fLife);


		//粒子大小
		_defaultParticleTemplate.setSizeRange(osgParticle::rangef(m_sParticleInfo.sParticleOpt.fSizeStart*_scale, m_sParticleInfo.sParticleOpt.fSizeEnd*_scale));

		//粒子透明度
		_defaultParticleTemplate.setAlphaRange(osgParticle::rangef(m_sParticleInfo.sColorOpt.fBeginAlpha, m_sParticleInfo.sColorOpt.fEndAlpha));

		//粒子颜色
		_defaultParticleTemplate.setColorRange(osgParticle::rangev4(
			osg::Vec4(m_sParticleInfo.sColorOpt.fBeginR, m_sParticleInfo.sColorOpt.fBeginG, m_sParticleInfo.sColorOpt.fBeginB, m_sParticleInfo.sColorOpt.fBeginAlpha), 
			osg::Vec4(m_sParticleInfo.sColorOpt.fEndR, m_sParticleInfo.sColorOpt.fEndG, m_sParticleInfo.sColorOpt.fEndB, m_sParticleInfo.sColorOpt.fEndAlpha)));

		//粒子半径
		_defaultParticleTemplate.setRadius(m_sParticleInfo.sParticleOpt.fRadius*_scale);

		//粒子质量
		_defaultParticleTemplate.setMass(m_sParticleInfo.sParticleOpt.fMass*_scale);

		//角度
		_defaultParticleTemplate.setAngle(osg::Vec3(
			osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngle.fX),
			osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngle.fY),
			osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngle.fZ)));

		//速度
		_defaultParticleTemplate.setVelocity(osg::Vec3(osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sSpeed.fX),
			osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sSpeed.fY),
			osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sSpeed.fZ)));

		//角加速度
		_defaultParticleTemplate.setAngularVelocity(osg::Vec3( osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngular.fX),
			osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngular.fY),
			osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngular.fZ)));


		//深度
		_defaultParticleTemplate.setDepth(m_sParticleInfo.sParticleOpt.fDepth);

	}

	void ParticleNodeEffect::setUpEmitterAndProgram()
	{
		// 设置粒子系统
		if (!_particleSystem)
		{
			_particleSystem = new osgParticle::ParticleSystem;
		}

		//粒子生命
		osgParticle::Particle pexplosion;

		if (_particleSystem.valid())
		{
			//设置纹理
			//setTextureFileName( m_sParticleInfo.szTexture);

			_particleSystem->setDefaultAttributes(_textureFileName, false, false);

			osgParticle::Particle& pexplosion = _particleSystem->getDefaultParticleTemplate();

			pexplosion.setLifeTime(m_sParticleInfo.sParticleOpt.fLife);

			//粒子大小
			pexplosion.setSizeRange(osgParticle::rangef(m_sParticleInfo.sParticleOpt.fSizeStart*_scale, m_sParticleInfo.sParticleOpt.fSizeEnd*_scale));

			//粒子透明度
			pexplosion.setAlphaRange(osgParticle::rangef(m_sParticleInfo.sColorOpt.fBeginAlpha, m_sParticleInfo.sColorOpt.fEndAlpha));

			//粒子颜色
			pexplosion.setColorRange(osgParticle::rangev4(
				osg::Vec4(m_sParticleInfo.sColorOpt.fBeginR, m_sParticleInfo.sColorOpt.fBeginG, m_sParticleInfo.sColorOpt.fBeginB, m_sParticleInfo.sColorOpt.fBeginAlpha), 
				osg::Vec4(m_sParticleInfo.sColorOpt.fEndR, m_sParticleInfo.sColorOpt.fEndG, m_sParticleInfo.sColorOpt.fEndB, m_sParticleInfo.sColorOpt.fEndAlpha)));

			//粒子半径
			pexplosion.setRadius(m_sParticleInfo.sParticleOpt.fRadius*_scale);

			//粒子质量
			pexplosion.setMass(m_sParticleInfo.sParticleOpt.fMass*_scale);

			//角度
			pexplosion.setAngle( osg::Vec3(osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngle.fX),
				osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngle.fY),
				osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngle.fZ)));

			//速度
			pexplosion.setVelocity(osg::Vec3(osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sSpeed.fX),
				osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sSpeed.fY),
				osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sSpeed.fZ)));

			//角加速度
			pexplosion.setAngularVelocity(osg::Vec3( osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngular.fX),
				osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngular.fY),
				osg::DegreesToRadians(m_sParticleInfo.sParticleOpt.sAngular.fZ)));


			//深度
			pexplosion.setDepth(m_sParticleInfo.sParticleOpt.fDepth);

		}

		// set up emitter
		if (!_emitter)
		{
			_emitter = new osgParticle::ModularEmitter;
			_emitter->setCounter(new osgParticle::RandomRateCounter);
			_emitter->setPlacer(new osgParticle::SectorPlacer);
			_emitter->setShooter(new osgParticle::RadialShooter);

		}

		if (_emitter.valid())
		{
			_emitter->setParticleSystem(_particleSystem.get());
			_emitter->setReferenceFrame(_useLocalParticleSystem?
				osgParticle::ParticleProcessor::ABSOLUTE_RF:
			osgParticle::ParticleProcessor::RELATIVE_RF);

			//_emitter->setParticleTemplate(pexplosion);

			//设置粒子启动发射时间
			_emitter->setStartTime(m_sParticleInfo.sShootOpt.fStartTime);
			_emitter->setLifeTime(m_sParticleInfo.sShootOpt.fLife);

			// 计数器
			osgParticle::RandomRateCounter* counter = dynamic_cast<osgParticle::RandomRateCounter*>(_emitter->getCounter());
			//设置发射粒子 浮动值
			counter->setRateRange(m_sParticleInfo.sShootOpt.fCountMin, m_sParticleInfo.sShootOpt.fCountMax);


			// 设置放置器
			osgParticle::SectorPlacer* placer = dynamic_cast<osgParticle::SectorPlacer*>(_emitter->getPlacer());
			//设置中心原点
			placer->setCenter(m_sParticleInfo.sParticleOpt.fX, m_sParticleInfo.sParticleOpt.fY, m_sParticleInfo.sParticleOpt.fZ);
			//设置位置圆半径
			placer->setRadiusRange(m_sParticleInfo.sShootOpt.fRadiusMin*_scale, m_sParticleInfo.sShootOpt.fRadiusMax*_scale);
			//设置粒子初始中心角
			double dMin = osg::DegreesToRadians( m_sParticleInfo.sShootOpt.fPhiMin);
			double dMax = osg::DegreesToRadians( m_sParticleInfo.sShootOpt.fPhiMax);
			placer->setPhiRange(dMin, dMax);    // 360?angle to make a circle
			
			// 创建发射器
			osgParticle::RadialShooter* shooter = dynamic_cast<osgParticle::RadialShooter*>(_emitter->getShooter());
			//设置初始速度
			shooter->setInitialSpeedRange(m_sParticleInfo.sShootOpt.fSpeedMin*_scale, m_sParticleInfo.sShootOpt.fSpeedMax*_scale);
			//设置航向角
			shooter->setPhiRange( osg::DegreesToRadians(m_sParticleInfo.sShootOpt.fPhiRangeMin),osg::DegreesToRadians(m_sParticleInfo.sShootOpt.fPhiRangeMax));
			shooter->setThetaRange( osg::DegreesToRadians(m_sParticleInfo.sShootOpt.fThetaRangeMin),osg::DegreesToRadians(m_sParticleInfo.sShootOpt.fThetaRangeMax));
			// 设置旋转角度浮动值
			shooter->setInitialRotationalSpeedRange(osgParticle::rangev3(
				osg::Vec3(m_sParticleInfo.sShootOpt.fRotateBeginX, m_sParticleInfo.sShootOpt.fRotateBeginY, m_sParticleInfo.sShootOpt.fRotateBeginZ),
				osg::Vec3(m_sParticleInfo.sShootOpt.fRotateEndX, m_sParticleInfo.sShootOpt.fRotateEndY, m_sParticleInfo.sShootOpt.fRotateEndZ)));

		}

		// 创建编程器
		if (!_program)
		{
			_program = new osgParticle::ModularProgram;
		}

		if (_program.valid())
		{
			_program->setParticleSystem(_particleSystem.get());

			// 创建加速度 操作器
			if ( m_sParticleInfo.sEffectOpt.bAccel )
			{
				float fX = m_sParticleInfo.sEffectOpt.sAccel.s3F.fX;
				float fY = m_sParticleInfo.sEffectOpt.sAccel.s3F.fY;
				float fZ = m_sParticleInfo.sEffectOpt.sAccel.s3F.fZ;
				osgParticle::AccelOperator *op = new osgParticle::AccelOperator;
				op->setAcceleration(osg::Vec3(fX, fY, fZ));
				op->setToGravity(m_sParticleInfo.sEffectOpt.sAccel.fGravity);

				_program->addOperator(op);  
			}

			//角速度 操作器  
			if ( m_sParticleInfo.sEffectOpt.bAngularAceel )
			{
				float fX = m_sParticleInfo.sEffectOpt.sAngularAceel.s3F.fX;
				float fY = m_sParticleInfo.sEffectOpt.sAngularAceel.s3F.fY;
				float fZ = m_sParticleInfo.sEffectOpt.sAngularAceel.s3F.fZ;
				osgParticle::AngularAccelOperator *op = new osgParticle::AngularAccelOperator;
				op->setAngularAcceleration(osg::Vec3(fX, fY, fZ));
				_program->addOperator(op);  
			}

			//角速度阻尼 操作器 
			if ( m_sParticleInfo.sEffectOpt.bAngularAccelDamping )
			{
				float fX = m_sParticleInfo.sEffectOpt.sAngularDamping.s3F.fX;
				float fY = m_sParticleInfo.sEffectOpt.sAngularDamping.s3F.fY;
				float fZ = m_sParticleInfo.sEffectOpt.sAngularDamping.s3F.fZ;
				osgParticle::AngularDampingOperator *op = new osgParticle::AngularDampingOperator;
				op->setDamping(osg::Vec3(fX, fY, fZ));
				op->setCutoffLow( m_sParticleInfo.sEffectOpt.sAngularDamping.fLow );
				op->setCutoffHigh( m_sParticleInfo.sEffectOpt.sAngularDamping.fHeight );
				_program->addOperator(op);  
			}

			//阻尼 操作器  
			if ( m_sParticleInfo.sEffectOpt.bDamping )
			{
				float fX = m_sParticleInfo.sEffectOpt.sDamping.s3F.fX;
				float fY = m_sParticleInfo.sEffectOpt.sDamping.s3F.fY;
				float fZ = m_sParticleInfo.sEffectOpt.sDamping.s3F.fZ;
				osgParticle::DampingOperator *op = new osgParticle::DampingOperator;
				op->setDamping(osg::Vec3(fX, fY, fZ));
				op->setCutoffLow( m_sParticleInfo.sEffectOpt.sAngularDamping.fLow );
				op->setCutoffHigh( m_sParticleInfo.sEffectOpt.sAngularDamping.fHeight );
				_program->addOperator(op);  
			}

			//爆炸 操作器 
			if ( m_sParticleInfo.sEffectOpt.bExplosion )
			{
				float fX = m_sParticleInfo.sEffectOpt.sExplosion.s3F.fX;
				float fY = m_sParticleInfo.sEffectOpt.sExplosion.s3F.fY;
				float fZ = m_sParticleInfo.sEffectOpt.sExplosion.s3F.fZ;
				osgParticle::ExplosionOperator *op = new osgParticle::ExplosionOperator;
				op->setCenter(osg::Vec3(fX, fY, fZ));
				op->setRadius( m_sParticleInfo.sEffectOpt.sExplosion.fRadius);
				op->setMagnitude( m_sParticleInfo.sEffectOpt.sExplosion.fMagnitude);
				op->setEpsilon( m_sParticleInfo.sEffectOpt.sExplosion.fExpsilon);
				op->setSigma( m_sParticleInfo.sEffectOpt.sExplosion.fSigma);
				_program->addOperator(op);  
			}

			//流体 操作器  
			if ( m_sParticleInfo.sEffectOpt.bFluidFriction )
			{
				float fX = m_sParticleInfo.sEffectOpt.sFluid.s3F.fX;
				float fY = m_sParticleInfo.sEffectOpt.sFluid.s3F.fY;
				float fZ = m_sParticleInfo.sEffectOpt.sFluid.s3F.fZ;
				osgParticle::FluidFrictionOperator *op = new osgParticle::FluidFrictionOperator;
				op->setWind(osg::Vec3(fX, fY, fZ));
				op->setFluidDensity( m_sParticleInfo.sEffectOpt.sFluid.fDensity);
				op->setOverrideRadius( m_sParticleInfo.sEffectOpt.sFluid.fRadius);
				op->setFluidViscosity( m_sParticleInfo.sEffectOpt.sFluid.fSpeed);
				if ( m_sParticleInfo.sEffectOpt.sFluid.bFliud)
				{
					op->setFluidToWater();
				}
				else
				{
					op->setFluidToAir();
				}
				_program->addOperator(op);  
			}

			//力 操作器  
			if ( m_sParticleInfo.sEffectOpt.bForce )
			{
				float fX = m_sParticleInfo.sEffectOpt.sForce.s3F.fX;
				float fY = m_sParticleInfo.sEffectOpt.sForce.s3F.fY;
				float fZ = m_sParticleInfo.sEffectOpt.sForce.s3F.fZ;
				osgParticle::ForceOperator *op = new osgParticle::ForceOperator;
				op->setForce(osg::Vec3(fX, fY, fZ));
				_program->addOperator(op);  
			}

			//轨迹 操作器
			if ( m_sParticleInfo.sEffectOpt.bOrbit )
			{
				float fX = m_sParticleInfo.sEffectOpt.sOrbit.s3F.fX;
				float fY = m_sParticleInfo.sEffectOpt.sOrbit.s3F.fY;
				float fZ = m_sParticleInfo.sEffectOpt.sOrbit.s3F.fZ;
				osgParticle::OrbitOperator *op = new osgParticle::OrbitOperator;
				op->setCenter(osg::Vec3(fX, fY, fZ));
				op->setEpsilon(m_sParticleInfo.sEffectOpt.sOrbit.fExpsilon);
				op->setMagnitude(m_sParticleInfo.sEffectOpt.sOrbit.fMagnitude);
				op->setMaxRadius(m_sParticleInfo.sEffectOpt.sOrbit.fRadius);

				_program->addOperator(op);  
			}

			//弹力
			if ( m_sParticleInfo.sEffectOpt.bBounce )
			{
				float fResilience = m_sParticleInfo.sEffectOpt.sBounce.fResilience ;
				float fFriction = m_sParticleInfo.sEffectOpt.sBounce.fFriction;
				float fCutOff = m_sParticleInfo.sEffectOpt.sBounce.fCutOff;
				osgParticle::BounceOperator* op = new osgParticle::BounceOperator;
				op->setResilience( fResilience );
				op->setFriction( fFriction );
				op->setCutoff( fCutOff );

				AddDomianOpertor( op,m_sParticleInfo.sEffectOpt.sBounce.sDomian,m_sParticleInfo.sEffectOpt.sBounce.uiCount );

				_program->addOperator(op); 
			}
			//下沉
			if ( m_sParticleInfo.sEffectOpt.bSink )
			{
				osgParticle::SinkOperator* op = new osgParticle::SinkOperator;

				if ( m_sParticleInfo.sEffectOpt.stSink.bStrategyOutSide )
				{
					op->setSinkStrategy( osgParticle::SinkOperator::SINK_OUTSIDE );
				}
				else
				{
					op->setSinkStrategy( osgParticle::SinkOperator::SINK_INSIDE );
				}

				AddDomianOpertor( op,m_sParticleInfo.sEffectOpt.stSink.sDomian,m_sParticleInfo.sEffectOpt.stSink.uiCount );

				_program->addOperator(op); 
			}
		}
	}


	////////         CExParticleNode        ////////
	CExParticleNode::CExParticleNode(CExParticleNodeOption *opt, FeUtil::CRenderContext* pRender)
		:CExLodNode(opt)
		,m_pRender(pRender)
		,m_rpTranslateGroup(NULL)
		,m_rpRotateGroup(NULL)
		,m_rpRoot(NULL)
		,m_unCurParticleID(0)
	{
		memset( &m_stCurParticleTemplate,0,sizeof(ST_PARTICLE_TEMPLATE_INFO));

		m_rpTranslateGroup = new osg::MatrixTransform;
		addChild(m_rpTranslateGroup.get());

		m_rpRotateGroup = new osg::MatrixTransform;
		m_rpTranslateGroup->addChild(m_rpRotateGroup.get());

		m_rpRoot = new osg::Group;
		m_rpRotateGroup->addChild(m_rpRoot);

		//m_rpSysUpdater = new osgParticle::ParticleSystemUpdater;
		//addChild( m_rpSysUpdater.get());
	}

	void CExParticleNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	void CExParticleNode::SetParticlePath(const std::string& strXmlPath)
	{
		m_strParticlePath = strXmlPath;
		GetOption()->GetPath() = m_strParticlePath;

		InitializeParicleTemplate();
	}

	std::string  CExParticleNode::GetParticlePath()
	{
		return GetOption()->GetPath();
	}

	///初始化粒子模板
	bool CExParticleNode::InitializeParicleTemplate()
	{
		ClearScene();

		//读取粒子模板
		ReadParticleTemplate(m_strParticlePath, m_stCurParticleTemplate);

		//设置当前编辑粒子
		unsigned int unNum = m_stCurParticleTemplate.unNum;
		for ( unsigned int i = 0; i < unNum; ++i )
		{
			//纹理
			std::string strTexture = m_stCurParticleTemplate.asParticleInfo[i].szTexture;
			m_stCurParticleInfo = m_stCurParticleTemplate.asParticleInfo[i];
			AddParticleNode(strTexture);
		}

		SetCurParticle( m_stCurParticleTemplate.asParticleInfo[0].unID );

		return true;
	}

	///添加当前粒子  
	unsigned int CExParticleNode::AddParticleNode(std::string strTexture)
	{
		if ( !m_rpTranslateGroup.valid())
		{
			return 0;
		}

		osg::ref_ptr<osg::Group> pGroup = new osg::Group;
		m_rpRoot->addChild( pGroup.get());

		osg::ref_ptr<ParticleNodeEffect> effect = new ParticleNodeEffect(m_stCurParticleInfo, true);
		effect->setScale(GetParticleScale());

		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		geode->addDrawable(effect->getParticleSystem());
		geode->getOrCreateStateSet()->setRenderingHint(100);
		geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);

		pGroup->addChild( effect->getEmitter() );
		pGroup->addChild( geode );
		pGroup->addChild( effect );

		//关联粒子信息与场景节点
		m_mapParticleNodetoInfo[effect] = m_stCurParticleInfo;

		return m_stCurParticleInfo.unID;
	}

	void CExParticleNode::SetParticlePosition(const osg::Vec3d& vecLLHDegree)
	{
		GetOption()->GetPosition() = vecLLHDegree;

		if (m_rpTranslateGroup.valid())
		{
			osg::Matrix mtr;
			FeUtil::DegreeLLH2Matrix(m_pRender, vecLLHDegree, mtr);
			m_rpTranslateGroup->setMatrix(mtr);
		}

		//ClearScene();

// 		unsigned int unNum = m_stCurParticleTemplate.unNum;
// 		for ( unsigned int i = 0; i < unNum; ++i )
// 		{
// 			m_stCurParticleInfo = m_stCurParticleTemplate.asParticleInfo[i];
// 			m_stCurParticleTemplate.asParticleInfo[i] = m_stCurParticleInfo;
// 
// 			//添加新粒子
			//std::string strTexture = m_stCurParticleInfo.szTexture;
			//AddParticleNode(strTexture);
// 		}
	}

	const osg::Vec3d CExParticleNode::GetParticlePosition()
	{
		return GetOption()->GetPosition();
	}

	void CExParticleNode::SetParticleAngle(osg::Vec3 vecAngle)
	{
		GetOption()->GetAngle() = vecAngle;
		
		if (m_rpRotateGroup.valid())
		{
			m_rpRotateGroup->setMatrix(
				osg::Matrix::rotate(
				osg::DegreesToRadians(vecAngle.x()), osg::X_AXIS,
				osg::DegreesToRadians(vecAngle.y()), osg::Y_AXIS,
				osg::DegreesToRadians(vecAngle.z()), osg::Z_AXIS));
		}

		//ClearScene();

// 		unsigned int unNum = m_stCurParticleTemplate.unNum;
// 
// 		for ( unsigned int i = 0; i < unNum; ++i )
// 		{
// 			m_stCurParticleInfo = m_stCurParticleTemplate.asParticleInfo[i];
// 
// 			m_stCurParticleInfo.sShootOpt.fPhiRangeMin += vecAngle.x();
// 			m_stCurParticleInfo.sShootOpt.fPhiRangeMax += vecAngle.x();
// 
// 			m_stCurParticleInfo.sShootOpt.fThetaRangeMin += vecAngle.y();
// 			m_stCurParticleInfo.sShootOpt.fThetaRangeMax += vecAngle.y();
// 
// 			m_stCurParticleTemplate.asParticleInfo[i] = m_stCurParticleInfo;
// 
// 			//添加新粒子
			//std::string strTexture = m_stCurParticleInfo.szTexture;
			//AddParticleNode(strTexture);
// 		}
	}

	const osg::Vec3 CExParticleNode::GetParticleAngle()
	{
		return GetOption()->GetAngle();
	}

	void CExParticleNode::SetParticleScale( double dScale )
	{
		GetOption()->GetScale() = dScale;

		ClearScene();

		unsigned int unNum = m_stCurParticleTemplate.unNum;
		for ( unsigned int i = 0; i < unNum; ++i )
		{
			m_stCurParticleInfo = m_stCurParticleTemplate.asParticleInfo[i];

			//添加新粒子
			std::string strTexture = m_stCurParticleInfo.szTexture;
			AddParticleNode(strTexture);
		}
	}

	const double CExParticleNode::GetParticleScale()
	{
		return GetOption()->GetScale();
	}

	void CExParticleNode::SetNullTexture()
	{
		memset( (void*)&m_stCurParticleInfo,0,sizeof(ST_PARTICLE_INFO));
		m_unCurParticleID = 0;
	}

	///设置当前粒子
	void CExParticleNode::SetCurParticle( unsigned int unIDTexture )
	{
		//保存之前子粒子信息
		int iPreIndex = 0;
		int iCurIndex = 0;
		unsigned int uiPreID = GetCurParticleID();

		bool bFind = false;
		unsigned int unNum = m_stCurParticleTemplate.unNum;
		for ( unsigned int i = 0; i < unNum; ++i)
		{
			if ( m_stCurParticleTemplate.asParticleInfo[i].unID == uiPreID)
			{
				iPreIndex = i;
				bFind = true;
			}

			if ( m_stCurParticleTemplate.asParticleInfo[i].unID == unIDTexture )
			{
				iCurIndex = i;
			}
		}

		m_stCurParticleInfo = m_stCurParticleTemplate.asParticleInfo[iCurIndex];
		SetCurParticleID( unIDTexture );
	}

	void CExParticleNode::SetCurParticleID( unsigned int unID )
	{
		m_unCurParticleID = unID;
	}

	unsigned int CExParticleNode::GetCurParticleID()
	{
		return m_unCurParticleID;
	}

	///清除场景
	void CExParticleNode::ClearScene()
	{
		//std::map<osg::Group*,ST_PARTICLE_INFO>::iterator it = m_mapParticleNodetoInfo.begin();
		//for (; it != m_mapParticleNodetoInfo.end(); it++ )
		//{
		//	//删除粒子
		//	m_rpRoot->removeChild( it->first );
		//}
		m_rpRoot->removeChildren(0, m_rpRoot->getNumChildren());
		m_mapParticleNodetoInfo.clear();

		SetNullTexture();
	}

	///获取粒子模板信息
	void CExParticleNode::ReadParticleTemplate( std::string strPath,ST_PARTICLE_TEMPLATE_INFO& stParticleTemplate )
	{
		osgEarth::ReadResult res = osgEarth::URI(strPath.c_str()).readString();
		if (!res.succeeded())
			return;

		osg::ref_ptr<osgDB::Options> pdbOptions = osgEarth::Registry::instance()->cloneOrCreateOptions();
		osgEarth::URIContext( strPath.c_str() ).apply( pdbOptions.get());

		std::stringstream in(res.getString());

		osgEarth::URIContext uriContext(pdbOptions);

		osg::ref_ptr<osgEarth::XmlDocument> doc = osgEarth::XmlDocument::load(in,uriContext);

		if (!doc.valid())
			return;

		/* 读取scene */
		osgEarth::Config docConf = doc->getConfig().child("particlelist");
		stParticleTemplate.uiParticleID = docConf.value("id",0 );
		memcpy( (void*)stParticleTemplate.szName,docConf.value("name").c_str(),PARTICLE_NAME_LENGHT);
		stParticleTemplate.unNum = docConf.value("count",0 );

		osgEarth::ConfigSet confSet = docConf.children("particle");
		unsigned int i = 0;
		for ( osgEarth::ConfigSet::iterator iter = confSet.begin(); iter != confSet.end(); ++iter,++i )
		{
			osgEarth::Config config = *iter;
			stParticleTemplate.asParticleInfo[i] = ReadParticleInfo(config,strPath);
		}
	}

	///  读取xml文件   粒子信息
	ST_PARTICLE_INFO CExParticleNode::ReadParticleInfo( osgEarth::Config config,std::string strPath )
	{
		ST_PARTICLE_INFO stParticle;
		memset( (void*)&stParticle,0,sizeof(ST_PARTICLE_INFO));

		if ( config.hasChild("id"))
		{
			stParticle.unID = config.value("id",0);
		}

		if ( config.hasChild("name"))
		{
			std::string strSrc = "/";
			std::string strDes = "\\\\";
			std::string::size_type pos = 0;  
			std::string::size_type srcLen = strSrc.size();  
			std::string::size_type desLen = strDes.size();  
			pos=strPath.find(strSrc, pos);   
			while ((pos != std::string::npos))  
			{  
				strPath.replace(pos, srcLen, strDes);  
				pos=strPath.find(strSrc, (pos+desLen));  
			} 

			char szPath[FREE_MAX_PATH];
			memset( (void*)szPath,0,FREE_MAX_PATH);
			memcpy((void*)szPath,strPath.c_str(),FREE_MAX_PATH);

			std::string strTempPath = FeUtil::StringDirectoryFmt(szPath);
			strTempPath = strTempPath.substr(0, strTempPath.size()-1);
			int nIndex = strTempPath.find_last_of("/");
			strTempPath = strTempPath.substr(0, nIndex);
			//::PathRemoveFileSpecA( szPath );	// 移除文件名
			//::PathAddBackslashA( szPath );	// 确保以"\"结尾

			memcpy((void*)szPath,strTempPath.c_str(), FREE_MAX_PATH);

			char szTexture[FREE_MAX_PATH];
			memcpy((void*)szTexture,config.value("name").c_str(),FREE_MAX_PATH);

			strcat(szPath,szTexture);
			memcpy((void*)stParticle.szTexture,szPath,FREE_MAX_PATH);
		}

		if ( config.hasChild("particleopt"))
		{
			//粒子配置
			ReadParticleOptConfig( config.child("particleopt"),stParticle.sParticleOpt);
		}

		if ( config.hasChild("coloropt"))
		{
			//颜色配置
			ReadColorOptConfig(config.child("coloropt"), stParticle.sColorOpt);
		}

		if ( config.hasChild("shootopt"))
		{
			//发射器配置
			ReadShootOptConfig(config.child("shootopt"),stParticle.sShootOpt);
		}

		if ( config.hasChild("effectopt"))
		{
			//粒子操作器
			ReadEffectOptConfig(config.child("effectopt"),stParticle.sEffectOpt);
		}

		return stParticle;
	}

	///读取xml文件 粒子配置
	bool CExParticleNode::ReadParticleOptConfig(osgEarth::Config config, ST_PARTICLE_OPITION& sParticleOpt)
	{
		sParticleOpt.fLife = config.value("life", 0.0 );
		sParticleOpt.fSizeStart = config.value("sizestart",0.0 );
		sParticleOpt.fSizeEnd = config.value("sizeend",0.0 );
		sParticleOpt.fX = config.value("x",0.0 );
		sParticleOpt.fY = config.value("y",0.0 );
		sParticleOpt.fZ = config.value("z",0.0 );

		if ( config.hasChild("startalpha"))
		{
			sParticleOpt.fAlphaStart = config.value("startalpha",1.0);
		}

		if ( config.hasChild("endalpha"))
		{
			sParticleOpt.fAlphaEnd = config.value("endalpha",1.0);
		}

		if ( config.hasChild("speed"))
		{
			osgEarth::Config con = config.child("speed");
			sParticleOpt.sSpeed.fX = con.value("x",0.0);
			sParticleOpt.sSpeed.fY = con.value("y",0.0);
			sParticleOpt.sSpeed.fZ = con.value("z",0.0);
		}

		if ( config.hasChild("angle"))
		{
			osgEarth::Config con = config.child("angle");
			sParticleOpt.sAngle.fX = con.value("x",0.0);
			sParticleOpt.sAngle.fY = con.value("y",0.0);
			sParticleOpt.sAngle.fZ = con.value("z",0.0);
		}

		if ( config.hasChild("angular"))
		{
			osgEarth::Config con = config.child("angular");
			sParticleOpt.sAngular.fX = con.value("x",0.0);
			sParticleOpt.sAngular.fY = con.value("y",0.0);
			sParticleOpt.sAngular.fZ = con.value("z",0.0);
		}

		sParticleOpt.fDepth = config.value("depth",0.0 );

		sParticleOpt.fRadius = config.value("radius",0.0 );
		sParticleOpt.fMass = config.value("mass",0.0 );

		return true;
	}

	///读取xml文件  粒子颜色配置
	bool CExParticleNode::ReadColorOptConfig( osgEarth::Config config,ST_COLOR_OPITION& sColorOpt)
	{
		sColorOpt.fBeginR = config.value("beginr",0.0 );
		sColorOpt.fBeginG = config.value("beging",0.0 );
		sColorOpt.fBeginB = config.value("beginb",0.0 );
		sColorOpt.fBeginAlpha = config.value("beginalpha",0.0 );
		sColorOpt.fEndR = config.value("endr",0.0 );
		sColorOpt.fEndG = config.value("endg",0.0 );
		sColorOpt.fEndB = config.value("endb",0.0 );
		sColorOpt.fEndAlpha = config.value("endalpha",0.0 );

		return true;
	}

	///读取xml文件 粒子发射器配置
	bool CExParticleNode::ReadShootOptConfig(osgEarth::Config config, ST_SHOOT_OPITION& sShootOpt)
	{
		sShootOpt.fLife = config.value("life",0.0 );
		sShootOpt.fRadiusMin = config.value("radiusmin",0.0 );
		sShootOpt.fRadiusMax = config.value("radiusmax",0.0 );
		sShootOpt.fPhiMin = config.value("phimin",0.0 );
		sShootOpt.fPhiMax = config.value("phimax",0.0 );
		sShootOpt.fSpeedMin = config.value("speedmin",0.0 );
		sShootOpt.fSpeedMax = config.value("speedmax",0.0 );
		sShootOpt.fCountMin = config.value("countmin",0.0 );
		sShootOpt.fCountMax = config.value("countmax",0.0 );
		sShootOpt.fPhiRangeMin = config.value("phirangemin",0.0 );
		sShootOpt.fPhiRangeMax = config.value("phirangemax",0.0 );
		sShootOpt.fThetaRangeMin = config.value("thetarangemin",0.0 );
		sShootOpt.fThetaRangeMax = config.value("thetarangemax",0.0 );
		sShootOpt.fRotateBeginX = config.value("rotatebeginx",0.0 );
		sShootOpt.fRotateBeginY = config.value("rotatebeginy",0.0 );
		sShootOpt.fRotateBeginZ = config.value("rotatebeginz",0.0 );
		sShootOpt.fRotateEndX = config.value("rotateendx",0.0 );
		sShootOpt.fRotateEndY = config.value("rotateendy",0.0 );
		sShootOpt.fRotateEndZ = config.value("rotateendz",0.0 );
		sShootOpt.fStartTime = config.value("starttime",0.0 );

		return true;
	}

	///  读取xml文件  粒子效果配置
	bool CExParticleNode::ReadEffectOptConfig(osgEarth::Config config, ST_EFFECT_OPITION& sEffectOpt)
	{
		//加速度
		if ( config.hasChild("accel") )
		{
			sEffectOpt.bAccel = true;
			osgEarth::Config conf = config.child("accel");
			sEffectOpt.sAccel.s3F.fX = conf.value("x",0.0 );
			sEffectOpt.sAccel.s3F.fY = conf.value("y",0.0 );
			sEffectOpt.sAccel.s3F.fZ = conf.value("z",0.0 );
			sEffectOpt.sAccel.fGravity = conf.value("gravity",0.0 );
		}
		//角加速度
		if ( config.hasChild("angularaceel") )
		{
			sEffectOpt.bAngularAceel = true;
			osgEarth::Config conf = config.child("angularaceel");
			sEffectOpt.sAngularAceel.s3F.fX = conf.value("x",0.0 );
			sEffectOpt.sAngularAceel.s3F.fY = conf.value("y",0.0 );
			sEffectOpt.sAngularAceel.s3F.fZ = conf.value("z",0.0 );
		}
		//角加速度阻尼
		if ( config.hasChild("angulardamping") )
		{
			sEffectOpt.bAngularAccelDamping = true;
			osgEarth::Config conf = config.child("angulardamping");
			sEffectOpt.sAngularDamping.s3F.fX = conf.value("x",0.0 );
			sEffectOpt.sAngularDamping.s3F.fY = conf.value("y",0.0 );
			sEffectOpt.sAngularDamping.s3F.fZ = conf.value("z",0.0 );
			sEffectOpt.sAngularDamping.fLow = conf.value("low",0.0 );
			sEffectOpt.sAngularDamping.fHeight = conf.value("height",0.0 );
		}
		//阻尼
		if ( config.hasChild("damping") )
		{
			sEffectOpt.bDamping = true;
			osgEarth::Config conf = config.child("damping");
			sEffectOpt.sDamping.s3F.fX = conf.value("x",0.0 );
			sEffectOpt.sDamping.s3F.fY = conf.value("y",0.0 );
			sEffectOpt.sDamping.s3F.fZ = conf.value("z",0.0 );
			sEffectOpt.sDamping.fLow = conf.value("low",0.0 );
			sEffectOpt.sDamping.fHeight = conf.value("height",0.0 );
		}
		//爆炸
		if ( config.hasChild("explosion") )
		{
			sEffectOpt.bExplosion = true;
			osgEarth::Config conf = config.child("explosion");
			sEffectOpt.sExplosion.s3F.fX = conf.value("x",0.0 );
			sEffectOpt.sExplosion.s3F.fY = conf.value("y",0.0 );
			sEffectOpt.sExplosion.s3F.fZ = conf.value("z",0.0 );
			sEffectOpt.sExplosion.fRadius = conf.value("radius",0.0 );
			sEffectOpt.sExplosion.fMagnitude = conf.value("magnitude",0.0 );
			sEffectOpt.sExplosion.fExpsilon = conf.value("exp",0.0 );
			sEffectOpt.sExplosion.fSigma = conf.value("sigma",0.0 );
		}
		//流体
		if ( config.hasChild("fluidfriction") )
		{
			sEffectOpt.bFluidFriction = true;
			osgEarth::Config conf = config.child("fluidfriction");
			sEffectOpt.sFluid.bFliud = conf.value("fluid",false );
			sEffectOpt.sFluid.s3F.fX = conf.value("x",0.0 );
			sEffectOpt.sFluid.s3F.fY = conf.value("y",0.0 );
			sEffectOpt.sFluid.s3F.fZ = conf.value("z",0.0 );
			sEffectOpt.sFluid.fRadius = conf.value("radius",0.0 );
			sEffectOpt.sFluid.fDensity = conf.value("density",0.0 );
			sEffectOpt.sFluid.fSpeed = conf.value("speed",0.0 );
		}
		//力
		if ( config.hasChild("force") )
		{
			sEffectOpt.bForce = true;
			osgEarth::Config conf = config.child("force");
			sEffectOpt.sForce.s3F.fX = conf.value("x",0.0);
			sEffectOpt.sForce.s3F.fY = conf.value("y",0.0);
			sEffectOpt.sForce.s3F.fZ = conf.value("z",0.0 );
		}
		//轨迹
		if ( config.hasChild("orbit") )
		{
			sEffectOpt.bOrbit = true;
			osgEarth::Config conf = config.child("force");
			sEffectOpt.sOrbit.s3F.fX = conf.value("x",0.0 );
			sEffectOpt.sOrbit.s3F.fY = conf.value("y",0.0 );
			sEffectOpt.sOrbit.s3F.fZ = conf.value("z",0.0 );
			sEffectOpt.sOrbit.fRadius = conf.value("radius",0.0 );
			sEffectOpt.sOrbit.fMagnitude = conf.value("magnitude",0.0 );
			sEffectOpt.sOrbit.fExpsilon = conf.value("exp",0.0 );
		}
		//弹力
		if ( config.hasChild("bounce") )
		{
			sEffectOpt.bBounce = true;
			osgEarth::Config conf = config.child("bounce");
			sEffectOpt.sBounce.fResilience = conf.value("resilience",0.0 );
			sEffectOpt.sBounce.fFriction = conf.value("friction",0.0 );
			sEffectOpt.sBounce.fCutOff = conf.value("cutOff",0.0 );
			sEffectOpt.sBounce.uiCount = conf.value("count",0 );
			if ( conf.hasChild("domian"))
			{
				ReadDomianOptConfig( conf.child("domian"),sEffectOpt.sBounce.sDomian);
			}
		}
		//下沉
		if ( config.hasChild("sink") )
		{
			sEffectOpt.bSink = true;
			osgEarth::Config conf = config.child("sink");
			sEffectOpt.stSink.bStrategyOutSide = conf.value("strategy",false );
			sEffectOpt.stSink.uiCount = conf.value("count",0 );
			if ( conf.hasChild("domian"))
			{
				ReadDomianOptConfig( conf.child("domian"),sEffectOpt.stSink.sDomian);
			}
		}

		return true;
	}

	///节点访问 更新场景渲染
	void CExParticleNode::traverse( osg::NodeVisitor& nv )
	{
		if ( nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR )
		{
			//获取粒子系统
			if ( !m_mapParticleNodetoInfo.empty())
			{
				std::map<osg::Group*,ST_PARTICLE_INFO>::iterator it = m_mapParticleNodetoInfo.begin();
				for (; it != m_mapParticleNodetoInfo.end(); ++it )
				{
					osgParticle::ParticleEffect* effect = dynamic_cast<osgParticle::ParticleEffect*>( it->first );
					if (effect)
					{
						//发射极
						osgParticle::ModularEmitter* emmiter = dynamic_cast<osgParticle::ModularEmitter*>(effect->getEmitter());
						if ( emmiter )
						{
							double dLife = emmiter->getLifeTime();
							if ( dLife != -1 )
							{
								if ( nv.getFrameStamp()->getSimulationTime() >= dLife)
								{
									emmiter->setEndless(false);
								}
							}
						}
					}
				}
			}
		}
		osg::Group::traverse(nv);
	}

	///读取xml文件  粒子效果域配置
	bool CExParticleNode::ReadDomianOptConfig( osgEarth::Config config,ST_EFFECT_OPITION_DOMIAN& stDomainOpt )
	{
		if ( config.hasChild("point") )
		{
			stDomainOpt.bPoint = true;
			osgEarth::Config conf = config.child("point");
			stDomainOpt.sPoint.unNO = conf.value("no",0);
			stDomainOpt.sPoint.s3F.fX = conf.value("x1",0.0);
			stDomainOpt.sPoint.s3F.fY = conf.value("y1",0.0);
			stDomainOpt.sPoint.s3F.fZ = conf.value("z1",0.0);
		}

		if ( config.hasChild("line") )
		{
			stDomainOpt.bLine = true;
			osgEarth::Config conf = config.child("line");
			stDomainOpt.sLine.unNO = conf.value("no",0);
			stDomainOpt.sLine.s3FV1.fX = conf.value("x1",0.0);
			stDomainOpt.sLine.s3FV1.fY = conf.value("y1",0.0);
			stDomainOpt.sLine.s3FV1.fZ = conf.value("z1",0.0);
			stDomainOpt.sLine.s3FV2.fX = conf.value("x2",0.0);
			stDomainOpt.sLine.s3FV2.fY = conf.value("y2",0.0);
			stDomainOpt.sLine.s3FV2.fZ = conf.value("z2",0.0);
		}

		if ( config.hasChild("triangle") )
		{
			stDomainOpt.bTri = true;
			osgEarth::Config conf = config.child("triangle");
			stDomainOpt.sTri.unNO = conf.value("no",0);
			stDomainOpt.sTri.s3FV1.fX = conf.value("x1",0.0);
			stDomainOpt.sTri.s3FV1.fY = conf.value("y1",0.0);
			stDomainOpt.sTri.s3FV1.fZ = conf.value("z1",0.0);
			stDomainOpt.sTri.s3FV2.fX = conf.value("x2",0.0);
			stDomainOpt.sTri.s3FV2.fY = conf.value("y2",0.0);
			stDomainOpt.sTri.s3FV2.fZ = conf.value("z2",0.0);
			stDomainOpt.sTri.s3FV3.fX = conf.value("x3",0.0);
			stDomainOpt.sTri.s3FV3.fY = conf.value("y3",0.0);
			stDomainOpt.sTri.s3FV3.fZ = conf.value("z3",0.0);

		}

		if ( config.hasChild("rect") )
		{
			stDomainOpt.bRect = true;
			osgEarth::Config conf = config.child("rect");
			stDomainOpt.sRect.unNO = conf.value("no",0);
			stDomainOpt.sRect.s3FV1.fX = conf.value("x1",0.0);
			stDomainOpt.sRect.s3FV1.fY = conf.value("y1",0.0);
			stDomainOpt.sRect.s3FV1.fZ = conf.value("z1",0.0);
			stDomainOpt.sRect.s3FV2.fX = conf.value("x2",0.0);
			stDomainOpt.sRect.s3FV2.fY = conf.value("y2",0.0);
			stDomainOpt.sRect.s3FV2.fZ = conf.value("z2",0.0);
			stDomainOpt.sRect.s3FV3.fX = conf.value("x3",0.0);
			stDomainOpt.sRect.s3FV3.fY = conf.value("y3",0.0);
			stDomainOpt.sRect.s3FV3.fZ = conf.value("z3",0.0);
		}

		if ( config.hasChild("plane") )
		{
			stDomainOpt.bPlane = true;
			osgEarth::Config conf = config.child("plane");
			stDomainOpt.sPlane.unNO = conf.value("no",0);
			stDomainOpt.sPlane.s3FV1.fX = conf.value("x1",0.0);
			stDomainOpt.sPlane.s3FV1.fY = conf.value("y1",0.0);
			stDomainOpt.sPlane.s3FV1.fZ = conf.value("z1",0.0);
			stDomainOpt.sPlane.fRadus = conf.value("w",0.0);

		}

		if ( config.hasChild("sphere") )
		{
			stDomainOpt.bSphere = true;
			osgEarth::Config conf = config.child("sphere");
			stDomainOpt.sSphere.unNO = conf.value("no",0);
			stDomainOpt.sSphere.s3FV1.fX = conf.value("x1",0.0);
			stDomainOpt.sSphere.s3FV1.fY = conf.value("y1",0.0);
			stDomainOpt.sSphere.s3FV1.fZ = conf.value("z1",0.0);
			stDomainOpt.sSphere.fRadus = conf.value("w",0.0);
		}

		if ( config.hasChild("box") )
		{
			stDomainOpt.bBox = true;
			osgEarth::Config conf = config.child("box");
			stDomainOpt.stBox.unNO = conf.value("no",0);
			stDomainOpt.stBox.s3FV1.fX = conf.value("x1",0.0);
			stDomainOpt.stBox.s3FV1.fY = conf.value("y1",0.0);
			stDomainOpt.stBox.s3FV1.fZ = conf.value("z1",0.0);
			stDomainOpt.stBox.s3FV2.fX = conf.value("x2",0.0);
			stDomainOpt.stBox.s3FV2.fY = conf.value("y2",0.0);
			stDomainOpt.stBox.s3FV2.fZ = conf.value("z2",0.0);
		}

		if ( config.hasChild("disk") )
		{
			stDomainOpt.bDisk = true;
			osgEarth::Config conf = config.child("disk");
			stDomainOpt.sDisk.unNO = conf.value("no",0);
			stDomainOpt.sDisk.s3FV1.fX = conf.value("x1",0.0);
			stDomainOpt.sDisk.s3FV1.fY = conf.value("y1",0.0);
			stDomainOpt.sDisk.s3FV1.fZ = conf.value("z1",0.0);
			stDomainOpt.sDisk.s3FV2.fX = conf.value("x2",0.0);
			stDomainOpt.sDisk.s3FV2.fY = conf.value("y2",0.0);
			stDomainOpt.sDisk.s3FV2.fZ = conf.value("z2",0.0);
			stDomainOpt.sDisk.fMin = conf.value("min",0.0);
			stDomainOpt.sDisk.fMax = conf.value("max",0.0);

		}

		return true;
	}

	CExParticleNode::~CExParticleNode(void)
	{
	}

	CExParticleNodeOption* CExParticleNode::GetOption()
	{
		return dynamic_cast<CExParticleNodeOption*>(m_rpOptions.get());
	}
}
