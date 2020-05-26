#include <osgGA/GUIEventAdapter>
#include <FeDraggerAttribute.h>

namespace FeNodeEditor
{
	CFeDraggerAttribute::CFeDraggerAttribute( void )
	{
		m_fLineWidth = 1.0;

		CreateAttributes();
	}

	bool CFeDraggerAttribute::HandleAttributes( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		switch (ea.getEventType())
		{
			// Pick start.
		case (osgGA::GUIEventAdapter::PUSH):
			{
				if(m_rpLineWidth.valid())
				{
					m_rpLineWidth->setWidth(m_fLineWidth*2);
				}
			}
			break;

			// Pick finish.
		case (osgGA::GUIEventAdapter::RELEASE):
			{
				if(m_rpLineWidth.valid())
				{
					m_rpLineWidth->setWidth(m_fLineWidth);
				}
			}
			break;
		}

		return true;
	}

	void CFeDraggerAttribute::CreateAttributes()
	{
		if(!m_rpLineWidth.valid())
		{
			m_rpLineWidth = new osg::LineWidth();
			m_rpLineWidth->setWidth(m_fLineWidth);
		}
	}

	void CFeDraggerAttribute::setLineWidth( float linePixelWidth )
	{
		if(m_rpLineWidth.valid())
		{
			m_fLineWidth = linePixelWidth;
			m_rpLineWidth->setWidth(m_fLineWidth);
		}
	}

}
