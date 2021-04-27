#ifndef IMAGEMAPADAPTER_H
#define IMAGEMAPADAPTER_H

#include "mapadapter.h"
#include <QImage>
#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QScrollArea>
#include <QScrollBar>
#include <QApplication>
#include <QImageReader>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QImageWriter>

namespace qmapcontrol
{
    //! MapAdapter for Images

    class ImageMapAdapter : public MapAdapter
    {
        Q_OBJECT
    public:

        ImageMapAdapter(const QString filepath);

        virtual ~ImageMapAdapter();
        virtual QPoint coordinateToDisplay(const QPointF&) const;
        virtual QPointF displayToCoordinate(const QPoint&) const;

        bool loadFile(const QString &);

    private slots:

        void zoomIn();
        void zoomOut();
        void normalSize();
        void fitToWindow();
    private:

        void createMenus();

        void setImage(const QImage &newImage);
        void scaleImage(double factor);
        void adjustScrollBar(QScrollBar *scrollBar, double factor);

        QImage image;
        QLabel *imageLabel;
        QScrollArea *scrollArea;
        double scaleFactor;


    protected:

        virtual bool isValid(int x, int y, int z) const;
        virtual void zoom_in();
        virtual void zoom_out();
        virtual QString query(int x, int y, int z) const;
        virtual int tilesonzoomlevel(int zoomlevel) const;
        virtual int xoffset(int x) const;
        virtual int yoffset(int y) const;


    };
}
#endif


