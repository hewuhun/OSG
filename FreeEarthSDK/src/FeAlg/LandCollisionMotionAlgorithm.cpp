#include <FeUtils/LandCollisionMotionAlgorithm.h>
#include <FeUtils/LandMotionAlgorithm.h>
#include <osgEarth/GeoMath>

#include <cmath>
#include <queue>
#include <map>
#include <vector>

#include <osgEarth/DPLineSegmentIntersector>
#include <FeUtils/CoordConverter.h>

namespace FeUtil
{
	bool FindCollisionPositionFromLandPath(CRenderContext *pRenderContext, std::vector<CMotionStateInfo>& vecStateInfo, double dTime, double dSpeed, CMotionStateInfo& output, int& lastPosition)
	{
		if (pRenderContext == NULL || vecStateInfo.size() < 2)
		{
			return false;
		}

		if (vecStateInfo[lastPosition].dTime > dTime)
		{
			return false;
		}

		if (dSpeed == 0.0)
		{
			return true;
		}

		if (lastPosition == 0)
		{
			if (vecStateInfo[0].dTime < 0.0)
			{
				double height = vecStateInfo[0].vecLLH.z();
				//FeUtil::CalculateCollisionPoint(pRenderContext, vecStateInfo[0].vecLLH);
				if (fabs(height - vecStateInfo[0].vecLLH.z()) > 50.0)
				{
					vecStateInfo[0].vecLLH.z() = height;
					return false;
				}

				FeUtil::DegreeLLH2XYZ(pRenderContext, vecStateInfo[0].vecLLH, vecStateInfo[0].vecXYZ);
				vecStateInfo[0].dTime = 0.0;
			}
			++lastPosition;
		}

		//OSG_NOTICE << "当前时间： " << dTime << std::endl;
		for (int i = lastPosition; i < vecStateInfo.size(); ++i)
		{
			if (vecStateInfo[i].dTime < 0.0)
			{
				//FeUtil::CalculateCollisionPoint(pRenderContext, vecStateInfo[i].vecLLH);
				FeUtil::DegreeLLH2XYZ(pRenderContext, vecStateInfo[i].vecLLH, vecStateInfo[i].vecXYZ);
				double dis = DistanceTwoXYZ(vecStateInfo[i - 1].vecXYZ, vecStateInfo[i].vecXYZ);
				double timeTemp = dis / dSpeed + vecStateInfo[i - 1].dTime;
				//OSG_NOTICE << "新的计算时间： " << timeTemp << std::endl;
				vecStateInfo[i].dTime = timeTemp;
				if (timeTemp > dTime)
				{
					double radio = (dTime - vecStateInfo[i - 1].dTime) / (timeTemp - vecStateInfo[i - 1].dTime);
					output.interpolate(radio, vecStateInfo[i - 1], vecStateInfo[i]);
					lastPosition = i - 1;
					return true;
				}
			}
			else
			{
				//OSG_NOTICE << "以前计算的时间： " << vecStateInfo[i].dTime << std::endl;
				if (vecStateInfo[i].dTime > dTime)
				{
					double radio = (dTime - vecStateInfo[i - 1].dTime) / (vecStateInfo[i].dTime - vecStateInfo[i - 1].dTime);
					output.interpolate(radio, vecStateInfo[i - 1], vecStateInfo[i]);
					lastPosition = i - 1;
					return true;
				}
			}
		}

		if (dTime > vecStateInfo[vecStateInfo.size() - 1].dTime)
		{
			lastPosition = vecStateInfo.size() - 1;
			output = vecStateInfo[lastPosition];
			return true;
		}
		return false;
	}

	bool CalculateCollisionSlopeAndPitchRadian(FeUtil::CRenderContext *pRenderContext, FeUtil::CMotionStateInfo& curState, osg::Vec3d front, osg::Vec3d back, osg::Vec3d left, osg::Vec3d right)
	{
		if (pRenderContext == NULL)
		{
			return false;
		}

		osg::Quat rotation;
		rotation.makeRotate(curState.vecPosture.z(), osg::Z_AXIS);

		FeUtil::CalculateCollisionPoint(pRenderContext, curState.vecLLH);  // 有错误，此处碰撞已经有小车参与 
		osg::Matrix curMatrix;
		FeUtil::DegreeLLH2Matrix(pRenderContext, curState.vecLLH, curMatrix);
		curMatrix.preMultRotate(rotation);

		front = front * curMatrix;
		back = back * curMatrix;
		left = left * curMatrix;
		right = right * curMatrix;

		osg::Vec3d frontLLH, backLLH, leftLLH, rightLLH;
		FeUtil::XYZ2DegreeLLH(pRenderContext, front, frontLLH);
		FeUtil::XYZ2DegreeLLH(pRenderContext, back, backLLH);
		FeUtil::XYZ2DegreeLLH(pRenderContext, left, leftLLH);
		FeUtil::XYZ2DegreeLLH(pRenderContext, right, rightLLH);
		FeUtil::CalculateCollisionPoint(pRenderContext, frontLLH);
		FeUtil::CalculateCollisionPoint(pRenderContext, backLLH);
		FeUtil::CalculateCollisionPoint(pRenderContext, leftLLH);
		FeUtil::CalculateCollisionPoint(pRenderContext, rightLLH);

		if (frontLLH.z() < backLLH.z())
		{
			CalculatePicthRadian(pRenderContext, curState.vecLLH, frontLLH, curState.vecPosture.x());
		}
		else {
			CalculatePicthRadian(pRenderContext, backLLH, curState.vecLLH, curState.vecPosture.x());
		}


		if (leftLLH.z() < rightLLH.z())
		{
			CalculatePicthRadian(pRenderContext, curState.vecLLH, leftLLH, curState.vecPosture.y());
		}
		else
		{
			CalculatePicthRadian(pRenderContext, rightLLH, curState.vecLLH, curState.vecPosture.y());
		}
		return true;
	}


	void CalculateCollisionPoint(FeUtil::CRenderContext *pRenderContext, osg::Vec3d& inOutPoint, double upDis)
	{
		// 检测碰撞点，重新确定规划点
		osg::Vec3d oldLLH = inOutPoint, oldXYZ;
		osg::Vec3d oldUpLLH = oldLLH, oldUpXYZ;
		oldUpLLH.z() += upDis;
		FeUtil::DegreeLL2LLH(pRenderContext, oldLLH);
		FeUtil::DegreeLLH2XYZ(pRenderContext, oldLLH, oldXYZ);
		FeUtil::DegreeLLH2XYZ(pRenderContext, oldUpLLH, oldUpXYZ);

		osgEarth::DPLineSegmentIntersector *picker = new osgEarth::DPLineSegmentIntersector(oldUpXYZ, oldXYZ);

		osgUtil::IntersectionVisitor iv(picker);

		pRenderContext->GetRoot()->accept(iv);

		osg::Vec3d newLLH = oldLLH;
		if (picker->containsIntersections())
		{
			osg::Vec3d newXYZ = picker->getFirstIntersection().getWorldIntersectPoint();
			FeUtil::XYZ2DegreeLLH(pRenderContext, newXYZ, newLLH);
		}
		inOutPoint = newLLH;
	}

}


