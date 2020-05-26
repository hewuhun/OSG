#include "SimicircleMenu.h"
#include <QTransform>

namespace FreeViewer
{
    BaseAction::BaseAction(const QString &strIconPath, bool bSelectable, QGraphicsItem *parent) :
    QGraphicsWidget(parent), m_strIconPath(strIconPath), m_bSelectable(bSelectable), m_bSelected(false)
    {
        //要设置接收鼠标悬浮事件
        setAcceptHoverEvents(true);
        setCacheMode(DeviceCoordinateCache);
    }


    void BaseAction::SetSelected(bool bSelected)
    {
        m_bSelected = bSelected;

        //调用paint，更新状态
        update();
    }

    void BaseAction::SetAngle(double dAngle)
    {
        m_dAngle = dAngle;
		
#ifdef QT4_VERSION
        rotate(m_dAngle);
#else
		this->setTransform(QTransform().rotate(m_dAngle));
#endif

        update();
    }

    void BaseAction::mousePressEvent(QGraphicsSceneMouseEvent *)
    {
        update();
    }

    void BaseAction::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
    {
        emit ClickedSignal();

        update();
    }

    void BaseAction::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
    {
        QGraphicsWidget::hoverEnterEvent(e);

        m_bHovering = true;

        emit HoverEnterSignal();

        update();
    }

    void BaseAction::hoverLeaveEvent(QGraphicsSceneHoverEvent *e)
    {
        QGraphicsWidget::hoverLeaveEvent(e);

        m_bHovering = false;

        emit HoverLeaveSignal();

        update();
    }


    LeafAction::LeafAction(const QString &strIconPath, bool bSelectable, QGraphicsItem *parent) :
    BaseAction(strIconPath, bSelectable, parent)
    {
        m_normalPix = QPixmap(":/images/2D3Dmenu/normal.png");
        m_hoverPix = QPixmap(":/images/2D3Dmenu/hover.png");
        m_pressedPix = QPixmap(":/images/2D3Dmenu/pressed.png");
        m_selectedPix = QPixmap(":/images/2D3Dmenu/selected.png");
    }

    QPainterPath LeafAction::shape() const
    {
        QPainterPath path;

        //这个矩形是外圆弧的外接矩形
        QRectF rectfOut(-104, -80, 208, 208);
        path.arcMoveTo(rectfOut, 67.5);
        //画外圆弧
        path.arcTo(rectfOut, 67.5, 45);
        //内圆弧的外接矩形
        QRectF rectfInner(-34, -34, 68, 68);
        //画内圆弧
        path.arcTo(rectfInner, 112.5, -45);

        //封闭内外圆弧
        path.closeSubpath();

        return path;
    }

    void LeafAction::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(widget);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRectF rect = boundingRect();

        bool bPressed = option->state & QStyle::State_Sunken;   //否按下
        bool bHover = option->state & QStyle::State_MouseOver;  //否hover

        //选择背景四态图片，如果有这么多态的话
        QPixmap bgPix = m_normalPix;
        QString strIconName = m_strIconPath + QString("normalhover.png");

        if (m_bSelected)
        {
            bgPix = m_selectedPix;
            strIconName = m_strIconPath + QString("selected.png");
        }
        else if (bPressed)
        {
            bgPix = m_pressedPix;
            strIconName = m_strIconPath + QString("pressed.png");
        }
        else if (bHover)
        {
            bgPix = m_hoverPix;
        }

        //画背景扇形图
        painter->drawPixmap(rect.x(), rect.y(), bgPix);

