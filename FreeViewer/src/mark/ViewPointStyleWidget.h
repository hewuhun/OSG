/**************************************************************************************************
* @file ViewPointStyleWidget.h
* @note 视点信息窗口类
* @author g00034
* @date 2017.02.20
**************************************************************************************************/

#ifndef VIEW_POINT_TAB_WIDGET_H_12
#define VIEW_POINT_TAB_WIDGET_H_12

#include <mark/BaseStyleWidget.h>
#include <FeExtNode/ExternNode.h>
#include <FeUtils/FreeViewPoint.h>
#include <FeShell/SystemService.h>
#include <mark/Common.h>

#include "ui_ViewPointStyleWidget.h"

namespace FreeViewer
{
	/**
    * @class CViewPointStyleWidget
    * @brief 视点信息窗口类， 用于显示视点信息
    * @author g00034
    */
	class CViewPointStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @note 构造和析构函数  
		*/
		CViewPointStyleWidget(
			FeShell::CSystemService* pSystemService, 
			FeExtNode::CExternNode* pExternNode, 
			bool bCreate,
			QWidget *parent = 0);
		~CViewPointStyleWidget();

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
		  * @brief 设置视点
		*/
		void SetViewPoint(const FeUtil::CFreeViewPoint& freeViewPoint);

		/**  
		  * @brief 内容初始化
		*/
		void BuildContext();

	public slots:
		/**  
		  * @brief 当前视点槽函数
		*/
		void SlotCurrentView(bool);

		/**  
		  * @brief 重置视点槽函数
		*/
		void SlotResetView(bool);

		/**  
		  * @brief 可视范围槽函数
		*/
		void SlotChangeRangeLow(double);
		void SlotChangeRangeHigh(double);

		/**  
		  * @note 修改视点界面槽函数
		*/
		void SlotUiChanged(double);

		/**  
		  * @note 改变模型位置时 视点发生变化
		*/
		void SlotChangedViwePointByMode(CFreeViewPoint);

	private:
		Ui::view_point_stye_widget			           ui;
		bool								           m_bCreate;

		/// 关联标记
		osg::observer_ptr<FeExtNode::CExternNode>      m_opExNode;

		/// 系统服务对象
		osg::observer_ptr<FeShell::CSystemService>     m_opSystemService;

		/// 初始值
	private:
		FeUtil::CFreeViewPoint  m_viewPoint;
		double                  m_dRangeLow;
		double                  m_dRangeHigh;

		//是否点击界面对视点发生修改
		bool						m_bViewPointChanged;
	};

}
#endif // MODEL_TAB_WIDGET_H_12

