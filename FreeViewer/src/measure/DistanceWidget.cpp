#include <measure/DistanceWidget.h>

#include <FeMeasure/SpaceDistMeasure.h>
#include <FeMeasure/SpaceAreaMeasure.h>
#include <FeMeasure/ShadowAreaMeasure.h>


namespace FreeViewer
{
	CMSimpleResultDlg::CMSimpleResultDlg( QWidget* parent )
		: CMeasureDisplayWidget(parent)
	{
		MEASURE_RESULT_DLG_SETUP_UI(ui);

		//表格行背景交替显示
		ui.tableWidget_control_point->setAlternatingRowColors(true);
	}

	void CMSimpleResultDlg::SetValueVisible( bool bVisible )
	{
		if(bVisible)
		{
			ui.lineEdit_value->show();
			ui.label_value->show();
		}
		else
		{
			ui.lineEdit_value->hide();
			ui.label_value->hide();
		}
	}

	void CMSimpleResultDlg::SetPointsVisible( bool bVisible )
	{
		if(bVisible)
		{
			ui.lineEdit_cp_num->show();
			ui.label_cp_num->show();
		}
		else
		{
			ui.lineEdit_cp_num->hide();
			ui.label_cp_num->hide();
		}
	}

	void CMSimpleResultDlg::ShowMeasurePoints( const osg::Vec3dArray& points )
	{
		ui.tableWidget_control_point->setRowCount(points.size());
		ui.tableWidget_control_point->setColumnCount(4);
		//设置表格列等宽
#ifdef QT4_VERSION
		ui.tableWidget_control_point->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
		ui.tableWidget_control_point->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
		ui.tableWidget_control_point->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
		ui.tableWidget_control_point->horizontalHeader()->setResizeMode(3, QHeaderView::Stretch);
#endif

		//所有文本居中显示
		for (int i = 0; i < points.size(); i++)
		{
			QTableWidgetItem* pTableItem0 = new QTableWidgetItem(QString::number(i+1));
			pTableItem0->setTextAlignment(Qt::AlignCenter);
			ui.tableWidget_control_point->setItem(i, 0, pTableItem0);

			QTableWidgetItem* pTableItem1 = new QTableWidgetItem(QString::number(points.at(i).x(), 'g', 8));
			pTableItem1->setTextAlignment(Qt::AlignCenter);
			ui.tableWidget_control_point->setItem(i, 1, pTableItem1);

			QTableWidgetItem* pTableItem2 = new QTableWidgetItem(QString::number(points.at(i).y(), 'g', 8));
			pTableItem2->setTextAlignment(Qt::AlignCenter);
			ui.tableWidget_control_point->setItem(i, 2, pTableItem2);

			QTableWidgetItem* pTableItem3 = new QTableWidgetItem(QString::number(points.at(i).z(), 'g', 8));
			pTableItem3->setTextAlignment(Qt::AlignCenter);
			ui.tableWidget_control_point->setItem(i, 3, pTableItem3);
		}

		ui.lineEdit_cp_num->setText(QString::number(points.size()));
	}

	void CMSimpleResultDlg::HandleMeasureMsg( EMeasureAction msg, CMeasure* pMeasure )
	{
		const osg::Vec3dArray& points = pMeasure->GetCtrlPoints();

		if(msg == EM_UPDATE)
		{
			ShowMeasurePoints(points);
		}
		else if(msg == EM_END)
		{
			const CDistMeasureResultData* distData = pMeasure->GetCurrentResult()->GetResultData<CDistMeasureResultData>();
			if(distData)
			{
				ui.lineEdit_value->setText(GetFormatValueString(distData->Value()));
			}
		}
	}

	void CMSimpleResultDlg::setLabelValueUI( QString str )
	{
		ui.label_value->setText(str);
	}

}


namespace FreeViewer
{
	QString CMDistanceWidget::GetFormatValueString( double dVal )
	{
		return (dVal >= 1000.0) ? QString::number(dVal / 1000.0) + " km " : QString::number(dVal) + " m ";
	}

	void CMDistanceWidget::setLabelValueUI( QString str )
	{
		CMSimpleResultDlg::setLabelValueUI(str);
	}

	QString CMAreaWidget::GetFormatValueString( double dVal )
	{
		return (dVal >= 1000000.0) ? 
			QString::number(dVal / 1000000.0, 'g', 8) + " km^2 " : QString::number(dVal, 'g', 8) + " m^2 ";
	}

	void CMVisibleLineWidget::ShowMeasurePoints( const osg::Vec3dArray& points )
	{
		CMSimpleResultDlg::ShowMeasurePoints(points);

		//if(points.size() > 1)
		//{
		//	ui.tableWidget_control_point->setItem(0, 0, new QTableWidgetItem(tr("Eye Point")));
		//	ui.tableWidget_control_point->item(0, 0)->setTextAlignment(Qt::AlignCenter);

		//	for (int i = 1; i < points.size(); i++)
		//	{
		//		ui.tableWidget_control_point->setItem(i, 0, new QTableWidgetItem(tr("Target Point")));
		//		ui.tableWidget_control_point->item(i, 0)->setTextAlignment(Qt::AlignCenter);
		//	}
		//}
	}


	void CMRadialLineWidget::ShowMeasurePoints( const osg::Vec3dArray& points )
	{
		//CMSimpleResultDlg::ShowMeasurePoints(points);
		//ui.lineEdit_cp_num->setText(QString::number(points.size()));

		if(points.size() > 1)
		{
			ui.tableWidget_control_point->setRowCount(1);
			ui.tableWidget_control_point->setColumnCount(4);

			//所有文本居中显示
			for (int i = 0; i < points.size(); i++)
			{
				QTableWidgetItem* pTableItem0 = new QTableWidgetItem(QString::number(i+1));
				pTableItem0->setTextAlignment(Qt::AlignCenter);
				ui.tableWidget_control_point->setItem(i, 0, pTableItem0);

				QTableWidgetItem* pTableItem1 = new QTableWidgetItem(QString::number(points.at(i).x(), 'g', 8));
				pTableItem1->setTextAlignment(Qt::AlignCenter);
				ui.tableWidget_control_point->setItem(i, 1, pTableItem1);

				QTableWidgetItem* pTableItem2 = new QTableWidgetItem(QString::number(points.at(i).y(), 'g', 8));
				pTableItem2->setTextAlignment(Qt::AlignCenter);
				ui.tableWidget_control_point->setItem(i, 2, pTableItem2);

				QTableWidgetItem* pTableItem3 = new QTableWidgetItem(QString::number(points.at(i).z(), 'g', 8));
				pTableItem3->setTextAlignment(Qt::AlignCenter);
				ui.tableWidget_control_point->setItem(i, 3, pTableItem3);
			}

			//ui.tableWidget_control_point->setItem(0, 0, new QTableWidgetItem("1"));
		}
	}
}