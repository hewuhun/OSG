#ifndef TD_SCENE_PANEL_H
#define TD_SCENE_PANEL_H 1

#include <QTimer>
#include <QWidget>
#include <QThread>
#include <QGridLayout>
#include <osgQt/GraphicsWindowQt>
#include <FeShell/SystemManager.h>

#include <FeEarth/Export.h>

namespace FeEarth
{
	class CInitThread: public QThread
	{
		Q_OBJECT;

	public:
		CInitThread(FeShell::CSystemManager* pSystemManager);

		~CInitThread();

	signals:
		void SignalInitDone(bool);

	protected:
		virtual void run();

	protected:
		osg::observer_ptr<FeShell::CSystemManager>  m_opSystemManager;
	};

	/**
	* @class CSceneWidget
	* @brief 渲染窗口
	* @note 用于渲染三维场景的窗口，定义了一个定时器，用于对场景的刷新
	* @author c00005
	*/
	class FEEARTH_EXPORT C3DScenePanel : public osgQt::GLWidget
	{
		Q_OBJECT;

	public:
		C3DScenePanel(const std::string& strDataPath = "", bool bThread = false);

	public:
		///初始化，其中为渲染的视口设置了图形上下文
		bool Initialize();

		bool UnInitialize();

		///获取三维管理器具
		FeShell::CSystemManager* GetSystemManager();

	signals:
		void SignalDropEvent(QObject *pWidget, osg::Vec3d llh);

		void Signal3DSceneInitDone();

	public slots:
		void SlotInitDone(bool);

	protected:
		void InstallFrameTimer();

		virtual void paintEvent( QPaintEvent* );
		virtual void resizeEvent( QResizeEvent* event );
		virtual void dragEnterEvent(QDragEnterEvent* event);
		virtual void dragMoveEvent(QDragMoveEvent* event);
		virtual void dropEvent(QDropEvent* event);

		virtual bool event( QEvent* event );

		osg::GraphicsContext::Traits* CreateTraits( osg::Referenced* pInheritedWindowData, int nX, int nY, int nW, int nH );

	private:
		QTimer										m_timer;

		std::string									m_strDataPath;

		bool										m_bThread;

		CInitThread*								m_pInitThread;

		osg::ref_ptr<FeShell::CSystemManager>		m_rpSystemManager;
	};
}


#endif // TD_SCENE_PANEL_H
