#ifndef RWASTYLES
#define RWASTYLES

#include <QObject>
#include <QString>
#include <QFont>

static QString rwaSimulationOnLabelActive  ("color: rgba(0, 0, 0, 100%);"
                                     "background-color: rgba(222, 222, 222, 0%);"
                                     "selection-color: yellow;"
                                     "selection-background-color: blue;"
                                     "border-style: outset;"
                                     "border-width: 1px;"
                                     "border-color: rgba(222, 222, 222, 0%);"
                                     "border-radius: 2px;"
                                     "font-size: 14px;"
                                     "font-family: Helvetica Neue DeskInterface;");

static QString rwaSimulationOnLabelNotActive  ("color: rgba(0, 100, 0, 100%);"
                                     "background-color: rgba(222, 222, 222, 0%);"
                                     "selection-color: yellow;"
                                     "selection-background-color: blue;"
                                     "border-style: outset;"
                                     "border-width: 1px;"
                                     "border-color: rgba(222, 222, 222, 0%);"
                                     "border-radius: 2px;"
                                     "font-size: 10px;"
                                     "font-family: Helvetica Neue DeskInterface;");

static QString rwaButtonStyleSheet  ("color: rgba(0, 0, 0, 100%);"
                                     "background-color: rgba(222, 222, 222, 90%);"
                                     "selection-color: yellow;"
                                     "selection-background-color: blue;"
                                     "border-style: outset;"
                                     "border-width: 1px;"
                                     "border-color: #666666;"
                                     "border-radius: 2px;"
                                     "font-size: 10px;"
                                     "font-family: Helvetica Neue DeskInterface;");

static QString rwaLargeLineEditStyle("color: rgba(0, 0, 0, 100%);"
                                     "background-color: rgba(222, 222, 222, 90%);"
                                     "selection-color: yellow;"
                                     "selection-background-color: blue;"
                                     "border-style: outset;"
                                     "border-width: 1px;"
                                     "border-color: #666666;"
                                     "border-radius: 2px;"
                                     "font-size: 10px;"
                                     "font-family: Helvetica Neue DeskInterface;");


static QString rwaSmallLineEditStyle( "color: rgba(0, 0, 0, 100%);"
                                      "background-color: rgba(222, 222, 222, 100%);"
                                      "selection-color: yellow;"
                                      "selection-background-color: blue;"
                                      "border-style: outset;"
                                      "border-width: 1px;"
                                      "border-color: #666666;"
                                      "border-radius: 2px;"
                                      "font-size: 10px;"
                                      "font-family: Helvetica Neue DeskInterface;");

static QString rwaInactiveLableStyle( "color: rgba(0, 0, 222, 60%);"
                                      "background-color: rgba(222, 222, 222, 40%);"
                                      "selection-color: yellow;"
                                      "selection-background-color: blue;"
                                      "border-style: outset;"
                                      "border-width: 1px;"
                                      "border-color: #666666;"
                                      "border-radius: 2px;"
                                      "font-size: 10px;"
                                      "font-family: Helvetica Neue DeskInterface;");

#endif // RWASTYLES

