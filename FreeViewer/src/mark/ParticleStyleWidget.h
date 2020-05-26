/**************************************************************************************************
* @file ParticleStyleWidget.h
* @note 粒子属性窗口
* @author w00040
* @data 2017-3-17
**************************************************************************************************/
#ifndef PARTICLE_STYLE_WIDGET_H
#define PARTICLE_STYLE_WIDGET_H

#include <mark/BaseStyleWidget.h>

#include <FeExtNode/ExParticleNode.h>

#include "ui_ParticleStyleWidget.h"

namespace FreeViewer
{
	/**
	  * @class CParticleStyleWidget
	  * @brief 粒子属性窗口类
	  * @note 设置粒子属性
	  * @author w00040
	*/
	class CParticleStyleWidget : public CBaseStyleWidget
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CParticleStyleWidget(FeExtNode::CExParticleNode* opMark, QWidget *parent = 0);

		/**  
		  * @brief 析构函数
		*/
		~CParticleStyleWidget();

		/**  
		  * @brief 初始化界面
		*/
		void InitWidget();

		/**  
		  * @brief 拒绝修改属性信息
		*/
		virtual bool Reject();

	protected:
		/**  
		  * @brief 设置粒子位置
		*/
		void SetParticleCenter();

		/**  
		  * @brief 设置角度
		*/
		void SetParticleAngle();

	protected slots:
		/**  
		  * @brief 经度改变槽函数
		*/
		void SlotLongitudeChanged(double value);

		/**  
		  * @brief 纬度改变槽函数
		*/
		void SlotLatitudeChanged(double value);

		/**  
		  * @brief 高度改变槽函数
		*/
		void SlotHeightChanged(double value);

		/**  
		  * @brief 角度X 改变槽函数
		*/
		void SlotAngleXChanged(double x);

		/**  
		  * @brief 角度Y 改变槽函数
		*/
		void SlotAngleYChanged(double y);

		/**  
		  * @brief 角度Z 改变槽函数
		*/
		void SlotAngleZChanged(double z);

		/**  
		  * @brief 添加粒子路径槽函数
		*/
		void SlotPathBtnClicked();

		/**  
		  * @brief 缩放系数改变
		*/
		void SlotScaleChanged(double);

	private:
		Ui::particle_styl_widget ui;

		///粒子节点
		osg::observer_ptr<FeExtNode::CExParticleNode>	               m_pParticleNode;

		FeExtNode::CExParticleNodeOption							   m_pParticleNodeOpt;
	};

}

#endif
