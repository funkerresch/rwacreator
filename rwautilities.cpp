#include "rwautilities.h"

#include<QDebug>
#include <QRegularExpression>
#include "clipper/clipper.hpp"

using namespace ClipperLib;

void RwaUtilities::debug2Terminal(const std::string file, const std::string func, int32_t line, const std::string message)
{
#ifdef QT_VERSION
     qDebug() << file.c_str() << " "<< func.c_str() << " " << line << " " << message.c_str();
#else
     cout << file << " " << func << " " << line << " " << message;
#endif
}

double RwaUtilities::degrees2radians(double degrees)
{
    return degrees * (M_PI/180);
}

double RwaUtilities::radians2degrees(double radians)
{
    return radians * (180/M_PI);
}

QPointF RwaUtilities::radians2degrees(QPointF radians)
{
    QPointF degrees;
    degrees.setX(radians2degrees(radians.x()));
    degrees.setY(radians2degrees(radians.y()));
    return degrees;
}

void RwaUtilities::copyCoordinate2Clipboard(RwaLocation1 *currentArea)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString text = QString("%1 %2").arg(currentArea->getCoordinates()[0]).arg(currentArea->getCoordinates()[1]);
    clipboard->setText(text);
}

void RwaUtilities::logLocationCoordinates(QPointF location)
{
    qDebug() << "Lon: "<< QString::number(location.x(), 'f', 10) << " Lat: " << QString::number(location.y(), 'f', 10);
}

void RwaUtilities::copyLocationCoordinates2Clipboard(QPointF location)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString text = QString("%1 %2").arg(location.x()).arg(location.y());
    clipboard->setText(text);
}

bool RwaUtilities::mouseDownOnPolygonVertex1(std::vector<double> p1, std::vector<double> p2, QPointF p)
{
    QPointF lp1(p1[0], p1[1]);
    QPointF lp2(p2[0], p2[1]);

    QLineF line = QLineF(lp1.x(), lp1.y(), lp2.x(),lp2.y());
    QLineF line1 = QLineF(p.x(), p.y(), lp1.x(),lp1.y());
    QLineF line2 = QLineF(p.x(), p.y(), lp2.x(),lp2.y());

    double d1 = line1.length();
    double d2 = line2.length();
    double lineLen = line.length();
    double buffer = 0.00002;

    if (d1+d2 >= lineLen-buffer && d1+d2 <= lineLen+buffer)
        return true;

    return false;
}

bool RwaUtilities::mouseDownOnPolygonVertex(QPointF lp1, QPointF lp2, QPointF p)
{
    QLineF line = QLineF(lp1.x(), lp1.y(), lp2.x(),lp2.y());
    QLineF line1 = QLineF(p.x(), p.y(), lp1.x(),lp1.y());
    QLineF line2 = QLineF(p.x(), p.y(), lp2.x(),lp2.y());

    double d1 = line1.length();
    double d2 = line2.length();
    double lineLen = line.length();
    double buffer = 0.00002;

    if (d1+d2 >= lineLen-buffer && d1+d2 <= lineLen+buffer)
        return true;

    return false;
}

QPointF RwaUtilities::calculatePointOnCircle(QPointF p1, double distance) // returns point with distance east of p1
{
    double d = distance / 1000;
    QPointF endCoordinate;

    endCoordinate.setX( (d)/71.5 + p1.x());
    endCoordinate.setY(p1.y());
    return endCoordinate;
}

QPointF RwaUtilities::calculateDestination(QPointF coordinates, double radius, double bearingInDegrees)
{
    double long1, long2;
    double lat1, lat2;
    double delta;
    double bearing;
    QPointF destination;

    bearing = degrees2radians(bearingInDegrees);
    lat1 = degrees2radians(coordinates.y());
    long1 = degrees2radians(coordinates.x());
    delta = radius /RWA_EARTHRADIUS;

    lat2 = radians2degrees(asin(sin(lat1) * cos(delta) + cos(lat1) * sin(delta) * cos(bearing)));
    long2 = fmod( (long1 - asin(sin(bearing)*sin(delta) / cos(lat1)) + M_PI), (2*M_PI)) - M_PI;
    long2 = radians2degrees(long2);

    destination.setX(long2);
    destination.setY(lat2);
    return destination;
}

