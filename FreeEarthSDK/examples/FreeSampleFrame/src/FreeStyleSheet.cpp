#include <FreeStyleSheet.h>

QString GetMiniPushBtnStyle()
{
	return QString("\
		QPushButton{min-width:30;max-width:30;min-height:22;max-height:22;border-image: url(:/images/mini_normal.png);}\
		QPushButton:hover{min-width:30;max-width:30;min-height:22;max-height:22;border-image: url(:/images/mini_hover.png);}\
		QPushButton:pressed{min-width:30;max-width:30;min-height:22;max-height:22;border-image: url(:/images/mini_press.png);}");
}

QString GetNormalPushBtnStyle()
{
	return QString("\
		QPushButton{min-width:30;max-width:30;min-height:22;max-height:22;border-image: url(:/images/normal_normal.png);}\
		QPushButton:hover{min-width:30;max-width:30;min-height:22;max-height:22;border-image: url(:/images/normal_hover.png);}\
		QPushButton:pressed{min-width:30;max-width:30;min-height:22;max-height:22;border-image: url(:/images/normal_press.png);}");
}

QString GetClosePushBtnStyle()
{
	return QString("\
		QPushButton{min-width:30;max-width:30;min-height:22;max-height:22;border-image: url(:/images/close_normal.png);}\
		QPushButton:hover{min-width:30;max-width:30;min-height:22;max-height:22;border-image: url(:/images/close_hover.png);}\
		QPushButton:pressed{min-width:30;max-width:30;min-height:22;max-height:22;border-image: url(:/images/close_press.png);}");
}

extern QString GetTitleLabelStyle()
{
	return QString("QLabel{color:rgba(0,198,255,255); font-family:\"Microsoft YaHei\"; font-size:24px; padding-left:90px;}");
}

extern QString GetTitleFrameStyle()
{
	return QString("QFrame{border-image:url(:/images/menu_background.png);}");
}

extern QString GetMainWindowStyle()
{
	return QString("QFrame{border-image:url(:/images/background.png);}");
}

