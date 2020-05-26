/**************************************************************************************************
* @file BillboardStyleWidget.h
* @note 广告牌属性窗口
* @author g00034
* @date 2017.02.20
**************************************************************************************************/

#ifndef BILLBOARD_TAB_WIDGET_H_12
#define BILLBOARD_TAB_WIDGET_H_12

#include <mark/BaseStyleWidget.h>

#include <mark/Common.h>
#include <FeExtNode/ExBillBoardNode.h>
#include <FeShell/SystemService.h>

#include "ui_BillboardStyleWidget.h"

namespace FreeViewer
{
	/**
	* @class CBillboardStyleWidget
	* @brief 广告牌属性窗口
	* @author g00034
	*/
	class CBillboardStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @note 构造和析构函数  
		*/
		CBillboardStyleWidget(FeShell::CSystemService* pSystemService, FeExtNode::CExBillBoardNode *pMiliMarkNode, bool bCreate, QWidget *parent = 0);
		~CBillboardStyleWidget();

		/**  
		  * @note 设置位置
		*/
		void SetCenture(osgEarth::GeoPoint centure);

		/**  
		  * @brief 拒绝修改属性信息
		  * @return bool
		*/
		virtual bool Reject();

	private:
		/**  
		  * @note 设置图片资源路径
		*/
		void SetMarkImgPath(const std::string& qstrRS);
		std::string GetMarkImgPath()const;

		/**  
		  * @note 设置角度
		*/
		void SetAngle(double dAngle);
		double GetAngle()const;
		bool GetAngle(double& dAngle)const;

		/**  
		  * @note 设置缩放
		*/
		void SetScale(double dScale);
		double GetScale()const;
		bool GetScale(double& dScale)const;

		/**  
		  * @note 设置图片尺寸
		*/
		void SetImageSize(osg::Vec2 vecImageSize);
		osg::Vec2 GetImageSize();

		/**  
		  * @note 设置位置
		*/
		osg::Vec3 GetCenture()const;
		bool GetCenture(osg::Vec3& vec)const;

	protected:
		/**  
		  * @note 初始化Tab
		*/
		void InitTab();

	protected slots:
		/**  
		  * @note 图片路径槽函数
		*/
		void SlotRSPath(bool);

		/**  
		  * @brief 路径值改变槽函数
		*/
		void SlotPathChanged(const QString& );

		/**  
		  * @note 位置槽函数
		*/
		void SlotLongChanged(double);
		void SlotHeightChanged( double dLong);
		void SlotLatChanged( double dLong);

		/**  
		  * @note 缩放槽函数
		*/
		void SlotScaleChanged( double dLong);

		/**  
		  * @note 旋转槽函数
		*/
		void SlotAngleChanged( double dLong);

		/**  
		  * @note 尺寸槽函数
		*/
		void SlotImageWidthChanged(int nWidht);
		void SlotImageHeightChanged(int nHeight);

		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

	private:
		Ui::billboard_style_widget	ui;

		bool						m_bCreate;

		/// 关联标记
		osg::observer_ptr<FeExtNode::CExBillBoardNode> m_opMark;
		osg::observer_ptr<FeShell::CSystemService>     m_opSystemService;

		///初始值
	private:
		FeExtNode::CExBillBoardNodeOption m_markOpt;
	};

}
#endif // BILLBOARD_TAB_WIDGET_H_12

