/**************************************************************************************************
* @file ArcStyleWidget.h
* @note 弧形标绘属性窗口
* @author g00034
* @date 2017.02.20
**************************************************************************************************/

#ifndef ARC_STYLE_WIDGET_H_
#define ARC_STYLE_WIDGET_H_

#include <mark/BaseStyleWidget.h>
#include <mark/Common.h>
#include <FeExtNode/ExSectorNode.h>
#include "ui_ArcStyleWidget.h"

namespace FreeViewer
{
	/**
	* @class CArcStyleWidget
	* @brief 弧形标绘属性窗口
	* @author g00034
	*/
	class CArcStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @note 构造和析构函数  
		*/
		CArcStyleWidget(FeExtNode::CExArcNode* opMark, QWidget *parent = 0);

		/**  
		  * @brief 拒绝修改属性信息
		  * @return bool
		*/
		virtual bool Reject();

	private:
		/**  
		  * @note 控件初始化
		*/
		void initWidget();

	protected slots:
		/**  
		  * @note 线段属性槽函数
		*/ 
		void SlotChangeLineWidth(double );
		void SlotChangeLineColor();
		void SlotChangeLineColorTransparant(double);

		/**  
		  * @note 弧形属性槽函数
		*/ 
		void SlotChangeArcColor();
		void SlotChangeArcColorTransparant(double);
		void SlotChangeArcStart(double);
		void SlotChangeArcEnd(double);
		void SlotChangeRadius(double);

		/**  
		  * @note 位置槽函数
		*/ 
		void SlotChangeArcLongitude(double);
		void SlotChangeArcLatitude(double);
		void SlotChangeHeight(double);

		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

	private:
		Ui::arc_style_widget ui;

		/// 关联标记对象
		osg::observer_ptr<FeExtNode::CExArcNode> m_opMark;

		///初始值
	private:
		FeExtNode::CExArcNodeOption m_markOpt;
	};
}

#endif //RECT_STYLE_WIDGET_H_