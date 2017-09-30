#include "enterkeyhandler.h"

EnterKeyHandler::EnterKeyHandler(QObject *parent) : QObject(parent)
{

}

EnterKeyHandler::~EnterKeyHandler()
{

}

bool EnterKeyHandler::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
//        qDebug("Keyboard pressed!");
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return) {
//            if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
//                emit shiftEnterKeyMacro();
//                qDebug("Modifier pressed!");
            emit enterKeyPressed();
            return QObject::eventFilter(obj, event);
        } else
            return QObject::eventFilter(obj, event);
    } else {
// standard event processing
        return QObject::eventFilter(obj, event);
    }
}
