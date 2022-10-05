#include "rwaattributeview.h"

RwaAttributeView::RwaAttributeView(QWidget* parent, RwaScene *scene)
  : RwaView(parent, scene)
{
    attributeGridLayout = new QGridLayout(this);
    attributeGridLayout->setVerticalSpacing(2);
    attributeGridLayout->setContentsMargins(0,4,0,4);
    attributeGridLayout->setAlignment(this, Qt::AlignLeft);
    attributeGridLayout->setHorizontalSpacing(25);
    assetAttrCounter = 0;
    assetAttributeGroup = new QButtonGroup(this);
    attributeFont = QFont("Arial", 10, QFont::StyleNormal,false);
    dynamicAddButtonFont = QFont("Courier", 10, QFont::StyleOblique,true);
    connect(assetAttributeGroup, SIGNAL(idToggled(int, bool)), this, SLOT(receiveCheckBoxAttributeValue(int, bool)));
    connect(assetAttributeGroup, SIGNAL(idReleased(int)), this, SLOT(receiveEditingFinished()));
    assetAttributeGroup->setExclusive(false);   
    scrollArea = new QScrollArea(parent);
    scrollArea->setWidget(this);
    setFrameStyle(QFrame::NoFrame);
    senderName = "name";
    lastSenderName = "";
    senderValue = "value";
    lastSenderValue = "";
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
    attrLineEdit->setMinimumSize(QSize(16,16));
    attrLineEdit->setMaximumSize(QSize(14,16));
    attrLineEdit->setObjectName(name);
    QLabel *attrLabel = new QLabel(name, attrLineEdit);
    attrLabel->setMinimumSize(QSize(14,15));
    attrLabel->setMaximumSize(QSize(14,15));
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
    assetAttrCounter++;
}

void RwaAttributeView::setLineEditSignal2editingFinished(QLineEdit *attrLineEdit)
{
    disconnect(attrLineEdit, SIGNAL(textEdited(QString)), this, SLOT(receiveLineEditAttributeValue(QString)));
    disconnect(attrLineEdit, SIGNAL(editingFinished()), this, SLOT(receiveEditingFinished()));
    connect(attrLineEdit, SIGNAL(editingFinished()) , this, SLOT(receiveLineEditAttributeValue()));
}

QComboBox *RwaAttributeView::addComboBoxAndLabel(QGridLayout *layout, QString name, QStringList values)
{
    QComboBox *attrComboBox = new QComboBox(this);
    attrComboBox->setObjectName(name);
    QLabel *attrLabel = new QLabel(name, this);
    attrComboBox->setMinimumSize(QSize(16,16));
    attrComboBox->setMaximumSize(QSize(16,16));
    attrComboBox->setFixedHeight(24);
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

//    QFrame* line = new QFrame(this);
//    line->setFrameShape(QFrame::HLine);
//    line->setFrameShadow(QFrame::Sunken);
//    layout->addWidget(line);

    layout->addWidget(attrComboBox, assetAttrCounter, 0);
    layout->addWidget(attrLabel, assetAttrCounter, 1);
//    layout->addWidget(line, assetAttrCounter, 0);

    foreach (QString string, values)
        attrComboBox->addItem(string);

    connect(attrComboBox, SIGNAL(activated(int)), this, SLOT(receiveComboBoxAttributeValue(int)));
    connect(attrComboBox, SIGNAL(textActivated(QString)), this, SLOT(receiveEditingFinished()));

    assetAttrCounter++;
//    assetAttrCounter++;
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

    connect((*attrComboBox), SIGNAL(activated(int)), this, SLOT(receiveComboBoxAttributeValue(int)));
    connect(*attrComboBox, SIGNAL(textActivated(QString)), this, SLOT(receiveEditingFinished()));

    assetAttrCounter++;
}


