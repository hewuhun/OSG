#include <mainWindow/FreeLineEdit.h>

	
	CFreeLineEdit::CFreeLineEdit( QWidget* pParent )
		:QLineEdit(pParent)
	{

	}

	CFreeLineEdit::~CFreeLineEdit()
	{

	}

	void CFreeLineEdit::mousePressEvent( QMouseEvent *event )
	{
		if (event->button())
		{
			emit clicked();
		}

		QLineEdit::mousePressEvent(event);
	}
