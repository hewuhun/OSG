/**************************************************************************************************
* @file SilverliningSetWidget.h
* @note 设置云层参数
* @author z00013
* @data 2016-3-8
**************************************************************************************************/
#ifndef SILVERLINING_SET_WIDGET_H
#define SILVERLINING_SET_WIDGET_H 1

#include <QDialog>
#include <FeShell/SystemService.h>
#include <FeShell/EnvironmentSerializer.h>

#include <FeSilverlining/SilverliningSys.h>

#include "ui_SilverLiningSetWidget.h"

namespace FreeViewer
{
	class CSettingWidget;

    /**
      * @class CSilverLiningSetWidget
      * @note 设置云层参数
      * @author z00013
    */
    class CSilverLiningSetWidget : public QDialog
    {
        Q_OBJECT

    public:

        CSilverLiningSetWidget(CSettingWidget* pSetttingWidget);

        ~CSilverLiningSetWidget();

	public:
		/**  
		  * @note 根据传入数据重置设置对话框以及对应的效果  
		  * @param data [in] 配置数据
		*/
		void Reset(FeShell::SEnvCloudSetData data);

    protected:
		/**
		*note 初始化窗口
		*/
		void InitWidget();

		/**
		*note 初始化上下文环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();


		void CloudEnable(bool);
	signals:
		void SignalFormCloud(bool);

	protected slots:
		void SlotCloudsVisibleChanged(bool); //云层开关
        void SlotCloudsAltitudeChanged(int); //云层高度控制
		void SlotCloudsThicknessChanged(int);//云层厚度
        void SlotCloudsDensityChanged(int); //云层密度
		void SlotCloudsDensityChanged(double);
        void SlotCloudsAlphaChanged(int); //云层透明度
		void SlotCloudsAlphaChanged(double);

		void SlotWindSpeedChanged(int); //风力速度
        void SlotWindDirectionChanged(int); //风力方向

        void SlotCloudTypeType(int nIndex); //云层形状
		
		void SlotSetCloud(bool);

    private:
		Ui::silverLining_set_widget    ui;

		CSettingWidget*											m_pSettingWidget;

		osg::observer_ptr<FeShell::CSystemService>				m_opSystemService;

		osg::observer_ptr<FeSilverLining::CSilverLiningSys>		m_opSilverLiningSys;
    };

}
#endif // SILVERLINING_SET_WIDGET_H
