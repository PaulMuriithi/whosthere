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

#include <QtQuick/QQuickView>

#include "imageprovider.h"
#include "whosthere.h"

extern QQuickView* viewer;
ImageProvider::ImageProvider() :
    QQuickImageProvider(QQmlImageProviderBase::Image)
{
}

QImage ImageProvider::requestImage(const QString & id, QSize * size, const QSize & requestedSize) {

    QVariant ret;
    QMetaObject::invokeMethod(viewer->rootObject(), "getPreviewImage",
            Q_RETURN_ARG(QVariant, ret),
            Q_ARG(QVariant, id));
    QByteArray data = ret.toByteArray();
    QImage image = QImage::fromData(data);
    qDebug() << "ImageProvider::requestImage(): rawImage, size: " << image.size() << " requested: " << requestedSize;
    if(requestedSize == QSize(-1,-1))
        return image;
    else {
        QImage scaled = image.scaled(requestedSize, Qt::KeepAspectRatio);
        qDebug() << "scaledImage, size: " << scaled.size();
        *size = scaled.size();
        return scaled;
    }
}
