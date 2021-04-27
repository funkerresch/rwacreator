#include "imagemapadapter.h"

namespace qmapcontrol
{
    ImageMapAdapter::ImageMapAdapter(const QString filepath)
        :MapAdapter("", "", 0, 0, 20),
        imageLabel(new QLabel),
        scrollArea(new QScrollArea),
        scaleFactor(1)
    {
        imageLabel->setBackgroundRole(QPalette::Base);
        imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        imageLabel->setScaledContents(true);

        scrollArea->setBackgroundRole(QPalette::Dark);
        scrollArea->setWidget(imageLabel);
        scrollArea->setVisible(false);
        loadFile(filepath);
    }

    ImageMapAdapter::~ImageMapAdapter()
    {
    }

    bool ImageMapAdapter::loadFile(const QString &fileName)
    {
        QImageReader reader(fileName);
        reader.setAutoTransform(true);
        const QImage newImage = reader.read();
        if (newImage.isNull()) {
            qDebug() << "Could not load image";
            return false;
        }

        setImage(newImage);

       // setWindowFilePath(fileName);

        const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
            .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
        //statusBar()->showMessage(message);
        return true;
    }

    void ImageMapAdapter::setImage(const QImage &newImage)
    {
        image = newImage;
        imageLabel->setPixmap(QPixmap::fromImage(image));
        scaleFactor = 1.0;

        scrollArea->setVisible(true);


    }

    static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
    {
        static bool firstDialog = true;

        if (firstDialog) {
            firstDialog = false;
            const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
            dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
        }

        QStringList mimeTypeFilters;
        const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
            ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
        foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
            mimeTypeFilters.append(mimeTypeName);
        mimeTypeFilters.sort();
        dialog.setMimeTypeFilters(mimeTypeFilters);
        dialog.selectMimeTypeFilter("image/jpeg");
        if (acceptMode == QFileDialog::AcceptSave)
            dialog.setDefaultSuffix("jpg");
    }



    void ImageMapAdapter::zoomIn()
    {
        scaleImage(1.25);
    }

    void ImageMapAdapter::zoomOut()
    {
        scaleImage(0.8);
    }

    void ImageMapAdapter::normalSize()
    {
        imageLabel->adjustSize();
        scaleFactor = 1.0;
    }

    void ImageMapAdapter::fitToWindow()
    {


    }

    void ImageMapAdapter::scaleImage(double factor)
    {
        Q_ASSERT(imageLabel->pixmap());
        scaleFactor *= factor;
        imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

        adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
        adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    }

    void ImageMapAdapter::adjustScrollBar(QScrollBar *scrollBar, double factor)
    {
        scrollBar->setValue(int(factor * scrollBar->value()
                                + ((factor - 1) * scrollBar->pageStep()/2)));
    }

    void ImageMapAdapter::zoom_in()
    {


    }
    void ImageMapAdapter::zoom_out()
    {

    }

    QPoint ImageMapAdapter::coordinateToDisplay(const QPointF& coordinate) const
    {

    }

    QPointF ImageMapAdapter::displayToCoordinate(const QPoint& point) const
    {


    }

    bool ImageMapAdapter::isValid(int x, int y, int z) const
    {

        return true;

    }
    int ImageMapAdapter::tilesonzoomlevel(int zoomlevel) const
    {
        return int(pow(2, zoomlevel));
    }
    int ImageMapAdapter::xoffset(int x) const
    {
        return 0;
    }
    int ImageMapAdapter::yoffset(int y) const
    {
        return 0;
    }

     QString ImageMapAdapter::query(int x, int y, int z) const
     {
         return "";
     }




}
