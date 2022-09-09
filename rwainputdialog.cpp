#include "rwainputdialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QDebug>

RwaInputDialog::RwaInputDialog(QWidget *parent, QStringList labels, QStringList values) : QDialog(parent)
{
    QFormLayout *lytMain = new QFormLayout(this);
    this->setMinimumWidth(800);

    for (int i = 0; i < labels.length(); ++i)
    {
        QLabel *tLabel = new QLabel(labels[i], this);
        QLineEdit *tLine = new QLineEdit(this);
        tLine->setMinimumWidth(600);
        tLine->setText(values[i]);
        lytMain->addRow(tLabel, tLine);

        fields << tLine;
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    lytMain->addWidget(buttonBox);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                   this, &RwaInputDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &RwaInputDialog::reject);
    Q_ASSERT(conn);

    setLayout(lytMain);
}

QStringList RwaInputDialog::getStrings(QWidget *parent, QStringList labels, QStringList values, bool *ok)
{
    RwaInputDialog *dialog = new RwaInputDialog(parent, labels, values);
    QStringList list;

    const int ret = dialog->exec();
    dialog->show();
    if (ok)
        *ok = !!ret;

    if (ret) {
        foreach (auto field, dialog->fields) {
            list << field->text();
        }
    }

    dialog->deleteLater();
    return list;
}
