#include <FeAlg/LandMotionAlgorithm.h>
#include <FeAlg/FlightMotionAlgorithm.h>

#include <osgEarth/GeoMath>

#include <cmath>
#include <queue>
#include <map>
#include <vector>

#include <FeUtils/CoordConverter.h>

using namespace FeUtil;
using namespace FeAlg;

namespace FeAlg
{
	bool LandInterpolationAndSphereSmooth(CRenderContext *pRenderContext, const std::vector<osg::Vec3d>& vecInput,
		std::vector<CMotionStateInfo>& vecOutput, double disSphereInter /*= 100.0*/, double disMaxDis /*= 5.0*/)
	{
		if (pRenderContext == NULL || vecInput.size() < 2 || disSphereInter < 0.0 || disMaxDis < 0.0)
		{
			return false;
		}

		// 首先进行球面插值, 插值的间距是disSphereInter, 球面插值只与经纬度有关系
		std::vector<CMotionStateInfo> vecStateInfo;
		InterpolationSphereRhumb(vecInput, vecStateInfo, disSphereInter);

		// 更新插入点的高度, 同时计算偏航角, 由于数据量大，遍历的同时计算偏航角, 偏航角与高度没有关系， 只与经纬度有关
		for (int i = 0; i < vecStateInfo.size()-1; ++i)
		{
			FeUtil::DegreeLL2LLH(pRenderContext, vecStateInfo[i].vecLLH);
			FeUtil::DegreeLLH2XYZ(pRenderContext, vecStateInfo[i].vecLLH, vecStateInfo[i].vecXYZ);
			CalculateYawRadian(pRenderContext, vecStateInfo[i], vecStateInfo[i + 1]);
		}
		FeUtil::DegreeLL2LLH(pRenderContext, vecStateInfo[vecStateInfo.size()-1].vecLLH);
		FeUtil::DegreeLLH2XYZ(pRenderContext, vecStateInfo[vecStateInfo.size()-1].vecLLH, vecStateInfo[vecStateInfo.size()-1].vecXYZ);
		vecStateInfo[vecStateInfo.size() - 1].vecPosture.z() = vecStateInfo[vecStateInfo.size() - 2].vecPosture.z();
		//OSG_NOTICE << "更新高度 与 世界坐标完成" << std::endl;

		// 在vec3dOut之间根据地形进行插值，间距是disMaxDis
		for (int i = 0; i < vecStateInfo.size() - 1; ++i)
		{
			//OSG_NOTICE << " 根据地形进行插值 =====  " << i << std::endl;
			vecOutput.push_back(vecStateInfo[i]);
			LandInterpolationBetweenTwoLLH(pRenderContext, vecStateInfo[i], vecStateInfo[i + 1], vecOutput, disMaxDis);
		}
		vecOutput.push_back(vecStateInfo[vecStateInfo.size() - 1]);
		//OSG_NOTICE << " 根据地形进行插值  完成  -- "  <<vecOutput.size() << std::endl;

		//for (int i = 0; i < vecOutput.size() - 1; ++i)
		//{
		//	if (DistanceTwoXYZ(vecOutput[i].vecXYZ, vecOutput[i+1].vecXYZ) > disMaxDis)
		//	{
		//		OSG_NOTICE << " 插值有问题 -- "  << i << std::endl;
		//	}
		//}
		return true;
	}

	void InterpolationSphereRhumb(const std::vector<osg::Vec3d>& vecInput, std::vector<CMotionStateInfo>& vecOutput, double disSphereInter)
	{
		for (int i = 0; i < vecInput.size() - 1; ++i)
		{
			vecOutput.push_back(CMotionStateInfo(vecInput[i]));
			double twoPointsDis = osgEarth::GeoMath::rhumbDistance(
							osg::DegreesToRadians(vecInput[i].y()), osg::DegreesToRadians(vecInput[i].x()),
							osg::DegreesToRadians(vecInput[i+1].y()), osg::DegreesToRadians(vecInput[i+1].x()));
			int numInter = twoPointsDis / disSphereInter;

			for (int j = 1; j < numInter; ++j)
			{
				double radLat, radLon;
				osgEarth::GeoMath::interpolate(
					osg::DegreesToRadians(vecInput[i].y())
					, osg::DegreesToRadians(vecInput[i].x())
					, osg::DegreesToRadians(vecInput[i+1].y())
					, osg::DegreesToRadians(vecInput[i+1].x())
					, double(j) / double(numInter)
					, radLat
					, radLon);

				vecOutput.push_back(CMotionStateInfo(osg::Vec3d(osg::RadiansToDegrees(radLon), osg::RadiansToDegrees(radLat), 0.0)));
			}
		}
		vecOutput.push_back(CMotionStateInfo(vecInput.at(vecInput.size() - 1)));
	}

