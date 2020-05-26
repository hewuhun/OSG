/**************************************************************************************************
* @file MotionStateInfo.h
* @note 模型运动数据类
* @author w00024
* @data 2016-4-9
**************************************************************************************************/
#ifndef FE_MOTION_STATE_INFO_H
#define FE_MOTION_STATE_INFO_H

#include <FeAlg/Export.h>

#include <osg/Vec3d>
#include <osg/Matrix>
#include <string>

namespace FeAlg
{
	/**
	* @class CMotionStateInfo
	* @brief 运动模拟基本数据类
	* @note
	* @author w00024
	*/
	class FEALG_EXPORT CMotionStateInfo
	{
	public:
		explicit CMotionStateInfo(osg::Vec3d llh = osg::Vec3d(0.0, 0.0, 0.0), osg::Vec3d posture = osg::Vec3d(0.0, 0.0, 0.0),
			double tim = -1.0, double spe = 0.0, osg::Vec3d xyz = osg::Vec3d(0.0, 0.0, 0.0))
			: vecLLH(llh)
			, vecPosture(posture)
			, dTime(tim)
			, dSpeed(spe) 
			, vecXYZ(xyz)
		{
		}

		osg::Vec3d vecLLH;    //经纬高位置
		osg::Vec3d vecPosture;//角度
		double dTime;         
		double dSpeed;        //速度
		osg::Vec3d vecXYZ;

		/**
		* @note 计算两点之间, 比例为radio处的点
		*/
        void interpolate(double radio, const CMotionStateInfo& start, const CMotionStateInfo& end);

        void getMatrix(osg::Matrixf& matrix) const;

        void getMatrix(osg::Matrixd& matrix) const;

		inline void getInverse(osg::Matrixf& matrix) const
		{
			/*matrix.makeRotate(_rotation.inverse());
			matrix.postMultScale(osg::Vec3d(1.0 / _scale.x(), 1.0 / _scale.y(), 1.0 / _scale.z()));
			matrix.preMultTranslate(-_position);*/
		}

		inline void getInverse(osg::Matrixd& matrix) const
		{
			/*matrix.makeRotate(_rotation.inverse());
			matrix.postMultScale(osg::Vec3d(1.0 / _scale.x(), 1.0 / _scale.y(), 1.0 / _scale.z()));
			matrix.preMultTranslate(-_position);*/
		}

	};
}

#endif //FE_MOTION_STATE_INFO_H
