#include <QLabel>
#include <QPushButton>
#include <QAction>
#include <QLayout>
#include <QMenu>
#include <QCoreApplication>
#include <QCloseEvent>

#include "utils.h"
#include "configwindow.h"
#include "keydialog.h"


ConfigWindow::ConfigWindow(std::map<std::string, QHotkey *> hotkeys, QWidget *parent)
    : QMainWindow(parent), trayIcon(new QSystemTrayIcon(this))
{

    this->hotkeys = hotkeys;
    settings = new QSettings("gazou", "gazou");
    widget = new QWidget(this);
    this->setCentralWidget(widget);

    verticalKeybindLabel = new QLabel(tr("Vertical OCR"));
    horizontalKeybindLabel = new QLabel(tr("Horizontal OCR"));
    verticalKeybindButton = new QPushButton(settings->value("Hotkeys/verticalOCR", "Alt+A").toString(), widget);
    verticalKeybindButton->setObjectName("VertBtn");
    horizontalKeybindButton = new QPushButton(settings->value("Hotkeys/horizontalOCR", "Alt+D").toString(), widget);
    horizontalKeybindButton->setObjectName("HorBtn");

    verticalKeybindLabel->setStyleSheet("margin-right: 5px;");
    horizontalKeybindLabel->setStyleSheet("margin-right: 8px;");
    QGridLayout *mainLayout = new QGridLayout(widget);
    mainLayout->addWidget(verticalKeybindLabel, 0, 0);
    mainLayout->addWidget(verticalKeybindButton, 0, 1);
    mainLayout->addWidget(horizontalKeybindLabel, 1, 0);
    mainLayout->addWidget(horizontalKeybindButton, 1, 1);
    setFixedSize(sizeHint());

    QMenu *menu = this->createMenu();
    this->trayIcon->setContextMenu(menu);

    QIcon appIcon = QIcon(":/tray.png");
    this->trayIcon->setIcon(appIcon);
    this->setWindowIcon(appIcon);

    this->trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, &ConfigWindow::iconActivated);

    connect(verticalKeybindButton, &QPushButton::clicked, this, &ConfigWindow::handleHotkeyButton);
    connect(horizontalKeybindButton, &QPushButton::clicked, this, &ConfigWindow::handleHotkeyButton);
}

QMenu *ConfigWindow::createMenu()
{
    QAction *quitAction = new QAction("&Quit", this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    QMenu *menu = new QMenu(this);
    menu->addAction(quitAction);

    return menu;
}

void ConfigWindow::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_MACOS
    if (!event->spontaneous() || !isVisible())
    {
        return;
    }
#endif
    if (trayIcon->isVisible())
    {
        hide();
        event->ignore();
    }
}

void ConfigWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        show();
        break;
    default:;
    }
}

void ConfigWindow::handleHotkeyButton()
{
    setRegistered(hotkeys, false);
    KeyDialog setKeyDialog;
    auto button = qobject_cast<QPushButton *>(sender());
    if (setKeyDialog.exec() == QDialog::Accepted)
    {
        button->setText(setKeyDialog.getKeySeq());
        QString shortKeyName = button->objectName() == "HorBtn" ? "horizontalOCR" : "verticalOCR";
        QString key = "Hotkeys/";
        key.append(shortKeyName);
        QString value = setKeyDialog.getKeySeq();
        settings->setValue(key, value);
        settings->sync();

        hotkeys[shortKeyName.toStdString()]->setShortcut(value);
    }
    setRegistered(hotkeys, true);
}
