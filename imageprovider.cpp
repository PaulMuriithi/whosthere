#include "imageprovider.h"
#include "whosthere.h"

ImageProvider::ImageProvider() :
    QQuickImageProvider(QQmlImageProviderBase::Image)
{
}

QImage ImageProvider::requestImage(const QString & id, QSize * size, const QSize & requestedSize) {

    WhosThere* whosthere = WhosThere::get();
    QByteArray data = QByteArray::fromBase64(whosthere->getPreviewImage(id));
    QImage image = QImage::fromData(data);
    *size = image.size();
    return image;
}
