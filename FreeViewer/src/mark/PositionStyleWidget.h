/**************************************************************************************************
* @file PositionStyleWidget.h
* @note 标记位置信息窗口
* @author g00034
* @date 2017.02.20
**************************************************************************************************/
#ifndef POSITION_STYLE_WIDGET_H
#define POSITION_STYLE_WIDGET_H


#include <mark/BaseStyleWidget.h>

#include <mark/Common.h>

#include <FeExtNode/ExPlaceNode.h>
#include "ui_PositionStyleWidget.h"


namespace FreeViewer
{
	/**
	  * @class CPositionStyleWidget
	  * @brief 标记位置信息窗口类
	  * @author g00034
	*/
	class CPositionStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @note 构造和析构函数  
		*/
		CPositionStyleWidget(FeExtNode::CExternNode* opMark, QWidget *parent = 0);
		~CPositionStyleWidget(void);

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
		  * @note 路径选择按钮槽函数  
		*/
		void slot_pathBtnClick();

		/**  
		  * @note 字体大小槽函数  
		*/
		void slot_fontSizeChanged(double value);

		/**  
		  * @brief 路径值改变槽函数
		*/
		void SlotPathChanged(const QString& );

		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

		/**  
		  * @brief 调节经度槽函数
		*/
		void SlotLongChanged(double);

		/**  
		  * @brief 调节纬度槽函数
		*/
		void SlotLatChanged(double);

		/**  
		  * @brief 调节高度槽函数
		*/
		void SlotHeightChanged(double);

	public:
		Ui::position_tab_widget ui;

		/// 关联标记对象
		osg::observer_ptr<FeExtNode::CExternNode> m_opMark;

		/// 初始值
	private:
		FeExtNode::CExPlaceNodeOption m_placeOpt;
	};
}

#endif // POSITION_STYLE_WIDGET_H
