#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include "settings.h"
#include "engine.h"

static const int ShrinkWidth = 230;
static const int ExpandWidth = 744;

#include <QMessageBox>
#include <QRegExpValidator>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);

#ifdef CHINESE_SUPPORT
    QRegExp pattern("[^-+@: =!#%*]+");
#else
    QRegExp pattern("[a-zA-Z_0-9]+");
#endif

    QRegExpValidator *validator = new QRegExpValidator(pattern, this);
    ui->nameLineEdit->setValidator(validator);

    ui->nameLineEdit->setText(Config.UserName);
    ui->hostLineEdit->setText(Config.HostAddress);
    ui->portLineEdit->setValidator(new QIntValidator(0, USHRT_MAX, ui->portLineEdit));
    ui->portLineEdit->setText(QString::number(Config.Port));
    ui->spinBox->setValue(Config.PlayerCount);

    ui->connectButton->setFocus();

    const General *avatar_general = Sanguosha->getGeneral(Config.UserAvatar);
    if(avatar_general){
        QPixmap avatar(avatar_general->getPixmapPath("big"));
        ui->avatarPixmap->setPixmap(avatar);
    }

    QList<const General*> generals = Sanguosha->findChildren<const General*>();
    foreach(const General *general, generals){
        QIcon icon(general->getPixmapPath("big"));
        QString text = Sanguosha->translate(general->objectName());
        QListWidgetItem *item = new QListWidgetItem(icon, text, ui->avatarList);
        item->setData(Qt::UserRole, general->objectName());
    }

    ui->avatarList->hide();

    setFixedHeight(height());
    setFixedWidth(ShrinkWidth);
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

void ConnectionDialog::on_connectButton_clicked()
{
    QString username = ui->nameLineEdit->text();
    username = username.trimmed();

    Config.setValue("UserName", Config.UserName = username);
    Config.setValue("HostAddress", Config.HostAddress = ui->hostLineEdit->text());
    Config.setValue("Port", Config.Port = ui->portLineEdit->text().toUShort());
    Config.setValue("PlayerCount", Config.PlayerCount = ui->spinBox->value());

    accept();
}

void ConnectionDialog::on_changeAvatarButton_clicked()
{
    if(ui->avatarList->isVisible()){
        QListWidgetItem *selected = ui->avatarList->currentItem();
        if(selected)
            on_avatarList_itemDoubleClicked(selected);
        else{
            ui->avatarList->hide();
            setFixedWidth(ShrinkWidth);
        }
    }else{
        ui->avatarList->show();
        setFixedWidth(ExpandWidth);
    }
}

void ConnectionDialog::on_avatarList_itemDoubleClicked(QListWidgetItem* item)
{    
    QString general_name = item->data(Qt::UserRole).toString();
    const General *general = Sanguosha->getGeneral(general_name);
    if(general){
        QPixmap avatar(general->getPixmapPath("big"));
        ui->avatarPixmap->setPixmap(avatar);
        Config.UserAvatar = general_name;
        Config.setValue("UserAvatar", general_name);
        ui->avatarList->hide();

        setFixedWidth(ShrinkWidth);
    }
}
