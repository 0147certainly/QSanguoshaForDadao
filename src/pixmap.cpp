#include "pixmap.h"

#include <QPainter>
#include <QGraphicsColorizeEffect>
#include <QMessageBox>

Pixmap::Pixmap(const QString &filename, bool center_as_origin)
    :pixmap(filename), markable(false), marked(false)
{
    if(pixmap.isNull()){
        QString warning = tr("Can not load image %1[%2]").arg(filename).arg(metaObject()->className());
        QMessageBox::warning(NULL, tr("Warning"), warning);
    }

    if(center_as_origin)
        setTransformOriginPoint(pixmap.width()/2, pixmap.height()/2);
}

Pixmap::Pixmap()
    :markable(false), marked(false)
{
}

QRectF Pixmap::boundingRect() const{
    return QRectF(0, 0, pixmap.width(), pixmap.height());
}

void Pixmap::changePixmap(const QString &filename){
    pixmap.load(filename);
}

void Pixmap::shift(){
    moveBy(-pixmap.width()/2, -pixmap.height()/2);
}

void Pixmap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->drawPixmap(0, 0, pixmap);
}

QVariant Pixmap::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change == ItemSelectedChange){
        if(value.toBool()){
            QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect(this);
            effect->setColor(QColor(0xCC, 0x00, 0x00));
            setGraphicsEffect(effect);
        }else
            setGraphicsEffect(NULL);
    }else if(change == ItemEnabledChange){
        if(value.toBool()){
            setOpacity(1.0);
        }else{
            setOpacity(0.7);
        }
    }

    return QGraphicsObject::itemChange(change, value);
}

void Pixmap::mousePressEvent(QGraphicsSceneMouseEvent *event){
    setSelected(true);
}


bool Pixmap::isMarked() const{
    return markable && marked;
}

bool Pixmap::isMarkable() const{
    return markable;
}

void Pixmap::mark(bool marked){
    if(markable){
        if(this->marked != marked){
            this->marked = marked;
            emit mark_changed();
        }
    }
}

void Pixmap::setMarkable(bool markable){
    this->markable = markable;
}
