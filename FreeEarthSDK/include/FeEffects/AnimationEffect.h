#ifndef _ANIMATION_EFFECT_H
#define _ANIMATION_EFFECT_H

/**************************************************************************************************
* @file AnimationEffect.h
* @note 动画控制效果
* @author z00013
* @data 2015-12-24
**************************************************************************************************/

#include <osg/Group>
#include <osg/AnimationPath>
#include <osg/NodeVisitor>

#include <FeEffects/Export.h>
#include <FeEffects/FreeEffect.h>

namespace FeEffect
{
	class FEEFFECTS_EXPORT CAnimationEffect : public CFreeEffect
	{
	public:
		typedef std::vector<osg::AnimationPathCallback*>	MODELANIMATION;

		CAnimationEffect(osg::Node *pModelNode);
		virtual ~CAnimationEffect();

		virtual bool CreateEffect();
		virtual bool StartEffect();

		virtual bool UpdataEffect();

		virtual bool ClearEffect();

		bool StopEffect();

		bool OppositeEffect(); 
	protected:
		MODELANIMATION		m_vModelAnimation;
		osg::Node		   *m_pModelNode;

		std::vector<osg::AnimationPath::TimeControlPointMap>  m_vecTimeControlPointMap;
		std::vector<osg::AnimationPath::TimeControlPointMap>  m_vecReverseTimeControlPointMap;
	};
}

namespace FeEffect
{
	class FEEFFECTS_EXPORT CAnimationVisitor :public osg::NodeVisitor
	{
	public:
		CAnimationVisitor();
		~CAnimationVisitor();
	public:
		virtual void apply(osg::MatrixTransform &pMt);
		CAnimationEffect::MODELANIMATION GetModelAnimation();

		std::vector<osg::AnimationPath::TimeControlPointMap> &GetVectorOfTimeControlPointMap();
		std::vector<osg::AnimationPath::TimeControlPointMap> &GetReverseVectorOfTimeControlPointMap();
	protected:

		CAnimationEffect::MODELANIMATION			m_modeAnimation;

		std::vector<osg::AnimationPath::TimeControlPointMap>  m_vecTimeControlPointMap;
		std::vector<osg::AnimationPath::TimeControlPointMap>  m_vecReverseTimeControlPointMap;
	};
}



namespace FeEffect
{
	
	/**
	  * @class CFeAnimationNode
	  * @brief 自带动画的模型节点，提供动画控制接口
	  * @author g00034
	*/
	class FEEFFECTS_EXPORT CFeAnimationNode : public osg::Group
	{
		/**
		  * @class CTimerCallback
		  * @brief 计时器回调，持续播放动画直到时间结束
		  * @author g00034
		*/
		class CTimerCallback : public osg::NodeCallback
		{
		public:
			CTimerCallback(CFeAnimationNode* pAnimNode);
			virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

			// 播放指定时长的动画
			void PlayTime(double dTime);
		private:
			osg::observer_ptr<CFeAnimationNode> m_opAnimNode;
			double m_dStartTime;
			double m_dTimeLong;
			bool   m_bStartPlay;
		};

	public:
		/**  
		  * @brief 开始播放动画
		*/
		CFeAnimationNode(const std::string& strModelPath, double dPeriodSec);

		CFeAnimationNode(osg::Node* pModelNode, double dPeriodSec);

		/**  
		  * @brief 开始播放动画
		*/
		void StartAnimation();

		/**  
		  * @brief 停止播放动画
		*/
		void StopAnimation();

		/**  
		  * @brief 重置动画
		*/
		void Reset();

		/**  
		  * @brief 使动画停留开始处
		*/
		void StepToBegin();

		/**  
		  * @brief 使动画停留结束处
		*/
		void StepToEnd();

		/**  
		  * @brief 使动画停留在指定时间
		*/
		void StepToTime( double dTime );

		/**  
		  * @brief 获取动画周期
		*/
		double GetPeriodSec();

		/**  
		  * @brief 播放指定时长的动画，时间到后自动停止
		*/
		void PlayForTime(double dTime);

	protected:
		/**  
		  * @brief 初始化模型
		*/
		void InitModel(osg::Node* pModelNode);

		/**  
		  * @brief 是否停止播放动画
		*/
		void SetAnimPause( bool isPause = true );
	
		/**  
		  * @brief 查找节点中的所有动画回调节点
		*/
		void FindAnimPath( osg::Node * rootNode);

	protected:
		/// 节点动画映射表
		std::map<osg::Node*, osg::AnimationPath*> m_vecAnimationPaths;

		// 一个动画周期（单位:秒）
		double m_dPeriodSec;

		// 播放回调
		osg::ref_ptr<CTimerCallback> m_rpTimerCB;
	};
}


#endif //_ANIMATION_EFFECT_H