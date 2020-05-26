/**************************************************************************************************
* @file LineStyleWidget.h
* @note 线标记属性窗口
* @author g00034
* @date 2017.02.20
**************************************************************************************************/

#ifndef LINE_STYLE_WIDGET_H
#endif  LINE_STYLE_WIDGET_H

#include <mark/BaseStyleWidget.h>

#include <mark/Common.h>

#include <FeExtNode/ExFeatureNode.h>

#include "ui_LineStyleWidget.h"

namespace FreeViewer
{
	/**
	* @class CLineStyleWidget
	* @brief 线和多边形样式
	* @note 线宽 线颜色 透明度 填充色
	* @author z00019
	*/
	class CLineStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @note 构造和析构函数  
		*/
		CLineStyleWidget(FeExtNode::CExFeatureNode* opMark, QWidget *parent = 0);

	public:
		/**  
		* @brief 设置多边形可见性
		* @note 如果用户选择线标记 则将多边形样式属性设置为隐藏  
		* @param 参数 [in] bVisible 多边形样式可见性
		* @return 无
		*/
		void setPolygonStyleVisible(bool bVisible);

		/**  
		  * @brief 确定修改属性信息
		  * @return bool
		*/
		virtual bool Accept();

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
		  * @note 线宽槽函数  
		*/
		void SlotChangeLineWidth(double );

		/**  
		  * @note 线的颜色变化槽函数  
		*/
		void SlotChangeLineColor();
		void SlotChangeLineColorTransparant(double);

		/**  
		  * @note 多边形颜色变化槽函数  
		*/
		void SlotChangePolygonColor();
		void SlotChangePolygonColorTransparant(double);

		/**  
		  * @note 高度变化槽函数  
		*/
		void SlotChangeHeight(double);
			
		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

	private:
		Ui::line_style_widget ui;

		/// 关联标记
		osg::observer_ptr<FeExtNode::CExFeatureNode> m_opMark;

		/// 初始值
	private:
		FeExtNode::CExFeatureNodeOption m_featureOpt;

	};
}


// LINE_STYLE_WIDGET_H