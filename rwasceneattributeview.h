/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwamapview.h
 * by Thomas Resch
 *
 */

#ifndef RWASCENEATTRIBUTEVIEW_H
#define RWASCENEATTRIBUTEVIEW_H

#include "rwaattributeview.h"

class RwaSceneAttributeView : public RwaAttributeView
{
    Q_OBJECT
public:
    RwaSceneAttributeView(QWidget *parent, RwaScene *scene);

public slots:
    void updateSceneArea();
    void setCurrentScene(RwaScene *scene);

    void receiveEditingFinished();
private slots:
    void receiveCheckBoxAttributeValue(int id, bool value);
    void receiveLineEditAttributeValue(const QString &value);
    void receiveLineEditAttributeValue();
    void receiveComboBoxAttributeValue(QString value);
    void receiveComboBoxAttributeValue(int index);
    void receiveFaderAttributeValue(int id);

signals:
    void sendCurrentSceneRadiusEdited();

private:
   //RwaView *mother;
   QList <QComboBox *> visitedSceneComboBoxes;
   qint32 visitedSceneConditionCount;

   void updateSceneComboBox(QComboBox *attrComboBox);
   void updateSceneAttr(QComboBox *attrComboBox, QString scene2compare);
};

#endif // RWASCENEATTRIBUTEVIEW_H
