/**************************************************************************************************
* @file DescribeStyleWidget.h
* @note 标记描述信息窗口
* @author g00034
* @date 2017.02.20
**************************************************************************************************/

#ifndef DESCRIBE_TAB_WIDGET_H_12
#define DESCRIBE_TAB_WIDGET_H_12

#include <mark/BaseStyleWidget.h>

#include <FeExtNode/ExternNode.h>
#include <mark/Common.h>
#include <FeExtNode/ExternNode.h>
#include "ui_DescribeStyleWidget.h"

namespace FreeViewer
{
	/**
	  * @class CDescribeStyleWidget
	  * @brief 标记描述窗口类
	  * @author g00034
	*/
	class CDescribeStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @note 构造和析构函数  
		*/
		CDescribeStyleWidget(FeExtNode::CExternNode* pExternNode, QWidget *parent = 0);
		~CDescribeStyleWidget();

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

	protected:
		/**  
		  * @note 内容初始化
		*/
		void BuildContext();

	protected slots:
		/**  
		  * @brief textEdit改变槽函数
		*/
		void SlotLimitMaxLength();

	private:
		Ui::describe_style_widget ui;

		/// 关联标记对象
		osg::observer_ptr<FeExtNode::CExternNode>      m_opExNode;

		/// 初始值
	private:
		std::string m_strDesc;
	};

}
#endif // DESCRIBE_TAB_WIDGET_H_12

