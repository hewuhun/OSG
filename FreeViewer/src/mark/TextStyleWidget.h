/**************************************************************************************************
* @file TextStyleWidget.h
* @note 文本标记
* @author w00040
* @data 2017-2-20
**************************************************************************************************/
#pragma once
#include <mark/BaseStyleWidget.h>

#include <mark/Common.h>

#include <FeExtNode/ExTextNode.h>
#include "ui_TextStyleWidget.h"

namespace FreeViewer
{
	/**
	  * @class CTextStyleWidget
	  * @brief 文本标记类
	  * @note 添加文本标记
	  * @author w00040
	*/
	class CTextStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
		CTextStyleWidget(FeExtNode::CExTextNode* opMark, QWidget *parent = 0);

		/**  
		  * @brief 析构函数
		*/
		~CTextStyleWidget(void);

		/**  
		  * @brief 拒绝修改属性信息
		  * @return bool
		*/
		virtual bool Reject();

	protected slots:
		/**  
		  * @brief 设置文本颜色槽函数
		*/
		void SlotTextColor();

		/**  
		  * @brief 设置文本大小
		*/
		void SlotTextSize(double );

		/**  
		  * @brief 文本标记经度位置改变槽函数
		*/
		void SlotChangeSectorLongitude(double);

		/**  
		  * @brief 文本标记纬度位置改变槽函数
		*/
		void SlotChangeSectorLatitude(double);

		/**  
		  * @brief 文本标记高度位置改变槽函数
		*/
		void SlotChangeHeight(double);
			
		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

	private:
		/**  
		  * @brief 初始化界面
		*/
		void initWidget();

	public:
		Ui::text_tab_widget ui;

		///文本节点
		osg::observer_ptr<FeExtNode::CExTextNode> m_opMark;

		///初始值
	private:
		FeExtNode::CExTextNodeOption m_markOpt;

	};
}
