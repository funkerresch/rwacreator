/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwaattributeview.h
 * by Thomas Resch
 * Base class for editors for accessing attributes and values
 *
 */

#ifndef RWAATTRIBUTEVIEW_H
#define RWAATTRIBUTEVIEW_H

#include "rwatoolbar.h"
#include "rwabackend.h"
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QCheckBox>
#include <QFont>
#include "rwaview.h"
#include <QEvent>

class LineEditAndLabel  : public QObject
{
    Q_OBJECT
    QLineEdit *lineEdit;
    QLabel *label;
    explicit LineEditAndLabel(QWidget* parent);
};

class RwaAttributeView  : public RwaView
{
    Q_OBJECT

public:
    explicit RwaAttributeView(QWidget* parent, RwaScene *scene);
    QScrollArea *scrollArea;

public slots:

    void addComboBoxAndLabel(QGridLayout *layout, QString name, QStringList values, QComboBox **attrComboBox, QLabel **attrLabel);
protected:

    int assetAttrCounter;
    QButtonGroup *assetAttributeGroup;
    QFont attributeFont;
    QFont dynamicAddButtonFont;
    QString senderName, lastSenderName;
    QString senderValue, lastSenderValue;
    QGridLayout *attributeGridLayout;
    bool updatingForm = false; // widget signals fired while populating/clearing the form must not write back into assets

    void setLineEditSignal2editingFinished(QLineEdit *attrLineEdit);
    void addSeparator(QGridLayout *layout);
    void addAttrCheckbox(QGridLayout *layout, QString name, int type);
    void addLineEditAndLabel(QGridLayout *layout, QString name, QLineEdit **attrLineEdit, QLabel **attrLabel);
    QLineEdit *addLineEditAndLabel(QGridLayout *layout, QString name);
    QComboBox *addComboBoxAndLabel(QGridLayout *layout, QString name, QStringList values);

    // Gain is stored linear (model, .rwa, Pd) but edited in dB: 0 = unity, "-inf" = silent.
    // dbTextToGain returns false for text that is not (yet) a number, so callers can ignore it.
    static QString gainToDbText(float gain);
    static bool dbTextToGain(const QString &text, float &gain);

    float calculate_window_height();
protected slots:
    virtual void receiveCheckBoxAttributeValue(int id, bool) = 0;
    virtual void receiveLineEditAttributeValue(const QString &text) = 0;
    virtual void receiveComboBoxAttributeValue(int index) = 0;
    virtual void receiveFaderAttributeValue(int id) = 0;
    virtual void receiveEditingFinished() = 0;
};

#endif // RWAATTRIBUTEVIEW_H
