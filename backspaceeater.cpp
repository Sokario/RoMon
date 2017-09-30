#include "backspaceeater.h"

BackspaceEater::BackspaceEater(QObject *parent) : QObject(parent)
{

}

BackspaceEater::~BackspaceEater()
{

}

bool BackspaceEater::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
//        qDebug("Keyboard pressed!");
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Backspace){
            QTextEdit *textEdit = static_cast<QTextEdit *>(obj);
            if (textEdit->textCursor().positionInBlock() >= CURSOR_POS)
                return QObject::eventFilter(obj, event);
            else {
//                qDebug("BackSpace ate!");
                return true;
            }
        } else
            return QObject::eventFilter(obj, event);
    } else {
// standard event processing
        return QObject::eventFilter(obj, event);
    }
}
