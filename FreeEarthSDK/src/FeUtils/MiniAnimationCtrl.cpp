
#include <osgDB/ReadFile>

#include <osgEarth/GeoMath>
#include <osgEarth/SpatialReference>

#include <FeUtils/CoordConverter.h>
#include <FeUtils/MiniAnimationCtrl.h>
#include <FeUtils/MathUtil.h>

#define FLOAT_EQUAL(x, y)  ((x<y+0.00001) && (x>y-0.00001))


namespace FeUtil
{

	void CAnimTimerCallback::reset()
	{
		FeUtil::AnimationPathCallback::reset();

		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_timerMutex);
		for(int n = 0; n < m_timeCallbacks.size(); n++)
		{
			m_timeCallbacks.at(n).m_bCalled = false;
		}
	}

	void CAnimTimerCallback::AddTimeCaller( double time, TimerCallFunc* func )
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_timerMutex);
		m_timeCallbacks.push_back(TimeCaller(func, time));
	}

	void CAnimTimerCallback::update( osg::Node& node )
	{
		FeUtil::AnimationPathCallback::update(node);

		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_timerMutex);
		for(int n = 0; n < m_timeCallbacks.size(); n++)
		{
			if(!m_timeCallbacks.at(n).m_bCalled && getAnimationTime() > m_timeCallbacks.at(n).m_time)
			{
				(*m_timeCallbacks.at(n).m_func)();
				m_timeCallbacks.at(n).m_bCalled = true;
			}
		}
	}

}


namespace FeUtil
{
	CMiniAnimationCtrl::CMiniAnimationCtrl(CRenderContext *pRenderContext,osg::Group* pAnimationMT,FeUtil::AnimationPathCallback *pPathCallback)
		:osg::Referenced()
		,m_dSpeed(10000.0)
		,m_dHeiOffset(1000.0)
		,m_dSpeedOffset(0.1)
		,m_dScale(1.0)
		,m_pTrackNode(NULL)
		,m_opAnimationMT(pAnimationMT)
		,m_pPathCallback(pPathCallback)
		,m_unLoop(FeUtil::AnimationPath::LOOP)
		,m_opRenderContext(pRenderContext)
		,m_rpApBuilder(NULL)
		,m_rpAnimationPath(NULL)
		,m_nIndex(4)
	{
		m_rpApBuilder = new FeUtil::CMiniAnimationPathBuilder(m_opRenderContext.get());
		Reset();
	}

	CMiniAnimationCtrl::~CMiniAnimationCtrl()
	{

	}

	void CMiniAnimationCtrl::Reset()
	{
		if(m_pPathCallback.valid())
		{
			OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
			m_pPathCallback->setPause(true);
			m_pPathCallback->reset();
			
			// 恢复到起始帧位置
			osg::MatrixTransform* pMt = dynamic_cast<osg::MatrixTransform*>(m_opAnimationMT.get());
			if(pMt && m_rpAnimationPath.valid())
			{
				FeUtil::AnimationPath::ControlPoint controlPoint;
				m_rpAnimationPath->getInterpolatedControlPoint(0, controlPoint);

				osg::Matrix matrix;
				controlPoint.getMatrix(matrix);
				pMt->setMatrix(matrix); 
			}
		}
	}

	bool CMiniAnimationCtrl::Start()
	{
		if(m_pPathCallback.valid())
		{
			OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
			
			m_pPathCallback->setPause(false);
 			m_pPathCallback->reset();
 			m_pPathCallback->setAnimationPath(m_rpAnimationPath.get());

			if(m_nIndex == 4)
			{
				m_dTimeMultiplier = m_pPathCallback->getTimeMultiplier();
			}
			else
			{
				m_dSpeedOffset = dComputeSpeedOffset[m_nIndex-1];
				m_dTimeMultiplier = m_pPathCallback->getTimeMultiplier()/m_dSpeedOffset;
				m_pPathCallback->setTimeMultiplier(m_dTimeMultiplier);
				m_nIndex = 4;
			}

			if(m_opAnimationMT.valid())
			{
				m_opAnimationMT->removeUpdateCallback(m_pPathCallback);
				m_opAnimationMT->addUpdateCallback(m_pPathCallback);
			}
			return true;
		}

		return false;
	}

