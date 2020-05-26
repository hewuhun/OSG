/**************************************************************************************************
* @file PlotEventHandler.h
* @note 军标事件处理器
* @author w00040
* @data 2017-11-03
**************************************************************************************************/
#ifndef PLOT_EVENT_HANDLER_H
#define PLOT_EVENT_HANDLER_H 1

#include <osgGA/GUIEventHandler>

#include <osgEarth/GeoData>

#include <FeExtNode/ExternNode.h>

namespace FreeViewer
{
	class CPlotPluginInterface;

	/**
	  * @class CPlotEventHandler
	  * @brief 军标事件处理类
	  * @note 用于在客户端处理军标的相关事件
	  * @author w00040
	*/
	class CPlotEventHandler : public osgGA::GUIEventHandler
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CPlotEventHandler(CPlotPluginInterface* pPlotWidget);

		enum E_MouseFlagType
		{
			E_MouseLeft = 0,	// 鼠标左键
			E_MouseRight,		// 鼠标右键
			E_MouseMove			// 鼠标移动
		};

	public:
		/**  
		  * @brief 设置当前处理的军标节点
		*/
		void SetActivePlotNode(FeExtNode::CExternNode* pMarkNode);
		
		/**  
		  * @brief 开启军标事件捕获
		*/
		void StartCapture();

		/**  
		  * @brief 停止军标事件捕获
		*/
		void StopCapture();

	protected:
		/**  
		  * @brief 事件处理函数
		*/
		bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

	protected:
		/**  
		  * @brief 计算当前军标信息
		*/
		void CalcPlotInfo(float x, float y);

	protected:
		/// 当前活动的标记节点
		osg::observer_ptr<FeExtNode::CExternNode>	m_opCurPlotNode;

		/// 标记管理对象
		CPlotPluginInterface*						m_pPlotWidget;

		/// 是否开启事件捕获
		bool										m_bStartCapture;

		/// 鼠标按下的X、Y位置
		float										m_fMouseDownX;  
		float										m_fMouseDownY;

		/// 当前鼠标点击的地球坐标
		osgEarth::GeoPoint							m_geoPoint;

		/// 当前军标尺寸，用于辅助计算
		double										m_dPlotSize;

		// 鼠标状态标识
		int											m_nMouseFlag;
	};
}
#endif
