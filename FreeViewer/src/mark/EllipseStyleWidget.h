/**************************************************************************************************
* @file EllipseStyleWidget.h
* @note 椭圆标记
* @author w00040
* @data 2017-2-20
**************************************************************************************************/
#ifndef ELLIPSE_STYLE_WIDGET_H_
#define ELLIPSE_STYLE_WIDGET_H_

#include <mark/BaseStyleWidget.h>
#include <mark/Common.h>
#include <FeExtNode/ExEllipseNode.h>
#include "ui_EllipseStyleWidget.h"

namespace FreeViewer
{
	/**
	  * @class CEllipseStyleWidget
	  * @brief 椭圆标记
	  * @note 添加椭圆标记
	  * @author w00040
	*/
	class CEllipseStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
		CEllipseStyleWidget(FeExtNode::CExEllipseNode* opMark, QWidget *parent = 0);

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
		  * @brief 椭圆背景填充色改变槽函数
		*/
		void SlotChangeEllipseColor();

		/**  
		  * @brief 椭圆背景填充色透明度改变槽函数
		*/
		void SlotChangeEllipseColorTransparant(double);

		/**  
		  * @brief 椭圆长半径改变槽函数
		*/
		void SlotChangeEllipselRadius(double);

		/**  
		  * @brief 椭圆短半径改变槽函数
		*/
		void SlotChangeEllipsesRadius(double);

		/**  
		  * @brief 椭圆中心点经度值改变槽函数
		*/
		void SlotChangeEllipseLongitude(double);

		/**  
		  * @brief 椭圆中心点纬度值改变槽函数
		*/
		void SlotChangeEllipseLatitude(double);

		/**  
		  * @brief 椭圆中心点高度值改变槽函数
		*/
		void SlotChangeHeight(double);
		
		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

	private:
		Ui::ellipse_style_widget ui;

		///椭圆形节点
		osg::observer_ptr<FeExtNode::CExEllipseNode> m_opMark;

		///初始值
	private:
		FeExtNode::CExEllipseNodeOption m_markOpt;
	};
}

#endif //RECT_STYLE_WIDGET_H_