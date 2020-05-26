/**************************************************************************************************
* @file MiniAnimationCtrl.h
* @note 动画控制器
* @author z00013
* @data 2014-4-5
**************************************************************************************************/

#ifndef FE_MINI_ANIMATION_PATH_CONTROL_H
#define FE_MINI_ANIMATION_PATH_CONTROL_H

#include <osg/MatrixTransform>
#include <FeUtils/MiniAnimation.h>

#include <FeUtils/RenderContext.h>
#include <FeUtils/MiniAnimationBuilder.h>

const double dComputeSpeedOffset[7] ={0.10,0.25,0.50,1.00,2.00,5.00,10.00};

namespace FeUtil
{
	/**
	  * @class CAnimTimerCallback
	  * @brief 支持定时事件的动画路径回调
	  * @note 可以添加定时任务，在指定时间点执行自定义操作
	  * @author g00034
	*/
	class FEUTIL_EXPORT CAnimTimerCallback : public FeUtil::AnimationPathCallback
	{
	public:
		/**
		  * @brief 事件回调函数对象，用户可从该类继承定制不同的操作
		*/
		typedef struct TimerCallFunc : public osg::Referenced
		{
			virtual void operator ()() {}
		}TimerCallFunc;

		/**
		  * @brief 事件回调类
		*/
		typedef struct TimeCaller
		{
			bool m_bCalled; // 标记是否被调用
			double m_time;  // 事件被调用的时间点
			osg::ref_ptr<TimerCallFunc> m_func;  // 事件回调函数

			explicit TimeCaller(TimerCallFunc* func, double time)
				: m_func(func)
				, m_time(time)
			{
				m_bCalled = false;
			}
		}TimeCaller;

		/**
		  * @brief 动画更新
		*/
		virtual void update(osg::Node& node);

		/**
		  * @brief 添加事件回调
		*/
		void AddTimeCaller(double time, TimerCallFunc* func);

		/**
		  * @brief 动画重置
		*/
		virtual void reset();

	private:
		std::vector< TimeCaller > m_timeCallbacks;
		OpenThreads::Mutex        m_timerMutex;
	};

	/**
    * @class CMiniAnimationCtrl
    * @brief 负责管理控制模型运行
    * @note 模型运行控制器
    * @author z00013
    */
	class FEUTIL_EXPORT CMiniAnimationCtrl : public osg::Referenced
	{
	public:
		CMiniAnimationCtrl(CRenderContext *pRenderContext,osg::Group* pAnimationMT,FeUtil::AnimationPathCallback *pPathCallback=new FeUtil::AnimationPathCallback);

		virtual ~CMiniAnimationCtrl();

	public:
		//开始
		bool Start();

		//停止
		bool Stop();

		bool Pause();

		bool ContinueMove();

		bool ReStart();

		void Reset();

		void Accelerate();

		void Decelerate();

		bool GetPause();

		void UpdataTimeMultiplier();

	public:
		//设置速度
		void SetSpeed(double dValue);

		double GetSpeed();
		
		//设置加速/减速 倍率
		void SetSpeedOffset(double dValue);

		double GetSpeedOffset();

		void SetHeightOffset(double dValue);

		double GetHeightOffset();

		osg::Group* GetAnimationMT();

		osg::Node* GetTrackMT();

		void SetTrackMT(osg::Node* pTrackNode);

		void SetAnimationMT(osg::Group* pAnimationMT);

		FeUtil::AnimationPathCallback* GetAnimationCallBack();

		//判断是否完成一周动画
		bool IsFinish();

		//设置循环状态
		void SetAnimationLoopMode(AnimationPath::LoopMode mode);

		unsigned int GetAnimationLoopMode()const;

		//设置是否每帧计算高度
		void SetFrameCalculate(bool bCalculate);

		void CreateAnimationPath(std::vector<osg::Vec3d> vectorData,double dSpeed);
		void CreateAnimationPath( std::vector<FeUtil::SDriveData> vectorDriveData );
	protected:
		double    m_dSpeed;
		double    m_dHeiOffset;
		double    m_dSpeedOffset;
		double    m_dScale;

		AnimationPath::LoopMode  m_unLoop;
		osg::ref_ptr<osg::Node>							m_pTrackNode;
		osg::observer_ptr<osg::Group>					m_opAnimationMT;
		osg::ref_ptr<FeUtil::AnimationPathCallback>		m_pPathCallback;

		osg::observer_ptr<CRenderContext>				m_opRenderContext;
		osg::ref_ptr<FeUtil::CMiniAnimationPathBuilder>	m_rpApBuilder;
		osg::ref_ptr<FeUtil::AnimationPath>			    m_rpAnimationPath;
		int												m_nIndex;
		double											m_dTimeMultiplier;

		OpenThreads::Mutex								m_mutex;

        std::map<osg::MatrixTransform *,osg::observer_ptr<FeUtil::AnimationPathCallback> >  m_vecMTCallback;

		
	};
}





#endif //FE_PATH_LINE_GEOMETRY_ELEMENT_H
