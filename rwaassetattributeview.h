#ifndef RWAASSETATTRIBUTEVIEW_H
#define RWAASSETATTRIBUTEVIEW_H


#include "rwaattributeview.h"

class RwaAssetAttributeView : public RwaAttributeView
{
    Q_OBJECT

public:
    RwaAssetAttributeView(QWidget *parent, RwaScene *scene);

public slots:
    void adaptSize(qint32 width, qint32 height);
    void receiveEditingFinished();
    void receiveCheckBoxAttributeValue(int id);


protected:
    void setCurrentAsset(RwaAsset1 *asset);

private slots:
    void receiveCheckBoxAttributeValue(int id, bool value);
    void receiveLineEditAttributeValue(const QString &text);
    void receiveComboBoxAttributeValue(QString value);
    void receiveFaderAttributeValue(int id);
    void receiveSelectedAssets(QStringList assets);

private:
   QStringList selectedAssets;
   QComboBox *reflectionCount;
   QLabel *reflectionCountLabel;
   QLineEdit *reflectionDampingLineEdit[16];
   QLabel *reflectionLabel[16];
};

#endif // RWAASSETATTRIBUTEVIEW_H