#ifndef RWAAPPLICATION_H
#define RWAAPPLICATION_H

/**
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * The RwaApplication class subclasses QApplication to receive document-open
 * requests from the operating system. On macOS, Finder (double-click, drag
 * onto the Dock icon, "Open With") never passes the file path via argv,
 * it sends an Apple Event which Qt translates into a QFileOpenEvent
 * delivered to the application object.
 *
 * Requests that arrive before the main window is ready (macOS may deliver
 * them at any point during startup) are queued and flushed by setReady().
 */

#include <QApplication>
#include <QFileOpenEvent>
#include <QStringList>

class RwaApplication : public QApplication
{
    Q_OBJECT

public:
    RwaApplication(int &argc, char **argv) : QApplication(argc, argv) {}

    /** Call once the main window exists and can accept documents.
      * Flushes open requests the OS delivered during startup. */
    void setReady()
    {
        ready = true;
        const QStringList queued = pending;
        pending.clear();
        for (const QString &path : queued)
            emit openDocumentRequested(path);
    }

signals:
    void openDocumentRequested(const QString &path);

protected:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen)
        {
            auto *fileOpenEvent = static_cast<QFileOpenEvent *>(event);
            const QString path = fileOpenEvent->url().isLocalFile()
                                     ? fileOpenEvent->url().toLocalFile()
                                     : fileOpenEvent->file();
            if (path.isEmpty())
                return true;

            if (ready)
                emit openDocumentRequested(path);
            else
                pending.append(path);
            return true;
        }
        return QApplication::event(event);
    }

private:
    bool ready = false;
    QStringList pending;
};

#endif
