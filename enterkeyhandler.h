#ifndef ENTERKEYHANDLER_H
#define ENTERKEYHANDLER_H

#include <QObject>
#include "QEvent"
#include "QKeyEvent"
#include "QApplication"

class EnterKeyHandler : public QObject
{
    Q_OBJECT

public:
    explicit EnterKeyHandler(QObject *parent = nullptr);
    ~EnterKeyHandler();

signals:
    void enterKeyPressed();

public slots:

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // ENTERKEYHANDLER_H