std::vector<double> RwaUtilities::calculateDestination1(std::vector<double> coordinates, double radius, double bearingInDegrees)
{
    double long1, long2;
    double lat1, lat2;
    double delta;
    double bearing;
    std::vector<double> destination(2, 0.0);

    bearing = degrees2radians(bearingInDegrees);
    lat1 = degrees2radians(coordinates[1]);
    long1 = degrees2radians(coordinates[0]);
    delta = radius /RWA_EARTHRADIUS;

    lat2 = radians2degrees(asin(sin(lat1) * cos(delta) + cos(lat1) * sin(delta) * cos(bearing)));
    long2 = fmod( (long1 - asin(sin(bearing)*sin(delta) / cos(lat1)) + M_PI), (2*M_PI)) - M_PI;
    long2 = radians2degrees(long2);

    destination[0] = (long2);
    destination[1] = (lat2);
    return destination;
}

int32_t RwaUtilities::calculateDistanceInMeters(std::vector<double> p1, std::vector<double> p2) // calculates Distance in km
{
    double R = 6373000; // Earth Radius in meters
    double lat1 = degrees2radians(p1[1]);
    double lat2 = degrees2radians(p2[1]);
    double dlon = degrees2radians( p2[0] - p1[0]);
    double dlat = degrees2radians( p2[1] - p1[1]);
    double a = pow((sin(dlat/2)),2) + cos(lat1) * cos(lat2) * pow((sin(dlon/2)),2) ;
    double c = 2 * atan2( sqrt(a), sqrt(1-a) ) ;
    int32_t d = static_cast<int32_t>(R * c);
    return d;
}

double RwaUtilities::calculateDistance1(std::vector<double> p1, std::vector<double> p2) // calculates Distance in km
{
    double R = 6373; // Earth Radius in km
    double lat1 = degrees2radians(p1[1]);
    double lat2 = degrees2radians(p2[1]);
    double dlon = degrees2radians( p2[0] - p1[0]);
    double dlat = degrees2radians( p2[1] - p1[1]);
    double a = pow((sin(dlat/2)),2) + cos(lat1) * cos(lat2) * pow((sin(dlon/2)),2) ;
    double c = 2 * atan2( sqrt(a), sqrt(1-a) ) ;
    double d = R * c;
    return d;
}

double RwaUtilities::calculateDistance(QPointF p1, QPointF p2)
{
    double R = 6373; // Earth Radius
    double lat1 = degrees2radians(p1.y());
    double lat2 = degrees2radians(p2.y());
    double dlon = degrees2radians( p2.x() - p1.x());
    double dlat = degrees2radians( p2.y() - p1.y());
    double a = pow((sin(dlat/2)),2) + cos(lat1) * cos(lat2) * pow((sin(dlon/2)),2) ;
    double c = 2 * atan2( sqrt(a), sqrt(1-a) ) ;
    double d = R * c;
    return d;
}

double RwaUtilities::calculateBearing1(std::vector<double> p1, std::vector<double> p2)
{
    double radians;
    double degrees;
    double phi1 = degrees2radians(p1[1]);
    double phi2 = degrees2radians(p2[1]);
    double lam1 = degrees2radians(p1[0]);
    double lam2 = degrees2radians(p2[0]);

    radians = atan2(sin(lam2-lam1)*cos(phi2),cos(phi1)*sin(phi2) - sin(phi1)*cos(phi2)*cos(lam2-lam1));
    degrees = radians2degrees(radians);
    //return degrees;
    return (((int)degrees+180) % 360); // offset for working correctly with the earplug~ in pd
}

double RwaUtilities::calculateBearing(QPointF p1, QPointF p2)
{
    double radians;
    double degrees;
    double phi1 = degrees2radians(p1.y());
    double phi2 = degrees2radians(p2.y());
    double lam1 = degrees2radians(p1.x());
    double lam2 = degrees2radians(p2.x());

    radians = atan2(sin(lam2-lam1)*cos(phi2),cos(phi1)*sin(phi2) - sin(phi1)*cos(phi2)*cos(lam2-lam1));
    degrees = radians2degrees(radians);
    //return degrees;
    return (((int)degrees+180) % 360); // offset for working correctly with the earplug~ in pd
}

