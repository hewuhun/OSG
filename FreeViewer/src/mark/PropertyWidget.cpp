#include <mark/PropertyWidget.h>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>

#include <FeExtNode/ExLabelNode.h>

#include <mainWindow/UIObserver.h>
#include <mainWindow/FreeUtil.h>
#include <mark/BaseStyleWidget.h>

#include <osg/Vec3d>

#ifdef __QT4_USE_UNICODE
#else
#pragma execution_character_set("UTF-8")
#endif

namespace FreeViewer
{
	CPropertyWidget::CPropertyWidget(FeExtNode::CExternNode* pMark, QWidget* pParent, bool bCreate)
		: CFreeDialog(pParent)
		,m_opMark(pMark)
		,m_bCreated(bCreate)
	{
		// 关闭时释放内存
		this->setAttribute(Qt::WA_DeleteOnClose);

		QWidget* widget = new QWidget(this);     
		ui.setupUi(widget);                      
		AddWidgetToDialogLayout(widget);         
		widget->setMinimumSize(widget->size());  

		ShowDialogNormal();

		InitWidget();
	}

	CPropertyWidget::~CPropertyWidget()
	{
		m_vecTab.clear();
	}

	void CPropertyWidget::InitWidget()
	{
		if(m_opMark.valid())
		{			
			m_vecTab.clear();
			BuildContext();
		}
	}

