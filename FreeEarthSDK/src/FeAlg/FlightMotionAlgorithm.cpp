#include <FeAlg/FlightMotionAlgorithm.h>
#include <FeAlg/LandMotionAlgorithm.h>

#include <osgEarth/GeoMath>

#include <cmath>
#include <queue>
#include <map>
#include <vector>

#include <FeUtils/CoordConverter.h>
#include <FeAlg/CoordConverter.h>
#include <osg/CoordinateSystemNode>

using namespace FeUtil;
using namespace FeAlg;

namespace FeAlg
{
	void FlightInterpolationAndSphereSmooth(CRenderContext *pRenderContext, const std::vector<osg::Vec3d>& vecInput,
		std::vector<CMotionStateInfo>& vecOutput, double disLineInter /*= 1000.0*/, double disPressIn /*= 200.0*/, 
		double disMaxSmooth /*= 10.0*/, double dRotatingPercent /*= 0.3*/, double dRollRadio /*= 90.0 / 60.0*/, double dRollMaxDegree /*= 60.0*/)
	{
		if (vecInput.size() < 2 || pRenderContext == NULL || disLineInter <= 0.0 || disPressIn <= 0.0 || disMaxSmooth <= 0.0 || dRotatingPercent < 0.0)
		{
			return;
		}
		std::vector<CMotionStateInfo> vecStateInput;
		for (int i = 0; i < vecInput.size(); ++i)
		{
			vecStateInput.push_back(CMotionStateInfo(vecInput[i]));
		}

		FlightInterpolationAndSphereSmooth(pRenderContext, vecStateInput, vecOutput, disLineInter, disPressIn, disMaxSmooth, dRotatingPercent, dRollRadio, dRollMaxDegree);

		// 更新角度
		for (int i = 0; i < vecOutput.size() - 1; ++i)
		{
			CalculateYawRadian(pRenderContext, vecOutput[i].vecLLH, vecOutput[i + 1].vecLLH, vecOutput[i].vecPosture.z());
			CalculatePicthRadian(pRenderContext, vecOutput[i].vecLLH, vecOutput[i + 1].vecLLH, vecOutput[i].vecPosture.x());
		}
		vecOutput.at(vecOutput.size() - 1).vecPosture.x() = vecOutput.at(vecOutput.size() - 2).vecPosture.x();
		vecOutput.at(vecOutput.size() - 1).vecPosture.z() = vecOutput.at(vecOutput.size() - 2).vecPosture.z();
	}

	void FlightInterpolationAndSphereSmooth(CRenderContext *pRenderContext, const std::vector<CMotionStateInfo>& vecInput,
		std::vector<CMotionStateInfo>& vecOutput, double disLineInter /*= 1000.0*/, double disPressIn /*= 200.0*/, 
		double disMaxSmooth /*= 10.0*/, double dRotatingPercent /*= 0.3*/, double dRollRadio /*= 90.0 / 60.0*/, double dRollMaxDegree /*= 60.0*/)
	{
		if (vecInput.size() < 2 || pRenderContext == NULL || disLineInter <= 0.0 || disPressIn <= 0.0 || disMaxSmooth <= 0.0 || dRotatingPercent < 0.0)
		{
			return;
		}

		// 添加初始点
		vecOutput.push_back(vecInput.at(0));

		// 处理中间拐弯点
		for (int i = 1; i < vecInput.size() - 1; ++i) 
		{
			// 首先处理拐角需要平滑的点
			osg::Vec3d startSmooth, endSmooth;
			// 寻找第一个平滑点，如果寻找成功，则在之前进行插值，如果寻找失败，选取中间点作为插入点。
			if (!GetFirstInterpolation(pRenderContext, vecInput[i].vecLLH, vecOutput.at(vecOutput.size()-1).vecLLH, startSmooth, disPressIn))
			{
				startSmooth= vecInput[i].vecLLH * 0.5 + vecOutput.at(vecOutput.size()-1).vecLLH * 0.5;
			}
			InterpolationSphere(pRenderContext, vecOutput.at(vecOutput.size()-1).vecLLH, startSmooth, vecOutput, disLineInter);
			if (!GetFirstInterpolation(pRenderContext, vecInput[i].vecLLH, vecInput[i + 1].vecLLH, endSmooth, disPressIn))
			{
				endSmooth = vecInput[i].vecLLH * 0.5 + vecInput[i + 1].vecLLH * 0.5;
			}
			InterpolationCornerSphereSmooth(pRenderContext, startSmooth, endSmooth, vecInput[i].vecLLH, vecOutput, disMaxSmooth, dRotatingPercent, dRollRadio, dRollMaxDegree);
		}
		InterpolationSphere(pRenderContext, vecOutput.at(vecOutput.size()-1).vecLLH, vecInput.at(vecInput.size()-1).vecLLH, vecOutput, disLineInter);
		vecOutput.push_back(vecInput.at(vecInput.size()-1));
	}

}

