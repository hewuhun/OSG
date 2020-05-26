#include <mainWindow/FolderWidget.h>

#include <mainWindow/UIObserver.h>
#include <mainWindow/FreeUtil.h>
#include <QMessageBox>

namespace FreeViewer
{
	CFolderWidget::CFolderWidget(QWidget* pParent)
		: CFreeDialog(pParent)
	{
		// 关闭时释放内存
		//this->setAttribute(Qt::WA_DeleteOnClose);

		QWidget* widget = new QWidget(this);     
		ui.setupUi(widget);                      
		AddWidgetToDialogLayout(widget);         
		widget->setMinimumSize(widget->size());  

		InitWidget();
	}

	CFolderWidget::~CFolderWidget()
	{

	}

	void CFolderWidget::InitWidget()
	{
		ui.title_lineEdit->setText(tr("Folder"));
		ui.title_lineEdit->setMaxLength(10);

		connect(ui.title_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(SlotTextChanged(QString)));
		connect(ui.btn_ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(ui.btn_cancel, SIGNAL(clicked()), this, SLOT(reject()));

		//关联关闭按钮信号和槽
		connect(m_pCloseBtn, SIGNAL(clicked()), this, SLOT(reject()));
	}

	void CFolderWidget::reject()
	{
		emit SignalFolderName(ui.title_lineEdit->text(), false);

		close();
	}

	void CFolderWidget::accept()
	{

		emit SignalFolderName(ui.title_lineEdit->text(), true);

		close();
	}

	void CFolderWidget::SlotTextChanged( QString text )
	{
		for (int i=0; i<text.size(); i++)
		{
			if (text.at(i) == '`'
				|| text.at(i) == '~'
				|| text.at(i) == '!'
				|| text.at(i) == '@'
				|| text.at(i) == '#'
				|| text.at(i) == '$'
				|| text.at(i) == '%'
				|| text.at(i) == '^'
				|| text.at(i) == '&'
				|| text.at(i) == '*'
				|| text.at(i) == '('
				|| text.at(i) == ')'
				|| text.at(i) == '-'
				|| text.at(i) == '_'
				|| text.at(i) == '='
				|| text.at(i) == '+'
				|| text.at(i) == '/'
				|| text.at(i) == '{'
				|| text.at(i) == '}'
				|| text.at(i) == '['
				|| text.at(i) == ']'
				|| text.at(i) == '|'
				|| text.at(i) == '\\'
				|| text.at(i) == ':'
				|| text.at(i) == ';'
				|| text.at(i) == '"'
				|| text.at(i) == '\''
				|| text.at(i) == '<'
				|| text.at(i) == '>'
				|| text.at(i) == ','
				|| text.at(i) == '.'
				|| text.at(i) == '?'
				)
			{
				QMessageBox::warning(this, tr("Warning"), tr("Input error,Please to reput!"), QMessageBox::Ok);
				ui.title_lineEdit->clear();
			}
		}
	}

}

