/**************************************************************************************************
* @file RectStyleWidget.h
* @note 矩形标记
* @author w00040
* @data 2017-2-20
**************************************************************************************************/
#ifndef RECT_STYLE_WIDGET_H_
#define RECT_STYLE_WIDGET_H_

#include <mark/BaseStyleWidget.h>
#include <mark/Common.h>
#include <FeExtNode/ExRectNode.h>
#include "ui_RectStyleWidget.h"

namespace FreeViewer
{
	/**
	  * @class CRectStyleWidget
	  * @brief 矩形标记
	  * @note 添加矩形标记
	  * @author w00040
	*/
	class CRectStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
		CRectStyleWidget(FeExtNode::CExRectNode* opMark, QWidget *parent = 0);

		/**  
		  * @brief 拒绝修改属性信息
		  * @return bool
		*/
		virtual bool Reject();

	private:
		/**  
		  * @brief 初始化界面
		*/
		void initWidget();

	protected slots:
		/**  
		  * @brief 线宽改变槽函数
		*/
		void SlotChangeLineWidth(double );

		/**  
		  * @brief 线颜色改变槽函数
		*/
		void SlotChangeLineColor();

		/**  
		  * @brief 线颜色透明度改变槽函数
		*/
		void SlotChangeLineColorTransparant(double);

		/**  
		  * @brief 矩形背景填充色改变槽函数
		*/
		void SlotChangeRectColor();

		/**  
		  * @brief 矩形背景填充色透明度改变槽函数
		*/
		void SlotChangeRectColorTransparant(double);

		/**  
		  * @brief 矩形高度值改变槽函数
		*/
		void SlotChangeRectLength(double);

		/**  
		  * @brief 矩形宽度值改变槽函数
		*/
		void SlotChangeRectWidth(double);

		/**  
		  * @brief 矩形中心点经度值改变槽函数
		*/
		void SlotChangeRectLongitude(double);

		/**  
		  * @brief 矩形中心点纬度值改变槽函数
		*/
		void SlotChangeRectLatitude(double);

		/**  
		  * @brief 矩形中心点高度值改变槽函数
		*/
		void SlotChangeHeight(double);
			
		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

	private:
		Ui::rect_style_widget ui;

		///矩形节点
		osg::observer_ptr<FeExtNode::CExRectNode> m_opMark;

		///初始值
	private:
		FeExtNode::CExRectNodeOption m_markOpt;

	};
}

#endif //RECT_STYLE_WIDGET_H_