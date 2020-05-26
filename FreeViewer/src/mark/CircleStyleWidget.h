/**************************************************************************************************
* @file CircleStyleWidget.h
* @note 圆标记
* @author w00040
* @data 2017-2-20
**************************************************************************************************/
#ifndef CIRCLE_STYLE_WIDGET_H_
#define CIRCLE_STYLE_WIDGET_H_

#include <mark/BaseStyleWidget.h>
#include <mark/Common.h>
#include <FeExtNode/ExCircleNode.h>
#include "ui_CircleStyleWidget.h"

namespace FreeViewer
{
	/**
	  * @class CCircleStyleWidget
	  * @brief 圆标记类
	  * @note 添加圆标记
	  * @author w00040
	*/
	class CCircleStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
		CCircleStyleWidget(FeExtNode::CExCircleNode* opMark, QWidget *parent = 0);

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
		  * @brief 圆背景填充色改变槽函数
		*/
		void SlotChangeCircleColor();

		/**  
		  * @brief 圆背景填充色透明度改变槽函数
		*/
		void SlotChangeCircleColorTransparant(double);

		/**  
		  * @brief 圆半径改变槽函数
		*/
		void SlotChangeRadius(double);

		/**  
		  * @brief 圆中心点经度值改变槽函数
		*/
		void SlotChangeCircleLongitude(double);

		/**  
		  * @brief 圆中心点纬度值改变槽函数
		*/
		void SlotChangeCircleLatitude(double);

		/**  
		  * @brief 圆中心点高度值改变槽函数
		*/
		void SlotChangeHeight(double);
		
		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

	private:
		Ui::circle_style_widget ui;

		///圆形节点
		osg::observer_ptr<FeExtNode::CExCircleNode> m_opMark;

		///初始值
	private:
		FeExtNode::CExCircleNodeOption m_markOpt;
	};
}

#endif //RECT_STYLE_WIDGET_H_