#include <string>
#include <QMenu>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QTreeWidget>
#include <osg/Multisample>
#include <osg/StateSet>
#include <osg/StateAttribute>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgEarth/IntersectionPicker>

#include <FeUtils/CoordConverter.h>
#include <FeUtils/PathRegistry.h>

#include <FeEarth/FreeEarth.h>
#include <FeEarth/3DSceneWidget.h>

namespace FeEarth
{
	CInitThread::CInitThread( FeShell::CSystemManager* pSystemManager )
		:QThread()
		,m_opSystemManager(pSystemManager)
		,m_bInitialized(false)
	{

	}

	CInitThread::~CInitThread()
	{

	}

	void CInitThread::run()
	{
		bool bState = false;
		if (m_opSystemManager.valid())
		{
			bState = m_opSystemManager->Initialize();
		}

		m_bInitialized = true;

		emit SignalInitDone(bState);
	}

	bool CInitThread::IsInitFinish()
	{ 
		return m_bInitialized; 
	}
}


namespace FeEarth
{
	C3DSceneWidget::C3DSceneWidget( const std::string& strDataPath, bool bThread )
		:osgQt::GLWidget(QGLFormat(QGL::SampleBuffers))
		,m_strDataPath(strDataPath)
		,m_rpSystemManager(NULL)
		,m_bThread(bThread)
		,m_pInitThread(NULL)
		,m_strEarthPath("")
	{
		setAcceptDrops(true);

		setGeometry(0, 0, 500, 500);
	}

	//初始化成功，返回false
	bool C3DSceneWidget::Initialize()
	{
		m_rpSystemManager = new FeEarth::CFreeEarth(m_strDataPath);
		m_rpSystemManager->SetEarthPath(m_strEarthPath);

		if (m_bThread)
		{
			m_pInitThread = new CInitThread(m_rpSystemManager.get());
			m_pInitThread->start();

			connect(m_pInitThread, SIGNAL(SignalInitDone(bool)), this, SLOT(SlotInitDone(bool)));
		}
		else
		{
			if(m_rpSystemManager->Initialize())
			{
				m_rpSystemManager->SetGraphicContext(
					new osgQt::GraphicsWindowQt(
					CreateTraits(new osgQt::GraphicsWindowQt::WindowData(this), x(), y(), width(), height())));

				InstallFrameTimer();
				return true;
			}
		}

		return false;
	}

	bool C3DSceneWidget::UnInitialize()
	{
		if (m_rpSystemManager)
		{
			m_rpSystemManager->UnInitialize();
			return true;
		}

		return false;
	}

	osg::GraphicsContext::Traits* C3DSceneWidget::CreateTraits( osg::Referenced* pInheritedWindowData, int nX, int nY, int nW, int nH )
	{
		//osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
		//ds->setNumMultiSamples(4); //多重采样设置反走样

		osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
		traits->readDISPLAY();
		if (traits->displayNum<0) traits->displayNum = 0;

		traits->windowName = "FreeEarth";
		traits->windowDecoration = false;
		traits->x = nX;
		traits->y = nY;
		traits->width = nW;
		traits->height = nH;
		traits->doubleBuffer = true;
		traits->inheritedWindowData = pInheritedWindowData;

		return traits.release();
	}

	void C3DSceneWidget::InstallFrameTimer()
	{
		connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
		m_timer.start();
	}

	void C3DSceneWidget::paintEvent( QPaintEvent* event)
	{
		if(m_rpSystemManager.valid())
		{
			m_rpSystemManager->Frame();
		}

		osgQt::GLWidget::paintEvent(event);
	}

	void C3DSceneWidget::resizeEvent( QResizeEvent* e )
	{
		if(m_pInitThread && !m_pInitThread->IsInitFinish())
		{
			return;
		}


		int nX(0);
		int nY(0);
		int nWidth(0);
		int nHeight(0);

		geometry().getRect(&nX, &nY, &nWidth, &nHeight);

		nY = nHeight - nY;

		if(m_rpSystemManager.valid())
		{
			m_rpSystemManager->ResizeGraphicsContext(nX, nY, nWidth, nHeight);
		}

		osgQt::GLWidget::resizeEvent(e);
	}


	void C3DSceneWidget::dragEnterEvent( QDragEnterEvent* e )
	{
		if( e->mimeData()->hasFormat("ModelPath") || qobject_cast<QTreeWidget*>(e->source()) != NULL )
			e->acceptProposedAction();
		else
			e->ignore();
		emit SignalDragEnterEvent(e);
	}

	void C3DSceneWidget::dragMoveEvent( QDragMoveEvent* e )
	{
		if(e->mimeData()->hasFormat("ModelPath") || qobject_cast<QTreeWidget*>(e->source()) != NULL)
		{
			QRect rect = geometry();
			int x = e->pos().x();
			int y = e->pos().y();
			y = rect.height() - y;

			osgEarth::IntersectionPicker picker(
				m_rpSystemManager->GetSystemService()->GetRenderContext()->GetView(), 
				m_rpSystemManager->GetSystemService()->GetRenderContext()->GetMapNode());
			osgEarth::IntersectionPicker::Hits hits;
			if(picker.pick(float(x),float(y),hits))
			{
				e->acceptProposedAction();
				return;
			}

		}

		e->ignore();
	}

	void C3DSceneWidget::dropEvent( QDropEvent* e )
	{
		if( false == e->mimeData()->hasFormat("ModelPath")  && qobject_cast<QTreeWidget*>(e->source()) == NULL )
		{
			return;
		}

		int nScreenX = e->pos().x();
		int nScreenY = e->pos().y();
		QRect r = geometry();
		nScreenY = r.height() - nScreenY;
		/*osg::Vec3d vecPos;
		vecPos.x() = nScreenX;
		vecPos.y() = nScreenY;*/
		osg::Vec3d llh;
		ScreenXY2DegreeLLH(m_rpSystemManager->GetRenderContext(), nScreenX, nScreenY, llh);
		//OSG_NOTICE << "llh x: " << llh.x() << "  y:  " << llh.y() << " z: " << llh.z() << std::endl;
		emit SignalDropEvent(e->source(), llh);
		emit SignalDropEvent(e);
		
	}

	FeShell::CSystemManager* C3DSceneWidget::GetSystemManager()
	{
		return m_rpSystemManager.get();
	}

	bool C3DSceneWidget::event( QEvent* event )
	{
		if(m_pInitThread && !m_pInitThread->IsInitFinish())
		{
			return false;
		}

		return QGLWidget::event( event );
	}

	void C3DSceneWidget::SlotInitDone( bool bState )
	{
		m_pInitThread->exit();

		if(bState)
		{
			m_rpSystemManager->SetGraphicContext(
				new osgQt::GraphicsWindowQt(
				CreateTraits(new osgQt::GraphicsWindowQt::WindowData(this), x(), y(), width(), height())));

			InstallFrameTimer();
		}

		emit Signal3DSceneInitDone(bState);
	}

	void C3DSceneWidget::SetEarthPath( std::string strPath )
	{
		m_strEarthPath = strPath;
	}

}