namespace FeAlg
{
	void InterpolationCornerSphereSmooth(CRenderContext *pRenderContext, 
		osg::Vec3d &start, osg::Vec3d end, osg::Vec3d control, std::vector<osg::Vec3d>& output, double disMaxInter /*= 10.0*/)
	{
		if (pRenderContext == NULL)
		{
			return;
		}

		// 计算球心与控制点向量
		osg::Vec3d startXYZ, endXYZ, controlXYZ;
		FeUtil::DegreeLLH2XYZ(pRenderContext, start, startXYZ);
		FeUtil::DegreeLLH2XYZ(pRenderContext, end, endXYZ);
		FeUtil::DegreeLLH2XYZ(pRenderContext, control, controlXYZ);

		// 修正位置, 让开始点、结束点距离控制点的距离相等。
		osg::Vec3d startVector = startXYZ - controlXYZ;
		osg::Vec3d endVector = endXYZ - controlXYZ;
		if (Length3D(startVector) > Length3D(endVector)) 
		{
			startXYZ = controlXYZ + startVector * (Length3D(endVector) / Length3D(startVector));
			FeUtil::XYZ2DegreeLLH(pRenderContext, startXYZ, start);
			startVector = startXYZ - controlXYZ;		
		} else
		{
			endXYZ = controlXYZ + endVector * (Length3D(startVector) / Length3D(endVector));
			endVector = endXYZ - controlXYZ;
		}
		
		osg::Vec3d midXYZ = startXYZ * 0.5 + endXYZ * 0.5;
		osg::Vec3d midVector = midXYZ - controlXYZ;

		// 计算半径, 首先计算夹角.
		long double angleHalfStartEnd = acosl(startVector * midVector / (Length3D(startVector)* Length3D(midVector)));
		long double radiusSphere = tan(angleHalfStartEnd) * Length3D(startVector);

		// 计算球心点坐标
		long double disCenterControl = radiusSphere / sin(angleHalfStartEnd);
		long double dRadio = disCenterControl / Length3D(midVector);
		osg::Vec3d centerPos = controlXYZ + midVector * dRadio;

		// 在压入点与压出点之间进行弧线插值，半径为radiusSphere
		std::map<long double, osg::Vec3d> interPoints;
		interPoints[0.0] = startXYZ;
		interPoints[Length3D(startXYZ-endXYZ)] = endXYZ;
		ProcessCornerSmooth(interPoints, centerPos, radiusSphere, disMaxInter);

		//osg::Vec3d midLLH;
		//FeUtil::XYZ2DegreeLLH(pRenderContext, midXYZ, midLLH);
		std::map<long double, osg::Vec3d>::const_iterator pIter, iterEnd = interPoints.end();
		for (pIter = interPoints.begin(); pIter != iterEnd; ++pIter)
		{
			osg::Vec3d pointLLH, pointXYZ(pIter->second);
			FeUtil::XYZ2DegreeLLH(pRenderContext, pointXYZ, pointLLH);
			output.push_back(pointLLH);
			//output.push_back(midLLH);
			//output.push_back(pointLLH);
		}
	}