        //图标要垂直的所以要让画笔反转一个扇形叶角度
        painter->translate(0, rect.height() / 2 + rect.y());
        painter->rotate(-m_dAngle);
        QPixmap iconPix(strIconName);
        //画图标
        painter->drawPixmap(-iconPix.width() / 2, -iconPix.height() / 2, iconPix);
        painter->restore();
    }


    SimicircleAction::SimicircleAction(const QString &strIconName, QGraphicsItem *parent) :
    BaseAction(strIconName, false, parent)
    {
        m_normalPix = QPixmap(":/images/2D3Dmenu/semicircle_normal.png");
        m_hoverPix = QPixmap(":/images/2D3Dmenu/semicircle_hover.png");
        m_pressedPix = QPixmap(":/images/2D3Dmenu/semicircle_pressed.png");
    }

    QPainterPath SimicircleAction::shape() const
    {
        QPainterPath path;

        path.moveTo(34, 0);
        path.arcTo(-34, -34, 68, 68, 0, 180);

        path.closeSubpath();

        return path;
    }

    void SimicircleAction::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(widget);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRectF rect = boundingRect();

        bool bPressed = option->state & QStyle::State_Sunken;   //是否按下
        bool bHover = option->state & QStyle::State_MouseOver;  //是否hover

        //选择背景三态图片，如果有这么多态的话
        QPixmap bgPix = m_normalPix;
        QString strIconName = m_strIconPath + QString(m_bLocked ? "lock_" : "unlock_") + QString("normalpressed.png");

        if (bPressed)
        {
            bgPix = m_pressedPix;
            strIconName = m_strIconPath + QString(m_bLocked ? "lock_" : "unlock_") + QString("normalpressed.png");
        }
        else if (bHover)
        {
            bgPix = m_hoverPix;
            strIconName = m_strIconPath + QString(m_bLocked ? "lock_" : "unlock_") + QString("hover.png");
        }

        //画背景圆
        painter->drawPixmap(rect.x(), rect.y(), bgPix);

        QPixmap iconPix(strIconName);

        //画图标
        painter->translate(0, rect.height() / 2 + rect.y());
        painter->drawPixmap(-iconPix.width() / 2, -iconPix.height() / 2, iconPix);
        painter->restore();
    }

    void SimicircleAction::SetLocked(bool bLocked)
    {
        m_bLocked = bLocked;

        setAcceptHoverEvents(!m_bLocked);

        update();
    }


    ActionGroup::ActionGroup(QObject *parent) :
    QObject(parent)
    {

    }

    void ActionGroup::AddAction(BaseAction *pAction, int id)
    {
        if (pAction != NULL)
        {
            m_actionIdMap.insert(id, pAction);

            pAction->setProperty("id", id);
            connect(pAction, SIGNAL(ClickedSignal()), this, SLOT(ActionClickedSlot()));
        }
    }

    BaseAction *ActionGroup::GetAction(int id)
    {
        if (m_actionIdMap.contains(id))
        {
            return m_actionIdMap.value(id);
        }
        else
        {
            return NULL;
        }
    }

    void ActionGroup::ActionClickedSlot()
    {
        BaseAction *pAction = qobject_cast<BaseAction*>(sender());
        int id = pAction->property("id").toInt();

        emit ActionClickedSignal(pAction);
        emit ActionClickedSignal(id);

        if (!pAction->IsSelectable())
        {
            return;
        }

        //找出选中的设置不选中
        for (QMap<int, BaseAction*>::iterator iter = m_actionIdMap.begin(); iter != m_actionIdMap.end(); ++iter)
        {
            if (iter.value() != pAction && iter.value()->IsSelected())
            {
                iter.value()->SetSelected(false);

                break;
            }
        }

        //设置当前选中
        pAction->SetSelected(true);
    }

    SimicircleMenu::SimicircleMenu(QGraphicsItem *parent, QGraphicsScene *scene) :
    QGraphicsWidget(parent), m_bLocked(false)
    {
        setAcceptHoverEvents(true);

        if (scene != NULL)
        {
            scene->addItem(this);
            setPos(0, 0);
            setZValue(1000);
        }

        CreateMenu();
        CreateAnimation();
    }

    QPainterPath SimicircleMenu::shape() const
    {
        QPainterPath path;

        path.moveTo(108, 0);
        path.arcTo(-108, -108, 216, 216, 0, 180);
        path.closeSubpath();

        return path;
    }

    //鼠标离开整个区域时合起，如果没有锁住的话
    void SimicircleMenu::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    {
        if (!m_pSimicircleAction->IsLocked())
        {
            ZoomOutSlot();
        }
    }

    void SimicircleMenu::CreateMenu()
    {
        m_pSimicircleAction = new SimicircleAction(QString(":/images/2D3Dmenu/"), this);
        m_pSimicircleAction->SetLocked(false);

        connect(m_pSimicircleAction, SIGNAL(ClickedSignal()), this, SLOT(LockSlot()));
        connect(m_pSimicircleAction, SIGNAL(HoverEnterSignal()), this, SLOT(ZoomInSlot()));

        ActionGroup *pGroup = new ActionGroup;
        //可以把叶子id作为命令信号发出
        connect(pGroup, SIGNAL(ActionClickedSignal(int)), this, SIGNAL(ActionClickedSignal(int)));

        QStringList icons = QStringList() << "3D_" << "2D3D_" << "2D_";

        for (int i = 0; i < 3; ++i)
        {
            LeafAction *pLeaf = new LeafAction(QString(":/images/2D3Dmenu/%1").arg(icons.at(i)), true, this);
            pLeaf->SetAngle(-60 + i * 60);
            m_leatActions.append(pLeaf);
            pGroup->AddAction(pLeaf, i);

            if (i == 0)
            {
                pLeaf->SetSelected(true);
            }
        }
    }

    void SimicircleMenu::CreateAnimation()
    {
        m_pZoomGroup = new QSequentialAnimationGroup;

        QParallelAnimationGroup *pScaleGroup = new QParallelAnimationGroup;

        //设置中心圆缩放动画
        QPropertyAnimation *pZoomAnimation = new QPropertyAnimation(m_pSimicircleAction, "scale");
        pZoomAnimation->setStartValue(0.5);
        pZoomAnimation->setEndValue(1);
        pZoomAnimation->setDuration(250);
        pScaleGroup->addAnimation(pZoomAnimation);

        //设置叶子缩放透明动画
        for (int i = 0; i < m_leatActions.count(); ++i)
        {
            QPropertyAnimation *pScaleAnimation = new QPropertyAnimation(m_leatActions[i], "scale");
            pScaleAnimation->setStartValue(0.5);
            pScaleAnimation->setEndValue(1);
            pScaleAnimation->setDuration(250);
            pScaleGroup->addAnimation(pScaleAnimation);

            QPropertyAnimation *pOpacityAnimation = new QPropertyAnimation(m_leatActions[i], "opacity");
            pOpacityAnimation->setStartValue(0);
            pOpacityAnimation->setEndValue(1);
            pOpacityAnimation->setDuration(250);
            pScaleGroup->addAnimation(pOpacityAnimation);
        }

        m_pZoomGroup->addAnimation(pScaleGroup);

        //设置叶子展合动画
        QParallelAnimationGroup *pExpandGroup = new QParallelAnimationGroup;

        //第1、3叶子往 2上展合
        QPropertyAnimation *pExpandAnimation = new QPropertyAnimation(m_leatActions[0], "rotation");
        pExpandAnimation->setStartValue(60);
        pExpandAnimation->setEndValue(0);
        pExpandAnimation->setDuration(250);
        pExpandGroup->addAnimation(pExpandAnimation);
        pExpandAnimation = new QPropertyAnimation(m_leatActions[2], "rotation");
        pExpandAnimation->setStartValue(-60);
        pExpandAnimation->setEndValue(0);
        pExpandAnimation->setDuration(250);
        pExpandGroup->addAnimation(pExpandAnimation);

        m_pZoomGroup->addAnimation(pExpandGroup);

        m_pZoomGroup->setDirection(QAbstractAnimation::Backward);
        m_pZoomGroup->start();
    }

    //锁住
    void SimicircleMenu::LockSlot()
    {
        m_pSimicircleAction->SetLocked(!m_pSimicircleAction->IsLocked());
    }

    //展开
    void SimicircleMenu::ZoomInSlot()
    {
        if (m_pZoomGroup->direction() == QAbstractAnimation::Backward)
        {
            m_pZoomGroup->setDirection(QAbstractAnimation::Forward);
            m_pZoomGroup->start();
        }
    }

    //合起
    void SimicircleMenu::ZoomOutSlot()
    {
        if (m_pZoomGroup->direction() == QAbstractAnimation::Forward)
        {
            m_pZoomGroup->setDirection(QAbstractAnimation::Backward);
            m_pZoomGroup->start();
        }
    }
}
