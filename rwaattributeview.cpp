#include "rwaattributeview.h"

RwaAttributeView::RwaAttributeView(QWidget* parent, RwaScene *scene)
  : RwaView(parent, scene)
{
    assetAttrCounter = 0;
    assetAttributeGroup = new QButtonGroup(this);
    attributeFont = QFont("Courier", 10, QFont::StyleNormal,false);
    dynamicAddButtonFont = QFont("Courier", 10, QFont::StyleOblique,true);
    connect(assetAttributeGroup, SIGNAL(buttonToggled(int, bool)), this, SLOT(receiveCheckBoxAttributeValue(int, bool)));
    assetAttributeGroup->setExclusive(false);   
    scrollArea = new QScrollArea(parent);
    scrollArea->setWidget(this);
    this->setFrameStyle(QFrame::NoFrame);
}

void RwaAttributeView::addAttrCheckbox(QGridLayout *layout, QString name, int type)
{
    QCheckBox *attrCheckbox = new QCheckBox(name, this);
    attrCheckbox->setObjectName(name);
    attrCheckbox->setFont(attributeFont);
    attrCheckbox->setChecked(false);
    attrCheckbox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    attrCheckbox->setMinimumWidth(240);
    assetAttributeGroup->addButton(attrCheckbox, type);
    layout->addWidget(attrCheckbox, assetAttrCounter, 0);
    assetAttrCounter++;
}

QLineEdit *RwaAttributeView::addLineEditAndLabel(QGridLayout *layout, QString name)
{
    QLineEdit *attrLineEdit = new QLineEdit("0", this);
    attrLineEdit->setMinimumSize(QSize(16,14));
    attrLineEdit->setMaximumSize(QSize(14,14));
    attrLineEdit->setObjectName(name);
    QLabel *attrLabel = new QLabel(name, attrLineEdit);
    attrLabel->setMinimumSize(QSize(14,14));
    attrLabel->setMaximumSize(QSize(14,14));
    attrLineEdit->setFont(attributeFont);
    attrLineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    attrLineEdit->setMinimumWidth(120);
    attrLabel->setFont(attributeFont);
    attrLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    attrLabel->setMinimumWidth(120);
    attrLabel->setMaximumWidth(120);
    layout->addWidget(attrLineEdit, assetAttrCounter, 0);
    layout->addWidget(attrLabel, assetAttrCounter, 1);
    connect(attrLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(receiveLineEditAttributeValue(const QString &)));
    connect(attrLineEdit, SIGNAL(editingFinished()), this, SLOT(receiveEditingFinished()));

   // connect(attrLineEdit, SIGNAL(text), this, SLOT(receiveLineEditAttributeValue(QString)));

    assetAttrCounter++;
    return attrLineEdit;
}

void RwaAttributeView::addLineEditAndLabel(QGridLayout *layout, QString name, QLineEdit **attrLineEdit, QLabel **attrLabel)
{
    *attrLineEdit = new QLineEdit("0", this);
    (*attrLineEdit)->setObjectName(name);
    (*attrLabel) = new QLabel(name, this);
    (*attrLineEdit)->setFont(attributeFont);
    (*attrLineEdit)->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    (*attrLineEdit)->setMinimumWidth(120);
    (*attrLabel)->setFont(attributeFont);
    (*attrLabel)->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    (*attrLabel)->setMinimumWidth(120);
    (*attrLabel)->setMaximumWidth(120);
    layout->addWidget(*attrLineEdit, assetAttrCounter, 0);
    layout->addWidget(*attrLabel, assetAttrCounter, 1);
    connect(*attrLineEdit, SIGNAL(&QLineEdit::textEdited), this, SLOT(&RwaAttributeView::receiveLineEditAttributeValue));
    connect(*attrLineEdit, SIGNAL(editingFinished()), this, SLOT(receiveEditingFinished()));

   // connect(attrLineEdit, SIGNAL(text), this, SLOT(receiveLineEditAttributeValue(QString)));

    assetAttrCounter++;
}

