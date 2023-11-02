#include <QLabel>

class MyLabel: public QLabel{
Q_OBJECT
public:
    MyLabel(){

    }
signals:
    void imageAvailable(const QImage &newValue);
public slots:
    void onImageAvailable(const QImage &newValue);
};