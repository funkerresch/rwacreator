#include "rwasearchdialog.h"
#include <QGridLayout>


RwaSearchDialog::RwaSearchDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Find Location"));
    findPlaces = new RwaSearchBox(this);
    findPlaces->setFixedWidth(400);
    label = new QLabel(tr("Search for location:"), this);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(label, 0, 0);
    mainLayout->addWidget(findPlaces, 0, 1);
    setLayout(mainLayout);
}

void RwaSearchDialog::setMapCoordinates(double lon, double lat)
{
    emit sendMapCoordinates(lon, lat);
}
