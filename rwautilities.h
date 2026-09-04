#ifndef RWAUTILITIES_H
#define RWAUTILITIES_H

#define RWA_EARTHRADIUS 6378137

#ifdef USINGQT
#include <QObject>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QPointF>
#include <QRectF>
#include <QLineF>
#include <QClipboard>
#include <QApplication>
#endif

#ifdef QT_VERSION
#include "rwalocation1.h"
#endif
#include <math.h>
#include <string>

//#include "rwascene.h"

class RwaUtilities
{
//#ifdef QT_VERSION
//    Q_OBJECT
//#endif
public:
    explicit RwaUtilities();
#ifdef QT_VERSION
    static QString getFileBaseName(QString fullpath);
    static QString getFileName(QString fullpath);
    static QString getPath(QString fullpath);
    static QString getDataType(QString fullpath);
    static double calculateDistance(QPointF p1, QPointF p2);
    static double calculateBearing(QPointF p1, QPointF p2);
    static QPointF calculatePointOnCircle(QPointF p1, double distance);
    static QPointF calculateDestination(QPointF coordinates, double radius, double bearingInDegrees);
    static void calculatePolygonOffset(QPointF middle, double offset, QVector <QPointF> *corners, QVector <QPointF> *offsetCorners);
    static QRectF calculateRectCorners(QPointF center, double width, double height);
    static QPointF calculateNorthWest(QPointF center, double width, double height);
    static QPointF calculateSouthEast(QPointF center, double width, double height);
    static void logLocationCoordinates(QPointF location);
    static void copyLocationCoordinates2Clipboard(QPointF location);
    static void emtpyDirectory(QString fullpath);
    static double calculateBearing(QPointF p1, QPointF p2, int headDirection);
    static bool mouseDownOnPolygonVertex1(std::vector<double> p1, std::vector<double> p2, QPointF p);
    static QString generateAssetsFolderPath(QString projectPath);
    static QString generateCompleteAssetPath(QString projectPath, QString fileName);
    static QString generateCompleteAssetPath(QString projectPath, std::string fileName);
    static void copyCoordinate2Clipboard(RwaLocation1 *currentArea);
    static bool mouseDownOnPolygonVertex(QPointF lp1, QPointF lp2, QPointF p);
#endif
    static std::string getFileName1(std::string fullpath);
    static double degrees2radians(double degrees);
    static double radians2degrees(double radians);
    static double calculateDistance1(std::vector<double> p1, std::vector<double> p2);
    static double calculateDistanceInMeters(std::vector<double> p1, std::vector<double> p2);
    static std::vector<double> calculateDestination1(std::vector<double> coordinates, double radius, double bearingInDegrees);
    static std::string getFileName(std::string fullpath);
    static double calculateBearing1(std::vector<double> p1, std::vector<double> p2);
    static void calculatePolygonOffset2(double offset, std::vector<std::vector<double> > &corners, std::vector<std::vector<double> > &offsetCorners);
    static double calculateBearing1(std::vector<double> p1, std::vector<double> p2, int headDirection);
    static bool coordinateWithinRectangle1(std::vector<double> p, std::vector<double> center, double width, double height);
    static bool coordinateWithinPolygon3(std::vector<double> p, std::vector<std::vector<double> > &corners);
    static void debug2Terminal(const std::string file, const std::string func, int32_t line, const std::string message);
    static bool coordinateWithinRectangle1(std::vector<double> p, std::vector<double> corner1, std::vector<double> corner2);
    static double calculateElevationEasy(std::vector<double> p1, std::vector<double> p2, double elevation, int headDirection);
    static double calculateDistanceWithAltitude(double hDist, double vDist);
};

#endif // RWAUTILITIES_H