	void InterpolationCornerSphereSmooth(CRenderContext *pRenderContext, osg::Vec3d start, osg::Vec3d end, osg::Vec3d control, 
		std::vector<CMotionStateInfo>& output, double disMaxInter /*= 10.0*/, 
		double dRotatingPercent /*= 0.3*/, double dRollRadio /*= 90.0 / 60.0*/, double dRollMaxDegree /*= 60.0*/)
	{
		// 计算插入点的点集
		std::vector<osg::Vec3d> vecLLHSet;
		InterpolationCornerSphereSmooth(pRenderContext, start, end, control, vecLLHSet, disMaxInter);
		// 对修正后的点进行插值
		InterpolationSphere(pRenderContext, output.at(output.size() - 1).vecLLH, start, output, disMaxInter);

		// 转换成MotionStateInfo
		std::vector<CMotionStateInfo> vecStateInfo;
		for (int i = 0; i < vecLLHSet.size(); ++i)
		{
			vecStateInfo.push_back(CMotionStateInfo(vecLLHSet[i]));
		}

		double yawRadianStart, yawRadianEnd, yawDiff;
		CalculateYawRadian(pRenderContext, start, control, yawRadianStart);
		CalculateYawRadian(pRenderContext, control, end, yawRadianEnd);

		if (yawRadianEnd * yawRadianStart < 0.0 && fabs(yawRadianStart - yawRadianEnd) >= osg::PI)
		{
			if (yawRadianStart > yawRadianEnd)
			{
				yawDiff = -(osg::PI * 2.0 - yawRadianStart + yawRadianEnd);
			}
			else {
				yawDiff = osg::PI * 2.0 - yawRadianEnd + yawRadianStart;
			}
		}
		else {
			yawDiff = yawRadianStart - yawRadianEnd;
		}

		dRotatingPercent = dRotatingPercent > 0.5 ? 0.5 : dRotatingPercent;

		double dRadianRoll = yawDiff / dRollRadio;

		if (fabs(dRadianRoll) > osg::DegreesToRadians(dRollMaxDegree))
		{
			dRadianRoll = fabs(dRadianRoll) * osg::DegreesToRadians(dRollMaxDegree) / dRadianRoll;
		}

		int iNumRotating = int(vecStateInfo.size() * dRotatingPercent);
		double dGapRotating = dRadianRoll / iNumRotating;

		for (int i = 0, j = vecStateInfo.size() - 1; i <= j; ++i, --j)
		{
			double dCurRoll;
			if (i < iNumRotating)
			{
				dCurRoll = i * dGapRotating;
			}
			else {
				dCurRoll = dRadianRoll;
			}
			vecStateInfo[i].vecPosture.y() = dCurRoll;
			vecStateInfo[j].vecPosture.y() = dCurRoll;
		}

		output.insert(output.end(), vecStateInfo.begin(), vecStateInfo.end());
	}

	void ProcessCornerSmooth(std::map<long double, osg::Vec3d>& mapOutput, 
		osg::Vec3d centerPosition, long double radius, long double maxDistance)
	{
		if (mapOutput.size() != 2)
		{
			return;
		}

		std::map<long double, osg::Vec3d>::const_iterator pIter = mapOutput.begin();
		osg::Vec3d start = pIter->second; 
		++pIter;
		osg::Vec3d end = pIter->second;

		// 建立待处理两点的坐标数组。
		typedef std::pair<osg::Vec3d, osg::Vec3d> Vec3dPair;
		std::queue<Vec3dPair> pairQueue;
		pairQueue.push(std::make_pair(start, end));
		
		while (!pairQueue.empty())
		{
			Vec3dPair points = pairQueue.front();
			if (Length3D(points.first-points.second) > maxDistance)
			{
				osg::Vec3d midPosition = points.first * 0.5 + points.second * 0.5;
				osg::Vec3d vectorMid = midPosition - centerPosition;
				long double radio = radius / Length3D(vectorMid);
				osg::Vec3d newPoint = centerPosition + vectorMid * radio;
				mapOutput[Length3D(newPoint-mapOutput.begin()->second)] = newPoint;
				pairQueue.push(std::make_pair(points.first, newPoint));
				pairQueue.push(std::make_pair(newPoint, points.second));
			} 
			pairQueue.pop();
		}
	}

