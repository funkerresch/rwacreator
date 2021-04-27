#include "rwastateeditor.h"

RwaStateEditor::RwaStateEditor(QWidget* parent, RwaScene *scene)
            : RwaView(parent, scene)
{
    QPushButton *submitChanges = new QPushButton(this);
    submitChanges->setText("Submit Changes");

    layout = new QBoxLayout(QBoxLayout::TopToBottom,this);
    layout->addWidget(submitChanges);

    currentState = NULL;
    editor = new RwaEditor();
    layout->addWidget(editor);

    connect(submitChanges, SIGNAL(clicked()),
              this, SLOT(receiveSubmitChanges()));

    connect(this, SIGNAL(sendCurrentStateString(QString)),
            backend, SLOT(receiveCurrentStateString(QString)));

    editor->appendPlainText("@scene: ");
    editor->appendPlainText("");
    editor->appendPlainText("@name: ");
    editor->appendPlainText("");
    editor->appendPlainText("@entryconditions: ");
    editor->appendPlainText("");
    editor->appendPlainText("@onenter: ");
    editor->appendPlainText("");
    editor->appendPlainText("@pathway: ");
    editor->appendPlainText("");
    editor->moveCursor(QTextCursor::Start);
}

void RwaStateEditor::receiveSubmitChanges()
{
    emit sendCurrentStateString(editor->toPlainText());
}

void RwaStateEditor::setCurrentState(RwaState *currentState)
{
    this->currentState = currentState;
    editor->selectAll(); // clear causes some setCurser out of range error..
    editor->cut();

    editor->appendPlainText(QString("@scene: %1;").arg(QString::fromStdString(currentState->getScene()->objectName())));
    editor->appendPlainText("");

    editor->appendPlainText(QString("@name: %1;").arg(QString::fromStdString(currentState->objectName())));
    editor->appendPlainText("");

    if(!(currentState->getCoordinates().size() == 0))
        editor->appendPlainText(QString("@entryconditions: gps %1 %2 %3;").arg(currentState->getCoordinates()[0]).arg(currentState->getCoordinates()[1]).arg(currentState->getRadius()));
    else
        editor->appendPlainText("@entryconditions: ");

    editor->appendPlainText("");

    editor->appendPlainText("@onenter: ");
    editor->appendPlainText("");

    editor->appendPlainText("@pathway: ");
    editor->appendPlainText("");
}

void RwaStateEditor::adaptSize(qint32 width, qint32 height)
{
    resize(QSize(width, height));
    //editor->resize(QSize(width-10, height));

}



