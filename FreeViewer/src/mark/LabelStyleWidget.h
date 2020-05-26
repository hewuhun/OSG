/**************************************************************************************************
* @file LabelStyleWidget.h
* @note 标牌标记
* @author w00040
* @data 2017-2-20
**************************************************************************************************/
#pragma once
#include <QWidget>
#include <QToolBar>
#include <QEvent>
#include <QMouseEvent>

#include <mark/BaseStyleWidget.h>
#include <mark/Common.h>
#include <FeExtNode/ExternNode.h>
#include <FeExtNode/ExLabelNode.h>
#include "ui_LabelStyleWidget.h"

namespace FreeViewer
{
	/**
	  * @class CLabelStyleWidget
	  * @brief 标牌标记类
	  * @note 添加标牌标记
	  * @author w00040
	*/
	class CLabelStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
		CLabelStyleWidget(FeExtNode::CExternNode* opMark, QWidget *parent = 0);

		/**  
		  * @brief 析构函数
		*/
		~CLabelStyleWidget(void);

		/**  
		  * @brief 拒绝修改属性信息
		  * @return bool
		*/
		virtual bool Reject();
		
	private:
		/**  
		* @note 初始化界面
		*/
		void InitWidget();

		/**  
		* @note 初始化节点树
		*/
		void InitLabelTree();
		
		/**  
		* @note 初始化树根节点
		*/
		void InitLabelTreeRoot();
		
		/**  
		* @note 初始化树子节点
		*/
		void AddTreeItem( QTreeWidgetItem* pParentItem, FeExtNode::CExternNode* pExternNode );

		/**  
		* @note 设置根节点信息
		*/
		void SetRoot();
		
		/**  
		* @note 设置子节点信息
		*/
		void SetItem(FeExtNode::CExLabelItemNode* pLabelItemNode);
		
		/**  
		* @note 设置文字
		*/
		void SetText(FeExtNode::CExLabelItemNode* pLabelItemNode);
		
		/**  
		* @note 设置位置
		*/
		void SetPosition();
		
		/**  
		* @note 设置文字颜色
		*/
		void SetTextColor(FeExtNode::CExLabelItemNode* pLabelItemNode);
		
		/**  
		* @note 设置背景颜色
		*/
		void SetBackColor(FeExtNode::CExLabelItemNode* pLabelItemNode);

		/**  
		* @note 设置背景图片
		*/
		void SetBackImage(FeExtNode::CExLabelItemNode* pLabelItemNode);
		
		/**  
		* @note 设置对齐方式
		*/
		void SetAlignment(FeExtNode::CExLabelItemNode* pLabelItemNode);
		
		/**  
		* @note 获取树节点对应Label节点
		*/
		FeExtNode::CExternNode* GetMarkByItem(QTreeWidgetItem* pItem);

	private slots:		
		/**  
		* @note 设置节点文字
		*/
		void SlotItemTextChanged(QString);

		/**  
		* @note 设置文字颜色
		*/
		void SlotTextColor();

		/**  
		* @note 设置背景颜色
		*/
		void SlotBackColor();

		/**  
		* @note 设置背景图片
		*/
		void SlotImagePath(bool);

		/**  
		* @note 背景图片路径变化
		*/
		void SlotImagePathChange(QString);

		/**  
		* @note 设置文字大小
		*/
		void SlotTextSizeChanged(double);

		/**  
		* @note 设置显示宽度
		*/
		void SlotLabelWidthChanged(double);

		/**  
		* @note 设置显示高度
		*/
		void SlotLabelHeightChanged(double);

		/**  
		* @note 设置经度
		*/
		void SlotLongChanged(double);

		/**  
		* @note 设置纬度
		*/
		void SlotLatChanged( double);

		/**  
		* @note 设置高度
		*/
		void SlotHeightChanged( double);

		/**  
		* @note 设置对齐方式
		*/
		void SlotAlignChanged(int);

		/**  
		* @note 添加节点
		*/
		void SlotAddItem();

		/**  
		* @note 删除节点
		*/
		void SlotDeleteItem();

		/**  
		* @note 清除节点
		*/
		void SlotClearItem();

		/**  
		* @note 点击树节点
		*/
		void SlotItemClicked(QTreeWidgetItem*, int);
			
		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);


	public:
		Ui::label_style_widget ui;//界面

		QTreeWidgetItem*	m_rootItem;//树根节点

		FeExtNode::CExLabelNode* m_pLabelMark;//Label根节点

		FeExtNode::CExLabelItemNode* pCurrentItemNode;//Label当前节点


		///初始值
	private:
		FeExtNode::CExLabelNodeOption m_markOpt;
	};
}
