#ifndef RWASEARCHDIALOG_H
#define RWASEARCHDIALOG_H


#include <QDialog>
#include <QDebug>
#include <QLabel>
#include "rwasearchbox.h"

class RwaSearchDialog : public QDialog
{
    Q_OBJECT

public:
    RwaSearchDialog(QWidget *parent = nullptr);

public slots:
    void setMapCoordinates(double lon, double lat);
private:
    QLabel *label;
    RwaSearchBox *findPlaces;
signals:
    void sendMapCoordinates(double lon, double lat);
};

#endif // RWASEARCHDIALOG_H
