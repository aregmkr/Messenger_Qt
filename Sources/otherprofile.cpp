#include "otherprofile.h"
#include <QDebug>
#include <QPixmap>
#include <QJsonDocument>
#include "v_chat_widget.h"
#include "globals.h"

OtherProfile::OtherProfile(QWidget *parent)
    : QWidget(parent)
{
    this->resize(400, 700);
    this->setFixedSize(400, 700);
    init();
    setup();
    setLanguage();
    styling();
    connections();
}

void OtherProfile::setLanguage()
{
    qDebug() << "Setting text in Other Proffile";
    goBackButton->setText(tr("Back"));
}

void OtherProfile::handleDataFromChat(QString nick)
{
    disconnect(client_other_profile, &HttpClient::responseReceived, nullptr, nullptr);

    connect(client_other_profile, &HttpClient::responseReceived, this, [=](QByteArray responseData) {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (!jsonDoc.isObject()) {
            qDebug() << "Invalid JSON response in get_contacts_info_and_show";
            return;
        }

        QJsonObject jsonObject = jsonDoc.object();
        QString name = jsonObject.value("name").toString();
        QString surname = jsonObject.value("surname").toString();
        QString nickname = jsonObject.value("nickname").toString();
        nameLabel->setText(name);
        surnameLabel->setText(surname);
        nicknameLabel->setText("@" + nickname);
    });
    QString contactInfoGetRequestLink = "https://synergy-iauu.onrender.com/getContactInfo/"
                                        + Globals::getInstance().getUserId() + "/" + nick;
    client_other_profile->getRequest(contactInfoGetRequestLink);
}

void OtherProfile::init()
{
    client_other_profile = new HttpClient;
    profilePhoto = new QLabel(this);
    nameLabel = new QLabel(this);
    surnameLabel = new QLabel(this);
    nicknameLabel = new QLabel(this);
    goBackButton = new QPushButton(this);
}

void OtherProfile::setup()
{
    nameLabel->setText("John");
    surnameLabel->setText("doe");
    nicknameLabel->setText("@johndoe");

    goBackButton->setGeometry(10, 10, 80, 40);
    profilePhoto->setGeometry(150, 10, 100, 100);

    nameLabel->setGeometry(20, 130, 360, 30);
    surnameLabel->setGeometry(20, 170, 360, 30);
    nicknameLabel->setGeometry(20, 210, 360, 30);

    QPixmap profilePic("");
    if (!profilePic.isNull())
    {
        profilePhoto->setPixmap(
            VChatWidget::cut_photo(":/pngs/panda.jpg", 100)); // stex petqa lini back ic ekac nkary
    }
    else
    {
        qDebug() << "Failed to load profile photo. Using default.";
        profilePhoto->setPixmap(VChatWidget::cut_photo(":/pngs/panda.jpg", 100));
    }
}

void OtherProfile::styling()
{
    QString labelStyle = R"(
        QLabel {
            color: #FFFFFF;
            font-size: 12px;
            border: 3px solid #420242;
            border-radius: 5px;
            padding: 3px;
        }
    )";

    nameLabel->setStyleSheet(labelStyle);
    surnameLabel->setStyleSheet(labelStyle);
    nicknameLabel->setStyleSheet(labelStyle);

    QString buttonStyle = R"(
        QPushButton {
            border: 1px solid #420242;
            border-radius: 5px;
            padding: 8px 15px;
            background-color: #420242;
            color: white;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #5A005A;
        }
        QPushButton:pressed {
            background-color: #2A002A;
        }
    )";

    goBackButton->setStyleSheet(buttonStyle);
}

void OtherProfile::connections()
{
    connect(goBackButton, &QPushButton::clicked, this, [this]() { emit goBackSignal(); });
}
