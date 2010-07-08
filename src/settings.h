#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QFont>
#include <QRectF>
#include <QPixmap>
#include <QScriptEngine>
#include <QBrush>

class Settings : public QSettings
{
public:
    explicit Settings(const QString &organization, const QString &application);
    void init();

    const QRectF Rect;
    QFont BigFont;
    QFont SmallFont;
    QFont TinyFont;

    QString UserName;
    QString HostAddress;
    ushort Port;
    QString UserAvatar;

    bool FitInView;
    bool UseOpenGL;
    bool EnableHotKey;

    QBrush BackgroundBrush;
};

extern Settings Config;

#endif // SETTINGS_H
