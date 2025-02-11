// messagesFrame->setFixedSize(380, 570);

#include "chat_widget.h"
#include "globals.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>

ChatWidget::ChatWidget(QString nick, QWidget *parent)
    : QWidget(parent)
{
    this->setFixedSize(400, 700);
    chat_client = new HttpClient();
    y = 20;
    x = 200;

    chatReloadTimer = new QTimer(this);
    chatReloadTimer->setInterval(10000);

    v_user = new VChatWidget("name", nick, "surname", this);
    v_user->setGeometry(120, 630, 272, 60);

    send_btn = new QPushButton(this);
    send_btn->setGeometry(340, 10, 50, 55);

    scroll = new QScrollArea(this);
    scroll->setGeometry(10, 70, 380, 560);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    contentWidget = new QWidget();
    contentWidget->setGeometry(0, 0, 380, 560);

    scroll->setWidget(contentWidget);

    backButton = new QPushButton(this);
    backButton->setGeometry(12, 632, 100, 55);
    backButton->setStyleSheet(
        "background-color: #8e15de; color: white; font-weight: bold; border-radius: 5px;");

    line = new QLineEdit(this);
    line->setObjectName("messageInput");
    line->setGeometry(10, 10, 325, 55);

    connect(v_user, &VChatWidget::clicked_vchat, this, &ChatWidget::handle_profile_signal);
    connect(backButton, &QPushButton::clicked, this, &ChatWidget::handle_go_back);
    connect(send_btn, &QPushButton::clicked, this, &ChatWidget::sendMessage);
    connect(line, &QLineEdit::returnPressed, this, &ChatWidget::handle_line);
    connect(chatReloadTimer, &QTimer::timeout, this, [=]() {
        loadChat(v_user->get_nick());
    });
    chatReloadTimer->start();
    loadChat(nick);
    setLanguage();
}

void ChatWidget::setNick(QString nick)
{
    clearMessages();
    v_user->set_nick(nick);
    loadChat(nick);
}

QString ChatWidget::getNick() const
{
    return v_user->get_nick();
}

void ChatWidget::handle_profile_signal(QString nick)
{
    emit other_profile_signal(nick);
}

void ChatWidget::handle_go_back()
{
    emit go_back_signal();
}

void ChatWidget::setLanguage()
{
    send_btn->setText(tr("✉️"));
    backButton->setText(tr("Back"));
    line->setPlaceholderText(tr("Type message..."));
}

void ChatWidget::sendMessage(bool isOutgoing)
{
    QString contact_nickname = v_user->get_nick();
    QString url_path = QString("https://synergy-iauu.onrender.com/sendMessage/%1/%2")
                           .arg(Globals::getInstance().getUserId())
                           .arg(contact_nickname);
    QUrl url(url_path);
    QJsonObject jsonData;
    jsonData["sender"] = Globals::getInstance().getUserId();
    jsonData["text"] = line->text();
    jsonData["send_date"] = QDate::currentDate().toString(Qt::ISODate);

    qDebug() << "Sending message to:" << url.toString();
    qDebug() << "Message content:" << jsonData;

    if (!line->text().isEmpty()) {
        addMessage(line->text(), isOutgoing);
        line->clear();
    }
    connect(chat_client, &HttpClient::responseReceived, this, [=](QByteArray responseData) {
        qDebug() << "Message sent. Reloading chat.";
        loadChat(contact_nickname);
    });

    chat_client->postRequest(url, jsonData);

}

void ChatWidget::addMessage(const QString &message_text, bool isOutgoing)
{
    ChatMessageWidget *message = new ChatMessageWidget(message_text, contentWidget);

    int messageWidth = message->width() + 20;
    int messageHeight = message->height() + 20;

    if (isOutgoing) {
        message->setGeometry(380 - messageWidth - 40, y, messageWidth, messageHeight);
    } else {
        message->setGeometry(20, y, messageWidth, messageHeight);
    }

    y += messageHeight + 10;
    int newContentHeight = y + messageHeight + 20;
    message->show();
    contentWidget->setMinimumHeight(newContentHeight);
}


void ChatWidget::clearMessages()
{
    QList<QWidget *> children = contentWidget->findChildren<QWidget *>();
    for (QWidget *child : children) {
        child->deleteLater();
    }
    y = 20;
    contentWidget->setMinimumHeight(0);
}

void ChatWidget::loadChat(QString nickname)
{
    clearMessages();
    QString url_path = QString("https://synergy-iauu.onrender.com/getMessageHistory/%1/%2")
                           .arg(Globals::getInstance().getUserId())
                           .arg(nickname);
    QUrl url(url_path);

    disconnect(chat_client, &HttpClient::responseReceived, nullptr, nullptr);

    connect(chat_client, &HttpClient::responseReceived, this, [=](QByteArray responseData) {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isObject()) {
            qDebug() << "Invalid JSON response in loadChat";
            return;
        }

        QJsonObject jsonObject = jsonDoc.object();
        if (!jsonObject.contains("conversation") || !jsonObject["conversation"].isArray()) {
            qDebug() << "No conversation data found";
            return;
        }

        QJsonArray messagesArray = jsonObject["conversation"].toArray();
        QString currentUserId = Globals::getInstance().getUserId();

        for (const QJsonValue &messageVal : messagesArray) {
            if (!messageVal.isObject()) continue;

            QJsonObject messageObj = messageVal.toObject();
            QString sender = messageObj["sender"].toString();
            QString text = messageObj["text"].toString();
            bool isOutgoing = (sender == currentUserId);

            addMessage(text, isOutgoing);
        }
    });

    chat_client->getRequest(url.toString());
}

void ChatWidget::handle_line()
{
    QString searchText = line->text();
    if (!searchText.isEmpty()) {
        qDebug() << "Search query:" << searchText;
        //chat filtration, request
        // if (scroll_widget->s)
        } else {
        qDebug() << "Search query is empty";
    }
}
