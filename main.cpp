/*
 * Copyright (C) 2013 Matthias Gehre <gehre.matthias@gmail.com>
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QDebug>

#include <TelepathyQt/Debug>
#include <TelepathyQt/Types>

#include "whosthere.h"
#include "imageprovider.h"

QQuickView* viewer;

int main(int argc, char ** argv)
{
    QGuiApplication app(argc, argv);

    Tp::registerTypes();
    Tp::enableDebug(true);
    Tp::enableWarnings(true);

    qmlRegisterType<WhosThere>("WhosThere", 1,0, "WhosThere");

    viewer = new QQuickView();
    viewer->engine()->addImportPath(":");
    viewer->engine()->addImageProvider("drawable", new ImageProvider);
    viewer->setResizeMode( QQuickView::SizeRootObjectToView );
    viewer->setSource(QUrl("qrc:whosthere.qml"));
    viewer->show();

    return app.exec();
}
