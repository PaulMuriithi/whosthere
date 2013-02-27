#include <QDebug>
#include "yowsup_main.h"
#include "yowsup.h"

YowSup::YowSup(QQuickItem *parent) :
    QQuickItem(parent), ys(0), ym(0)
{
}

void YowSup::login(QString username, QString password) {

    if(ym || ys) {
        emit authFail(username, "Already connected");
        return;
    }

    yowsup_main* yb = new yowsup_main("com.yowsup.methods", "/com/yowsup/methods", QDBusConnection::sessionBus(), parent());
    if( !yb->isValid() ) {
        emit authFail(username, QString("Dbus error:") + yb->lastError().message());
        return;
    }
    yb->init("0").waitForFinished();
    delete yb;

    ym = new yowsup_methods("com.yowsup.methods", "/com/yowsup/0/methods", QDBusConnection::sessionBus(), parent());
    if( !ym->isValid() ) {
        emit authFail(username, QString("Dbus error:") + ym->lastError().message());
        return;
    }
    ys = new yowsup_signals("com.yowsup.signals", "/com/yowsup/0/signals", QDBusConnection::sessionBus(), parent());
    if( !ys->isValid() ) {
        emit authFail(username, QString("Dbus error:") + ys->lastError().message());
        return;
    }

    connect(ys, SIGNAL(auth_fail(const QString&, const QString&)), this, SIGNAL(authFail(const QString&, const QString&)));
    connect(ys, SIGNAL(auth_success(const QString&)), this, SIGNAL(authSuccess(const QString&)));

    qDebug() << "YowSup login";
    username = "14155280162";
    password = "rhOQgh1kt4oVjn9zp8QUZGUUCw4=";
    QDBusPendingReply<> r = ym->auth_login(username, password);
    r.waitForFinished();
    if( r.isError() ) {
        emit authFail(username, QString("Dbus error on auth_login:") + r.error().message());
        return;
    }
}
