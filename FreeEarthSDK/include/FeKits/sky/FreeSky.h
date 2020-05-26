/**************************************************************************************************
* @file FreeSky.h
* @note 星空效果
* @author l00008
* @data 2014-04-29
**************************************************************************************************/
#ifndef FE_FREE_SKY_H
#define FE_FREE_SKY_H

#include <OpenThreads/Mutex>

#include <osg/NodeCallback>
#include <osgViewer/View>
#include <osgEarth/MapNode>
#include <osgEarthUtil/Sky>
#include <FeKits/sky/HgNebula.h>

#include <FeKits/Export.h>
#include <FeKits/KitsCtrl.h>

namespace FeKit
{
	class CAnimateSkyCallback;
	class CSychLocalTimeCallback;

	/**
	* @class CFreeSky
	* @brief 星空类，封装了系统提供的星空背景
	* @note 提供统一的星空背景来显示远距离恒星、太阳、月亮等
	* @author l00008
	*/
	class FEKIT_EXPORT CFreeSky : public osg::Group, public CKitsCtrl
	{
	public:
		CFreeSky(FeUtil::CRenderContext* pContext);

	public:
		/**
		* 使用MapNode创建一个星空节点
		*/
		bool Initialize();

		/**
		* 使用MapNode和选项创建一个星空节点
		*/
		bool Initialize( const osgEarth::Util::SkyOptions&  options);

		/**
		*@note: 关联View
		*/
		void Attach(osgViewer::View* pView, int nLightNum = 0);

		/**
		*@note: 显隐控制,是对总体天体系统的控制，只有当其显示时其他的比如太阳、月亮、
		*		星系、星云才会显示，否则只是作为可显示状态，但未必显示。同理当其隐藏时
		*		其他的无论出于显示或者隐藏状态都无法显示。
		*/
		virtual void Show();

		/**
		*@note: 显隐控制,是对总体天体系统的控制，只有当其显示时其他的比如太阳、月亮、
		*		星系、星云才会显示，否则只是作为可显示状态，但未必显示。同理当其隐藏时
		*		其他的无论出于显示或者隐藏状态都无法显示。
		*/
		virtual void Hide();

		///控制类型的接口，主要控制状态的显隐
	public:
		/**
		*@note: 设置太阳的显隐
		*/
		void SetSunVisible(bool bValue);

		/**
		*@note: 获取太阳的显隐
		*/
		bool GetSunVisible();

		/**
		*@note: 设置月球的显隐
		*/
		void SetMoonVisible(bool bValue);

		/**
		*@note: 获取月球的显隐
		*/
		bool GetMoonVisible();

		/**
		*@note: 设置星系的显隐
		*/
		void SetStarsVisible(bool bValue);

		/**
		*@note: 获取星系的显隐
		*/
		bool GetStarsVisible();

		/**
		*@note: 设置星云的显隐
		*/
		void SetNebulaVisible(bool bValue);

		/**
		*@note: 获取星云的显隐
		*/
		bool GetNebulaVisible();

		/**
		*@note: 开启或者关闭大气的显隐
		*/
		void SetAtmosphereVisible(bool bState);

		/**
		*@note: 获取大气的显隐
		*/
		bool GetAtmosphereVisible() const; 

		/**
		*@note: 开启或者关闭时间的流逝状态
		*/
		void SetTimeFlyVisible(bool bState);

		/**
		*@note: 获取是否开启或者关闭时间的流逝
		*/
		bool GetTimeFlyVisible() const; 

		/**
		*@note: 开启或者关闭同步本地时间
		*/
		void SetSyncLocalTime(bool bState);

		/**
		*@note: 获取是否开启或者关闭同步本地时间
		*/
		bool GetSyncLocalTime() const; 

	public:
		/**
		*@note: 设置当前时间流逝速度
		*/
		void SetAnimationRate(double dRate);

		/**
		*@note: 获取当前时间流逝速度
		*/
		double GetAnimationRate() const;

		/**
		*@note: 设置当前的UTC时间
		*/
		void SetDateTime(const osgEarth::DateTime& time);

		/**
		*@note: 获得当前的UTC时间
		*/
		osgEarth::DateTime GetDateTime() const;

		/**
		*@note: 设置环境光照，这个设置只有在光照开启状态下有效
		*/
		void SetAmbient(const osg::Vec4& vecAmbient);

		/**
		*@note: 获得环境光照
		*/
		osg::Vec4 GetAmbient() const;

		///获得一些特殊的节点，这些一般用户不会使用
	public:
		///获得地球大气层节点
		osg::Node* GetAtmosphereNode();

		///获得太阳实体节点
		osg::Node* GetSunNode();

		///获得月亮实体节点
		osg::Node* GetMoonNode();

		///获得远距离恒星实体节点
		osg::Node* GetStarsNode();

		///获得星云节点
		osg::Node* GetNebulaNode();

		///获得星SkyNode节点
		osg::Light* GetLight();

		/**  
		  * @note 大气节点雾化参数
		*/
		osg::Uniform* GetUniformFogEnabled();

		osg::Uniform* GetUniformFogColor();

	protected:
		virtual ~CFreeSky(void);

	protected:
		///太阳天空节点
		osg::observer_ptr<osgEarth::Util::SkyNode>		m_opSkyNode;   

		///星云背景节点
		osg::observer_ptr<CHgNebula>					m_opNebulaNode;

		///时间流逝是否开启
		bool                                            m_bAnimationSwitch;

		///时间流逝的回调
		osg::ref_ptr<CAnimateSkyCallback>               m_rpAnimation;

		///同步本地时间是否开启
		bool											m_bSychLocalTime;

		///同步本地时间回调
		osg::ref_ptr<CSychLocalTimeCallback>			m_rpSychCallback;
	};

	/**
	* @class CAnimateSkyCallback
	* @brief 更新时间的动画回调
	* @note 通过此动画回调，可以更新时间来显示一天中的不同时刻地球状态
	* @author l00008
	*/
	class CAnimateSkyCallback : public osg::NodeCallback
	{    
	public:
		CAnimateSkyCallback( double rate = 1440 );

	public:
		/**
		*@note: 设置更新的速率
		*/
		void SetUpdateRate(double dRate);

		/**
		*@note: 获取更新的速率
		*/
		double GetUpdateRate();

		/**
		*@note: 用于重置参数
		*/	
		void Reset();

		/**
		*@note: 每一帧更新的操作，逐步的增加时间
		*/
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

	protected:
		virtual ~CAnimateSkyCallback();

	private:
		///累计时间
		double m_dAccumTime;

		///上一次时间
		double m_dPrevTime;    

		///速率
		double m_dRate;

		///互斥体
		OpenThreads::Mutex   m_rateMutex;
	};

	/**
	* @class CAnimateSkyCallback
	* @brief 更新时间的动画回调
	* @note 通过此动画回调，可以更新时间来显示一天中的不同时刻地球状态
	* @author l00008
	*/
	class CSychLocalTimeCallback : public osg::NodeCallback
	{    
	public:
		CSychLocalTimeCallback();

	public:
		/**
		*@note: 每一帧更新的操作，逐步的增加时间
		*/
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

	protected:
		virtual ~CSychLocalTimeCallback();
	};
}

#endif //FE_FREE_SKY_H