	void LandInterpolationBetweenTwoLLH(CRenderContext *pRenderContext, CMotionStateInfo stateStart,
		CMotionStateInfo stateEnd, std::vector<CMotionStateInfo>& vecOutput, double disMaxDis /*= 5.0*/)
	{
		if (pRenderContext == NULL || disMaxDis <= 0.0)
		{
			return;
		}

		std::map<long double, CMotionStateInfo> mapDisToVec3d;
		typedef std::pair<CMotionStateInfo, CMotionStateInfo> Vec3dPair;
		std::queue<Vec3dPair> pairQueue;
		pairQueue.push(std::make_pair(stateStart, stateEnd));

		while (!pairQueue.empty())
		{
	        Vec3dPair curPair = pairQueue.front();
			//OSG_NOTICE << " queue Num: " << pairQueue.size() << std::endl;
			if (DistanceTwoXYZ(curPair.first.vecXYZ, curPair.second.vecXYZ) > disMaxDis)
			{
				osg::Vec3d mid = curPair.first.vecLLH * 0.5 + curPair.second.vecLLH	* 0.5;
				CMotionStateInfo midState(mid, stateStart.vecPosture);
				FeUtil::DegreeLL2LLH(pRenderContext, midState.vecLLH);
				FeUtil::DegreeLLH2XYZ(pRenderContext, midState.vecLLH, midState.vecXYZ);

				mapDisToVec3d[DistanceTwoLL(stateStart.vecLLH, midState.vecLLH)] = midState;
				pairQueue.push(std::make_pair(curPair.first, midState));
				pairQueue.push(std::make_pair(curPair.second, midState));
			}
			pairQueue.pop();
		}

		std::map<long double, CMotionStateInfo>::const_iterator iter = mapDisToVec3d.begin(), iterEnd = mapDisToVec3d.end();
		for (; iter != iterEnd; ++iter)
		{
			vecOutput.push_back(iter->second);
		}
	}

	void CalculateYawRadian(CRenderContext *pRenderContext, CMotionStateInfo &first, const CMotionStateInfo &second)
	{
		if (pRenderContext == NULL)
		{
			return;
		}

		double& yawRadian = first.vecPosture.z();
		if (fabs(first.vecLLH.y() - second.vecLLH.y()) <= DBL_EPSILON)
		{
			double detal = second.vecLLH.x() - first.vecLLH.x();//计算经度差
			if (detal < -180.0 || (detal>0.0 && detal < 180.0))
			{
				// 在其右侧180度内
				yawRadian = osg::DegreesToRadians(-90.0);//逆时针转为正向，所以在右侧时转的角度为负值。
			}
			else
			{
				// 在其左侧180度内
				yawRadian = osg::DegreesToRadians(90.0);
			}
		}
		// 如果第一个点的经度和第二个点的经度相同则航向角为0度或者180度
		else if (fabs(first.vecLLH.x() - second.vecLLH.x()) <= DBL_EPSILON)
		{
			double detal = second.vecLLH.y() - first.vecLLH.y();
			if (detal > 0.0)
			{
				yawRadian = osg::DegreesToRadians(0.0);
			}
			else
			{
				yawRadian = osg::DegreesToRadians(180.0);
			}
		}
		// 第一个点和第二个点的经度和维度都不同，此种情况下只考虑xy坐标就可以了不用考虑z即只在XOY平面做计算就可以了
		else
		{
			osg::Vec3d three(first.vecLLH.x(), second.vecLLH.y(), first.vecLLH.z()), threeXYZ;
			FeUtil::DegreeLLH2XYZ(pRenderContext, three, threeXYZ);
			osg::Vec3d secondBottom(second.vecLLH.x(), second.vecLLH.y(), first.vecLLH.z()), secondBottomXYZ;
			FeUtil::DegreeLLH2XYZ(pRenderContext, secondBottom, secondBottomXYZ);
			long double dDistance1 = DistanceTwoXYZ(threeXYZ, secondBottomXYZ);
			long double dDistance2 = DistanceTwoXYZ(first.vecXYZ, threeXYZ);

			yawRadian = atanl(dDistance1 / dDistance2);
			////如果第二个点在第一个点的右边
			double detalX = second.vecLLH.x() - first.vecLLH.x();
			if (detalX < -180 || (detalX>0 && detalX < 180))
			{
				if (second.vecLLH.y() > first.vecLLH.y())
				{
					yawRadian = -yawRadian;
				}
				else
				{
					yawRadian = -(osg::PI - yawRadian);
				}
			}
			//第二点在第一个点的左侧
			else
			{
				if (second.vecLLH.y() < first.vecLLH.y())
				{
					yawRadian = osg::PI - yawRadian;
				}
			}
		}
	}

