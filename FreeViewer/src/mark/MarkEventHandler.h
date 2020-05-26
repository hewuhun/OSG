/**************************************************************************************************
* @file MarkEventHandler.h
* @note 客户端场景标记事假处理器
* @author g00034
* @data 2017-02-09
**************************************************************************************************/
#ifndef MARK_EVENT_HANDLER_H
#define MARK_EVENT_HANDLER_H 1

#include <osgGA/GUIEventHandler>

#include <osgEarth/GeoData>

#include <FeExtNode/ExternNode.h>

namespace FreeViewer
{
	class CMarkPluginInterface;

	/**
	  * @class CMarkEventHandler
	  * @brief 标记事件处理类
	  * @note 用于在客户端处理标记的相关事件
	  * @author g00034
	*/
	class CMarkEventHandler : public osgGA::GUIEventHandler
	{
	public:
		/**  
		  * @note 构造函数  
		  * @param pMarkWidget [in] 标记管理类
		*/
		CMarkEventHandler(CMarkPluginInterface* pMarkWidget);

	public:
		/**  
		  * @brief 设置当前处理的标记节点
		*/
		void SetActiveMarkNode(FeExtNode::CExternNode* pMarkNode);
		
		/**  
		  * @brief 开启标记事件捕获
		*/
		void StartCapture();

		/**  
		  * @brief 停止标记事件捕获
		*/
		void StopCapture();

	protected:
		/**  
		  * @brief 事件处理函数
		*/
		bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

	protected:
		/**  
		  * @brief 计算当前标记信息
		*/
		void CalcMarkInfo(float x, float y);

	protected:
		/// 当前活动的标记节点
		osg::observer_ptr<FeExtNode::CExternNode>  m_opCurMarkNode;

		/// 标记管理对象
		CMarkPluginInterface*                      m_pMarkWidget;

		/// 是否开启事件捕获
		bool                                       m_bStartCapture;

		/// 鼠标按下的X、Y位置
		float								       m_fMouseDownX;  
		float								       m_fMouseDownY;

		/// 当前鼠标点击的地球坐标
		osgEarth::GeoPoint                         m_geoPoint;

		/// 当前标记尺寸，用于辅助计算
		double                                     m_dMarkSize;

	};
}
#endif // MARK_EVENT_HANDLER_H
