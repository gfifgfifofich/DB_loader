#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QWidget>

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    ColorButton(QString text) : QPushButton(text) {};
public slots:

    void on_pressed()
    {
        emit cb_pressed(this->whatsThis());
    }
signals:

    void cb_pressed(QString);
};

#endif // COLORBUTTON_H