double RwaUtilities::calculateBearing(QPointF p1, QPointF p2, qint32 headDirection)
{
    double radians;
    double degrees;
    double phi1 = degrees2radians(p1.y());
    double phi2 = degrees2radians(p2.y());
    double lam1 = degrees2radians(p1.x());
    double lam2 = degrees2radians(p2.x());

    radians = atan2(sin(lam2-lam1)*cos(phi2),cos(phi1)*sin(phi2) - sin(phi1)*cos(phi2)*cos(lam2-lam1));
    degrees = radians2degrees(radians);
    degrees -= headDirection;
    degrees += 360;
    return (((int)degrees+180) % 360); // offset for working correctly with the earplug~ in pd
}

double RwaUtilities::calculateBearing1(std::vector<double> p1, std::vector<double> p2, qint32 headDirection)
{
    double radians;
    double degrees;
    double phi1 = degrees2radians(p1[1]);
    double phi2 = degrees2radians(p2[1]);
    double lam1 = degrees2radians(p1[0]);
    double lam2 = degrees2radians(p2[0]);

    radians = atan2(sin(lam2-lam1)*cos(phi2),cos(phi1)*sin(phi2) - sin(phi1)*cos(phi2)*cos(lam2-lam1));
    degrees = radians2degrees(radians);
    degrees -= headDirection;
    degrees += 360;
    return (((int)degrees+180) % 360); // offset for working correctly with the earplug~ in pd
}

QRectF RwaUtilities::calculateRectCorners(QPointF center, double width, double height)
{

    QPointF w = calculateDestination(center, width/2, 90);
    QPointF e = calculateDestination(center, width/2, 270);
    QPointF nw = ( calculateDestination(w, height/2, 0) );
   // QPointF sw = (calculateDestination(w, height/2, 180) );
    //QPointF ne = (calculateDestination(e, height/2, 0) );
    QPointF se = (calculateDestination(e, height/2, 180) );
    return QRectF(nw, se);

}

QPointF RwaUtilities::calculateNorthWest(QPointF center, double width, double height)
{

    QPointF w = calculateDestination(center, width/2, 90);
    QPointF e = calculateDestination(center, width/2, 270);
    QPointF nw = ( calculateDestination(w, height/2, 0) );
   // QPointF sw = (calculateDestination(w, height/2, 180) );
    //QPointF ne = (calculateDestination(e, height/2, 0) );
    return nw;
}

QPointF RwaUtilities::calculateSouthEast(QPointF center, double width, double height)
{

    QPointF w = calculateDestination(center, width/2, 90);
    QPointF e = calculateDestination(center, width/2, 270);
   // QPointF sw = (calculateDestination(w, height/2, 180) );
    //QPointF ne = (calculateDestination(e, height/2, 0) );
    QPointF se = (calculateDestination(e, height/2, 180) );
    return se;
}

bool RwaUtilities::coordinateWithinRectangle1(std::vector<double> p, std::vector<double> corner1, std::vector<double> corner2)
{
    double minX = fmin(corner1[0], corner2[0]);
    double maxX = fmax(corner1[0], corner2[0]);
    double minY = fmin(corner1[1], corner2[1]);
    double maxY = fmax(corner1[1], corner2[1]);

    if(p[0] >= minX && p[0] <= maxX && p[1] >= minY && p[1] <= maxY)
        return true;
    else
        return false;
}

