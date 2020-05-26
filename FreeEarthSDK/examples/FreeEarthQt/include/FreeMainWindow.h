#ifndef FREE_MAIN_WINDOW_H
#define FREE_MAIN_WINDOW_H 1

#include <QAction>
#include <QEvent>
#include <QDockWidget>
#include <QMainWindow>
#include <QMetaObject>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QTranslator>
#include <QSplashScreen>
#include <QTextEdit>
#include <QWidget>
#include <FeShell/SystemSerializer.h>

#include <FeEarth/3DSceneWidget.h>

/**
* @class CFreeMainWindow
* @brief Qt主窗口
* @note Qt主窗口，渲染窗口以及所有的控件都是通过该窗口进行显示的
*/
class CFreeMainWindow : public QMainWindow
{
	Q_OBJECT
public:

	CFreeMainWindow(const std::string& strDataPath = "", QWidget *parent = 0, Qt::WindowFlags flags = 0);

	~CFreeMainWindow();

	/**  
	* @note 系统初始化
	*/
	bool Initialize();

	/**  
	* @note 系统反初始化
	*/
	bool UnInitialize();


	bool eventFilter(QObject *obj, QEvent *event);

private:
	bool InitializeUI();

private:
	FeEarth::C3DSceneWidget*							m_p3DScenePanel;
};


#endif 