	long double DistanceTwoXYZ(const osg::Vec3d &first, const osg::Vec3d& second)
	{
		osg::Vec3d result = first - second;
		return sqrtl(result.x() * result.x() + result.y() * result.y() + result.z() * result.z());
	}

	long double DistanceTwoLL(const osg::Vec3d &first, const osg::Vec3d& second)
	{
		long double dis = osgEarth::GeoMath::distance(
			osg::DegreesToRadians(first.y()),
			osg::DegreesToRadians(first.x()),
			osg::DegreesToRadians(second.y()),
			osg::DegreesToRadians(second.x()));
		return dis;
	}
}

namespace FeAlg
{
	bool FindPositionFromLandPath(CRenderContext *pRenderContext, std::vector<CMotionStateInfo>& vecStateInfo,
		double dTime, double dSpeed, CMotionStateInfo& output, int& lastPosition)
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
				FeUtil::DegreeLL2LLH(pRenderContext, vecStateInfo[0].vecLLH);
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
				FeUtil::DegreeLL2LLH(pRenderContext, vecStateInfo[i].vecLLH);
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

	bool CalculateSlopeAndPitchRadian(CRenderContext *pRenderContext, CMotionStateInfo& curState, osg::Vec3d front, osg::Vec3d back, osg::Vec3d left, osg::Vec3d right)
	{
		if (pRenderContext == NULL)
		{
			return false;
		}

		osg::Quat rotation;
		rotation.makeRotate(curState.vecPosture.z(), osg::Z_AXIS);

		FeUtil::DegreeLL2LLH(pRenderContext, curState.vecLLH);
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
		FeUtil::DegreeLL2LLH(pRenderContext, frontLLH);
		FeUtil::DegreeLL2LLH(pRenderContext, backLLH);
		FeUtil::DegreeLL2LLH(pRenderContext, leftLLH);
		FeUtil::DegreeLL2LLH(pRenderContext, rightLLH);

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

	void CalculatePicthRadian(CRenderContext *pRenderContext, osg::Vec3d first, osg::Vec3d second, double &pitchRadian)
	{
		if (pRenderContext == NULL)
		{
			return; 
		}

		// 记录第一个点转换为世界坐标后的值
		osg::Vec3d firstXYZd;
		FeUtil::DegreeLLH2XYZ(pRenderContext, first, firstXYZd);
		// 记录第二个点转换为世界坐标后的值
		osg::Vec3d secondXYZd;
		FeUtil::DegreeLLH2XYZ(pRenderContext, second, secondXYZd);

		// 记录第二点下面一点的位置
		osg::Vec3d secondBottom(second.x(), second.y(), first.z()), secondBottomXYZd;
		FeUtil::DegreeLLH2XYZ(pRenderContext, secondBottom, secondBottomXYZd);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 首先计算俯仰角, 在高度方向上，通过世界坐标计算角度，初始时飞机方向朝北，计算角度后得人为调整。
		if (fabs(first.z() - second.z()) <= DBL_EPSILON)
		{
			pitchRadian = 0.0;
		}
		else
		{
			long double distance = DistanceTwoXYZ(firstXYZd, secondXYZd);
			long double detal = DistanceTwoXYZ(secondXYZd, secondBottomXYZd);
			if (detal >= distance)
			{
				if (detal > 0.0) {
					pitchRadian = osg::DegreesToRadians(90.0);
				}
				else {
					pitchRadian = osg::DegreesToRadians(-90.0);
				}
			}
			else
			{
				pitchRadian = asinl(detal / distance);
			}
			if (second.z() < first.z() && pitchRadian > 0.0)
			{
				pitchRadian = -pitchRadian;
			}
		}
	}

	bool GetFirstInterpolationRhumb(const osg::Vec3d start, const osg::Vec3d end, osg::Vec3d& output, double deltaDistance)
	{
		double dDistance = osgEarth::GeoMath::rhumbDistance(
			osg::DegreesToRadians(start.y()), osg::DegreesToRadians(start.x()),
			osg::DegreesToRadians(end.y()), osg::DegreesToRadians(end.x()));

		int nTotalNum = dDistance / deltaDistance;

		for (int i = 1; i < nTotalNum; i++)
		{
			double radLat, radLon;
			osgEarth::GeoMath::interpolate(
				osg::DegreesToRadians(start.y())
				, osg::DegreesToRadians(start.x())
				, osg::DegreesToRadians(end.y())
				, osg::DegreesToRadians(end.x())
				, double(i) / double(nTotalNum)
				, radLat
				, radLon);

			output.set(osg::Vec3d(osg::RadiansToDegrees(radLon), osg::RadiansToDegrees(radLat), 0.0));
			return true;
		}
		return false;
	}

	void InterpolationSphereRhumbAndCorner(CRenderContext *pRenderContext, const std::vector<osg::Vec3d>& vecInput, std::vector<CMotionStateInfo>& vecOutput,
		double disPressIn, double disSphereInter, double disMaxDis)
	{
		if (vecInput.size() < 2 || pRenderContext == NULL || disSphereInter <= 0.0 || disPressIn <= 0.0 || disMaxDis <= 0.0 )
		{
			return;
		}

		// 添加初始点
		vecOutput.push_back(CMotionStateInfo(vecInput.at(0)));

		// 处理中间拐弯点
		for (int i = 1; i < vecInput.size() - 1; ++i)
		{
			// 首先处理拐角需要平滑的点
			osg::Vec3d startSmooth, endSmooth;
			// 寻找第一个平滑点，如果寻找成功，则在之前进行插值，如果寻找失败，选取中间点作为插入点。
			if (!GetFirstInterpolationRhumb(vecInput[i], vecOutput.at(vecOutput.size() - 1).vecLLH, startSmooth, disPressIn))
			{
				startSmooth = vecInput[i] * 0.5 + vecOutput.at(vecOutput.size() - 1).vecLLH * 0.5; 
			}
			startSmooth.z() = 0.0;
			InterpolationSphereRhumb(vecOutput.at(vecOutput.size() - 1).vecLLH, startSmooth, vecOutput, disSphereInter);
			if (!GetFirstInterpolationRhumb(vecInput[i], vecInput[i + 1], endSmooth, disPressIn))
			{
				endSmooth = vecInput[i] * 0.5 + vecInput[i + 1] * 0.5; 
			}
			endSmooth.z() = 0.0;
			std::vector<osg::Vec3d> vecCorner;
			InterpolationCornerSphereSmooth(pRenderContext, startSmooth, endSmooth, 
				osg::Vec3d(vecInput[i].x(), vecInput[i].y(), 0.0), vecCorner, disMaxDis);
			for (int i = 0; i < vecCorner.size(); ++i) 
			{
				vecOutput.push_back(CMotionStateInfo(vecCorner[i]));
			}
		}
		InterpolationSphereRhumb(vecOutput.at(vecOutput.size() - 1).vecLLH, vecInput.at(vecInput.size() - 1), vecOutput, disSphereInter);
		vecOutput.push_back(CMotionStateInfo(vecInput.at(vecInput.size() - 1)));
	}

	bool LandInterpolationAndSphereCornerSmooth(CRenderContext *pRenderContext, const std::vector<osg::Vec3d>& vecInput,
		std::vector<CMotionStateInfo>& vecOutput, double disSphereInter /*= 100.0*/, double disPressIn /*= 20.0*/, double disMaxDis /*= 1.0*/)
	{
		if (pRenderContext == NULL || vecInput.size() < 2 || disSphereInter < 0.0 || disMaxDis < 0.0)
		{
			return false;
		}

		// 首先进行球面插值, 插值的间距是disSphereInter, 球面插值只与经纬度有关系
		std::vector<CMotionStateInfo> vecStateInfo;
		InterpolationSphereRhumbAndCorner(pRenderContext, vecInput, vecStateInfo, disPressIn, disSphereInter, disMaxDis);

		// 更新插入点的高度, 同时计算偏航角, 由于数据量大，遍历的同时计算偏航角, 偏航角与高度没有关系， 只与经纬度有关
		for (int i = 0; i < vecStateInfo.size() - 1; ++i)
		{
			FeUtil::DegreeLL2LLH(pRenderContext, vecStateInfo[i].vecLLH);
			FeUtil::DegreeLLH2XYZ(pRenderContext, vecStateInfo[i].vecLLH, vecStateInfo[i].vecXYZ);
			CalculateYawRadian(pRenderContext, vecStateInfo[i], vecStateInfo[i + 1]);
		}
		FeUtil::DegreeLL2LLH(pRenderContext, vecStateInfo[vecStateInfo.size() - 1].vecLLH);
		FeUtil::DegreeLLH2XYZ(pRenderContext, vecStateInfo[vecStateInfo.size() - 1].vecLLH, vecStateInfo[vecStateInfo.size() - 1].vecXYZ);
		vecStateInfo[vecStateInfo.size() - 1].vecPosture.z() = vecStateInfo[vecStateInfo.size() - 2].vecPosture.z();
		//OSG_NOTICE << "更新高度 与 世界坐标完成" << std::endl;

		// 在vec3dOut之间根据地形进行插值，间距是disMaxDis
		for (int i = 0; i < vecStateInfo.size() - 1; ++i)
		{
			//OSG_NOTICE << " 根据地形进行插值 =====  " << i << std::endl;
			vecOutput.push_back(vecStateInfo[i]);
			LandInterpolationBetweenTwoLLH(pRenderContext, vecStateInfo[i], vecStateInfo[i + 1], vecOutput, disMaxDis);
		}
		vecOutput.push_back(vecStateInfo[vecStateInfo.size() - 1]);
		//OSG_NOTICE << " 根据地形进行插值  完成  -- "  <<vecOutput.size() << std::endl;

		//for (int i = 0; i < vecOutput.size() - 1; ++i)
		//{
		//	if (DistanceTwoXYZ(vecOutput[i].vecXYZ, vecOutput[i+1].vecXYZ) > disMaxDis)
		//	{
		//		OSG_NOTICE << " 插值有问题 -- "  << i << std::endl;
		//	}
		//}
		return true;
	}

	void InterpolationSphereRhumb(const osg::Vec3d start, const osg::Vec3d end, std::vector<CMotionStateInfo>& output, 
		double deltaDistance)
	{
		double dDistance = osgEarth::GeoMath::rhumbDistance(
			osg::DegreesToRadians(start.y()), osg::DegreesToRadians(start.x()),
			osg::DegreesToRadians(end.y()), osg::DegreesToRadians(end.x()));

		int nTotalNum = dDistance / deltaDistance;

		for (int i = 1; i < nTotalNum; i++)
		{
			double radLat, radLon;
			osgEarth::GeoMath::interpolate(
				osg::DegreesToRadians(start.y())
				, osg::DegreesToRadians(start.x())
				, osg::DegreesToRadians(end.y())
				, osg::DegreesToRadians(end.x())
				, double(i) / double(nTotalNum)
				, radLat
				, radLon);

			output.push_back(CMotionStateInfo(osg::Vec3d(osg::RadiansToDegrees(radLon), osg::RadiansToDegrees(radLat), 0.0)));
		}
	}

}


