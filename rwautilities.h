#ifndef RWAUTILITIES_H
#define RWAUTILITIES_H

#define RWA_EARTHRADIUS 6378137

#include <QObject>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QPointF>
#include <QRectF>
#include <QLineF>
#include <QClipboard>
#include <QApplication>
#include <math.h>
#include "rwalocation1.h"
//#include "rwascene.h"

class RwaUtilities : public QObject
{
    Q_OBJECT
public:
    explicit RwaUtilities(QObject *parent = 0);
    static bool checkDataType(QString fullpath);
    static QString getFileBaseName(QString fullpath);
    static QString getFileName(QString fullpath);
    static QString getPath(QString fullpath);
    static QString getDataType(QString fullpath);
    static double calculateDistance(QPointF p1, QPointF p2);
    static double calculateBearing(QPointF p1, QPointF p2);
    static double degrees2radians(double degrees);
    static double radians2degrees(double radians);
    static QPointF calculatePointOnCircle(QPointF p1, double distance);
    static void emtpyDirectory(QString fullpath);
    static QPointF calculateDestination(QPointF coordinates, double radius, double bearingInDegrees);

    static double calculateBearing(QPointF p1, QPointF p2, qint32 headDirection);
    static QPointF radians2degrees(QPointF radians);
    static bool coordinateWithinRectangle(QPointF p, QPointF center, double width, double height);
    static bool mouseDownOnPolygonVertex(QPointF lp1, QPointF lp2, QPointF p);
    static bool coordinateWithinPolygon1(QPointF p, QVector<QPointF> *corners);
    static void copyCoordinate2Clipboard(RwaLocation1 *currentArea);
    static void calculatePolygonOffset(QPointF middle, double offset, QVector <QPointF> *corners, QVector <QPointF> *offsetCorners);
    static QRectF calculateRectCorners(QPointF center, double width, double height);
    static QPointF calculateNorthWest(QPointF center, double width, double height);
    static QPointF calculateSouthEast(QPointF center, double width, double height);
    static void logLocationCoordinates(QPointF location);
    static void copyLocationCoordinates2Clipboard(QPointF location);
    static void calculatePolygonOffset1(double offset, QVector<QPointF> *corners, QVector<QPointF> *offsetCorners);
    static double calculateDistance1(std::vector<double> p1, std::vector<double> p2);
    static int32_t calculateDistanceInMeters(std::vector<double> p1, std::vector<double> p2);
    static std::vector<double> calculateDestination1(std::vector<double> coordinates, double radius, double bearingInDegrees);
    static std::string getFileName(std::string fullpath);
    static double calculateBearing1(std::vector<double> p1, std::vector<double> p2);

    static bool coordinateWithinPolygon2(QPointF p, std::vector<std::vector<double> > &corners);
    static void calculatePolygonOffset2(double offset, std::vector<std::vector<double> > &corners, std::vector<std::vector<double> > &offsetCorners);
    static bool mouseDownOnPolygonVertex1(std::vector<double> p1, std::vector<double> p2, QPointF p);
    static double calculateBearing1(std::vector<double> p1, std::vector<double> p2, qint32 headDirection);
    static bool coordinateWithinRectangle1(std::vector<double> p, std::vector<double> center, double width, double height);
    static bool coordinateWithinPolygon3(std::vector<double> p, std::vector<std::vector<double> > &corners);
    static void debug2Terminal(const std::string file, const std::string func, int32_t line, const std::string message);
    static bool coordinateWithinRectangle1(std::vector<double> p, std::vector<double> corner1, std::vector<double> corner2);    
    static QString generateAssetsFolderPath(QString projectPath);
    static QString generateCompleteAssetPath(QString projectPath, QString fileName);
    static QString generateCompleteAssetPath(QString projectPath, std::string fileName);
    static double calculateElevationEasy(std::vector<double> p1, std::vector<double> p2, double elevation, qint32 headDirection);
    static double calculateDistanceWithAltitude(double hDist, double vDist);
};

#endif // RWAUTILITIES_H
