/**************************************************************************************************
* @file 3DSceneWidget.h
* @note 该文件用于定义三维渲染窗口的初始化工作
* @author c00005
* @data 2017-2-24
**************************************************************************************************/
#ifndef THREE_SCENE_WIDGET_H
#define THREE_SCENE_WIDGET_H 1

#include <QTimer>
#include <QWidget>
#include <QThread>
#include <QGridLayout>
#include <osgQt/GraphicsWindowQt>
#include <FeShell/SystemManager.h>

#include <FeEarth/Export.h>

namespace FeEarth
{
	/**
	  * @class CInitThread
	  * @brief 三维初始化线程
	  * @note 三维初始化线程，用于创建子线程初始化三维，初始化完成后会发送信号
	  * @author c00005
	*/
	class CInitThread: public QThread
	{
		Q_OBJECT;

	public:
		/**  
		* @brief 三维初始化线程构造函数
		* @param pSystemManager [in] 三维渲染管理器
		*/
		CInitThread(FeShell::CSystemManager* pSystemManager);

		/**  
		  * @brief 析构函数
		*/
		~CInitThread();

	public:
		/**  
		  * @brief 是否初始化成功
		*/
		bool IsInitFinish();

	signals:
		/**  
		  * @brief 初始化完成信号
		*/
		void SignalInitDone(bool);

	protected:
		/**  
		  * @brief 线程运行入口
		*/
		virtual void run();

	protected:
		///三维渲染管理器
		osg::observer_ptr<FeShell::CSystemManager>  m_opSystemManager;

		/// 是否初始化成功
		bool                                        m_bInitialized;
	};

	/**
	* @class C3DSceneWidget
	* @brief 三维渲染窗口
	* @note 用于渲染三维场景的窗口，定义了一个定时器，用于对场景的刷新。
	*		同时包含两种初始化模式，一种为直接初始化（主线程中初始化），另一种为线程初始化，即使用单独线程进行场景初始化，
	*		使用线程初始化时，需要注意线程初始化完成后，会发射初始化完成信号，用户需要响应该信号完成余下的三维窗口初始化工作。
	* @author c00005
	*/
	class FEEARTH_EXPORT C3DSceneWidget : public osgQt::GLWidget
	{
		Q_OBJECT;

	public:
		/**  
		  * @brief 三维渲染窗口构造函数
		  * @note 用于渲染三维场景的窗口，定义了一个定时器，用于对场景的刷新
		  * @param strDataPath [in] 三维场景资源路径
		  * @param bThread [in] 是否使用线程初始化
		*/
		C3DSceneWidget(const std::string& strDataPath = "", bool bThread = false);

	public:
		/**  
		  * @note 初始化，其中为渲染的视口设置了图形上下文  
		*/
		bool Initialize();

		/**  
		  * @note 反初始化，卸载时使用
		*/
		bool UnInitialize();

	public:
		/**  
		  * @note 获取三维渲染管理器
		*/
		FeShell::CSystemManager* GetSystemManager();
		
		/**
		*@note: 设置用户earth文件的接口，优先使用该路径
		*/
		void SetEarthPath(std::string strPath);

	signals:
		/**  
		  * @note 三维线程初始化完毕时发射的信号  
		*/
		void Signal3DSceneInitDone(bool);

		/**  
		  * @note 鼠标拖拽释放时发射的信号
		*/
		void SignalDropEvent(QObject *pWidget, osg::Vec3d llh);

			/**  
		  * @note 鼠标拖拽释放时发射的信号
		*/
		void SignalDropEvent(QDropEvent*);

		/**  
		  * @brief 拖拽进入事件信号
		*/
		void SignalDragEnterEvent(QDragEnterEvent*);


	protected slots:
		/**  
		  * @note 初始化完成的槽函数
		*/
		void SlotInitDone(bool);

	protected:
		/**  
		  * @note 安装渲染定时器
		*/
		void InstallFrameTimer();

		osg::GraphicsContext::Traits* CreateTraits( osg::Referenced* pInheritedWindowData, int nX, int nY, int nW, int nH );

	protected:
		/**  
		  * @note Qt窗口事件处理
		*/
		virtual void paintEvent( QPaintEvent* );
		virtual void resizeEvent( QResizeEvent* event );
		virtual void dragEnterEvent(QDragEnterEvent* event);
		virtual void dragMoveEvent(QDragMoveEvent* event);
		virtual void dropEvent(QDropEvent* event);

		virtual bool event( QEvent* event );

	private:
		/// 定时器对象
		QTimer										m_timer;

		/// 是否启用线程初始化
		bool										m_bThread;

		/// 线程初始化对象
		CInitThread*								m_pInitThread;

		/// 数据路径
		std::string									m_strDataPath;

		/// 系统管理接口
		osg::ref_ptr<FeShell::CSystemManager>		m_rpSystemManager;

		///用户earth文件路径
		std::string									m_strEarthPath;
	};
}


#endif // THREE_SCENE_WIDGET_H 
