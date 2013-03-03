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
    QByteArray data = QByteArray::fromBase64(ret.toByteArray());
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
