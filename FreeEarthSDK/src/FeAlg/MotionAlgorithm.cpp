#include <FeAlg/MotionAlgorithm.h>

#include <fstream>
#include <iomanip>

namespace FeAlg
{
	osg::Node *RecursionNode(osg::Node *node, const std::string &strName)
	{
		if (strName.empty())
		{
			return NULL;
		}

		osg::Group *gp = node->asGroup();

		if (gp == NULL)
		{
			return NULL;
		}

		int numChild = gp->getNumChildren();
		for (int i = 0; i < numChild; ++i)
		{
			if (gp->getChild(i)->getName() == strName)
			{
				osg::Node *result = gp->getChild(i);
				return result;
			}
		}
		for (int i = 0; i < numChild; ++i)
		{
			if (RecursionNode(gp->getChild(i), strName))
			{
				return RecursionNode(gp->getChild(i), strName);
			}
		}
		return NULL;
	}

	void CalculateKeyPointsFromPath(const std::vector<CMotionStateInfo>& vecSateInfo, std::vector<osg::Vec3d>& vecKeyPoints, double dDiffYawDegree /*= 1.0*/, double dDiffPitchDegree /*= 0.5*/)
	{
		if (vecSateInfo.size() < 2)
		{
			return;
		}

		std::vector<CMotionStateInfo> keyPoints;

		keyPoints.push_back(vecSateInfo[0]);
		for (int i = 1; i < vecSateInfo.size() - 1; ++i)
		{
			if (fabs(osg::RadiansToDegrees(keyPoints[keyPoints.size() - 1].vecPosture.z()) - osg::RadiansToDegrees(vecSateInfo[i].vecPosture.z())) > dDiffYawDegree)
			{
				keyPoints.push_back(vecSateInfo[i]);
				continue;
			}
			if (fabs(osg::RadiansToDegrees(keyPoints[keyPoints.size() - 1].vecPosture.x()) - osg::RadiansToDegrees(vecSateInfo[i].vecPosture.x())) > dDiffPitchDegree)
			{
				keyPoints.push_back(vecSateInfo[i]);
			}
		}
		keyPoints.push_back(vecSateInfo[vecSateInfo.size() - 1]);

		for (int i = 0; i < keyPoints.size(); ++i)
		{
			vecKeyPoints.push_back(keyPoints[i].vecLLH);
		}
	}

	bool WritePathToFile(const std::string& strFile, const std::vector<CMotionStateInfo>& outStateInfo, const std::vector<osg::Vec3d> &outEditPoints)
	{
        std::fstream outFile(strFile.c_str(), std::ios_base::out | std::ios_base::trunc);
		if (outFile.is_open())
		{
			outFile << outStateInfo.size() << std::endl;
			for (int i = 0; i < outStateInfo.size(); ++i)
			{
				outFile << std::setprecision(DBL_DIG) << outStateInfo[i].vecLLH.x() << "  ";
				outFile << std::setprecision(DBL_DIG) << outStateInfo[i].vecLLH.y() << "  ";
				outFile << std::setprecision(DBL_DIG) << outStateInfo[i].vecLLH.z() << "  ";
				outFile << std::setprecision(DBL_DIG) << outStateInfo[i].vecPosture.x() << "  ";
				outFile << std::setprecision(DBL_DIG) << outStateInfo[i].vecPosture.y() << "  ";
				outFile << std::setprecision(DBL_DIG) << outStateInfo[i].vecPosture.z() << std::endl;
			}

			outFile << outEditPoints.size() << std::endl;
			for (int i = 0; i < outEditPoints.size(); ++i)
			{
				outFile << std::setprecision(DBL_DIG) << outEditPoints[i].x() << "  ";
				outFile << std::setprecision(DBL_DIG) << outEditPoints[i].y() << "  ";
				outFile << std::setprecision(DBL_DIG) << outEditPoints[i].z() << std::endl;
			}
			outFile.close();
			return true;
		}
		return false;
	}

	bool ReadFileToPath(const std::string& strFile, std::vector<CMotionStateInfo>& inputStateInfo, std::vector<osg::Vec3d> &inputEditPoints)
	{
		inputEditPoints.clear();
		inputStateInfo.clear();
        std::fstream inputFile(strFile.c_str(), std::ios::in);
		if (inputFile.is_open())
		{
			int num;
			inputFile >> num;
			for (int i = 0; i < num; ++i)
			{
				CMotionStateInfo stateInfo;
				inputFile >> stateInfo.vecLLH.x();
				inputFile >> stateInfo.vecLLH.y();
				inputFile >> stateInfo.vecLLH.z();
				inputFile >> stateInfo.vecPosture.x();
				inputFile >> stateInfo.vecPosture.y();
				inputFile >> stateInfo.vecPosture.z();
				inputStateInfo.push_back(stateInfo);
			}			
			inputFile >> num;
			for (int i = 0; i < num; ++i)
			{
				osg::Vec3d stateInfo;
				inputFile >> stateInfo.x();
				inputFile >> stateInfo.y();
				inputFile >> stateInfo.z();
				inputEditPoints.push_back(stateInfo);
			}
			inputFile.close();
			return true;
		}
		return false;
	}

	bool PointRotation(const osg::Vec3d& in, osg::Vec3d& out, double rotateRadian)
	{
		double multi = 1000.0;
		osg::Vec3d inMulti = in * multi;
		double radius = sqrtl(inMulti.x() * inMulti.x() + inMulti.y() * inMulti.y());
		double dAngle = acosl(inMulti.x() / radius);
		if (dAngle > osg::PI_2)
		{
			/// 说明是二三象限
			if (inMulti.y() < 0)
			{
				dAngle = osg::PI - dAngle + osg::PI;
			}
		}
		else{
			/// 说明是一四象限
			if (inMulti.y() < 0)
			{
				dAngle = osg::PI * 2.0 - dAngle;
			}
		}
		dAngle -= rotateRadian;
		while (dAngle > osg::PI * 2.0)
		{
			dAngle -= (osg::PI*2.0);
		}
		osg::Vec3d pos(radius*cos(dAngle), radius*sin(dAngle), 0.0);
		out = pos / multi;
		return true;
	}

	bool PointRotation(const std::vector<osg::Vec3d> & in, std::vector<osg::Vec3d>& out, double rotateRadian)
	{
		out.clear();
		if (in.empty())
		{
			return false;
		}

		/// 先放大一千倍，然后旋转, 提高精度
		for (int i = 0; i < in.size(); ++i)
		{
			osg::Vec3d outPut;
			PointRotation(in[i], outPut, rotateRadian);
			out.push_back(outPut);
		}
		return true;
	}
}

