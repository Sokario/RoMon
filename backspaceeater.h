#ifndef BACKSPACEEATER_H
#define BACKSPACEEATER_H

#include <QObject>
#include "QEvent"
#include "QKeyEvent"
#include "QTextEdit"

#define CURSOR_POS 6

class BackspaceEater : public QObject
{
    Q_OBJECT
public:
    explicit BackspaceEater(QObject *parent = nullptr);
    ~BackspaceEater();

signals:

public slots:

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // BACKSPACEEATER_H
