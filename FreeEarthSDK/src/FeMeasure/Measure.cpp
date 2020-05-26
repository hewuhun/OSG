#include <algorithm>
#include <FeMeasure/Measure.h>

namespace FeMeasure
{

	CMeasureResult::~CMeasureResult()
	{
		if(m_pResultData)
		{
			delete m_pResultData;
		}
	}

	CMeasureResult::CMeasureResult( FeUtil::CRenderContext* pRenderContext ) 
		: m_opRenderContext(pRenderContext)
		, m_pResultData(0)
	{
	}

	bool CMeasureResult::Calculate( const osg::Vec3dArray& vecPointsLLH )
	{
		if(!m_pResultData) m_pResultData = CreateResultData();

		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutexCalc);
		return CalculateImplement(vecPointsLLH);
	}

	bool CMeasureResult::FinalCalculate()
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutexCalc);
		return FinalCalculateImplement();
	}

}


namespace FeMeasure
{
	CMeasure::CMeasure(FeUtil::CRenderContext* pRenderContext)
		: m_opRenderContext(pRenderContext)
		, m_bDoingMeasure(false)
	{
		MeasureType() = EM_Nothing;
		m_rpVertexes = new osg::Vec3dArray;
	}

	CMeasure::~CMeasure()
	{
	}

	void CMeasure::AddCtrlPoint( const osg::Vec3d& vecPosLLH )
	{
		if(IsMeasuring()) 
		{
			AddCtrlPointImplement(vecPosLLH);
		}
	}

	void CMeasure::SetCtrlPoints( const osg::Vec3dArray& vecPointsLLH )
	{
		if(IsMeasuring())
		{
			SetCtrlPointsImplement(vecPointsLLH);
		}
	}

	void CMeasure::AddCtrlPointImplement( const osg::Vec3d& vecPosLLH )
	{
		if(m_rpVertexes.valid())
		{
			m_rpVertexes->push_back(vecPosLLH);
		}
	}

	void CMeasure::SetCtrlPointsImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		if(m_rpVertexes.valid())
		{
			m_rpVertexes->assign(vecPointsLLH.begin(), vecPointsLLH.end());
		}
	}

	bool CMeasure::ReplaceCtrlPoint( const osg::Vec3d& vecPosLLH, int nIndex /*= -1*/ )
	{
		if(!IsMeasuring() || !m_rpVertexes.valid() || m_rpVertexes->empty()) return false;

		if(-1 == nIndex)
		{
			m_rpVertexes->back() = vecPosLLH;
		}
		else if(nIndex < m_rpVertexes->size() && nIndex >= 0)
		{
			m_rpVertexes->at(nIndex) = vecPosLLH;
		}
		else
		{
			return false;
		}
		return true;
	}

	const osg::Vec3dArray& CMeasure::GetCtrlPoints()
	{
		return Vertexes();
	}

	void CMeasure::ClearCtrlPoints()
	{
		if(IsMeasuring())
		{
			m_rpVertexes->clear();
		}
	}

	void CMeasure::CalcNewResult()
	{
		if(IsMeasuring())
		{
			CreateNewResult();
			UpdateResult();
		}
	}

	void CMeasure::ClearResult()
	{
		MEASURE_RESULT_LIST::iterator iter = m_listResult.begin();
		while(iter != m_listResult.end())
		{
			removeChild(iter->get());
			iter++;
		}
		m_listResult.clear();
	}

	void CMeasure::UpdateResult()
	{
		if(IsMeasuring())
		{
			GetOrCreateCurrentResult()->Calculate(Vertexes());
			SendMeasureMsg(EM_UPDATE);
		}
	}

	bool CMeasure::BeginMeasure()
	{
		if(IsMeasuring()) return false;
		m_bDoingMeasure = true;

		ClearCtrlPoints();
		SendMeasureMsg(EM_BEGIN);

		return true;
	}

	bool CMeasure::EndMeasure()
	{
		if(!IsMeasuring()) return false;
		
		GetOrCreateCurrentResult()->FinalCalculate();
		
		SendMeasureMsg(EM_END);
		m_bDoingMeasure = false;

		return true;
	}

	void CMeasure::RegisterObserver( CMeasureCallback* pObserver )
	{
		if(pObserver)
		{
			m_listObserver.push_back(pObserver);
		}
	}

	void CMeasure::RemoveObserver( CMeasureCallback* pObserver )
	{
		MEASURE_OBSERVER_LIST::iterator itr = std::find(m_listObserver.begin(), m_listObserver.end(), pObserver);
		if(itr != m_listObserver.end())
		{
			m_listObserver.erase(itr);
		}
	}

	void CMeasure::SendMeasureMsg( EMeasureAction action )
	{
		for(MEASURE_OBSERVER_LIST::iterator itr = m_listObserver.begin(); itr != m_listObserver.end(); ++itr)
		{
			if((*itr).valid())
			{
				(*itr).get()->SendMsg(action, this);
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	/// 工具函数，考虑放到FeUtil中
	std::string ReviseDistanceCalculate( double dDistance )
	{
		std::stringstream ss;

		if(dDistance >= 1000.0)
		{
			dDistance /= 1000.0;
			ss << std::setiosflags(std::ios::fixed)<<std::setprecision(2) << dDistance << " km ";
		}
		else
		{
			ss << std::setiosflags(std::ios::fixed)<<std::setprecision(4) << dDistance << " m ";
		}

		return ss.str();
	}

	std::string ReviseAreaCalculate(double dArea)
	{
		std::stringstream ss;

		if (dArea >= 1000000.0) //平方千米
		{
			dArea /= 1000000.0;
			ss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << dArea << " km^2 ";
		}
		else
		{
			ss << std::setiosflags(std::ios::fixed) << std::setprecision(4) << dArea << " m^2 ";
		}

		return ss.str();
	}
}

