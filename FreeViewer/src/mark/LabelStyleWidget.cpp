#include <QColorDialog>
#include <QCheckBox>
#include <QFileDialog>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/EventListener.h>
#include <mark/LabelStyleWidget.h>

#include <mainWindow/FreeUtil.h>
#include <mainWindow/Free3DDockTreeWidget.h>

namespace FreeViewer
{
	CLabelStyleWidget::CLabelStyleWidget(FeExtNode::CExternNode* opMark, QWidget *parent) 
		: CBaseStyleWidget(parent)
	{
		ui.setupUi(this);
		m_pLabelMark = dynamic_cast<FeExtNode::CExLabelNode*>(opMark);
		//setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
		SetStyleSheet(this, ":/css/docktree.qss");
		InitWidget();

		ui.Hei->hide();
		ui.HeiSpin->hide();
		ui.label_hei->hide();
	}


	CLabelStyleWidget::~CLabelStyleWidget(void)
	{
	}

	bool CLabelStyleWidget::Reject()
	{
		return true;
	}

	void CLabelStyleWidget::InitWidget()
	{
		setWindowTitle(tr("label"));
		InitLabelTree();
		if (m_rootItem == ui.LabelTree->currentItem())
			SetRoot();
		else
			SetItem(dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(ui.LabelTree->currentItem())));

		// 限定输入最大长度
		ui.NewItemTextEdit->setMaxLength(25);
		ui.ItemTextEdit->setMaxLength(25);

