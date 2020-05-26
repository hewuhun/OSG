/**************************************************************************************************
* @file ProfileAnalysisWidget.h
* @note 剖面分析结果显示窗口
* @author c00005
* @data 2015-8-25
**************************************************************************************************/
#ifndef PROFILE_ANALYSIS_WIDGET_H
#define PROFILE_ANALYSIS_WIDGET_H 1

#include <qwt_plot_curve.h>
#include <qwt_plot.h>

#include <QColor>
#include <QPainter>

#include <ui_ProfileAnalysisWidget.h>

#include <FeMeasure/ProfileAnalysisMeasure.h>

#include <measure/MeasureDisplayWidget.h>

namespace FreeViewer
{
    using namespace FeMeasure;

    /**
      * @class CProfileAnalysisGraph
      * @note 剖面分析曲线绘制类
      * @author c00005
    */
    class CProfileAnalysisGraph:
        public QWidget
    {
    public:
        CProfileAnalysisGraph();
        ~CProfileAnalysisGraph();

    public:
        void Update(double* pXValue, double* pYValue, int nCount);

    protected:
        virtual void paintEvent( QPaintEvent * event);

    private:
        QwtPlotCurve	m_plotCurve;
        QwtPlot*		m_pPlot;
		std::vector<double> m_vecX;
		std::vector<double> m_vecY;
    };

    /**
      * @class CMProfileAnalysisWidget
      * @brief 剖面分析结果显示窗口
      * @note 剖面分析结果显示窗口
      * @author c00005
    */
    class CMProfileAnalysisWidget : public CMeasureDisplayWidget
    {
        Q_OBJECT

    public:
        CMProfileAnalysisWidget( QWidget* parent);
        ~CMProfileAnalysisWidget();

   private:
		/**  
		  * @note 派生类实现该接口类处理测量消息，输入参数有效
		*/
		virtual void HandleMeasureMsg(EMeasureAction msg, CMeasure* pMeasure);

		/**  
		  * @note 更新显示
		*/
		void UpdateDisplay();

    protected slots:
        void SlotBeginAnalyze();

    protected:
        Ui::ProfileAnalysisWidget				ui;

        CProfileAnalysisGraph*					m_pProfileGraph;

        osg::observer_ptr<FeMeasure::CProfileAnalysisMeasure> m_opProfileAnalysisMeasure;
    };
}
#endif //PROFILE_ANALYSIS_WIDGET_H
