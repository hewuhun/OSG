/**************************************************************************************************
* @file CEarthManipulatorCtrl.h
* @note 与osgEarth的地形操作器相关的控制器，导航器通过该控制器控制地球
* @author l00008
* @data 2014-02-20
**************************************************************************************************/
#ifndef FE_NAVIGATOR_EARTH_NAVIGATOR_CTRL_H
#define FE_NAVIGATOR_EARTH_NAVIGATOR_CTRL_H

#include <osgViewer/View>
#include <osgGA/GUIEventHandler>

#include <FeKits/manipulator/FreeEarthManipulator.h>
#include <FeUtils/RenderContext.h>

#include <FeKits/Export.h>
#include <FeKits/navigator/NavigatorCtrl.h>

namespace FeKit
{
	class CComponentBase;

	/**
	* @class CEarthManipulatorCtrl
	* @brief 导航器控制器
	* @note 与osgEarth的地形操作器相关的控制器，这个控制器和特定的操作器相关。
	* @author l00008
	*/
	class FEKIT_EXPORT CEarthManipulatorCtrl : public CNavigatorCtrl
	{
	public:
		/**  
		* @brief 导航器控制器构造函数
		* @param pManipulator [in] 当前使用的地球操作器
		*/
		CEarthManipulatorCtrl(FeKit::FreeEarthManipulator* pManipulator);

		~CEarthManipulatorCtrl(void);

	public:
		/**  
		* @brief 事件处理器
		*/
		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

		/**  
		* @brief 更新偏航角度
		*/
		virtual void UpdateHeading(double dValue);

		/**  
		* @brief 停止更新偏航角
		*/
		virtual void StopUpdateHeading();

		/**  
		* @brief 更新拖动位置
		*/
		virtual void UpdatePan(double dX, double dY);

		/**  
		* @brief 停止更新拖动位置
		*/
		virtual void StopUpdatePan();

		/**  
		* @brief 更新旋转角度，横滚和俯仰角
		*/
		virtual void UpdateRotate(double dAzimuth, double dPitch);

		/**  
		* @brief 停止更新旋转角度，横滚和俯仰角
		*/
		virtual void StopUpdateRotate();

		/**  
		* @brief 更新缩放比例
		*/
		virtual void UpdateZoom(double dRate);

		/**  
		* @brief 停止更新缩放比例
		*/
		virtual void StopUpdateZoom();

		/**  
		* @brief 添加激活的组件
		*/
		virtual void AddActiveComponent(CBasePart* pPart);

	protected:
		/**  
		* @brief 更新总入口
		*/
		virtual void Update();

		/**  
		* @brief 将导航器的值转为地球操作器的值
		*/
		double Navigator2Manipulator(double dValue);

		/**  
		* @brief 将地球操作器的值转为导航器的值
		*/
		double Manipulator2Navigator(double dValue);

	protected:
		///是否缩放
		bool                                                    m_bIsZoom;
		///缩放比例
		double                                                  m_dZoomRate;

		///是否旋转
		bool                                                    m_bIsRotate;
		///横滚角
		double                                                  m_dAzimuth;
		///俯仰角
		double                                                  m_dPitch;

		///是否可移动
		bool                                                    m_bIsMove;
		///X方向移动距离
		double                                                  m_dX;
		///Y方向移动距离
		double                                                  m_dY;

		///地球操作器
		osg::observer_ptr<FeKit::FreeEarthManipulator>			m_opFreeEarthManipulator;
		///激活的组件
		osg::observer_ptr<CBasePart>                            m_opActiveCompass;
	};
}

#endif //FE_NAVIGATOR_EARTH_NAVIGATOR_CTRL_H