bool RwaUtilities::coordinateWithinRectangle1(std::vector<double> p, std::vector<double> center, double width, double height)
{
    double testx = p[0];
    double testy = p[1];
    std::vector<double> w(2, 0.0);
    std::vector<double> e(2, 0.0);
    std::vector<double> nw(2, 0.0);
    std::vector<double> sw(2, 0.0);
    std::vector<double> ne(2, 0.0);
    std::vector<double> se(2, 0.0);

    w = calculateDestination1(center, width/2, 90);
    e = calculateDestination1(center, width/2, 270);
    nw = ( calculateDestination1(w, height/2, 0) );

    if(nw[0] < 0)
        nw[0] = (nw[0] + 360);

    sw = (calculateDestination1(w, height/2, 180) );
    if(sw[0] < 0)
        sw[0] = (sw[0] + 360);

    ne = (calculateDestination1(e, height/2, 0) );
    if(ne[0] < 0)
        ne[0] = (ne[0] + 360);

    se = (calculateDestination1(e, height/2, 180) );
    if(se[0] < 0)
        se[0] = (se[0] + 360);

    if(testx > nw[0] && testx < ne[0] && testy > se[1] && testy < ne[1])
        return true;
    else
        return false;
}

bool RwaUtilities::coordinateWithinRectangle(QPointF p, QPointF center, double width, double height)
{
    float testx = p.x();
    float testy = p.y();

    QPointF w = calculateDestination(center, width/2, 90);
    QPointF e = calculateDestination(center, width/2, 270);

    QPointF nw = ( calculateDestination(w, height/2, 0) );

    if(nw.x() < 0)
        nw.setX(nw.x() + 360);

    QPointF sw = (calculateDestination(w, height/2, 180) );
    if(sw.x() < 0)
        sw.setX(sw.x() + 360);

    QPointF ne = (calculateDestination(e, height/2, 0) );
    if(ne.x() < 0)
        ne.setX(ne.x() + 360);

    QPointF se = (calculateDestination(e, height/2, 180) );
    if(se.x() < 0)
        se.setX(se.x() + 360);

    if(testx > nw.x() && testx < ne.x() && testy > se.y() && testy < ne.y())
        return true;
    else
        return false;
}

bool RwaUtilities::coordinateWithinPolygon3(std::vector<double> p, std::vector<std::vector<double>> &corners)
{
      bool  oddNodes=0;
      int j = corners.size()-1;
      int i = 0;

      for (i=0; i<corners.size(); i++)
      {
         if ( (corners[i][1]<p[1] && corners[j][1]>=p[1]) || (corners[j][1]<p[1] && corners[i][1]>=p[1] ))
         {
            if (corners[i][0]+(p[1]-corners[i][1])/(corners[j][1]-corners[i][1])*(corners[j][0]-corners[i][0])<p[0])
            {
                oddNodes=!oddNodes;


            }
         }
         j=i;
      }

      return oddNodes;
}

bool RwaUtilities::coordinateWithinPolygon2(QPointF p, std::vector<std::vector<double>> &corners)
{
      bool  oddNodes=0;
      int j = corners.size()-1;
      int i = 0;

      for (i=0; i<corners.size(); i++)
      {
         if ( (corners[i][1]<p.y() && corners[j][1]>=p.y()) || (corners[j][1]<p.y() && corners[i][1]>=p.y() ))
         {
            if (corners[i][0]+(p.y()-corners[i][1])/(corners[j][1]-corners[i][1])*(corners[j][0]-corners[i][0])<p.x())
            {
                oddNodes=!oddNodes;


            }
         }
         j=i;
      }

      return oddNodes;
}

bool RwaUtilities::coordinateWithinPolygon1(QPointF p, QVector <QPointF> *corners)
{
      bool  oddNodes=0;
      int j = corners->count()-1;
      int i = 0;

      for (i=0; i<corners->count(); i++)
      {
         if ( (corners->data()[i].y()<p.y() && corners->data()[j].y()>=p.y()) ||  (corners->data()[j].y()<p.y() && corners->data()[i].y()>=p.y() ))
         {
            if (corners->data()[i].x()+(p.y()-corners->data()[i].y())/(corners->data()[j].y()-corners->data()[i].y())*(corners->data()[j].x()-corners->data()[i].x())<p.x())
            {
                oddNodes=!oddNodes;
            }
         }
         j=i;
      }

      return oddNodes;
}

