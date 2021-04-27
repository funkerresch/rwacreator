#include "rwagamemanifestview.h"

RwaGameManifestView::RwaGameManifestView(QWidget* parent, RwaScene *scene)
    : RwaView(parent, scene)
{
    layout = new QBoxLayout(QBoxLayout::TopToBottom,this);
    QPushButton *submitChanges = new QPushButton(this);
    submitChanges->setText("Submit Changes");
    layout->addWidget(submitChanges);

    editor = new RwaEditor();
    layout->addWidget(editor);

    connect(this, SIGNAL(sendManifestString(QString)),
        backend, SLOT(receiveGameManifestString(QString)));

    connect(submitChanges, SIGNAL(clicked()),
              this, SLOT(receiveSubmitChanges()));

    editor->appendPlainText("@enabled: ");
    editor->appendPlainText("");
    editor->appendPlainText("@hero: ");
    editor->appendPlainText("");
    editor->appendPlainText("@object: ");
    editor->appendPlainText("");
    editor->appendPlainText("@npc: ");
    editor->appendPlainText("");
    editor->moveCursor(QTextCursor::Start);
}

void RwaGameManifestView::receiveSubmitChanges()
{
    emit sendManifestString(editor->toPlainText());
    qDebug() << "sendgamemanifset";
}
void RwaGameManifestView::adaptSize(qint32 width, qint32 height)
{
    resize(QSize(width, height));
    //editor->resize(QSize(width-10, height));

}