	bool CMiniAnimationCtrl::Stop()
	{
		if(m_pPathCallback.valid() && m_rpAnimationPath.valid())
		{
			OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
			m_pPathCallback->reset();
			m_pPathCallback->setPause(true);
			//m_opAnimationPath->clear();
			m_pPathCallback->setAnimationPath(NULL);
			if(m_opAnimationMT.valid())
			{
				m_opAnimationMT->removeUpdateCallback(m_pPathCallback);
			}
			return true;
		}

		return true;
	}

	bool CMiniAnimationCtrl::Pause()
	{
		if(m_pPathCallback.valid())
		{
			m_pPathCallback->setPause(true);
			return true;
		}

		return false;
	}

	bool CMiniAnimationCtrl::ReStart()
	{
		if(m_pPathCallback.valid())
		{
			m_pPathCallback->setPause(false);
			return true;
		}

		return false;
	}

	bool CMiniAnimationCtrl::GetPause()
	{
		if(m_pPathCallback.valid())
		{
			return m_pPathCallback->getPause();
		}
		return false;
	}

	void CMiniAnimationCtrl::Accelerate()
	{
		if(m_pPathCallback.valid())
		{
			if(m_nIndex == 4)
			{
				m_dTimeMultiplier = m_pPathCallback->getTimeMultiplier();
			}
			++m_nIndex;
			if(m_nIndex > 7)
			{
				--m_nIndex;
				return;
			}

			m_dSpeedOffset = dComputeSpeedOffset[m_nIndex-1];
			UpdataTimeMultiplier();

		}

		//if(m_pPathCallback.valid())
		//{
		//	double dTempSpeedOffset = m_pPathCallback->getTimeMultiplier();
		//	double dLatestTime = m_pPathCallback->_latestTime;
		//	double dFirstTime = m_pPathCallback->_firstTime; 
		//	double dTime = (dLatestTime - dFirstTime)*dTempSpeedOffset;

		//	dTempSpeedOffset *= 1.1;
		//	m_pPathCallback->setTimeMultiplier(dTempSpeedOffset);

		//	m_pPathCallback->_firstTime = -(dTime/dTempSpeedOffset - dLatestTime);

		//}
	}

	void CMiniAnimationCtrl::Decelerate()
	{
		if(m_pPathCallback.valid())
		{
			if(m_nIndex == 4)
			{
				m_dTimeMultiplier = m_pPathCallback->getTimeMultiplier();
			}
			--m_nIndex;
			if(m_nIndex < 1)
			{
				++m_nIndex;
				return;
			}
			m_dSpeedOffset = dComputeSpeedOffset[m_nIndex-1];
			UpdataTimeMultiplier();

			//double dTempSpeedOffset = m_pPathCallback->getTimeMultiplier();
			//double dLatestTime = m_pPathCallback->_latestTime;
			//double dFirstTime = m_pPathCallback->_firstTime; 
			//double dTime = (dLatestTime - dFirstTime)*dTempSpeedOffset;

			//dTempSpeedOffset /= 1.1;
			//m_pPathCallback->setTimeMultiplier(dTempSpeedOffset);

			//m_pPathCallback->_firstTime = -(dTime/dTempSpeedOffset - dLatestTime);

		}
	}

	void CMiniAnimationCtrl::UpdataTimeMultiplier()
	{
		if(m_pPathCallback.valid())
		{
			double dTempSpeedOffset = m_pPathCallback->getTimeMultiplier();
			double dLatestTime = m_pPathCallback->_latestTime;
			double dFirstTime = m_pPathCallback->_firstTime; 
			double dTime = (dLatestTime - dFirstTime)*dTempSpeedOffset;

			dTempSpeedOffset = /*m_dTimeMultiplier **/m_dSpeedOffset;
			m_pPathCallback->setTimeMultiplier(dTempSpeedOffset);

			m_pPathCallback->_firstTime = -(dTime/dTempSpeedOffset - dLatestTime);

		}
	}

