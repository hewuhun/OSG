/**************************************************************************************************
* @file ModelStyleWidget.h
* @note 模型属性窗口
* @author g00034
* @date 2017.02.20
**************************************************************************************************/
#ifndef MODEL_TAB_WIDGET_H_12
#define MODEL_TAB_WIDGET_H_12

#include <mark/BaseStyleWidget.h>

#include <mark/Common.h>
#include <FeExtNode/ExLodModelNode.h>
#include <FeShell/SystemService.h>

#include "ui_ModelStyleWidget.h"

namespace FreeViewer
{
	/**
	  * @class CModelStyleWidget
	  * @brief 模型属性窗口类
	  * @author g00034
	*/
	class CModelStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @note 构造和析构函数  
		*/
		CModelStyleWidget(FeShell::CSystemService* pSystemService, FeExtNode::CExLodModelNode *pModelNode, bool bCreate, QWidget *parent = 0);
		~CModelStyleWidget();

		/**  
		  * @brief 拒绝修改属性信息
		  * @return bool
		*/
		virtual bool Reject();

	private:
		/**  
		  * @note 角度设置  
		*/
		void SetAngle(double dAngle);
		double GetAngle()const;
		bool GetAngle(double& dAngle)const;

		/**  
		  * @note 大小设置  
		*/
		void SetScale(double dScale);
		double GetScale()const;
		bool GetScale(double& dScale)const;

		/**  
		  * @note 位置设置  
		*/
		void SetCenture(const osg::Vec3& centure);
		osg::Vec3 GetCenture()const;
		bool GetCenture(osg::Vec3& vec)const;

	protected:

		void InitTab();

		//初始化服务界面
		void InitService();

	protected slots:
		/**  
		  * @note 模型选择槽函数
		*/
		void SlotSimplePath(bool);
		void SlotServicePath(int);

		/**  
		  * @note 模型可视范围槽函数
		*/
		void SlotSimpleLow(double);
		void SlotSimpleHigh(double);
		void SlotServiceLow(double);
		void SlotServiceHigh(double);

		/**  
		  * @note 模型位置槽函数  
		*/
		void SlotLongChanged(double);
		void SlotHeightChanged( double dLong);
		void SlotLatChanged( double dLong);

		/**  
		  * @note 模型缩放槽函数  
		*/
		void SlotScaleChanged( double dLong);

		/**  
		  * @note 模型旋转槽函数
		*/
		void SlotZAngleChanged( double dLong);
		void SlotYAngleChanged( double dLong);
		void SlotXAngleChanged( double dLong);

		/**  
		  * @note 模型路径槽函数
		*/
		void SlotSimplePathChanged(const QString&);
		void SlotServicePathChanged(const QString&);

		/**  
		  * @note 服务模型链接槽函数
		*/
		void SlotServicePathLinked();

		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

		/**  
		  * @note radio切换槽函数
		*/
		void SLotLocalRadioClicked();
		void SLotServiceRadioClicked();

		/**  
		  * @note 服务地址栏被点击
		*/
		void SlotUrlClicked();

		/**  
		  * @note 服务地址栏丢失焦点
		*/
		void SlotUrlEditFinished();

	private:
		Ui::model_style_widget                         ui;
		bool                                           m_bCreate;
		bool											m_bServicePathChanged;

		/// 显示范围
		double                                         dSimpleLow;
		double                                         dSimpleHigh;
		double                                         dNormalLow;
		double                                         dNormalHigh;
		double                                         dBestLow;
		double                                         dBestHigh;
		double										  dServiceLow;
		double										  dServiceHigh;

		/// 模型节点
		FeExtNode::CExLodModelNode *	               m_pModelNode;

		/// 系统服务对象
		osg::observer_ptr<FeShell::CSystemService>     m_opSystemService;

		///初始值
	private:
		/// 整个节点的属性
		FeExtNode::CExLodModelNodeOption               m_markOpt; 

		/// 各个分级模型的属性
		typedef struct ModelInfo
		{
			FeExtNode::CExLodModelGroup::LodNodeInfo lodNodeInfo;
			FeExtNode::CExLodNodeOption  lodNodeOpt;

			ModelInfo(){}
			ModelInfo(const FeExtNode::CExLodModelGroup::LodNodeInfo& info)
			{
				lodNodeInfo = info;
				
				if(lodNodeInfo.node.valid())
				{
					lodNodeOpt = *(lodNodeInfo.node->GetOption());
				}
			}
		}ModelInfo;
		std::map<int, ModelInfo> m_mapModelInfo;
	};

}
#endif // MODEL_TAB_WIDGET_H_12

