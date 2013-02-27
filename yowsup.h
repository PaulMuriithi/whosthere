#ifndef YOWSUP_H
#define YOWSUP_H

#include <QQuickItem>
#include "yowsup_signals.h"
#include "yowsup_methods.h"

class YowSup : public QQuickItem
{
    Q_OBJECT
public:
    explicit YowSup(QQuickItem *parent = 0);
    yowsup_signals* ys;
    yowsup_methods* ym;

signals:
    void authFail(const QString& username, const QString& reason);
    void authSuccess(const QString& username);
public slots:
    void login(QString username, QString password);
};

#endif // YOWSUP_H
