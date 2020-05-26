/**************************************************************************************************
* @file TiltModelStyleWidget.h
* @note 倾斜摄影标记
* @author w00040
* @data 2017-2-20
**************************************************************************************************/
#ifndef TiltMODEL_TAB_WIDGET_H_12
#define TiltMODEL_TAB_WIDGET_H_12

#include <mark/BaseStyleWidget.h>

#include <mark/Common.h>
#include <FeExtNode/ExTiltModelNode.h>
#include <FeShell/SystemService.h>

#include "ui_TiltModelStyleWidget.h"

namespace FreeViewer
{
	/**
	  * @class CTiltModelStyleWidget
	  * @brief 倾斜摄影模型类
	  * @note 添加倾斜摄影模型
	  * @author w00040
	*/
	class CTiltModelStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
		CTiltModelStyleWidget(FeShell::CSystemService* pSystemService, FeExtNode::CExTiltModelNode *pTiltModelNode, bool bCreate, QWidget *parent = 0);
		
		/**  
		  * @brief 析构函数
		*/
		~CTiltModelStyleWidget();

		/**  
		  * @brief 设置中心点位置
		*/
		void SetCenture(const osg::Vec3& center);

		/**  
		  * @brief 拒绝修改属性信息
		*/
		virtual bool Reject();

	private:
		/**  
		  * @brief 设置模型路径
		*/
		void SetRS(const std::string& qstrRS);

		/**  
		  * @brief 获取模型路径
		*/
		std::string GetRS()const;

		/**  
		  * @brief 设置模型旋转角度
		*/
		void SetAngle(double dAngle);

		/**  
		  * @brief 获取模型旋转角度
		*/
		double GetAngle()const;

		/**  
		  * @brief 获取模型旋转角度
		*/
		bool GetAngle(double& dAngle)const;

		/**  
		  * @brief 设置模型缩放比例
		*/
		void SetScale(double dScale);

		/**  
		  * @brief 获取模型缩放比例
		*/
		double GetScale()const;

		bool GetScale(double& dScale)const;

		/**  
		  * @brief 获取模型中心点位置
		*/
		osg::Vec3 GetCenture()const;

		/**  
		  * @brief 获取模型中心点位置
		*/
		bool GetCenture(osg::Vec3& vec)const;

		/**  
		  * @brief 浏览配置文件路径
		*/
		void BrowseConfigPath();

		/**  
		  * @brief 浏览数据路径
		*/
		void BrowseDataPath();

		/**  
		  * @note 显示服务界面
		*/
		void ShowServericUi(bool);

signals:
		/**  
		  * @note 改变视点界面
		*/
		void SignalViewPointChanged(CFreeViewPoint);

	protected:
		/**  
		  * @brief 初始化tab
		*/
		void InitTab();

		/**  
		  * @note 初始化服务界面
		*/
		void InitService();

	protected slots:
		/**  
		  * @brief 模型经度值改变槽函数
		*/
		void SlotLonChanged(double);

		/**  
		  * @brief 模型高度值改变槽函数
		*/
		void SlotHeightChanged( double dLong);

		/**  
		  * @brief 模型纬度值改变槽函数
		*/
		void SlotLatChanged( double dLong);

		/**  
		  * @brief 模型缩放比例值改变槽函数
		*/
		void SlotScaleChanged( double dLong);

		/**  
		  * @brief 模型旋转角度值改变槽函数
		*/
		void SlotAngleChanged( double dLong);

		/**  
		  * @brief 浏览模型路径按钮点击槽函数
		*/
		void SlotBorwseBtnClicked(bool);

		/**  
		  * @note 服务界面显示槽函数
		*/
		void SlotServiceRadioClicked();

		/**  
		  * @note 本地界面显示槽函数
		*/
		void SlotLocalRadioClikced();

		/**  
		  * @note 服务comboBox切换槽函数
		*/
		void SlotServicePath(int);

		/**  
		  * @note 服务链接槽函数
		*/
		void SlotServicePathLinked();

		/**  
		  * @note 服务地址栏被点击
		*/
		void SlotUrlClicked();

		/**  
		  * @note 服务地址栏丢失焦点
		*/
		void SlotUrlEditFinished();

		/**  
		  * @note 服务路径发生改变
		*/
		void SlotServicePathChanged(const QString&);

	private:
		Ui::tiltmodel_style_widget                     ui;

		bool						                   m_bCreate;
		bool											m_bServicePathChanged;
		bool											m_bInit;		//是否为初始化

		///倾斜摄影数据节点
		FeExtNode::CExTiltModelNode *	               m_pTiltModelNode;

		///系统服务
		osg::observer_ptr<FeShell::CSystemService>     m_opSystemService;

		///初始值
	private:
		FeExtNode::CExTiltModelNodeOption				m_markOpt;
	};

}
#endif // TiltMODEL_TAB_WIDGET_H_12