extern QString GetControlWidgetStyle()
{
	return QString("\
			QWidget{font:1spt \"微软雅黑\";}\
            QFrame#FrameControlWidget{border-image:url(:/images/dialog_background.png)}\
            QPushButton { height:20px; color:white; padding:0px 20px 0px 20px; border:  1px solid rgb(30,89,98); background-color: rgba(13, 43, 48, 50);}\
            QPushButton:hover{ padding:0px 20px 0px 20px; border:  1px solid rgb(45,183,213); background-color: rgba(30, 89, 98, 200);}\
            QPushButton:pressed { padding:0px 20px 0px 20px; border:  1px solid rgb(45,183,213); background-color: rgba(30, 89, 98, 200);}\
            QGroupBox { background: transparent; border: 1px solid rgb(44,80,114); color: rgb(249,133,0); padding-top:5px; margin-top: 2.5ex;}\
            QGroupBox::title { subcontrol-origin: margin;subcontrol-position: top left; left:10px; margin-left: 0px; padding:0 1px;}\
            QSlider:horizontal { background: transparent; border: none;}\
            QSlider::groove:horizontal { border: none;  height: 2px; margin: 1px 0;}\
            QSlider::handle:horizontal { background-color: qlineargradient(spread:reflect, x1:0, y1:0, x2:0, y2:0.499636, stop:0 rgba(255, 255, 255, 255), stop:1 rgba(110, 110, 110, 255)); width: 4px; margin: -8px 0;}\
            QSlider::add-page:horizontal{ background: rgb(242, 154, 118);}\
            QSlider::sub-page:horizontal{ background: rgb(0, 183, 238);}\
            QLabel{border-image:url();color:white; background: transparent; border: none;}\
            QRadioButton {color:white; background: transparent; border: none;}\
            QRadioButton::indicator { width: 15px; height: 15px;}\
            QRadioButton::indicator::unchecked { image: url(:/images/radioBtn_unchecked.png);}\
            QRadioButton::indicator:unchecked:hover { image: url(:/images/radioBtn_unchecked_hover.png);}\
            QRadioButton::indicator:unchecked:pressed { image: url(:/images/radioBtn_unchecked_pressed.png);}\
            QRadioButton::indicator::checked { image: url(:/images/radioBtn_checked.png);}\
            QRadioButton::indicator:checked:hover { image: url(:/images/radioBtn_checked_hover.png);}\
            QRadioButton::indicator:checked:pressed { image: url(:/images/radioBtn_checked_pressed.png);}\
            QCheckBox { color:white; spacing: 5px; background: transparent; border: none;}\
            QCheckBox::indicator { width: 13px; height: 13px;}\
            QCheckBox::indicator:unchecked { image: url(:/images/checkBox_unchecked.png);}\
            QCheckBox::indicator:unchecked:hover { image: url(:/images/checkBox_unchecked_hover.png);}\
            QCheckBox::indicator:unchecked:pressed { image: url(:/images/checkBox_unchecked_pressed.png);}\
            QCheckBox::indicator:checked { image: url(:/images/checkBox_checked.png);}\
            QCheckBox::indicator:checked:hover { image: url(:/images/checkBox_checked_hover.png);}\
            QCheckBox::indicator:checked:pressed { image: url(:/images/checkBox_checked_pressed.png);}\
            QSpinBox {color:white; background: transparent; border: 1px solid rgb(44,80,114); max-height: 22px; min-height: 22px;}\
            QSpinBox::up-button { width: 19px; height: 10px; border-left: 1px solid rgb(44,80,114); border-bottom: 1px solid rgb(44,80,114); background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 rgba(28, 63, 93, 255), stop:1 rgba(39, 125, 212, 255));}\
            QSpinBox::up-arrow { border-image: url(:/images/spinBox_up.png);}\
            QSpinBox::up-arrow:hover { border-image: url(:/images/spinBox_up_hover.png);}\
            QSpinBox::up-arrow:pressed { border-image: url(:/images/spinBox_up_pressed.png);}\
            QSpinBox::down-button { width: 19px; height: 10px; border-left: 1px solid rgb(44,80,114); border-top: 1px solid rgb(44,80,114); background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 rgba(28, 63, 93, 255), stop:1 rgba(39, 125, 212, 255));}\
            QSpinBox::down-arrow { border-image: url(:/images/spinBox_down.png);}\
            QSpinBox::down-arrow:hover { border-image: url(:/images/spinBox_down_hover.png);}\
            QSpinBox::down-arrow:pressed { border-image: url(:/images/spinBox_down_pressed.png);}\
            QDoubleSpinBox {color:white; background: transparent; border: 1px solid rgb(44,80,114); max-height: 22px; min-height: 22px;}\
            QDoubleSpinBox::up-button { width: 19px; height: 10px; border-left: 1px solid rgb(44,80,114); border-bottom: 1px solid rgb(44,80,114); background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 rgba(28, 63, 93, 255), stop:1 rgba(39, 125, 212, 255));}\
            QDoubleSpinBox::up-arrow { border-image: url(:/images/spinBox_up.png);}\
            QDoubleSpinBox::up-arrow:hover { border-image: url(:/images/spinBox_up_hover.png);}\
            QDoubleSpinBox::up-arrow:pressed { border-image: url(:/images/spinBox_up_pressed.png);}\
            QDoubleSpinBox::down-button { width: 19px; height: 10px; border-left: 1px solid rgb(44,80,114); border-top: 1px solid rgb(44,80,114); background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 rgba(28, 63, 93, 255), stop:1 rgba(39, 125, 212, 255));}\
            QDoubleSpinBox::down-arrow { border-image: url(:/images/spinBox_down.png);}\
            QDoubleSpinBox::down-arrow:hover { border-image: url(:/images/spinBox_down_hover.png);}\
            QDoubleSpinBox::down-arrow:pressed { border-image: url(:/images/spinBox_down_pressed.png);}\
            QComboBox {color:white;border: 1px solid rgb(60,154,168); background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 rgba(26, 104, 115, 0), stop:1 rgba(74, 188, 205, 0));}\
            QComboBox::drop-down {background: transparent; border: none; subcontrol-origin: padding; subcontrol-position: top right; width: 20px; padding-right: 2px; padding-bottom: 2px;}\
            QComboBox::down-arrow {image: url(:/images/down.png);}\
            QComboBox:editable { background: transparent; background-color:rgb(26, 104, 115);}\
            QScrollBar {background: transparent; border: 1px solid rgb(60,154,168); }\
            QScrollBar::add-page, QScrollBar::sub-page { background: transparent;}\
            QScrollBar::add-line, QScrollBar::sub-line { border: 1px solid rgb(60,154,168); background: rgb(26, 104, 115); width: 10px; height: 10px; subcontrol-origin: margin; }\
            QScrollBar:vertical { width: 12px; margin: 11px 0px; }\
            QScrollBar::handle:vertical { width: 10px; border: 1px solid rgb(60,154,168);}\
            QScrollBar::handle:vertical { background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:1, y2:1, stop:0 rgba(26, 104, 115, 0), stop:1 rgba(74, 188, 205, 0));}\
            QScrollBar::add-line:vertical { subcontrol-position: bottom;}\
            QScrollBar::sub-line:vertical { subcontrol-position: top;}\
            QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical { width: 8px; height: 6px;}\
            QScrollBar::up-arrow:vertical { image: url(:/images/up.png);}\
            QScrollBar::up-arrow:vertical:hover { image: url(:/images/up_hover.png);}\
            QScrollBar::up-arrow:vertical:pressed { image: url(:/images/up_pressed.png);}\
            QScrollBar::down-arrow:vertical { image: url(:/images/down.png);}\
            QScrollBar::down-arrow:vertical:hover { image: url(:/images/down_hover.png);}\
            QScrollBar::down-arrow:vertical:pressed { image: url(:/images/down_pressed.png);}\
            QAbstractScrollArea{color:white;border-image:url(:/images/combox_background.png);background: transparent;border: 1px solid rgb(60,154,168); }\
			");
}

extern QString GetLabelTitleStyle()
{
	return QString("QLabel{border-image:url();color:rgb(255, 170, 0);font: 75 20pt \"华文楷体\";}");
}

extern QString GetLabelContentStyle()
{
	return QString("QLabel{border-image:url();color: rgb(255, 255, 255);font:10pt \"微软雅黑\";}");
}
