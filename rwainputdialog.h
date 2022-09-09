/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwainputdialog.h
 * by Thomas Resch
 *
 */

#ifndef RWAINPUTDIALOG_H
#define RWAINPUTDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;

class RwaInputDialog : public QDialog
{
    Q_OBJECT
public:
    RwaInputDialog(QWidget *parent = nullptr, QStringList labels = QStringList(), QStringList values = QStringList());

    static QStringList getStrings(QWidget *parent, QStringList _labels, QStringList values, bool *ok = nullptr);

    private:
        QList<QLineEdit*> fields;
        QList<QLabel*> labels;
};

#endif // RWAINPUTDIALOG_H