	void CMiniAnimationCtrl::SetSpeed( double dValue )
	{
		m_dSpeed = dValue;
	}

	double CMiniAnimationCtrl::GetSpeed()
	{
		return m_dHeiOffset;
	}

	void CMiniAnimationCtrl::SetSpeedOffset( double dValue )
	{
		m_dSpeedOffset = dValue;
		UpdataTimeMultiplier();
	}

	double CMiniAnimationCtrl::GetSpeedOffset()
	{
		return m_dSpeedOffset;
	}

	void CMiniAnimationCtrl::SetHeightOffset( double dValue )
	{
		m_dHeiOffset = dValue;
	}

	double CMiniAnimationCtrl::GetHeightOffset()
	{
		return m_dHeiOffset;
	}

	osg::Node* CMiniAnimationCtrl::GetTrackMT()
	{
		return m_pTrackNode.get();
	}

	osg::Group* CMiniAnimationCtrl::GetAnimationMT()
	{
		return m_opAnimationMT.get();
	}

	void CMiniAnimationCtrl::SetAnimationMT(osg::Group* pAnimationMT)
	{
		m_opAnimationMT = pAnimationMT;
		if(m_opAnimationMT.valid() /*&& !m_vecMTCallback.count(m_opAnimationMT.get())*/)
		{
			m_opAnimationMT->addUpdateCallback(m_pPathCallback.get());	
			//m_vecMTCallback[m_opAnimationMT.get()] = m_pPathCallback.get();
		}
		
	}

	void CMiniAnimationCtrl::SetTrackMT( osg::Node* pTrackNode )
	{
		m_pTrackNode = pTrackNode;
	}

	FeUtil::AnimationPathCallback* CMiniAnimationCtrl::GetAnimationCallBack()
	{
		return m_pPathCallback.get();
	}

	bool CMiniAnimationCtrl::IsFinish()
	{
		if(m_pPathCallback)
		{
			return m_pPathCallback->IsFinish();
		}

		return false;

	}

	void CMiniAnimationCtrl::SetAnimationLoopMode( AnimationPath::LoopMode mode )
	{
		if(m_pPathCallback)
		{
			m_unLoop = mode;
			m_pPathCallback->SetAnimationLoopMode(mode);
			m_rpAnimationPath->setLoopMode(mode);
		}	
	}

	void CMiniAnimationCtrl::SetFrameCalculate( bool bCalculate )
	{
		if(m_pPathCallback)
		{
			m_pPathCallback->SetFrameCalculate(bCalculate);
		}	
	}

	unsigned int CMiniAnimationCtrl::GetAnimationLoopMode() const
	{
		return m_unLoop;
	}

	void CMiniAnimationCtrl::CreateAnimationPath( std::vector<osg::Vec3d> vectorData,double dSpeed )
	{
		if(vectorData.size() >= 2 && m_rpApBuilder.valid() && m_pPathCallback)
		{
			std::vector<osg::Vec3d> outPutVertexs;
			FeUtil::GetInterpolation(vectorData,outPutVertexs);
 			m_rpAnimationPath = m_rpApBuilder->CreateAnimationPath(outPutVertexs,dSpeed);
			m_pPathCallback->setAnimationPath(m_rpAnimationPath.get());
		}
	}

	void CMiniAnimationCtrl::CreateAnimationPath( std::vector<FeUtil::SDriveData> vectorDriveData )
	{
		if(vectorDriveData.size() >= 2 && m_rpApBuilder.valid() && m_pPathCallback.valid())
		{
			OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
			m_rpAnimationPath = m_rpApBuilder->CreateAnimationPath(vectorDriveData);
			m_pPathCallback->setAnimationPath(m_rpAnimationPath.get());
		}
	}

	bool CMiniAnimationCtrl::ContinueMove()
	{
		if(m_pPathCallback.valid())
		{
			m_pPathCallback->setPause(false);
			return true;
		}

		return false;
	}

}





