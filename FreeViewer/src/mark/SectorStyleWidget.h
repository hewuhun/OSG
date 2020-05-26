/**************************************************************************************************
* @file SectorStyleWidget.h
* @note 扇形标记
* @author w00040
* @data 2017-2-20
**************************************************************************************************/
#ifndef SECTOR_STYLE_WIDGET_H_
#define SECTOR_STYLE_WIDGET_H_

#include <mark/BaseStyleWidget.h>
#include <mark/Common.h>
#include <FeExtNode/ExSectorNode.h>
#include "ui_SectorStyleWidget.h"

namespace FreeViewer
{
	/**
	  * @class CSectorStyleWidget
	  * @brief 扇形标记
	  * @note 添加扇形标记
	  * @author w00040
	*/
	class CSectorStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
		CSectorStyleWidget(FeExtNode::CExSectorNode* opMark, QWidget *parent = 0);

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
		  * @brief 扇形背景填充色改变槽函数
		*/
		void SlotChangeSectorColor();

		/**  
		  * @brief 扇形背景填充色透明度改变槽函数
		*/
		void SlotChangeSectorColorTransparant(double);

		/**  
		  * @brief 扇形起点位置改变槽函数
		*/
		void SlotChangeSectorStart(double);

		/**  
		  * @brief 扇形终点位置改变槽函数
		*/
		void SlotChangeSectorEnd(double);

		/**  
		  * @brief 扇形半径改变槽函数
		*/
		void SlotChangeRadius(double);

		/**  
		  * @brief 扇形中心点经度值改变槽函数
		*/
		void SlotChangeSectorLongitude(double);

		/**  
		  * @brief 扇形中心点纬度值改变槽函数
		*/
		void SlotChangeSectorLatitude(double);

		/**  
		  * @brief 扇形中心点高度值改变槽函数
		*/
		void SlotChangeHeight(double);
			
		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

	private:
		Ui::sector_style_widget ui;

		///扇形节点
		osg::observer_ptr<FeExtNode::CExSectorNode> m_opMark;

		///初始值
	private:
		FeExtNode::CExSectorNodeOption m_markOpt;
	};
}

#endif //RECT_STYLE_WIDGET_H_