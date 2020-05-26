#include <mark/MarkEventHandler.h>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeKits/mapScale/MapScale.h>

#include <mark/MarkPluginInterface.h>
#include <mark/MarkVisitProcessor.h>

namespace FreeViewer
{
	CMarkEventHandler::CMarkEventHandler( CMarkPluginInterface* pMarkWidget ) 
		: m_pMarkWidget(pMarkWidget)
	{

	}

	void CMarkEventHandler::SetActiveMarkNode( FeExtNode::CExternNode* pMarkNode )
	{
		m_opCurMarkNode = pMarkNode; 
	}

	void CMarkEventHandler::StartCapture()
	{ 
		m_bStartCapture = true;
	}

	void CMarkEventHandler::StopCapture()
	{
		m_bStartCapture = false; 
	}

	void CMarkEventHandler::CalcMarkInfo(float x, float y)
	{
		if(m_pMarkWidget)
		{
			/// 计算位置
			osg::Vec3d vecWorld, vecLLH(0,0,0);

			osg::observer_ptr<osgEarth::MapNode> opMapNode = m_pMarkWidget->GetSystemService()->GetRenderContext()->GetMapNode();

			if (opMapNode.valid())
			{
				if (opMapNode->getTerrain()->getWorldCoordsUnderMouse(
					m_pMarkWidget->GetSystemService()->GetRenderContext()->GetView(), x, y, vecWorld))
				{
					FeUtil::XYZ2DegreeLLH(m_pMarkWidget->GetSystemService()->GetRenderContext(), vecWorld, vecLLH);
					m_geoPoint.set(opMapNode->getMapSRS(), vecLLH, osgEarth::ALTMODE_ABSOLUTE);
					if (m_geoPoint.z() < 0) m_geoPoint.z() = 0;
				}
			}

			///计算标绘初始的大小，使用视点距离/10.0
			m_dMarkSize = m_pMarkWidget->GetSystemService()->GetManipulatorManager()->GetViewPoint().focalPoint()->z()/10.0;
		}
	}

	bool CMarkEventHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		if(!m_pMarkWidget || !m_bStartCapture || !m_opCurMarkNode.valid()) return false;

		if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
		{
			m_fMouseDownX = ea.getX();
			m_fMouseDownY = ea.getY();
		}
		else if(ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
		{
			if(abs(ea.getX() - m_fMouseDownX) > 3 || abs(ea.getY() - m_fMouseDownY) > 3 )
			{
				return false;
			}

			if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			{
				/// 计算标记信息
				CalcMarkInfo(ea.getX(), ea.getY());

				/// 设置位置
				CMarkVisitProcessor::CProcessData<osgEarth::GeoPoint> pointData(m_geoPoint);
				m_pMarkWidget->GetMarkProcessor().DoProcess(m_opCurMarkNode.get(), CMarkVisitProcessor::E_MARK_SET_POSITION, pointData);

				/// 设置大小
				CMarkVisitProcessor::CProcessData<double> sizeData(m_dMarkSize);
				m_pMarkWidget->GetMarkProcessor().DoProcess(m_opCurMarkNode.get(), CMarkVisitProcessor::E_MARK_SET_SIZE, sizeData);

				/// 设置视点
				FeUtil::CFreeViewPoint pSysViewPoint = m_pMarkWidget->GetSystemService()->GetViewPoint();
				m_opCurMarkNode.get()->SetViewPoint(FeUtil::CFreeViewPoint(m_geoPoint.x(), m_geoPoint.y(), pSysViewPoint.GetHei(),
					pSysViewPoint.GetHeading(), pSysViewPoint.GetPitch(), pSysViewPoint.GetRange(),pSysViewPoint.GetTime()));

				if(!m_pMarkWidget->IsDrawingOrEditing())
				{
					/// 通知标记管理类开始绘制
					m_pMarkWidget->StartDrawMark();
				}
			}
			else if(ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
			{
				/// 通知标记管理类结束绘制
				CMarkVisitProcessor MarkVisitProcessor = m_pMarkWidget->GetMarkProcessor();
				MarkVisitProcessor.DoProcess(m_opCurMarkNode.get(), CMarkVisitProcessor::E_MARK_STOP_DRAW);
				if (MarkVisitProcessor.GetFlag())
				{
					m_pMarkWidget->SuccessToDraw();
				}
				else
				{
					m_pMarkWidget->FailedToDraw();
				}
			}
		}
// 		else if(ea.getEventType() == osgGA::GUIEventAdapter::MOVE)
// 		{
// 			if(m_pMarkWidget->IsDrawingOrEditing())
// 			{
// 				if(FeExtNode::CExStraightArrowNode* en =  dynamic_cast<FeExtNode::CExStraightArrowNode *>(m_opCurMarkNode.get()))
// 				{
// 					CalcMarkInfo(ea.getX(), ea.getY());
// 
// 					/// 更新位置
// 					CMarkVisitProcessor::CProcessData<osgEarth::GeoPoint> pointData(m_geoPoint);
// 					m_pMarkWidget->GetMarkProcessor().DoProcess(m_opCurMarkNode.get(), CMarkVisitProcessor::E_MARK_UPDATE_POSITION, pointData);
// 				}
// 			}
// 		}

		return false;
	}

}