void RwaUtilities::calculatePolygonOffset2(double offset, std::vector<std::vector<double>> &corners, std::vector<std::vector<double>> &offsetCorners)
{
    Path subj;
    Paths solution;
    Path first;

    int i = 0;

    for (i=0; i<corners.size(); i++)
    {
        subj << IntPoint(corners[i][0]*10000000, corners[i][1]*10000000);
        //qDebug() << "ORG" << corners->data()[i].x() << " " << corners->data()[i].y();
    }

    ClipperOffset co;
    co.AddPath(subj, jtSquare, etClosedPolygon);
    co.Execute(solution, offset * 100);

    if(solution.empty())
        return;


    first = solution.data()[0];
    //qDebug() << first.size();

    for (i=0; i<first.size(); i++)
    {
        QPointF offsetPoint = QPointF(first.at(i).X/10000000.0, first.at(i).Y/10000000.0);
        std::vector<double> tmp(2, 0.0);
        tmp[0] = offsetPoint.x();
        tmp[1] = offsetPoint.y();

        offsetCorners.push_back(tmp);
        //qDebug() << "OFFSET "<< offsetPoint.x() << " " << offsetPoint.y();
    }
}

void RwaUtilities::calculatePolygonOffset1(double offset, QVector <QPointF> *corners, QVector <QPointF> *offsetCorners)
{
    Path subj;
    Paths solution;
    Path first;

    int i = 0;

    for (i=0; i<corners->count(); i++)
    {
        subj << IntPoint(corners->data()[i].x()*10000000, corners->data()[i].y()*10000000);
        //qDebug() << "ORG" << corners->data()[i].x() << " " << corners->data()[i].y();
    }

    ClipperOffset co;
    co.AddPath(subj, jtSquare, etClosedPolygon);
    co.Execute(solution, offset * 100);

    if(solution.empty())
        return;


    first = solution.data()[0];
    //qDebug() << first.size();

    for (i=0; i<first.size(); i++)
    {
        QPointF offsetPoint = QPointF(first.at(i).X/10000000.0, first.at(i).Y/10000000.0);
        offsetCorners->append(offsetPoint);
        //qDebug() << "OFFSET "<< offsetPoint.x() << " " << offsetPoint.y();
    }
}

void RwaUtilities::calculatePolygonOffset(QPointF middle, double offset, QVector <QPointF> *corners, QVector <QPointF> *offsetCorners)
{
      double bearing;
      double oldDistance;
      double newDistance;
      QPointF newCorner = QPointF(0,0);
      int i = 0;

      for (i=0; i<corners->count(); i++)
      {
          bearing = (RwaUtilities::calculateBearing(middle, corners->data()[i]));

          oldDistance = RwaUtilities::calculateDistance(middle, corners->data()[i]);
          newDistance = oldDistance*1000 + offset;
          newCorner = RwaUtilities::calculateDestination(middle, newDistance, bearing);

          if(newCorner.y() > middle.y())
          {
              double mirror = (newCorner.y() - middle.y()) * 2;
              newCorner.setY(newCorner.y() - mirror );
          }
          else
          {
              double mirror = (middle.y() - newCorner.y()) * 2;
              newCorner.setY(newCorner.y() + mirror );
          }

          offsetCorners->append(newCorner);
      }
}

RwaUtilities::RwaUtilities(QObject *parent) :
    QObject(parent)
{
}

bool RwaUtilities::checkDataType(QString fullpath)
{
    return false;

}

std::string RwaUtilities::getFileName(std::string fullpath)
{
    QFileInfo info(QString::fromStdString(fullpath));
    QString fileName = info.fileName();
    return fileName.toStdString();
}

QString RwaUtilities::getFileBaseName(QString fullpath)
{
    QFileInfo info(fullpath);
    QString fileName = info.baseName();
    return fileName;
}

QString RwaUtilities::getFileName(QString fullpath)
{
    QFileInfo info(fullpath);
    QString fileName = info.fileName();
    return fileName;
}

QString RwaUtilities::getPath(QString fullpath)
{
    QFileInfo info(fullpath);
    QString path = info.path();
    return path;
}


QString RwaUtilities::getDataType(QString fullpath)
{
    return "";
}

void RwaUtilities::emtpyDirectory(QString fullpath)
{
    QString path = fullpath;
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.*");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
    {
        dir.remove(dirFile);
    }
}


