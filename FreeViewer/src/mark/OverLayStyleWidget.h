/**************************************************************************************************
* @file OverlayStyleWidget.h
* @note 遮盖标绘属性窗口
* @author g00034
* @date 2017.02.20
**************************************************************************************************/
#ifndef MiliMark_OverLay_TAB_WIDGET_H
#define MiliMark_OverLay_TAB_WIDGET_H

#include <mark/BaseStyleWidget.h>

#include <mark/Common.h>
#include <FeExtNode/ExOverLayNode.h>
#include <FeShell/SystemService.h>

#include "ui_OverLayStyleWidget.h"

namespace FreeViewer
{
	/**
	* @class COverLayStyleWidget
	* @brief 遮盖标绘属性窗口
	* @author g00034
	*/
	class COverLayStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT
	public:
		/**  
		  * @note 构造和析构函数  
		*/
		COverLayStyleWidget(FeShell::CSystemService* pSystemService, FeExtNode::CExOverLayNode *pMiliMarkNode, bool bCreate, QWidget *parent = 0);
		~COverLayStyleWidget();

		/**  
		  * @note 设置位置
		*/
		void SetCenture(const osgEarth::GeoPoint& centure);

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
		  * @note 设置位置
		*/
		osg::Vec3 GetCenture()const;
		bool GetCenture(osg::Vec3& vec)const;

		/**  
		  * @note 设置透明度
		*/
		void SetAlpha(double dAlpha);
		double GetAlpha();

		/**  
		  * @note 设置图片尺寸
		*/
		void SetImageSize( osg::Vec2 vecImageSize );
		osg::Vec2 GetImageSize();

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
		void SlotImageWidthChanged( double nWidht );
		void SlotImageHeightChanged( double nHeight );

		/**  
		  * @note 透明度槽函数
		*/
		void SlotAlphaChanged(double dAlpha);
			
		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int);

	private:
		Ui::overlay_style_widget ui;

		bool						m_bCreate;

		/// 关联标记对象
		osg::observer_ptr<FeExtNode::CExOverLayNode> m_opMark;
		osg::observer_ptr<FeShell::CSystemService>   m_opSystemService;

		///初始值
	private:
		FeExtNode::CExOverLayNodeOption m_markOpt;
	};

}
#endif // MiliMark_OverLay_TAB_WIDGET_H

