
#include <osgDB/ReadFile>

#include <osgEarth/GeoMath>
#include <osgEarth/SpatialReference>

#include <FeUtils/CoordConverter.h>
#include <FeUtils/MiniAnimationBuilder.h>
#include <FeUtils/MathUtil.h>

#define FLOAT_EQUAL(x, y)  ((x<y+0.00001) && (x>y-0.00001))


namespace FeUtil
{
	CMotionStateInfo::CMotionStateInfo( osg::Vec3d llh /*= osg::Vec3d(0.0, 0.0, 0.0)*/, 
		osg::Vec3d posture /*= osg::Vec3d(0.0, 0.0, 0.0)*/, 
		double tim /*= -1.0*/, 
		double spe /*= 0.0*/, 
		osg::Vec3d xyz /*= osg::Vec3d(0.0, 0.0, 0.0)*/, 
		osg::Vec3d scale /*= osg::Vec3d(1,1,1)*/ )
		: vecLLH(llh)
		, vecPosture(posture)
		, dTime(tim)
		, dSpeed(spe) 
		, vecXYZ(xyz)
		, vecScale(scale)
	{

	}

	void CMotionStateInfo::interpolate(double radio, const CMotionStateInfo& start, const CMotionStateInfo& end)
	{
		vecLLH = start.vecLLH + (end.vecLLH - start.vecLLH) * radio;
		vecPosture = start.vecPosture + (end.vecPosture - start.vecPosture) * radio;
		double yawRadianStart = start.vecPosture.z();
		double yawRadianEnd = end.vecPosture.z();
		if (yawRadianEnd * yawRadianStart < 0.0 && fabs(yawRadianStart - yawRadianEnd) >= osg::PI)
		{
			if (yawRadianStart > yawRadianEnd)
			{
				vecPosture.z() = yawRadianStart + (osg::PI*2.0 - yawRadianStart + yawRadianEnd) * radio;
			}
			else {
				vecPosture.z() = yawRadianStart + (-osg::PI*2.0 - yawRadianStart + yawRadianEnd) * radio;
			}
		}

		dTime = start.dTime + (end.dTime - start.dTime) * radio;
		dSpeed = start.dSpeed + (end.dSpeed - start.dSpeed) * radio;
	}
}


namespace FeUtil
{
	CMiniAnimationPathBuilder::CMiniAnimationPathBuilder(CRenderContext *pRenderContext)
		:osg::Referenced()
		,m_rpAnimationPath(NULL)
		,m_opRenderContext(pRenderContext)
	{
		
	}

	CMiniAnimationPathBuilder::~CMiniAnimationPathBuilder()
	{

	}

	FeUtil::AnimationPath *CMiniAnimationPathBuilder::CreateAnimationPath( const std::vector<SDriveData> &vecDriveData )
	{
		if(!m_rpAnimationPath.valid())
		{
			m_rpAnimationPath = new FeUtil::AnimationPath();
			
		}
		else
		{
			m_rpAnimationPath->clear();
		}
		//osg::ref_ptr<FeUtil::AnimationPath> rpAnimationPath = new FeUtil::AnimationPath();
		m_rpAnimationPath->setLoopMode(AnimationPath::NO_LOOPING);
		unsigned unVertexNum = vecDriveData.size();
		if(unVertexNum <= 0)
		{
			return NULL;
		}

		double dTime = vecDriveData.at(0).dTime;

		osg::Vec3d vecCur;
		osg::Vec3d vecNext;
		osg::Matrix matrixCur;

		
		for(unsigned unIndex = 0; unIndex < unVertexNum; ++unIndex)
		{
			osg::Vec3d vecLLaFrom = osg::Vec3d(vecDriveData.at(unIndex).vecLLH);
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),vecLLaFrom,vecCur);

			FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(),vecLLaFrom,matrixCur);

			osg::Quat rotation;
			osg::Vec3d vecPosture =  vecDriveData.at(unIndex).vecPosture;
			rotation.makeRotate(/*osg::DegreesToRadians*/(vecPosture.x()) ,osg::X_AXIS,
				/*osg::DegreesToRadians*/(vecPosture.y()),osg::Y_AXIS,
				/*osg::DegreesToRadians*/(vecPosture.z()),osg::Z_AXIS);

			//matrixCur.preMultRotate(rotation);		
			double dCurrT = vecDriveData.at(unIndex).dTime - dTime;

			//if(dCurrT < 0.00000001)
			//{
			//	return NULL;
			//}

			m_rpAnimationPath->insert(dCurrT, FeUtil::AnimationPath::ControlPoint(vecCur, rotation*matrixCur.getRotate(), vecDriveData.at(unIndex).vecScale));
		}
		//return rpAnimationPath.release();
		return m_rpAnimationPath.get();
	}

	FeUtil::AnimationPath *CMiniAnimationPathBuilder::CreateAnimationPath(const std::vector<osg::Vec3d> &vecLLHSet, double dSpeed)
	{
		if (!m_rpAnimationPath.valid())
		{
			m_rpAnimationPath = new FeUtil::AnimationPath();

		}
		else
		{
			m_rpAnimationPath->clear();
		}

		unsigned unVertexNum = vecLLHSet.size();
		if (unVertexNum <= 0)
		{
			return NULL;
		}

		// 设置循环模型
		m_rpAnimationPath->setLoopMode(AnimationPath::NO_LOOPING);

		double dTime = 0.0;

		osg::Vec3d vecCur;
		osg::Vec3d vecNext;
		osg::Matrix matrixCur;

		for (unsigned unIndex = 0; unIndex < unVertexNum-1; ++unIndex)
		{
			osg::Vec3d vecLLaFrom = osg::Vec3d(vecLLHSet.at(unIndex));
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), vecLLaFrom, vecCur);

			FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), vecLLaFrom, matrixCur);

			osg::Quat rotation;
			osg::Vec3d vecPosture(0.0, 0.0, 0.0); 
			FeUtil::GetAngle(m_opRenderContext.get(), vecLLHSet.at(unIndex), vecLLHSet.at(unIndex + 1), vecPosture.z(), vecPosture.x());
			rotation.makeRotate(vecPosture.x(), osg::X_AXIS, vecPosture.y(), osg::Y_AXIS, vecPosture.z(), osg::Z_AXIS);

			matrixCur.preMultRotate(rotation);

			m_rpAnimationPath->insert(dTime, FeUtil::AnimationPath::ControlPoint(vecCur, matrixCur.getRotate()));

			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), vecLLHSet.at(unIndex+1), vecNext);
			dTime += FeUtil::GetRunTime(vecCur, vecNext, dSpeed);
		}

		m_rpAnimationPath->insert(dTime, FeUtil::AnimationPath::ControlPoint(vecNext, matrixCur.getRotate()));

		return m_rpAnimationPath.get();
	}

}

namespace FeUtil
{
	CRotateAnimationPath::CRotateAnimationPath(CRenderContext *pRenderContext)
		:osg::Referenced()
		,m_rpAnimationPath(NULL)
		,m_opRenderContext(pRenderContext)
	{

	}

	CRotateAnimationPath::~CRotateAnimationPath()
	{

	}

	//按角度,绕Z轴转
	FeUtil::AnimationPath *CRotateAnimationPath::CreateAnimationPath(osg::Vec3d vecCenter,std::vector<RotateData> vecRotateData)
	{
		if(!m_rpAnimationPath.valid())
		{
			m_rpAnimationPath = new FeUtil::AnimationPath();

		}
		else
		{
			m_rpAnimationPath->clear();
		}
		m_rpAnimationPath->setLoopMode(AnimationPath::NO_LOOPING);
		unsigned unVertexNum = vecRotateData.size();
		if(unVertexNum <= 0)
		{
			return NULL;
		}

		double dTime = 0.0;
		double dCurrZAngle = vecRotateData.at(0).dAngle;
		
		for(unsigned unIndex = 0; unIndex < unVertexNum; ++unIndex)
		{
			osg::Quat rotation;
			double dCurrZAngle = vecRotateData.at(unIndex).dAngle;
			rotation.makeRotate(osg::DegreesToRadians(dCurrZAngle),osg::Z_AXIS);
			//matrixCur.setRotate(rotation);		
			m_rpAnimationPath->insert(dTime, FeUtil::AnimationPath::ControlPoint(vecCenter, rotation));
			//rpAnimationPath->insert(dTime, FeUtil::AnimationPath::ControlPoint(vecCenter, matrixCur.getRotate()));
			dTime += 0.5;
		}

		return m_rpAnimationPath.get();
	}

	//按周期转
	FeUtil::AnimationPath * CRotateAnimationPath::CreateAnimationPath( osg::Vec3d vecCenter,double dPeriod )
	{
		if(!m_rpAnimationPath.valid())
		{
			m_rpAnimationPath = new FeUtil::AnimationPath();

		}
		m_rpAnimationPath->setLoopMode(FeUtil::AnimationPath::LOOP);
		if(dPeriod <= 0)
		{
			return NULL;
		}
		
		int numSamples = 36;
		float yaw = 0.0f;
		float yaw_delta = 360/((float)numSamples);

		double time=0.0f;
		double time_delta = dPeriod/(double)numSamples;

		for(int i=0;i<= numSamples; ++i)
		{
			//osg::Vec3 position(m_vecCenter + osg::Vec3(sinf(yaw) ,cosf(yaw) , 0.0f));
			osg::Quat rotation;//(osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Z_AXIS));
			rotation.makeRotate(osg::DegreesToRadians(yaw),osg::Z_AXIS);

			m_rpAnimationPath->insert(time,FeUtil::AnimationPath::ControlPoint(vecCenter,rotation));

			yaw += yaw_delta;
			time += time_delta;

		}

		return m_rpAnimationPath.get();
	}


}







