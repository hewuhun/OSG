/**************************************************************************************************
* @file MeasurePluginInterface.h
* @note 量算分析相关的界面类
* @author g00034
* @data 2017-01-17
**************************************************************************************************/
#ifndef MEASURE_PLUGIN_INTERFACE_H
#define MEASURE_PLUGIN_INTERFACE_H 1

#include <FeMeasure/MeasureSys.h>
#include <FeMeasure/MeasureFactory.h>

#include <mainWindow/UIObserver.h>

#include <measure/MeasureDisplayWidget.h>


namespace FreeViewer
{
    /**
      * @class CMeasurePluginInterface
      * @note 管理量算分析的界面
      * @author l00008
    */
    class CMeasurePluginInterface : public CUIObserver
    {
        Q_OBJECT

    public:
		/**  
		* @note 构造函数  
		* @param pMainWindow [in] 主窗口
		*/
		CMeasurePluginInterface(CFreeMainWindow* pMainWindow);

        ~CMeasurePluginInterface();

    protected:
		void InitWidget();

		void BuildContext();
	
	protected:
		/// 关闭最后一次测量
		void CloseLastMeasureWidget();

		///互斥按钮处理
		void SetMutex(QAction* pCurrentAction);

	protected slots:
		/// 量算对应的槽函数
		void SlotSpaceDistMeasure(bool bAction);
		void SlotSurfDistMeasure(bool bAction);
		void SlotHeiDistMeasure(bool bAction);
		void SlotSpaceAreaMeasure(bool bAction);
		void SlotShadowAreaMeasure(bool bAction);
		void SlotLineVisibleMeasure(bool bAction);
		void SlotRadialLineVisibleMeasure(bool bAction);
		void SlotBuildingVisibleMeasure(bool bAction);
		void SlotFloodMeasure(bool bAction);
		void SlotProfileMeasure(bool bAction);
		void SlotGradientMeasure(bool bAction);
		//等高线测量
		void SlotContourMeasure(bool bAction);
		void SlotFillCutMeasure(bool bAction);

	public slots:
		void SlotDeleteMeasure(bool bAction);

		/**  
		  * @brief 创建并开始测量
		  * @note 启动指定的测量功能  
		  * @param factory [in] 测量工厂对象
		  * @param pWidget [in] 测量显示窗口对象
		  * @param title [in]   窗口标题
		  * @param pToolAction [in] 对应工具栏 Action
		  * @return 无返回值
		*/
		void CreateAndBeginMeasure(FeMeasure::IMeasureFactory* factory, 
			CMeasureDisplayWidget* pWidget, QString title, QAction* pToolAction);

    private:
		QAction*											m_pSpaceDistAction;        //直线距离测量
		QAction*											m_pSurfaceDistAction;      //地表距离测量
		QAction*											m_pHeightDistAction;       //高差分析
		QAction*											m_pSpaceAreaAction;        //空间面积测量
		QAction*											m_pShadowAreaAction;	   //投影面积测量
		QAction*											m_pLineVisibleAction;      //可视分析
		QAction*											m_pRadialLineVisibleAction;//视域分析
		QAction*											m_pBuildingsVisibleAction; //建筑群视域分析
		QAction*											m_pProfileAnalysisAction;  //剖面分析
		QAction*											m_pGradientAnalysisAction; //坡度坡向分析
		QAction*											m_pFloodAnalysisAction;    //淹没分析
		QAction*											m_pFillCutAnalysisAction;  //填挖方分析
		QAction*											m_pContourAction;				//等高线测量
		QAction*											m_pDeleteAction;           //删除当前操作结果
		QAction*											m_pActiveAction;           //当前活动的Action

		osg::ref_ptr<FeMeasure::CMeasureSys>			    m_rpSystem;                //量算系统
		osg::ref_ptr<FeMeasure::IMeasureFactory>            m_rpFactory;               //测量工厂对象
		CMeasureDisplayWidget*                              m_pMeasureResultDlg;       //测量结果显示窗口
    };

}
#endif // MEASURE_PLUGIN_INTERFACE_H