	bool GetFirstInterpolation(CRenderContext *pRenderContext, const osg::Vec3d start, const osg::Vec3d end, osg::Vec3d& output, double deltaDistance)
	{
		//std::vector<osg::Vec3d> vecArray;
		//vecArray.push_back(start);
		//vecArray.push_back(end);
		//double dDistance = osgEarth::GeoMath::distance(vecArray);
		osg::Vec3d startXYZ, endXYZ;
		FeUtil::DegreeLLH2XYZ(pRenderContext, start, startXYZ);
		FeUtil::DegreeLLH2XYZ(pRenderContext, end, endXYZ);
		double deltaHei = end.z() - start.z();

		int nTotalNum = DistanceTwoXYZ(startXYZ, endXYZ) / deltaDistance;

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

			output.set(osg::Vec3d(osg::RadiansToDegrees(radLon), osg::RadiansToDegrees(radLat), start.z() + deltaHei*1.0*i / nTotalNum));
			return true;
		}
		return false;
	}
	
	void InterpolationSphere(CRenderContext *pRenderContext, const osg::Vec3d start, const osg::Vec3d end, std::vector<CMotionStateInfo>& output, double deltaDistance)
	{
		//std::vector<osg::Vec3d> vecArray;
		//vecArray.push_back(start);
		//vecArray.push_back(end);
		//double dDistance = osgEarth::GeoMath::distance(vecArray);
		osg::Vec3d startXYZ, endXYZ;
		FeUtil::DegreeLLH2XYZ(pRenderContext, start, startXYZ);
		FeUtil::DegreeLLH2XYZ(pRenderContext, end, endXYZ);
		double deltaHei = end.z() - start.z();

		int nTotalNum = DistanceTwoXYZ(startXYZ, endXYZ) / deltaDistance;

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

			output.push_back(CMotionStateInfo(osg::Vec3d(osg::RadiansToDegrees(radLon), osg::RadiansToDegrees(radLat), start.z() + deltaHei*1.0*i / nTotalNum)));
		}
	}

	long double Length3D(const osg::Vec3d &point)
	{
		return sqrtl(point.x() * point.x() + point.y() * point.y() + point.z() * point.z());
	}

	// 此种方法当经纬度很相近时出错。
	double CalculateYawRadian02(double lat1Rad, double lon1Rad, double lat2Rad, double lon2Rad)
	{
		if (fabs(lat1Rad - lat2Rad) <= osg::DegreesToRadians(0.0000001))
		{
			double detal = lon2Rad - lon1Rad;//计算经度差
			if (detal < -180.0 || (detal>0.0 && detal < 180.0))
			{
				// 在其右侧180度内
				return osg::DegreesToRadians(90.0);//逆时针转为正向，所以在右侧时转的角度为负值。
			}
			else
			{
				// 在其左侧180度内
				return osg::DegreesToRadians(-90.0);
			}
		}
		// 如果第一个点的经度和第二个点的经度相同则航向角为0度或者180度
		else if (fabs(lon1Rad - lon2Rad) <= osg::DegreesToRadians(0.0000000000001))
		{
			double detal = lat2Rad - lat1Rad;
			if (detal > 0.0)
			{
				return (0.0);
			}
			else
			{
				return osg::DegreesToRadians(-180.0);
			}
		}

		double dLat = (lat2Rad - lat1Rad);
		double dLon = (lon2Rad - lon1Rad);
		double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
			cos(lat1Rad) *  cos(lat2Rad) *
			sin(dLon / 2.0) * sin(dLon / 2.0);
		double dR = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

		double bearingRad01 = acosl((sin(lat2Rad) - sin(lat1Rad)*cos(dR)) / (cos(lat1Rad)*sin(dR)));
		double bearingRad02 = asinl((cos(dR) - sin(lat1Rad)*sin(lat2Rad)) * tan(lon2Rad - lon1Rad) / sin(dR) / cos(lat1Rad));
		double bearingRad = fabs(bearingRad01) > fabs(bearingRad02) ? bearingRad01 : bearingRad02;
		bearingRad *= (bearingRad01 * bearingRad02 / fabs(bearingRad01 *bearingRad02));
		return bearingRad;
	}

	void CalculateYawRadian(CRenderContext *pRenderContext, const osg::Vec3d &first, const osg::Vec3d &second, double &yawRadian)
	{
		if (fabs(first.y() - second.y()) <= DBL_EPSILON)
		{
			double detal = second.x() - first.x();//计算经度差
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
		else if (fabs(first.x() - second.x()) <= DBL_EPSILON)
		{
			double detal = second.y() - first.y();
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
			osg::Vec3d three(first.x(), second.y(), first.z()), threeXYZ;
			threeXYZ = FeAlg::DegreeLLH2XYZ(three);
			osg::Vec3d secondBottom(second.x(), second.y(), first.z()), secondBottomXYZ;
			secondBottomXYZ = FeAlg::DegreeLLH2XYZ(secondBottom);
			long double dDistance1 = DistanceTwoXYZ(threeXYZ, secondBottomXYZ);
			osg::Vec3d firstXYZ;
			firstXYZ = FeAlg::DegreeLLH2XYZ(first);
			long double dDistance2 = DistanceTwoXYZ(firstXYZ, threeXYZ);

			yawRadian = atanl(dDistance1 / dDistance2);
			////如果第二个点在第一个点的右边
			double detalX = second.x() - first.x();
			if (detalX < -180 || (detalX>0 && detalX < 180))
			{
				if (second.y() > first.y())
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
				if (second.y() < first.y())
				{
					yawRadian = osg::PI - yawRadian;
				}
			}
		}
	}

	bool FindPositionFromFlightPath(std::vector<CMotionStateInfo>& vecStateInfo, double dTime, double dSpeed, CMotionStateInfo& output, int& lastPosition)
	{
		// 判断情况
		if (vecStateInfo.size() < 2 || lastPosition >= vecStateInfo.size() - 1 || vecStateInfo[lastPosition].dTime > dTime)
		{
			if (lastPosition == 0)
			{
				output = vecStateInfo[lastPosition];	
				return true;
			}
			return false;
		}

		if (dSpeed <= 0.0)
		{
			return true;
		}

		if (lastPosition == 0)
		{
			if (vecStateInfo[0].dTime < 0.0)
			{
				vecStateInfo[0].vecXYZ = DegreeLLH2XYZ(vecStateInfo[0].vecLLH);
				vecStateInfo[0].dTime = 0.0;
			}
			++lastPosition;
		}

		//OSG_NOTICE << "当前时间： " << dTime << std::endl;
		for (int i = lastPosition; i < vecStateInfo.size(); ++i)
		{
			if (vecStateInfo[i].dTime < 0.0)
			{
				vecStateInfo[i].vecXYZ = DegreeLLH2XYZ(vecStateInfo[i].vecLLH);

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

	double ComputeDistance(const osg::Vec3d startPoint, const osg::Vec3d endPoint)
	{
		double distance = sqrt( (endPoint.x() - startPoint.x())*(endPoint.x() - startPoint.x())
							+ (endPoint.y() - startPoint.y())*(endPoint.y() - startPoint.y()) );

		return distance;
	}

}
