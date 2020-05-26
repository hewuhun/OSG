#include <mark/MarkVisitProcessor.h>

#include <FeUtils/PathRegistry.h>
#include <FeExtNode/ExLineNode.h>
#include <FeExtNode/ExternNode.h>
#include <FeExtNode/ExLodModelNode.h>
#include <FeExtNode/ExPlaceNode.h>
#include <FeExtNode/ExPolygonNode.h>
#include <FeExtNode/ExSectorNode.h>
#include <FeExtNode/ExRectNode.h>
#include <FeExtNode/ExEllipseNode.h>
#include <FeExtNode/ExCircleNode.h>
#include <FeExtNode/ExTextNode.h>
#include <FeExtNode/ExTiltModelNode.h>
#include <FeExtNode/ExLabelNode.h>
#include <FeExtNode/ExBillBoardNode.h>
#include <FeExtNode/ExOverLayNode.h>
//#include <FeExtNode/ExAssaultNode.h>
//#include <FeExtNode/ExAttackNode.h>
#include <FeExtNode/ExParticleNode.h>

#include <mark/MarkPluginInterface.h>

#include <mainWindow/FreeUtil.h>

#ifdef __QT4_USE_UNICODE
#else
#pragma execution_character_set("UTF-8")
#endif

namespace FreeViewer
{
	CMarkVisitProcessor::CMarkVisitProcessor() 
		: m_eProcessStyle(E_MARK_DO_NOTHING)
		, m_bMarkFlag(true)
	{
	}

	void CMarkVisitProcessor::CommonTypeHandle( FeExtNode::CExPointNode& externNode )
	{
		switch (m_eProcessStyle)
		{
		case E_MARK_SET_POSITION:
			{
				osgEarth::GeoPoint* pGeoPoint = GetMarkData<osgEarth::GeoPoint>();
				if(pGeoPoint) externNode.SetPosition(*pGeoPoint);
			}break;
		}
	}

	void CMarkVisitProcessor::CommonTypeHandle(FeExtNode::CExLocalizedNode& externNode)
	{
		switch (m_eProcessStyle)
		{
		case E_MARK_SET_POSITION:
			{
				osgEarth::GeoPoint* pGeoPoint = GetMarkData<osgEarth::GeoPoint>();
				if(pGeoPoint) externNode.SetPosition(*pGeoPoint);
 			}break;

		case E_MARK_INITIALIZE:
			{
				externNode.SetFillColor(osg::Vec4d(1.0,1.0,0.0,0.5));
				externNode.SetLineColor(osg::Vec4d(0.0,1.0,0.0,1.0));
				externNode.SetLineWidth(1.0);
			}break;
		}
	}

	void CMarkVisitProcessor::CommonTypeHandle(FeExtNode::CExFeatureNode& externNode)
	{
		switch (m_eProcessStyle)
		{
		case E_MARK_SET_POSITION:
			{
				osgEarth::GeoPoint* pGeoPoint = GetMarkData<osgEarth::GeoPoint>();
				if(pGeoPoint) externNode.PushBackVertex(pGeoPoint->vec3d());
			}break;

		case E_MARK_INITIALIZE:
			{
				externNode.SetFillColor(osg::Vec4d(1.0,1.0,0.0,0.5));
				externNode.SetLineColor(osg::Vec4d(0.0,1.0,0.0,1.0));
				externNode.SetLineWidth(1.0);
			}break;
		}
	}

