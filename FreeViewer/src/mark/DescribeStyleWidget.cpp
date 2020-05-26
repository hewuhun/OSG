#include <mark/DescribeStyleWidget.h>

namespace FreeViewer
{


	CDescribeStyleWidget::CDescribeStyleWidget( FeExtNode::CExternNode* pExternNode, QWidget *parent)
		:CBaseStyleWidget(parent)
		,m_opExNode(pExternNode)
	{
		ui.setupUi(this);
		setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

		BuildContext();
	}

	CDescribeStyleWidget::~CDescribeStyleWidget()
	{

	}

	void CDescribeStyleWidget::BuildContext()
	{
		if(m_opExNode.valid())
		{
			m_strDesc = m_opExNode->GetDescription();

			setWindowTitle(tr("Describe"));
			ui.description_textEdit->setPlainText(m_opExNode->GetDescription().c_str());
			connect(ui.description_textEdit, SIGNAL(textChanged()), this, SLOT(SlotLimitMaxLength()));
		}
	}

	bool CDescribeStyleWidget::Accept()
	{
		if(m_opExNode.valid())
		{
			m_opExNode->SetDescription(ui.description_textEdit->toPlainText().toStdString());
		}
		return true;
	}

	bool CDescribeStyleWidget::Reject()
	{
		if(m_opExNode.valid())
		{
			m_opExNode->SetDescription(m_strDesc);
		}
		return CBaseStyleWidget::Reject();
	}

	void CDescribeStyleWidget::SlotLimitMaxLength()
	{
		// 限定标记描述信息最大长度
		int nMaxLength = 300;
		QString strText = ui.description_textEdit->toPlainText();
		int nLength = strText.count();
		if (nLength > nMaxLength)
		{
			int nPos = ui.description_textEdit->textCursor().position();
			strText.remove( (nPos - (nLength - nMaxLength)), (nLength - nMaxLength) );
			ui.description_textEdit->setText(strText);
			QTextCursor cursor = ui.description_textEdit->textCursor();
			cursor.setPosition(nPos - (nLength - nMaxLength));
			ui.description_textEdit->setTextCursor(cursor);
		}
	}

}

