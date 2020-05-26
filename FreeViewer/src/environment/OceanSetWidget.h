/**************************************************************************************************
* @file OceanSetWidget.h
* @note 设置海洋参数
* @author l00008
* @data 2015-4-27
**************************************************************************************************/
#ifndef OCEAN_SET_WIDGET_H
#define OCEAN_SET_WIDGET_H 1

#include <QDialog>

#include <FeShell/SystemService.h>
#include <FeShell/EnvironmentSerializer.h>

#include <FeOcean/OceanSys.h>

#include "ui_OceanSetWidget.h"

namespace FreeViewer
{
	class CSettingWidget;

    /**
      * @class COceanSetWidget
      * @note 设置海洋效果参数
      * @author l00008
    */
    class COceanSetWidget : public QDialog
    {
        Q_OBJECT

    public:
        COceanSetWidget( CSettingWidget* pSetttingWidget );

        ~COceanSetWidget();
		
	public:
		/**  
		  * @note 根据传入数据重置设置对话框以及对应的效果  
		  * @param data [in] 配置数据
		*/
		void Reset(FeShell::SEnvOceanSetData data);

    protected:
		/**
		*note 初始化窗口
		*/
		void InitWidget();

		/**
		*note 初始化上下文环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

		/**
		*note 海洋控制面板是否可以使用
		*/
		void ControlWidgetEnable(bool);

	signals:
		void SignalFromOcean(bool);

	protected slots:
		void SlotOceanVisibleChanged(bool);		//海洋开关
        void SlotOceanLevelChanged(int);		//海洋高度控制
        void SlotViewPointChanged(int);			//海洋视点高度控制
        void SlotAboveChanged(int);				//海洋上气雾效果
		void SlotAboveColorChanged();			//海洋上气雾效果颜色
        void SlotBelowChanged(int);				//海洋下气雾效果
		void SlotBelowColorChanged();			//海洋下气雾效果颜色

		void SlotWindSpeedChanged(int);			//风力速度
		void SlotWindDirectionChanged(int);		//浪花、风力方向
		void SlotWindLengthDoubleChange(double);		//风波长
		void SlotWindLengthChange(int);

		void SlotSetOcean(bool);

	private:
		Ui::ocean_set_widget    ui;

		CSettingWidget*									m_pSettingWidget;

		osg::observer_ptr<FeShell::CSystemService>		m_opSystemService;

		osg::observer_ptr<FeOcean::COceanSys>			m_opOcean;
		
		double									m_dSpeed;
		double									m_dDirection;
		double									m_dLength;
    };

}
#endif // OCEAN_SET_WIDGET_H