	void CPropertyWidget::BuildContext()
	{
		QValidator* pIDValidator = new QIntValidator(this);
		ui.id_lineEdit->setValidator(pIDValidator);
		std::string strTest = UnintToString(m_opMark->GetID());
		ui.id_lineEdit->setText(strTest.c_str());
		m_strName = m_opMark->GetName();
		ui.title_lineEdit->setText(ConvertToCurrentEncoding(m_strName));
		ui.title_lineEdit->setMaxLength(25); // 限定标题输入最大长度

		connect(ui.title_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(SlotTitleChanged(QString)));
		connect(ui.btn_ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(ui.btn_cancel, SIGNAL(clicked()), this, SLOT(reject()));

		//关联关闭按钮信号和槽
		connect(m_pCloseBtn, SIGNAL(clicked()), this, SLOT(reject()));
	}

	std::string CPropertyWidget::UnintToString(unsigned int unInt)
	{
		return FeUtil::IntToString(unInt);
	}

	void CPropertyWidget::SetTitleFlag(EMarkStyle nFlag)
	{
		switch(nFlag)
		{
			//标记
		case E_PLACE_NODE:
			SetTitleText(tr("PlaceNode"));
			break;
		case E_COMPOSE_NODE:
			SetTitleText(tr("ComposeNode"));
			break;
		case E_MODEL_NODE:
			SetTitleText(tr("ModelNode"));
			break;
		case E_LINE_NODE:
			SetTitleText(tr("LineNode"));
			break;
		case E_POLYGON_NODE:
			SetTitleText(tr("PolygonNode"));
			break;
		//case E_STRING_ARROW_NODE:
		//	SetTitleText(tr("StringArrowNode"));
		//	break;
		//case E_ASSAULT_NODE:
		//	SetTitleText(tr("AssaultNode"));
		//	break;
		//case E_ATTACK_NODE:
		//	SetTitleText(tr("AttackNode"));
		//	break;
		case E_BILL_BOARD_NODE:
			SetTitleText(tr("BillBoardNode"));
			break;
		case E_OVERLAY_NODE:
			SetTitleText(tr("OverlayNode"));
			break;
		case E_ARC_NODE:
			SetTitleText(tr("ArcNode"));
			break;
		case E_ELLIPSE_NODE:
			SetTitleText(tr("EllipseNode"));
			break;
		case E_RECT_NODE:
			SetTitleText(tr("RectNode"));
			break;
		case E_SECTOR_NODE:
			SetTitleText(tr("SectorNode"));
			break;
		case E_CIRCLE_NODE:
			SetTitleText(tr("CircleNode"));
			break;
		case E_TEXT_NODE:
			SetTitleText(tr("TextNode"));
			break;
		case E_LABEL_NODE:
			SetTitleText(tr("LabelNode"));
			break;
		case E_TILT_MODEL_NODE:
			SetTitleText(tr("TiltModelNode"));
			break;
		case E_PARTICLE_NODE:
			SetTitleText(tr("ParticleNode"));
			break;

			//军标
		case E_STRAIGHT_ARROW:
			SetTitleText(tr("StraightArrow"));
			break;
		case E_STRAIGHT_MORE_ARROW:
			SetTitleText(tr("StraightMoreArrow"));
			break;
		case E_DOVETAIL_DIAGONAL_ARROW:
			SetTitleText(tr("DovetailDiagonalArrow"));
			break;
		case E_DOVETAIL_DIAGONAL_MORE_ARROW:
			SetTitleText(tr("DovetailDiagonalMoreArrow"));
			break;
		case E_DOVETAIL_STRAIGHT_ARROW:
			SetTitleText(tr("DoveTailStraightArrow"));
			break;
		case E_DOVETAIL_STRAIGHT_MORE_ARROW:
			SetTitleText(tr("DoveTailStraightMoreArrow"));
			break;
		case E_DIAGONAL_ARROW:
			SetTitleText(tr("DiagonalArrow"));
			break;
		case E_DIAGONAL_MORE_ARROW:
			SetTitleText(tr("DiagonalMoreArrow"));
			break;
		case E_DOUBLE_ARROW:
			SetTitleText(tr("DoubleArrow"));
			break;
		case E_CLOSE_CURVE:
			SetTitleText(tr("CloseCurve"));
			break;
		case E_GATHERING_PLACE:
			SetTitleText(tr("GatheringPlace"));
			break;
		//case E_CURVE_FLAG:
		//	SetTitleText(tr("CurveFlag"));
		//	break;
		//case E_RECT_FLAG:
		//	SetTitleText(tr("RectFlag"));
		//	break;
		//case E_TRIANGLE_FLAG:
		//	SetTitleText(tr("TriangleFlag"));
		//	break;
		case E_ROUNDED_RECT:
			SetTitleText(tr("RoundedRect"));
			break;
		case E_BEZIER_CURVE_ARROW:
			SetTitleText(tr("BezierCurveArrow"));
			break;
		case E_POLY_LINE_ARROW:
			SetTitleText(tr("PolyLineArrow"));
			break;
		case E_SECTOR_SEARCH:
			SetTitleText(tr("SectorSearch"));
			break;
		case E_PARALLEL_SEARCH:
			SetTitleText(tr("ParallerSearch"));
			break;
		case E_CARDINAL_CURVE_ARROW:
			SetTitleText(tr("CardinalCurveArrow"));
			break;
		case E_STRAIGHT_LINE_ARROW:
			SetTitleText(tr("StraightLineArrow"));
			break;
		//case E_FREE_LINE:
		//	SetTitleText(tr("FreeLine"));
		//	break;
		//case E_FREE_POLYGON:
		//	SetTitleText(tr("FreePolygon"));
		//	break;
		default:
			break;
		}
	}

	void CPropertyWidget::reject()
	{
		m_opMark->SetName(m_strName);

		for(TabVector::iterator itr = m_vecTab.begin(); itr != m_vecTab.end(); ++itr)
		{
			if(*itr)
			{
				(*itr)->Reject();
			}
		}

		emit sig_reject(m_opMark.get(),m_bCreated);
		close();
	}

	void CPropertyWidget::accept()
	{
		m_opMark->SetName(ConvertToSDKEncoding(ui.title_lineEdit->text()));

		for(TabVector::iterator itr = m_vecTab.begin(); itr != m_vecTab.end(); ++itr)
		{
			if(*itr)
			{
				(*itr)->Accept();
			}
		}
	
		emit sig_accept(m_opMark.get(), m_bCreated);
		close();
	}

	int CPropertyWidget::AddTab( CBaseStyleWidget* pTab )
	{
		if(pTab)
		{
			ui.tabWidget->addTab(pTab, pTab->windowTitle());
			m_vecTab.push_back(pTab);
			ui.tabWidget->setCurrentWidget(pTab);

		
			connect(this, SIGNAL(sig_SDKEventHappened(unsigned int)), pTab, SLOT(SlotSDKEventHappend(unsigned int)));
		}

		return 0;
	}

	void CPropertyWidget::SlotTitleChanged( QString strValue )
	{
		if(m_opMark.valid())
		{
			m_opMark->SetName(ConvertToSDKEncoding(strValue));
		}
	}

}

