#include <plot/PlotEventHandler.h>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeKits/mapScale/MapScale.h>
#include <FePlots/PolygonPlot.h>
#include <FePlots/LinePlot.h>

#include <plot/PlotPluginInterface.h>
#include <plot/PlotVisitProcessor.h>

namespace FreeViewer
{
	CPlotEventHandler::CPlotEventHandler( CPlotPluginInterface* pPlotWidget ) 
		: m_pPlotWidget(pPlotWidget)
	{

	}

	void CPlotEventHandler::SetActivePlotNode( FeExtNode::CExternNode* pPlotNode )
	{
		m_opCurPlotNode = pPlotNode; 
	}

	void CPlotEventHandler::StartCapture()
	{ 
		m_bStartCapture = true;
	}

	void CPlotEventHandler::StopCapture()
	{
		m_bStartCapture = false; 
	}

	void CPlotEventHandler::CalcPlotInfo(float x, float y)
	{
		if(m_pPlotWidget)
		{
			/// 计算位置
			osg::Vec3d vecWorld, vecLLH(0,0,0);

			osg::observer_ptr<osgEarth::MapNode> opMapNode = m_pPlotWidget->GetSystemService()->GetRenderContext()->GetMapNode();

			if (opMapNode.valid())
			{
				if (opMapNode->getTerrain()->getWorldCoordsUnderMouse(
					m_pPlotWidget->GetSystemService()->GetRenderContext()->GetView(), x, y, vecWorld))
				{
					FeUtil::XYZ2DegreeLLH(m_pPlotWidget->GetSystemService()->GetRenderContext(), vecWorld, vecLLH);
					m_geoPoint.set(opMapNode->getMapSRS(), vecLLH, osgEarth::ALTMODE_ABSOLUTE);
					if (m_geoPoint.z() < 0) m_geoPoint.z() = 0;
				}
			}

			///计算标绘初始的大小，使用视点距离/10.0
			m_dPlotSize = m_pPlotWidget->GetSystemService()->GetManipulatorManager()->GetViewPoint().focalPoint()->z()/10.0;
		}
	}

	bool CPlotEventHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		if(!m_pPlotWidget || !m_bStartCapture || !m_opCurPlotNode.valid()) return false;

		if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
		{
			m_fMouseDownX = ea.getX();
			m_fMouseDownY = ea.getY();
		}
		else if(ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
		{
			if( !(ea.getX() == m_fMouseDownX && ea.getY() == m_fMouseDownY) )
			{
				return false;
			}

			if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			{
				/// 计算军标信息
				CalcPlotInfo(ea.getX(), ea.getY());

				/// 设置位置
				CPlotVisitProcessor::CProcessData<osgEarth::GeoPoint> pointData(m_geoPoint);
				m_pPlotWidget->GetPlotProcessor().DoProcess(m_opCurPlotNode.get(), CPlotVisitProcessor::E_PLOT_SET_POSITION, pointData);

				/// 设置大小
				CPlotVisitProcessor::CProcessData<double> sizeData(m_dPlotSize);
				m_pPlotWidget->GetPlotProcessor().DoProcess(m_opCurPlotNode.get(), CPlotVisitProcessor::E_PLOT_SET_SIZE, sizeData);

				/// 设置视点
				FeUtil::CFreeViewPoint pSysViewPoint = m_pPlotWidget->GetSystemService()->GetViewPoint();
				m_opCurPlotNode.get()->SetViewPoint(FeUtil::CFreeViewPoint(m_geoPoint.x(), m_geoPoint.y(), pSysViewPoint.GetHei(),
					pSysViewPoint.GetHeading(), pSysViewPoint.GetPitch(), pSysViewPoint.GetRange(),pSysViewPoint.GetTime()));

				if(!m_pPlotWidget->IsDrawingOrEditing())
				{
					/// 通知军标管理类开始绘制
					m_pPlotWidget->StartDrawPlot();
				}

				m_nMouseFlag = E_MouseLeft;
			}
			else if(ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
			{
				/// 通知军标管理类结束绘制
				CPlotVisitProcessor PlotVisitProcessor = m_pPlotWidget->GetPlotProcessor();

				if (m_nMouseFlag == E_MouseMove)
				{
					PlotVisitProcessor.DoProcess(m_opCurPlotNode.get(), CPlotVisitProcessor::E_PLOT_RIGHT_STOP_DRAW);
				}
				else if (m_nMouseFlag == E_MouseLeft)
				{
					PlotVisitProcessor.DoProcess(m_opCurPlotNode.get(), CPlotVisitProcessor::E_PLOT_LEFT_STOP_DRAW);
				}
				
				if ( PlotVisitProcessor.GetFlag())
				{
					m_pPlotWidget->SuccessToDraw();
				}
				else
				{
					m_pPlotWidget->FailedToDraw();
				}

				m_nMouseFlag = E_MouseRight;
			}
		}
		else if(ea.getEventType() == osgGA::GUIEventAdapter::MOVE)
		{
			if(m_pPlotWidget->IsDrawingOrEditing())
			{
				if(FePlots::CPolygonPlot* en =  dynamic_cast<FePlots::CPolygonPlot *>(m_opCurPlotNode.get()))
				{
					CalcPlotInfo(ea.getX(), ea.getY());

					/// 更新位置
					CPlotVisitProcessor::CProcessData<osgEarth::GeoPoint> pointData(m_geoPoint);
					m_pPlotWidget->GetPlotProcessor().DoProcess(m_opCurPlotNode.get(), CPlotVisitProcessor::E_PLOT_UPDATE_POSITION, pointData);
				}
				else if (FePlots::CLinePlot* en =  dynamic_cast<FePlots::CLinePlot *>(m_opCurPlotNode.get()))
				{
					CalcPlotInfo(ea.getX(), ea.getY());

					/// 更新位置
					CPlotVisitProcessor::CProcessData<osgEarth::GeoPoint> pointData(m_geoPoint);
					m_pPlotWidget->GetPlotProcessor().DoProcess(m_opCurPlotNode.get(), CPlotVisitProcessor::E_PLOT_UPDATE_POSITION, pointData);
				}
			}

			m_nMouseFlag = E_MouseMove;
		}
  		else if (ea.getEventType() == osgGA::GUIEventAdapter::DOUBLECLICK)
  		{
  			if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
  			{
  				/// 通知军标管理类结束绘制
				CPlotVisitProcessor PlotVisitProcessor = m_pPlotWidget->GetPlotProcessor();
				PlotVisitProcessor.DoProcess(m_opCurPlotNode.get(), CPlotVisitProcessor::E_PLOT_DOUBLE_STOP_DRAW);
				if (PlotVisitProcessor.GetFlag())
				{
					m_pPlotWidget->SuccessToDraw();
				}
				else
				{
					m_pPlotWidget->FailedToDraw();
				}
  			}
  		}

		return false;
	}

}