		connect(ui.ItemTextEdit, SIGNAL(textEdited(QString)), this, SLOT(SlotItemTextChanged(QString)));
		connect(ui.TextSizeSpin, SIGNAL(valueChanged(double)), this, SLOT(SlotTextSizeChanged(double)));
		connect(ui.LabelWidthSpin, SIGNAL(valueChanged(double)), this, SLOT(SlotLabelWidthChanged(double)));
		connect(ui.LabelHeightSpin, SIGNAL(valueChanged(double)), this, SLOT(SlotLabelHeightChanged(double)));
		connect(ui.LonSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotLongChanged(double)));
		connect(ui.LatSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotLatChanged(double)));
		connect(ui.HeiSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotHeightChanged(double)));
		connect(ui.TextColorBtn,SIGNAL(clicked()),this,SLOT(SlotTextColor()));
		connect(ui.BackColorBtn,SIGNAL(clicked()),this,SLOT(SlotBackColor()));
		connect(ui.AlignCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(SlotAlignChanged(int)));
		connect(ui.LabelTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)),this, SLOT(SlotItemClicked(QTreeWidgetItem*, int)));
		connect(ui.ImagePathButton,SIGNAL(clicked (bool)),this,SLOT(SlotImagePath(bool)));
		connect(ui.ImagePathEdit,SIGNAL(textEdited(QString)),this,SLOT(SlotImagePathChange(QString)));
	}

	void CLabelStyleWidget::SlotItemTextChanged( QString strText)
	{
		std::string str = strText.toStdString();

		FeExtNode::CExternNode* pNode = GetMarkByItem(ui.LabelTree->currentItem());

		if(FeExtNode::CExLabelItemNode* pItem = dynamic_cast<FeExtNode::CExLabelItemNode*>(pNode))
		{
			pItem->SetText(ConvertToSDKEncoding(strText));
		}
		else if(FeExtNode::CExLabelNode* pLabel = dynamic_cast<FeExtNode::CExLabelNode*>(pNode))
		{
			pLabel->SetText(ConvertToSDKEncoding(strText));
		}

		ui.LabelTree->currentItem()->setText(0, strText);
	}

	void CLabelStyleWidget::SlotTextSizeChanged( double tsize )
	{
		if (pCurrentItemNode = dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(ui.LabelTree->currentItem())))
			pCurrentItemNode->SetFontSize(tsize);
		else
			m_pLabelMark->SetFontSize(tsize);
	}

	void CLabelStyleWidget::SlotLabelWidthChanged( double tsize )
	{
		if (pCurrentItemNode = dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(ui.LabelTree->currentItem())))
			pCurrentItemNode->SetWidth(tsize);
		else
			m_pLabelMark->SetWidth(tsize);
	}

	void CLabelStyleWidget::SlotLabelHeightChanged( double tsize )
	{
		if (pCurrentItemNode = dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(ui.LabelTree->currentItem())))
			pCurrentItemNode->SetHeight(tsize);
		else
			m_pLabelMark->SetHeight(tsize);
	}

	void CLabelStyleWidget::SlotTextColor()
	{
		if (pCurrentItemNode = dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(ui.LabelTree->currentItem())))
		{
			QColorDialog* pColorDlg = new QColorDialog(QColor(pCurrentItemNode->GetTextColor().r(), pCurrentItemNode->GetTextColor().g(), 
				pCurrentItemNode->GetTextColor().b(), pCurrentItemNode->GetTextColor().a()));
			pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

			if (pColorDlg->exec() == QDialog::Accepted)
			{
				QString strTextColor;
				strTextColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
					+ QString::number(pColorDlg->currentColor().blue()) + ");";
				ui.TextColorBtn->setStyleSheet(strTextColor);
				pCurrentItemNode->SetTextColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, 1));
			}
		}
		else
		{
			QColorDialog* pColorDlg = new QColorDialog(QColor(m_pLabelMark->GetTextColor().r(), m_pLabelMark->GetTextColor().g(), 
				m_pLabelMark->GetTextColor().b(), m_pLabelMark->GetTextColor().a()));
			pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

			if (pColorDlg->exec() == QDialog::Accepted)
			{
				QString strTextColor;
				strTextColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
					+ QString::number(pColorDlg->currentColor().blue()) + ");";
				ui.TextColorBtn->setStyleSheet(strTextColor);
				m_pLabelMark->SetTextColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, 1));
			}
		}
		
	}

	void CLabelStyleWidget::SlotBackColor()
	{
		if (pCurrentItemNode = dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(ui.LabelTree->currentItem())))
		{
			QColorDialog* pColorDlg = new QColorDialog(QColor(pCurrentItemNode->GetBackgroundColor().r(), pCurrentItemNode->GetBackgroundColor().g(), 
				pCurrentItemNode->GetBackgroundColor().b(), pCurrentItemNode->GetBackgroundColor().a()));
			pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

			if (pColorDlg->exec() == QDialog::Accepted)
			{
				QString strBackColor;
				strBackColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
					+ QString::number(pColorDlg->currentColor().blue()) + ");";
				ui.BackColorBtn->setStyleSheet(strBackColor);
				pCurrentItemNode->SetBackgroundColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, 1));
			}
		}
		else
		{
			QColorDialog* pColorDlg = new QColorDialog(QColor(m_pLabelMark->GetBackgroundColor().r(), m_pLabelMark->GetBackgroundColor().g(), 
				m_pLabelMark->GetBackgroundColor().b(), m_pLabelMark->GetBackgroundColor().a()));
			pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

			if (pColorDlg->exec() == QDialog::Accepted)
			{
				QString strBackColor;
				strBackColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
					+ QString::number(pColorDlg->currentColor().blue()) + ");";
				ui.BackColorBtn->setStyleSheet(strBackColor);
				m_pLabelMark->SetBackgroundColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, 1));
			}
		}	
	}

	void CLabelStyleWidget::SlotLongChanged( double dLong)
	{
		osg::Vec3d vecLLH = m_pLabelMark->GetBindingPointLLH();
		vecLLH.x() = dLong;
		m_pLabelMark->SetBindingPointLLH(vecLLH);
	}

	void CLabelStyleWidget::SlotLatChanged( double dLat )
	{
		osg::Vec3d vecLLH = m_pLabelMark->GetBindingPointLLH();
		vecLLH.y() = dLat;
		m_pLabelMark->SetBindingPointLLH(vecLLH);
	}

	void CLabelStyleWidget::SlotHeightChanged( double dHei )
	{
		osg::Vec3d vecLLH = m_pLabelMark->GetBindingPointLLH();
		vecLLH.z() = dHei;
		m_pLabelMark->SetBindingPointLLH(vecLLH);
	}

	void CLabelStyleWidget::SlotAlignChanged( int index )
	{
		if (pCurrentItemNode = dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(ui.LabelTree->currentItem())))
		{
			switch(index)
			{
			case 0:
				pCurrentItemNode->SetTextAlign(FeExtNode::CExLabelItemOption::ALIGN_LEFT);
				break;
			case 1:
				pCurrentItemNode->SetTextAlign(FeExtNode::CExLabelItemOption::ALIGN_CENTER);
				break;
			case 2:
				pCurrentItemNode->SetTextAlign(FeExtNode::CExLabelItemOption::ALIGN_RIGHT);
				break;
			default:
				break;
			}
		}
		else
		{
			switch(index)
			{
			case 0:
				m_pLabelMark->SetTextAlign(FeExtNode::CExLabelItemOption::ALIGN_LEFT);
				break;
			case 1:
				m_pLabelMark->SetTextAlign(FeExtNode::CExLabelItemOption::ALIGN_CENTER);
				break;
			case 2:
				m_pLabelMark->SetTextAlign(FeExtNode::CExLabelItemOption::ALIGN_RIGHT);
				break;
			default:
				break;
			}
		}
	}

	void CLabelStyleWidget::SlotImagePath( bool )
	{
		QString filePath;
		QString dirPath = FeFileReg->GetDataPath().c_str();
		dirPath += "/texture/mark";
#ifdef WIN32
		QFileDialog* pFileDialog = new QFileDialog();
		pFileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		filePath = pFileDialog->getOpenFileName(0, tr("open File"), dirPath, tr("Label Files(*.png);;All files (*.*)"));
		if(!filePath.isNull())
		{
			ui.ImagePathEdit->setText(filePath);
			SlotImagePathChange(filePath);
		}
#else
		QFileDialog fileDialog(0, tr("open File"), dirPath, tr("Label Files(*.png);;All files (*.*)"));
		if (fileDialog.exec())
		{
			filePath = fileDialog.selectedFiles().first();
			ui.ImagePathEdit->setText(filePath);
			SlotImagePathChange(filePath);
		}
#endif
	}

	void CLabelStyleWidget::InitLabelTree()
	{
		InitLabelTreeRoot();
		FeExtNode::CExLabelItemNodeList ChildrenList=m_pLabelMark->GetChildList();
		FeExtNode::CExLabelItemNodeList::iterator itr = ChildrenList.begin();
		for (++itr; itr != ChildrenList.end(); ++itr)
		{
			QTreeWidgetItem* pParentItem = new QTreeWidgetItem(m_rootItem, QStringList(ConvertToCurrentEncoding((*itr)->GetText())));
			pParentItem->setFlags(pParentItem->flags() &~ Qt::ItemIsDragEnabled &~Qt::ItemIsDropEnabled);
            pParentItem->setData(0, Qt::UserRole, QVariant::fromValue((FeExtNode::CExternNode*)(*itr).get()));
			FeExtNode::CExLabelItemNodeList ChildList = (*itr)->GetChildList();
			FeExtNode::CExLabelItemNodeList::iterator itrChild = ChildList.begin();
			for (; itrChild != ChildList.end(); ++itrChild)
			{
				AddTreeItem(pParentItem, (*itrChild).get());
			}
			pParentItem->setExpanded(true);
			ui.LabelTree->setCurrentItem(m_rootItem);
		}	
	}

	void CLabelStyleWidget::InitLabelTreeRoot()
	{
		m_rootItem = new QTreeWidgetItem(ui.LabelTree, QStringList(ConvertToCurrentEncoding(m_pLabelMark->GetText())));
		connect(ui.AddButton, SIGNAL(clicked()), this, SLOT(SlotAddItem()));
		connect(ui.DeleteButton, SIGNAL(clicked()), this, SLOT(SlotDeleteItem()));
		connect(ui.ClearButton, SIGNAL(clicked()), this, SLOT(SlotClearItem()));
		m_rootItem->setData(0, Qt::UserRole, "root");
		ui.LabelTree->setCurrentItem(m_rootItem);
		m_rootItem->setExpanded(true);
		m_rootItem->setFlags(m_rootItem->flags() &~ Qt::ItemIsDragEnabled &~Qt::ItemIsDropEnabled);
	}

	void CLabelStyleWidget::AddTreeItem( QTreeWidgetItem* pParentItem, FeExtNode::CExternNode* pExternNode )
	{
		FeExtNode::CExLabelItemNode* pLabelItemNode = dynamic_cast<FeExtNode::CExLabelItemNode*>(pExternNode);
		if (pExternNode && pParentItem)
		{
			QTreeWidgetItem* pItem = new QTreeWidgetItem(pParentItem,  QStringList(ConvertToCurrentEncoding(pLabelItemNode->GetText())));
            pItem->setData(0, Qt::UserRole, QVariant::fromValue((FeExtNode::CExternNode*)pLabelItemNode));
			pItem->setExpanded(true);
			pItem->setFlags(pItem->flags() &~ Qt::ItemIsDragEnabled &~Qt::ItemIsDropEnabled);
			FeExtNode::CExLabelItemNodeList ChildrenList=pLabelItemNode->GetChildList();
			FeExtNode::CExLabelItemNodeList::iterator itr = ChildrenList.begin();
			for (; itr != ChildrenList.end(); ++itr)
			{
				AddTreeItem(pItem, (*itr).get());
			}
		}
	}

	void CLabelStyleWidget::SlotAddItem()
	{
		QTreeWidgetItem* pNewItem = NULL;
		QTreeWidgetItem* pParentItem = NULL;

		FeExtNode::CExLabelItemNode* pMark = NULL;
		
		QString StrText = ui.NewItemTextEdit->text().isEmpty() ?
			tr("New Item") : ui.NewItemTextEdit->text();
		
		if (GetMarkByItem(ui.LabelTree->currentItem()))
		{
			pParentItem = ui.LabelTree->currentItem();
		}	
		else if(ui.LabelTree->currentItem()->parent())
		{
			if(FeExtNode::CExLabelItemNode* pParentMark = dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(ui.LabelTree->currentItem()->parent())))
			{
				pParentItem = ui.LabelTree->currentItem()->parent();
			}
		}
		if(pParentItem && GetMarkByItem(pParentItem))
		{
			if (FeExtNode::CExLabelItemNode* pParentMark = dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(pParentItem)))
			{
				FeUtil::FEID idItem = pParentMark->AddChildItem(ConvertToSDKEncoding(StrText));
				pMark = pParentMark->GetChildItem(idItem);

			}
			else if (FeExtNode::CExLabelNode* pParentMark = dynamic_cast<FeExtNode::CExLabelNode*>(GetMarkByItem(pParentItem)))
			{
				FeUtil::FEID idItem = pParentMark->AddLabelItem(ConvertToSDKEncoding(StrText));
				pMark = pParentMark->GetLabelItem(idItem);
			}
			pNewItem = new QTreeWidgetItem(pParentItem, QStringList(StrText));
			pParentItem->setExpanded(true); 
			ui.LabelTree->setCurrentItem(pParentItem);
		}
        pNewItem->setData(0, Qt::UserRole, QVariant::fromValue((FeExtNode::CExternNode*)pMark));
		if (pNewItem)
		{
			pNewItem->setFlags(pNewItem->flags() &~Qt::ItemIsDragEnabled &~Qt::ItemIsDropEnabled );
		}
	}

	void CLabelStyleWidget::SlotDeleteItem()
	{
		QTreeWidgetItem* pCurrentItem = ui.LabelTree->currentItem();
		if (pCurrentItem == m_rootItem)
		{
			return;
		}
		if (pCurrentItem && GetMarkByItem(pCurrentItem))
		{
			if (pCurrentItem->childCount() != 0)
			{
				QTreeWidgetItem* pDeleteItem = NULL;
				while (pCurrentItem->childCount() != 0)
				{
					pDeleteItem = pCurrentItem->child(0);
					dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(pCurrentItem))->RemoveChildItem(GetMarkByItem(pDeleteItem)->GetID());
					pCurrentItem->removeChild(pDeleteItem);
					delete pDeleteItem;
				}
			}
			if (pCurrentItem->parent() && GetMarkByItem(pCurrentItem->parent()))
			{
				if (pCurrentItem->parent()==m_rootItem)
					dynamic_cast<FeExtNode::CExLabelNode*>(GetMarkByItem(pCurrentItem->parent()))->RemoveLabelItem(GetMarkByItem(pCurrentItem)->GetID());
				else
					dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(pCurrentItem->parent()))->RemoveChildItem(GetMarkByItem(pCurrentItem)->GetID());
				pCurrentItem->parent()->removeChild(pCurrentItem);
				delete pCurrentItem;
			}
			else
			{
				m_pLabelMark->RemoveLabelItem(GetMarkByItem(pCurrentItem)->GetID());			
				delete pCurrentItem;
			}
		}
	}

	void CLabelStyleWidget::SlotClearItem()
	{
		QTreeWidgetItem* pCurrentItem = ui.LabelTree->currentItem();
		if (pCurrentItem && GetMarkByItem(pCurrentItem))
		{
			if (pCurrentItem->childCount() != 0)
			{
				QTreeWidgetItem* pDeleteItem = NULL;
				while (pCurrentItem->childCount() != 0)
				{
					pDeleteItem = pCurrentItem->child(0);
					if (pCurrentItem == m_rootItem)
						dynamic_cast<FeExtNode::CExLabelNode*>(GetMarkByItem(pCurrentItem))->RemoveLabelItem(GetMarkByItem(pDeleteItem)->GetID());
					else
						dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(pCurrentItem))->RemoveChildItem(GetMarkByItem(pDeleteItem)->GetID());				
					pCurrentItem->removeChild(pDeleteItem);
					delete pDeleteItem;
				}
			}
		}
	}

	void CLabelStyleWidget::SlotItemClicked( QTreeWidgetItem* pItem, int nIndex)
	{
		if (pItem == m_rootItem)
			SetRoot();
		else
			SetItem(dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(pItem)));
	}

	FeExtNode::CExternNode* CLabelStyleWidget::GetMarkByItem( QTreeWidgetItem* pItem )
	{
		if (pItem == m_rootItem)
		{
			return m_pLabelMark;
		}
		if (pItem)
		{
            return pItem->data(0, Qt::UserRole).value<FeExtNode::CExternNode*>();
		}

		return NULL;
	}

	void CLabelStyleWidget::SetText(FeExtNode::CExLabelItemNode* pLabelItemNode)
	{
		ui.LabelTree->currentItem()->setText(0, ConvertToCurrentEncoding(pLabelItemNode->GetText()));
		ui.ItemTextEdit->setText(ConvertToCurrentEncoding(pLabelItemNode->GetText()));
		//ui.ItemTextEdit->setEnabled(true);
		ui.TextSizeSpin->setValue(pLabelItemNode->GetFontSize());
		ui.LabelWidthSpin->setValue(pLabelItemNode->GetWidth());
		ui.LabelHeightSpin->setValue(pLabelItemNode->GetHeight());
	}

	void CLabelStyleWidget::SetPosition()
	{
		ui.LonSpin->setValue(m_pLabelMark->GetBindingPointLLH().x());
		ui.LatSpin->setValue(m_pLabelMark->GetBindingPointLLH().y());
		ui.HeiSpin->setValue(m_pLabelMark->GetBindingPointLLH().z());
	}

	void CLabelStyleWidget::SetTextColor( FeExtNode::CExLabelItemNode* pLabelItemNode )
	{
		QString strTextColor;
		strTextColor = "background-color:rgb(" 
			+ QString::number(pLabelItemNode->GetTextColor().r() * 255) + ","
			+ QString::number(pLabelItemNode->GetTextColor().g() * 255) + ","
			+ QString::number(pLabelItemNode->GetTextColor().b() * 255) + ");";
		ui.TextColorBtn->setStyleSheet(strTextColor);
	}

	void CLabelStyleWidget::SetBackColor( FeExtNode::CExLabelItemNode* pLabelItemNode )
	{
		QString strBackColor;
		strBackColor = "background-color:rgb(" 
			+ QString::number(pLabelItemNode->GetBackgroundColor().r() * 255) + ","
			+ QString::number(pLabelItemNode->GetBackgroundColor().g() * 255) + ","
			+ QString::number(pLabelItemNode->GetBackgroundColor().b() * 255) + ");";
		ui.BackColorBtn->setStyleSheet(strBackColor);
	}

	void CLabelStyleWidget::SetBackImage( FeExtNode::CExLabelItemNode* pLabelItemNode )
	{
		ui.ImagePathEdit->setText(pLabelItemNode->GetBackgroundImage().c_str());
	}

	void CLabelStyleWidget::SetAlignment( FeExtNode::CExLabelItemNode* pLabelItemNode )
	{
		if (pLabelItemNode->GetTextAlign() == 0)
		{
			ui.AlignCombo->setCurrentIndex(0);
		}
		else if (pLabelItemNode->GetTextAlign() == 1)
		{
			ui.AlignCombo->setCurrentIndex(1);
		}
		else if (pLabelItemNode->GetTextAlign() == 2)
		{
			ui.AlignCombo->setCurrentIndex(2);
		}
	}

	void CLabelStyleWidget::SetRoot()
	{
		ui.LabelTree->currentItem()->setText(0, ConvertToCurrentEncoding(m_pLabelMark->GetText()));
		ui.ItemTextEdit->setText(ConvertToCurrentEncoding(m_pLabelMark->GetText()));
		ui.LonSpin->setValue(m_pLabelMark->GetBindingPointLLH().x());
		ui.LatSpin->setValue(m_pLabelMark->GetBindingPointLLH().y());
		ui.HeiSpin->setValue(m_pLabelMark->GetBindingPointLLH().z());
		ui.TextSizeSpin->setValue(m_pLabelMark->GetFontSize());
		ui.LabelWidthSpin->setValue(m_pLabelMark->GetWidth());
		ui.LabelHeightSpin->setValue(m_pLabelMark->GetHeight());  
		QString strTextColor;
		strTextColor = "background-color:rgb(" 
			+ QString::number(m_pLabelMark->GetTextColor().r() * 255) + ","
			+ QString::number(m_pLabelMark->GetTextColor().g() * 255) + ","
			+ QString::number(m_pLabelMark->GetTextColor().b() * 255) + ");";
		ui.TextColorBtn->setStyleSheet(strTextColor);

		QString strBackColor;
		strBackColor = "background-color:rgb(" 
			+ QString::number(m_pLabelMark->GetBackgroundColor().r() * 255) + ","
			+ QString::number(m_pLabelMark->GetBackgroundColor().g() * 255) + ","
			+ QString::number(m_pLabelMark->GetBackgroundColor().b() * 255) + ");";
		ui.BackColorBtn->setStyleSheet(strBackColor);
		ui.ImagePathEdit->setText(m_pLabelMark->GetBackgroundImage().c_str());
		if (m_pLabelMark->GetTextAlign() == 0)
		{
			ui.AlignCombo->setCurrentIndex(0);
		}
		else if (m_pLabelMark->GetTextAlign() == 1)
		{
			ui.AlignCombo->setCurrentIndex(1);
		}
		else if (m_pLabelMark->GetTextAlign() == 2)
		{
			ui.AlignCombo->setCurrentIndex(2);
		}
	}

	void CLabelStyleWidget::SetItem(FeExtNode::CExLabelItemNode* pLabelItemNode)
	{
		SetText(pLabelItemNode);
		SetPosition();
		SetTextColor(pLabelItemNode);
		SetBackColor(pLabelItemNode);
		SetBackImage(pLabelItemNode);
		SetAlignment(pLabelItemNode);
	}

	void CLabelStyleWidget::SlotImagePathChange( QString str)
	{
		if (str.isEmpty())
		{
			if (m_rootItem == ui.LabelTree->currentItem())
			{
				m_pLabelMark->RemoveBackgroundImage();
				m_pLabelMark->SetBackgroundColor(osg::Vec4(0.3, 0, 1, 0.5));
			}
			else
			{
				FeExtNode::CExLabelItemNode* pLabelItemNode = (dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(ui.LabelTree->currentItem())));
				pLabelItemNode->RemoveBackgroundImage();
				pLabelItemNode->SetBackgroundColor(osg::Vec4(0.3, 0, 1, 0.5));
			}
		}
		else
		{
			QString strPath = str;
			FormatPathString(strPath);

			if (m_rootItem == ui.LabelTree->currentItem())
			{
				m_pLabelMark->SetBackgroundImage(strPath.toStdString());
				m_pLabelMark->SetBackgroundColor(osg::Vec4(1.0, 1.0, 1.0, 1));
			}
			else
			{
				FeExtNode::CExLabelItemNode* pLabelItemNode = (dynamic_cast<FeExtNode::CExLabelItemNode*>(GetMarkByItem(ui.LabelTree->currentItem())));
				pLabelItemNode->SetBackgroundImage(strPath.toStdString());
				pLabelItemNode->SetBackgroundColor(osg::Vec4(1.0, 1.0, 1.0, 1));
			}
		}

	}

	void CLabelStyleWidget::SlotSDKEventHappend( unsigned int eType)
	{
// 		if(!m_opMark.valid()) return;
// 
// 		if(FeUtil::E_EVENT_TYPE_OBJECT_POSITION_CHANGED == eType)
// 		{
// 			osgEarth::GeoPoint vpos = m_opMark->GetPosition();	
// 			ui.clongitudeSpin->setValue(vpos.x());
// 			ui.clatitudeSpin->setValue(vpos.y());
// 			ui.cheightSpin->setValue(vpos.z());
// 		}
	}

}
