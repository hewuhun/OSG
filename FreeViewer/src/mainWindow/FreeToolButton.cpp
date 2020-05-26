#include <mainWindow/FreeToolButton.h>

#include <mainWindow/FreeUtil.h>

#include <QMouseEvent>
#include <QWidgetAction>

namespace FreeViewer
{
	CFreeToolButton::CFreeToolButton( 
		QString strName, 
		QString strNormalIcon,
		QString strHoverIcon, 
		QString strPressIcon, 
		QString strDisableIcon,
		QWidget* parent)
		:QToolButton(parent)
		,m_strNormalIcon(strNormalIcon)
		,m_strHoverIcon(strHoverIcon)
		,m_strPressedIcon(strPressIcon)
		,m_strDisableIcon(strDisableIcon)
		,m_bMouseMove(false)
		,m_pMenu(NULL)
	{
		setText(strName);
		setToolTip(strName);
		setIcon(QIcon(m_strNormalIcon));

		connect(this, SIGNAL(toggled(bool)), this, SLOT(SlotChecked(bool)));
	}

	CFreeToolButton::~CFreeToolButton()
	{

	}

	void CFreeToolButton::Normal()
	{
		setIcon(QIcon(m_strNormalIcon));
	}

	void CFreeToolButton::Hover()
	{
		setIcon(QIcon(m_strHoverIcon));
	}

	void CFreeToolButton::Pressed()
	{
		setIcon(QIcon(m_strPressedIcon));
	}

	void CFreeToolButton::Disable()
	{
		setIcon(QIcon(m_strDisableIcon));
	}

	void CFreeToolButton::mousePressEvent( QMouseEvent * event )
	{
		m_bMouseMove = false;
		if (false == isChecked())
		{
			Pressed();
		}

		//QToolButton::mousePressEvent(event);
	}

	void CFreeToolButton::mouseMoveEvent( QMouseEvent * event )
	{
		m_bMouseMove = true;

		//QToolButton::mouseMoveEvent(event);
	}

	void CFreeToolButton::mouseReleaseEvent( QMouseEvent * event )
	{
		if (false == m_bMouseMove)
		{
			if (false == isChecked())
			{
				emit clicked(true);
			}
			else
			{
				emit clicked(false);
			}
		}

		if (false == isChecked())
		{
			Normal();
		}

		//QToolButton::mouseReleaseEvent(event);
	}

	void CFreeToolButton::enterEvent( QEvent * event )
	{
		if (false == isChecked())
		{
			Hover();
		}

		QToolButton::enterEvent(event);
	}

	void CFreeToolButton::leaveEvent( QEvent * event )
	{
		if (false == isChecked())
		{
			Normal();
		}

		QToolButton::leaveEvent(event);
	}

	void CFreeToolButton::SlotChecked( bool bChecked )
	{
		if (bChecked)
		{
			Pressed();
		}
		else
		{
			Normal();
		}
	}

	CFreeToolButton* CFreeToolButton::AddMenuAction( 
		QString strName,
		QString strNormalIcon,
		QString strHoverIcon,
		QString strPressIcon,
		QString strDisableIcon )
	{
		//如果按钮中的菜单为空，则创建菜单
		if (NULL == m_pMenu)
		{
			m_pMenu = new QMenu(this);
			setMenu(m_pMenu);
			setPopupMode(QToolButton::MenuButtonPopup);
		}

		//创建新的按钮，并将此按钮添加到菜单中
		CFreeToolButton* pToolButton = new CFreeToolButton(
			strName, strNormalIcon, strHoverIcon, strPressIcon, strDisableIcon, this);
		pToolButton->setFixedSize(g_sizeToolButton);
		pToolButton->setIconSize(g_sizeToolButton);
		pToolButton->setToolTip(strName);
		connect(pToolButton, SIGNAL(released()), m_pMenu, SLOT(hide()));

		QWidgetAction* pAction = new QWidgetAction(m_pMenu);
		pAction->setDefaultWidget(pToolButton);
		m_pMenu->addAction(pAction);

		return pToolButton;
	}
}