	/*
	void CMarkVisitProcessor::CommonTypeHandle(FeExtNode::CExStraightArrowNode& externNode)
	{
		CommonTypeHandle((FeExtNode::CExFeatureNode&)externNode);

		if(E_MARK_UPDATE_POSITION == m_eProcessStyle)
		{
			osgEarth::GeoPoint* pGeoPoint = GetMarkData<osgEarth::GeoPoint>();
			if(pGeoPoint) externNode.ReplaceBackVertex(pGeoPoint->vec3d());
		}
	}

	bool CMarkVisitProcessor::VisitEnter(FeExtNode::CExAssaultNode& externNode)
	{
		CommonTypeHandle(externNode);

		if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/assault.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("突击方向")));
		}
		else if(E_MARK_STOP_DRAW == m_eProcessStyle)
		{
			// 解决右键直接结束后，缺少最后一个控制点的问题
			externNode.PushBackVertex(externNode.Back());
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter(FeExtNode::CExAttackNode& externNode)
	{
		CommonTypeHandle(externNode);

		if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/attack.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("进攻方向")));
		}
		else if(E_MARK_STOP_DRAW == m_eProcessStyle)
		{
			// 解决右键直接结束后，缺少最后一个控制点的问题
			externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter(FeExtNode::CExStraightArrowNode& externNode)
	{
		CommonTypeHandle(externNode);

		if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/direct_arrow.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("直箭头")));
		}
		else if(E_MARK_STOP_DRAW == m_eProcessStyle)
		{
			// 解决右键直接结束后，缺少最后一个控制点的问题
			externNode.PushBackVertex(externNode.Back());
		}

		return true;
	}
	*/

