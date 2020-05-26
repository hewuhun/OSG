#include <measure/ProfileAnalysisWidget.h>

#include <FeMeasure/ProfileAnalysisMeasure.h>

#include <QGridLayout>
#include <QTableWidgetItem>
#include <QSizePolicy>
#include <qwt_plot_grid.h>
#include <qwt_plot_zoomer.h>

namespace FreeViewer
{
	CProfileAnalysisGraph::CProfileAnalysisGraph()
		:QWidget()
	{
		QGridLayout* pLayout = new QGridLayout();
		this->setLayout(pLayout);
		QWidget* pMiddleWidget = new QWidget;
		pLayout->addWidget(pMiddleWidget);

		//初始化画布
		m_pPlot = new QwtPlot(pMiddleWidget);
		m_pPlot->setAxisTitle(0, QwtText(tr("Height(m)"), QwtText::RichText));
		m_pPlot->setAxisTitle(2, QwtText(tr("Distance(Km)"), QwtText::RichText));
		QFont axisFont("Times", 8, QFont::Normal);
		m_pPlot->setAxisFont(0, axisFont);
		m_pPlot->setAxisFont(2, axisFont);

		//添加网格背景
		QwtPlotGrid *grid = new QwtPlotGrid;  
		grid->setMajorPen(QPen(QColor(44,80,114, 100), 0, Qt::SolidLine));  
		grid->attach(m_pPlot);  

		//初始化曲线
        m_plotCurve.setPen( Qt::green );
		m_plotCurve.setStyle( QwtPlotCurve::Lines );
		m_plotCurve.attach(m_pPlot);

		//曲线抗锯齿
		m_plotCurve.setRenderHint(QwtPlotItem::RenderAntialiased,true);
	}

	CProfileAnalysisGraph::~CProfileAnalysisGraph()
	{

	}

	void CProfileAnalysisGraph::paintEvent( QPaintEvent *event )
	{
		QRect r = contentsRect();
		m_pPlot->resize(r.width()-20, r.height()-20);
	}

	void CProfileAnalysisGraph::Update( double* pXValue, double* pYValue, int nCount )
	{
		m_vecX.resize(nCount);
		m_vecY.resize(nCount);
		for(int n =0; n < nCount; n++)
		{
			m_vecX.at(n) = pXValue[n];
			m_vecY.at(n) = pYValue[n];
		}

		m_plotCurve.setRawSamples( m_vecX.data(), m_vecY.data(), nCount );

		m_pPlot->replot();
	}
}

namespace FreeViewer
{
	CMProfileAnalysisWidget::CMProfileAnalysisWidget( QWidget* parent)
		: CMeasureDisplayWidget(parent)
		,m_opProfileAnalysisMeasure(NULL)
	{
		MEASURE_RESULT_DLG_SETUP_UI(ui);

		//表格行背景交替显示
		ui.tableWidget_SamplePoint->setAlternatingRowColors(true);
		//设置表格列等宽
#ifdef QT4_VERSION
		ui.tableWidget_SamplePoint->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
		ui.tableWidget_SamplePoint->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
		ui.tableWidget_SamplePoint->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
		ui.tableWidget_SamplePoint->horizontalHeader()->setResizeMode(3, QHeaderView::Stretch);
#endif

		//曲线绘制
		m_pProfileGraph = new CProfileAnalysisGraph();
		m_pProfileGraph->setStyleSheet("background: transparent; border: none;");
		ui.gridLayout_Graph->addWidget(m_pProfileGraph);
		ui.tableWidget_SamplePoint->setMinimumSize(200, 200);
		m_pProfileGraph->setMinimumSize(200, 200);
	
		connect(ui.pushButton_Analyze, SIGNAL(pressed()), this, SLOT(SlotBeginAnalyze()));
	}

	CMProfileAnalysisWidget::~CMProfileAnalysisWidget()
	{
		if(m_pProfileGraph)
		{
			delete m_pProfileGraph;
		}
	}
	
	void CMProfileAnalysisWidget::HandleMeasureMsg( EMeasureAction msg, CMeasure* pMeasure )
	{
		if(m_opProfileAnalysisMeasure.get() != pMeasure)
		{
			m_opProfileAnalysisMeasure = dynamic_cast<CProfileAnalysisMeasure*>(pMeasure);
		}

		if(EM_END == msg || EM_RESULT_CHANGED == msg)
		{
			UpdateDisplay();
		}
	}

