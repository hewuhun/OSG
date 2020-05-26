#ifndef FE_MINI_ANIMATION_PATH_BUILDER_H
#define FE_MINI_ANIMATION_PATH_BUILDER_H

#include <osg/MatrixTransform>
#include <FeUtils/MiniAnimation.h>

#include <FeUtils/RenderContext.h>

namespace FeUtil
{
// 	struct SDriveData
// 	{
// 		osg::Vec3d vecLLH;    //经纬高位置
// 		osg::Vec3d vecPosture;//角度
// 		double dTime;         
// 		double dSpeed;        //速度，目前没有用
// 	};

	struct FEUTIL_EXPORT CMotionStateInfo
	{
	public:
		explicit CMotionStateInfo(
			osg::Vec3d llh = osg::Vec3d(0.0, 0.0, 0.0), 
			osg::Vec3d posture = osg::Vec3d(0.0, 0.0, 0.0),
			double tim = -1.0, 
			double spe = 0.0, 
			osg::Vec3d xyz = osg::Vec3d(0.0, 0.0, 0.0),
			osg::Vec3d scale = osg::Vec3d(1,1,1));

	public:
		void interpolate(double radio, const CMotionStateInfo& start, const CMotionStateInfo& end);

	public:
		osg::Vec3d vecLLH;		//经纬高位置
		osg::Vec3d vecPosture;	//角度
		double dTime;			//时间
		double dSpeed;			//速度
		osg::Vec3d vecXYZ;		//位置
		osg::Vec3d vecScale;    // 缩放比例
	};

	typedef CMotionStateInfo SDriveData;


	class FEUTIL_EXPORT CMiniAnimationPathBuilder : public osg::Referenced
	{
	public:
		CMiniAnimationPathBuilder(CRenderContext *pRenderContext);

		virtual ~CMiniAnimationPathBuilder();

	public:
		FeUtil::AnimationPath * CreateAnimationPath(const std::vector<SDriveData> &vecDriveData);
		
		/**
		* @brief 根据输入的点集创建动画路径
		* @param vecLLHSet 输入的经纬度点集
		* @param dSpeed 目标物体运动的速度
		* @return 创建的动画路径
		*/
		FeUtil::AnimationPath *CreateAnimationPath(const std::vector<osg::Vec3d> &vecLLHSet, double dSpeed = 200.0);

	protected:
		osg::observer_ptr<CRenderContext>      m_opRenderContext;

		osg::ref_ptr<FeUtil::AnimationPath>    m_rpAnimationPath;
	};
}

namespace FeUtil
{
	struct RotateData
	{
		double dTime;
		double dAngle;//角度
	};
	class FEUTIL_EXPORT CRotateAnimationPath : public osg::Referenced
	{
	public:
		CRotateAnimationPath(CRenderContext *pRenderContext);

		virtual ~CRotateAnimationPath();

	public:
		FeUtil::AnimationPath * CreateAnimationPath(osg::Vec3d vecCenter,std::vector<RotateData> vecRotateData);

		FeUtil::AnimationPath * CreateAnimationPath(osg::Vec3d vecCenter,double dPeriod);

	protected:
		osg::observer_ptr<CRenderContext>      m_opRenderContext;

		osg::ref_ptr<FeUtil::AnimationPath>    m_rpAnimationPath;

	};
}

#endif //FE_MINI_ANIMATION_PATH_BUILDER_H
