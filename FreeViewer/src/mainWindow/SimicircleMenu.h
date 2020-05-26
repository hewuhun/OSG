/**************************************************************************************************
* @file SimicircleMenu.h
* @note 半圆形菜单
* @author w00040
* @data 2017-2-22
**************************************************************************************************/
#ifndef SIMICIRCL_EMENU_H
#define SIMICIRCL_EMENU_H

#include <QtGui>
#include <QGraphicsWidget>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

namespace FreeViewer
{
	/**
	  * @class BaseAction
	  * @brief 菜单项基类
	  * @note 实现选中 发单击消息 设置角度 等基本功能
	  * @author w00040
	*/
	class BaseAction : public QGraphicsWidget
	{
	    Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
	    explicit BaseAction(const QString &strIconPath, bool bSelectable = false, QGraphicsItem *parent = 0);
	
		/**  
		  * @brief 获取hover状态
		*/
	    bool IsHovering() const { return m_bHovering; }
	
		/**  
		  * @brief 获取是否可选中状态
		*/
	    bool IsSelectable() const { return m_bSelectable; }
	
		/**  
		  * @brief 获取是否选中状态
		*/
	    bool IsSelected() const { return m_bSelected; }
	
		/**  
		  * @brief 设置选中状态
		*/
	    void SetSelected(bool bSelected);
	
		/**  
		  * @brief 获取item所在的角度
		*/
	    double Angle() const { return m_dAngle; }
	
		/**  
		  * @brief 设置item所在的角度
		*/
	    void SetAngle(double dAngle);
	
	protected:
		/**  
		  * @brief 鼠标点击事件
		*/
	    void mousePressEvent(QGraphicsSceneMouseEvent *);
	
		/**  
		  * @brief 鼠标释放事件
		*/
	    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
	
		/**  
		  * @brief 鼠标悬浮进入事件
		*/
	    void hoverEnterEvent(QGraphicsSceneHoverEvent *);
	
		/**  
		  * @brief 鼠标悬浮离开事件
		*/
	    void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
	
	signals:
		/**  
		  * @brief 鼠标点击信号
		*/
	    void ClickedSignal();
	
		/**  
		  * @brief 鼠标悬浮进入信号
		*/
	    void HoverEnterSignal();
	
		/**  
		  * @brief 鼠标悬浮离开信号
		*/
	    void HoverLeaveSignal();
	
	protected:
	    //图标路径
	    QString m_strIconPath;
	    // 是否可选中
	    bool m_bSelectable;
	    // 是否选中
	    bool m_bSelected;
	    // 是否hover状态
	    bool m_bHovering;
	    // item所在的角度
	    double m_dAngle;
	
	    // 四态背景图片，背景和图标分开的，图标作为参数传进来
	    QPixmap m_normalPix;
	    QPixmap m_hoverPix;
	    QPixmap m_pressedPix;
	    QPixmap m_selectedPix;
	};
	
	/**
	  * @class LeafAction
	  * @brief 四周叶子
	  * @note 绘制item
	  * @author w00040
	*/
	class LeafAction : public BaseAction
	{
	    Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
	    LeafAction(const QString &strIconPath, bool bSelectable = true, QGraphicsItem *parent = 0);
	
		/**  
		  * @brief 返回整个item可见矩形区域
		*/
	    QRectF boundingRect() const { return QRectF(-52, -104, 104, 74); }
	
		/**  
		  * @brief 返回item可见形状路径，用于响应鼠标事件
		*/
	    QPainterPath shape() const;
	
		/**  
		  * @brief 绘制item
		*/
	    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	};
	
	/**
	  * @class SimicircleAction
	  * @brief 半圆
	  * @note 点击中心圆能实现锁住动画
	  * @author w00040
	*/
	class SimicircleAction : public BaseAction
	{
	    Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
	    SimicircleAction(const QString &strIconName, QGraphicsItem *parent = 0);
	
		/**  
		  * @brief 返回整个item可见矩形区域
		*/
	    QRectF boundingRect() const { return QRectF(-34, -34, 68, 34); }
	
		/**  
		  * @brief 返回item可见形状路径，用于响应鼠标事件
		*/
	    QPainterPath shape() const;
	
		/**  
		  * @brief 绘制item
		*/
	    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	    /**  
	      * @brief 获取中心圆锁动状态
	    */
	    bool IsLocked() const { return m_bLocked; }
	
		/**  
		  * @brief 点击中心圆能实现锁住动画
		*/
	    void SetLocked(bool bLocked);
	
	private:
		//中心圆锁动状态
	    bool m_bLocked;
	};
	
	/**
	  * @class ActionGroup
	  * @brief 类似 QButtonGroup
	  * @note 用于按钮互斥选中
	  * @author w00040
	*/
	class ActionGroup : public QObject
	{
	    Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
	    ActionGroup(QObject *parent = 0);
	
		/**  
		  * @brief 添加 Action
		*/
	    void AddAction(BaseAction *pAction, int id);
	
		/**  
		  * @brief 获取 Action
		*/
	    BaseAction *GetAction(int id);
	
	signals:
		/**  
		  * @brief Action信号
		*/
	    void ActionClickedSignal(BaseAction *pAction);
	
	    void ActionClickedSignal(int id);
	
	public slots:
		/**  
		  * @brief Action槽
		*/
	    void ActionClickedSlot();
	
	private:
	    QMap<int, BaseAction*> m_actionIdMap;
	
	};
	
	/**
	  * @class SimicircleMenu
	  * @brief 半圆形菜单
	  * @note 创建半圆形菜单
	  * @author w00040
	*/
	class SimicircleMenu : public QGraphicsWidget
	{
	    Q_OBJECT
	public:
		/**  
		  * @brief 构造函数
		*/
	    explicit SimicircleMenu(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
	
		/**  
		  * @brief 返回整个item可见矩形区域
		*/
	    QRectF boundingRect() const { return QRectF(-108, -108, 216, 108); }
	
		/**  
		  * @brief 返回item可见形状路径，用于响应鼠标事件
		*/
	    QPainterPath shape() const;
	
	protected:
		/**  
		  * @brief 鼠标悬浮离开事件
		*/
	    void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
	
	private:
		/**  
		  * @brief 创建菜单
		*/
	    void CreateMenu();
	
		/**  
		  * @brief 创建中心圆动画
		*/
	    void CreateAnimation();
	
	signals:
		/**  
		  * @brief Action信号
		*/
	    void ActionClickedSignal(int id);
	
	public slots:
		/**  
		  * @brief 是否锁定槽函数
		*/
	    void LockSlot();
	
		/**  
		  * @brief 展开槽函数
		*/
	    void ZoomInSlot();
	
		/**  
		  * @brief 合起槽函数
		*/
	    void ZoomOutSlot();
	
	private:
	    QList<LeafAction*> m_leatActions;
	    SimicircleAction *m_pSimicircleAction;
	
	    QSequentialAnimationGroup *m_pZoomGroup;
	    bool m_bLocked;
	
	};
}

#endif // SIMICIRCLEMENU_H