void RwaAttributeView::setLineEditSignal2editingFinished(QLineEdit *attrLineEdit)
{
    disconnect(attrLineEdit, SIGNAL(textEdited(QString)), this, SLOT(receiveLineEditAttributeValue(QString)));
    connect(attrLineEdit, SIGNAL(editingFinished()) , this, SLOT(receiveLineEditAttributeValue()));
}

/*void RwaAttributeView::addPlusButtonForDynamicGuiElements(QGridLayout *layout, QString name)
{

    QPushButton *dynamicAddButton = new QPushButton(QString("+ %1").arg(name), this);
    dynamicAddButton->setFlat(true);
    dynamicAddButton->setObjectName(name);
    dynamicAddButton->setFont(dynamicAddButtonFont);
    layout->addWidget(dynamicAddButton, assetAttrCounter, 0);

    connect(dynamicAddButton, SIGNAL(clicked(bool)), this, SLOT(receiveDynamicPlusButton()));
    assetAttrCounter++;
}*/

QComboBox *RwaAttributeView::addComboBoxAndLabel(QGridLayout *layout, QString name, QStringList values)
{
    //RwaMemoryComboBox *attrComboBox = new RwaMemoryComboBox(this);
    QComboBox *attrComboBox = new QComboBox(this);
    attrComboBox->setObjectName(name);
    QLabel *attrLabel = new QLabel(name, this);
    attrComboBox->setMinimumSize(QSize(16,16));
    attrComboBox->setMaximumSize(QSize(16,16));
    attrLabel->setMinimumSize(QSize(14,14));
    attrLabel->setMaximumSize(QSize(14,14));

    attrComboBox->setFont(attributeFont);
    attrComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    attrComboBox->setMinimumWidth(120);
    attrComboBox->setMaximumWidth(120);
    //attrComboBox->setMaximumHeight(14);
    attrComboBox->setMinimumHeight(14);
   // attrComboBox->setContentsMargins(0,2,0,0);
    attrLabel->setFont(attributeFont);
    attrLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    attrLabel->setMinimumHeight(14);
    attrLabel->setMinimumWidth(120);

    layout->addWidget(attrComboBox, assetAttrCounter, 0);
    layout->addWidget(attrLabel, assetAttrCounter, 1);

    foreach (QString string, values)
        attrComboBox->addItem(string);

    connect(attrComboBox, SIGNAL(activated(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));

    assetAttrCounter++;
    return attrComboBox;
}

void RwaAttributeView::addComboBoxAndLabel(QGridLayout *layout, QString name, QStringList values, QComboBox **attrComboBox, QLabel **attrLabel)
{
    *attrComboBox = new QComboBox(this);
    *attrLabel = new QLabel(name, this);
    (*attrComboBox)->setObjectName(name);
    (*attrComboBox)->setMinimumSize(QSize(16,16));
    (*attrComboBox)->setMaximumSize(QSize(16,16));
    (*attrLabel)->setMinimumSize(QSize(14,14));
    (*attrLabel)->setMaximumSize(QSize(14,14));

    (*attrComboBox)->setFont(attributeFont);
    (*attrComboBox)->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    (*attrComboBox)->setMinimumWidth(120);
    (*attrComboBox)->setMaximumWidth(120);
    //attrComboBox->setMaximumHeight(14);
    (*attrComboBox)->setMinimumHeight(14);
   // attrComboBox->setContentsMargins(0,2,0,0);
    (*attrLabel)->setFont(attributeFont);
    (*attrLabel)->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    (*attrLabel)->setMinimumHeight(14);
    (*attrLabel)->setMinimumWidth(120);

    layout->addWidget((*attrComboBox), assetAttrCounter, 0);
    layout->addWidget((*attrLabel), assetAttrCounter, 1);

    foreach (QString string, values)
        (*attrComboBox)->addItem(string);

    connect((*attrComboBox), SIGNAL(activated(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
    //connect(attrComboBox, SIGNAL(closePopup()), this, SLOT(receiveComboBoxAttributeValue()));

    assetAttrCounter++;
}


