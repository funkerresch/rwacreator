#ifndef RWAASSETATTRIBUTEVIEW_H
#define RWAASSETATTRIBUTEVIEW_H

#include "rwaattributeview.h"

class RwaAssetAttributeView : public RwaAttributeView
{
    Q_OBJECT

public:
    RwaAssetAttributeView(QWidget *parent, RwaScene *scene);

public slots:
    void receiveEditingFinished();
    void receiveCheckBoxAttributeValue(int id);
    void receiveCheckBoxAttributeValue(int id, bool value);

    void setCurrentState(RwaState *state);
    void setCurrentScene(RwaScene *scene);
protected:
    void setCurrentAsset(RwaAsset1 *asset);

private slots:
    void receiveLineEditAttributeValue(const QString &text);
    void receiveComboBoxAttributeValue(int index);
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