	bool CMarkVisitProcessor::VisitExit( FeExtNode::CExComposeNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/add_folder.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("文件夹")));
		}
		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExPlaceNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetImagePath(FeFileReg->GetFullPath("texture/mark/point_32.png"));
			externNode.SetName(ConvertToSDKEncoding(QString("标记点")));
		}
		else if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/point.png");
		}
		
		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExLodModelNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_SET_POSITION == m_eProcessStyle)
		{
			osgEarth::GeoPoint* pGeoPoint = GetMarkData<osgEarth::GeoPoint>();
			if(pGeoPoint) externNode.SetPositionByLLH(pGeoPoint->vec3d());
		}
		else if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/model.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetLodModelNode(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE, FeFileReg->GetFullPath("model/Tank.ive"));
			externNode.SetName(ConvertToSDKEncoding(QString("模型")));
		}
		else if (E_MARK_SET_SIZE == m_eProcessStyle)
		{
			double* dSize = GetMarkData<double>();
			if(dSize) externNode.SetScale(osg::Vec3d(*dSize/10,*dSize/10,*dSize/10));
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExLineNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/line.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("线标记")));
		}
		else if (E_MARK_STOP_DRAW == m_eProcessStyle)
		{
			m_bMarkFlag = externNode.CheckedVertex();
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExPolygonNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/polygon.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("多边形")));
		}
		else if (E_MARK_STOP_DRAW == m_eProcessStyle)
		{
			m_bMarkFlag = externNode.CheckedVertex();
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExBillBoardNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/billboard.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetImagePath(FeFileReg->GetFullPath("texture/mark/mark_default.png"));
			externNode.SetName(ConvertToSDKEncoding(QString("广告牌")));
		}
		else if (E_MARK_SET_SIZE == m_eProcessStyle)
		{
			double* dSize = GetMarkData<double>();
			if(dSize) externNode.SetImageSize(osg::Vec2(*dSize/2,*dSize/2));
		}
		
		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExOverLayNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/cover.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetImagePath(FeFileReg->GetFullPath("texture/mark/mark_default.png"));
			externNode.SetName(ConvertToSDKEncoding(QString("地面遮盖")));
		}
		else if (E_MARK_SET_SIZE == m_eProcessStyle)
		{
			double* dSize = GetMarkData<double>();
			if(dSize) externNode.SetImageSize(osg::Vec2(*dSize/2,*dSize/2));
		}
		
		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExArcNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_SET_SIZE == m_eProcessStyle)
		{
			double* dSize = GetMarkData<double>();
			if(dSize) externNode.SetRadius(*dSize);
		}
		else if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/arc.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("弧形标记")));
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExEllipseNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_SET_SIZE == m_eProcessStyle)
		{
			double* dSize = GetMarkData<double>();
			if(dSize)
			{
				externNode.SetLongRadius(*dSize/2);
				externNode.SetShortRadius(*dSize/4);
			}
		}
		else if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/ellipse.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("椭圆标记")));
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExRectNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_SET_SIZE == m_eProcessStyle)
		{
			double* dSize = GetMarkData<double>();
			if(dSize)
			{
				externNode.SetHeight(*dSize);
				externNode.SetWidth(*dSize);
			}
		}
		else if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/rectangle.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("矩形标记")));
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExSectorNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_SET_SIZE == m_eProcessStyle)
		{
			double* dSize = GetMarkData<double>();
			if(dSize)
			{
				externNode.SetRadius(*dSize/2);
			}
		}
		else if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/sector.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("扇形标记")));
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExCircleNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_SET_SIZE == m_eProcessStyle)
		{
			double* dSize = GetMarkData<double>();
			if(dSize)
			{
				externNode.SetRadius(*dSize/2);
			}
		}
		else if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/circle.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("圆形标记")));
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExTiltModelNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/oblique_model.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("倾斜摄影模型")));
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExLabelNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_SET_POSITION == m_eProcessStyle)
		{
			osgEarth::GeoPoint* pGeoPoint = GetMarkData<osgEarth::GeoPoint>();
			if(pGeoPoint) externNode.SetBindingPointLLH(pGeoPoint->vec3d());
		}
		else if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/label.png");
		}
		else if(E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetWidth(130);
			externNode.SetHeight(30);
			externNode.SetFontSize(18);
			externNode.SetTextColor(osg::Vec4d(1.0, 1.0, 0, 1));
			externNode.SetBackgroundColor(osg::Vec4d(1.0, 1.0, 1.0, 1));
			externNode.SetBackgroundImage(FeFileReg->GetFullPath("texture/mark/label_title.png"));
			externNode.SetName(ConvertToSDKEncoding(QString(QObject::tr("标牌"))));
			externNode.SetText(ConvertToSDKEncoding(QString(QObject::tr("LabelTitle"))));
			externNode.SetClampToTerrain(true);

			FeUtil::FEID itemID = externNode.AddLabelItem(ConvertToSDKEncoding(QString(QObject::tr("LabelItem"))));
			if(externNode.GetLabelItem(itemID))
			{
				externNode.GetLabelItem(itemID)->SetBackgroundImage(FeFileReg->GetFullPath("texture/mark/label_item.png"));
			}
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExTextNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("文本标记")));
		}
		else if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/text.png");
		}

		return true;
	}

	bool CMarkVisitProcessor::VisitEnter( FeExtNode::CExParticleNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_MARK_SET_POSITION == m_eProcessStyle)
		{
			osgEarth::GeoPoint* pGeoPoint = GetMarkData<osgEarth::GeoPoint>();
			if(pGeoPoint) externNode.SetParticlePosition(pGeoPoint->vec3d());
		}
		else if (E_MARK_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetMarkData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/particle.png");
		}
		else if (E_MARK_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("粒子效果")));
			externNode.SetParticlePath(FeFileReg->GetFullPath("particle/particle/x6/x6.xml"));
		}
		else if (E_MARK_SET_SIZE == m_eProcessStyle)
		{
			double* dSize = GetMarkData<double>();
			if(dSize) externNode.SetParticleScale((*dSize)/10);
		}

		return true;
	}

	void CMarkVisitProcessor::DoProcess( FeExtNode::CExternNode* pMarkNode , E_Mark_Process_Type eProcessType)
	{
		if(pMarkNode)
		{
			m_eProcessStyle = eProcessType;
			pMarkNode->Accept(*this);
		}
	}

	bool CMarkVisitProcessor::GetFlag()
	{
		return m_bMarkFlag;
	}

}