	void CMProfileAnalysisWidget::UpdateDisplay()
	{
		if(!m_opProfileAnalysisMeasure.valid()) return;

		const CProfileAnalysisResultData* resultData = m_opProfileAnalysisMeasure.get()->GetCurrentResult(
			)->GetResultData<CProfileAnalysisResultData>();

		if(!resultData) return;
		
		const osg::Vec3dArray* pArray = resultData->SamplePoints();
		if(!pArray) return;


		//设置起始点以及终止点的位置信息
		if (!pArray->empty())
		{
			osg::Vec3d beginPoint = pArray->at(0);
			osg::Vec3d endPoint = pArray->at(pArray->size() - 1);

			ui.label_beginLon->setText(QString::number(beginPoint.x(), 'g', 6));
			ui.label_beginLat->setText(QString::number(beginPoint.y(), 'g', 6));
			ui.label_beginHei->setText(QString::number(beginPoint.z(), 'g', 6));

			ui.label_endLon->setText(QString::number(endPoint.x(), 'g', 6));
			ui.label_endLat->setText(QString::number(endPoint.y(), 'g', 6));
			ui.label_endHei->setText(QString::number(endPoint.z(), 'g', 6));
		}

		//设置最高以及最低海拔处的位置信息
		ui.label_maxLon->setText(QString::number(resultData->HighestPoint().x(), 'g', 6));
		ui.label_maxLat->setText(QString::number(resultData->HighestPoint().y(), 'g', 6));
		ui.label_maxHei->setText(QString::number(resultData->HighestPoint().z(), 'g', 6));
		ui.label_minLon->setText(QString::number(resultData->LowestPoint().x(), 'g', 6));
		ui.label_minLat->setText(QString::number(resultData->LowestPoint().y(), 'g', 6));
		ui.label_minHei->setText(QString::number(resultData->LowestPoint().z(), 'g', 6));

		//设置采样间距以及采样点数
		ui.label_IntervalDis->setText(QString::number(resultData->SampleDistance() / 1000.0));
		ui.spinBox_IntervalNum->setValue(resultData->SampleNum());

		//将所有的点添加到列表中
		ui.tableWidget_SamplePoint->setRowCount(pArray->size());
		ui.tableWidget_SamplePoint->setColumnCount(4);

		for (int i = 0; i < pArray->size(); i++)
		{
			ui.tableWidget_SamplePoint->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
			ui.tableWidget_SamplePoint->setItem(i, 1, new QTableWidgetItem(QString::number(pArray->at(i).x(), 'g', 8)));
			ui.tableWidget_SamplePoint->setItem(i, 2, new QTableWidgetItem(QString::number(pArray->at(i).y(), 'g', 8)));
			ui.tableWidget_SamplePoint->setItem(i, 3, new QTableWidgetItem(QString::number(pArray->at(i).z(), 'g', 8)));
		}

		//所有文本居中显示
		for (int i = 0; i < pArray->size(); i++)
		{
			ui.tableWidget_SamplePoint->item(i, 0)->setTextAlignment(Qt::AlignCenter);
			ui.tableWidget_SamplePoint->item(i, 1)->setTextAlignment(Qt::AlignCenter);
			ui.tableWidget_SamplePoint->item(i, 2)->setTextAlignment(Qt::AlignCenter);
			ui.tableWidget_SamplePoint->item(i, 3)->setTextAlignment(Qt::AlignCenter);
		}

		//绘制二维平面的图形
		if (pArray->size())
		{
			double* pXValue = new double[pArray->size()];
			double* pYValue = new double[pArray->size()];

			for (int i = 0; i < pArray->size(); i++)
			{
				pYValue[i] = pArray->at(i).z();
				pXValue[i] = resultData->SurfaceDistance() / (pArray->size()*1000.0)*i;
			}

			m_pProfileGraph->Update(pXValue, pYValue, pArray->size());
			delete pXValue;
			delete pYValue;
		}
	}

	void CMProfileAnalysisWidget::SlotBeginAnalyze()
	{
		if(m_opProfileAnalysisMeasure.valid())
		{
			const CProfileAnalysisResultData* resultData = m_opProfileAnalysisMeasure.get()->GetCurrentResult(
				)->GetResultData<CProfileAnalysisResultData>();
			
			if (resultData && ui.spinBox_IntervalNum->value() != resultData->SampleNum())
			{
				m_opProfileAnalysisMeasure.get()->SetSamplePoint(ui.spinBox_IntervalNum->value());
			}
		}
	}

}
