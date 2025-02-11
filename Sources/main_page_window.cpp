#include "main_page_window.h"
#include <QJsonArray>
#include <QJsonDocument>
#include "globals.h"

MainPageWindow::MainPageWindow(QWidget *parent)
    : QWidget(parent)
    , scroll_widget(new ScrollWidget(this))
    , SearchButton(new QPushButton(this))
    , ProfileButton(new QPushButton(this))
    , searchBar(new QLineEdit(this))
    , chat(new QLabel(this))
{
    QPixmap searchIcon(":/pngs/searchicon.png");
    QPixmap profileIcon(VChatWidget::cut_photo(":/pngs/panda.jpg", 40));
    client_main_page = new HttpClient();

    SearchButton->setGeometry(340, 10, 50, 50);
    SearchButton->setIcon(searchIcon);
    SearchButton->setIconSize(searchIcon.size());
    SearchButton->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: transparent;"
                                "}"
                                "QPushButton:hover {"
                                "    background-color: rgba(142, 21, 222, 0.1);"
                                "}"
                                "QPushButton:pressed {"
                                "    background-color: rgba(142, 21, 222, 0.2);"
                                "}");

    searchBar->setGeometry(70, 10, 265, 50);
    searchBar->setStyleSheet("QLineEdit {"
                             "    border: 2px solid #8e15de;"
                             "    border-radius: 10px;"
                             "    padding: 5px;"
                             "    background-color: #D4D4D6;"
                             "    font-size: 14px;"
                             "    font-family: 'Century Gothic', sans-serif;"
                             "    color: #333333;"
                             "}"
                             "QLineEdit:hover {"
                             "    border: 2px solid #9b30ff;"
                             "}"
                             "QLineEdit:focus {"
                             "    border: 2px solid #ba55d3;"
                             "    background-color: #f5f5f5;"
                             "}");

    chat->setGeometry(170, 30, 100, 100);
    chat->setStyleSheet("color: #8e15de; font-size: 16px; font-family: 'Century Gothic', "
                        "sans-serif; font-weight: bold;");

    ProfileButton->setGeometry(10, 10, 50, 50);
    ProfileButton->setIcon(profileIcon);
    ProfileButton->setIconSize(profileIcon.size());
    ProfileButton->setStyleSheet("QPushButton {"
                                 "    border: none;"
                                 "    background-color: transparent;"
                                 "}"
                                 "QPushButton:hover {"
                                 "    background-color: rgba(142, 21, 222, 0.1);"
                                 "}"
                                 "QPushButton:pressed {"
                                 "    background-color: rgba(142, 21, 222, 0.2);"
                                 "}");
    connections();
    setLanguage();
}

void MainPageWindow::setLanguage()
{
    chat->setText(tr("Chats"));
    searchBar->setPlaceholderText(tr("search..."));
}

void MainPageWindow::connections()
{
    connect(ProfileButton, &QPushButton::clicked, this, &MainPageWindow::handleProfileButton);

    connect(SearchButton, &QPushButton::clicked, this, &MainPageWindow::handleSearch);

    connect(searchBar, &QLineEdit::returnPressed, this, &MainPageWindow::handleSearch);

    connect(this, &MainPageWindow::contact_successfully_added_to_scrollWidget, this, [this](){
        scroll_widget->clear_chats();
        auto ptr =  contacts.back();
        scroll_widget->add_chat(ptr);
        connect(ptr, &VChatWidget::clicked_vchat, this, &MainPageWindow::handle_vchat_click);
        scroll_widget->show_chats();
    });

    connect(this, &MainPageWindow::matched_contact_or_other_user_added_successfully, this, [this](bool contact_or_otherUser){
        scroll_widget->clear_search_chats();
        if(contact_or_otherUser)
        {
            auto ptr = matched_contacts.back();
            scroll_widget->add_matched_contact(ptr);
            qDebug()<<"Added matched contact to scroll widget"<<ptr ->get_nick();
            connect(ptr, &VChatWidget::clicked_vchat, this, &MainPageWindow::vchat_clicked_from_search_pg);
        }
        else
        {
            auto ptr = matched_other_users.back();
            scroll_widget->add_matched_other_users(ptr);
            qDebug()<<"Added matched other user to scroll widget"<<ptr ->get_nick();
            connect(ptr, &VChatWidget::clicked_vchat, this, &MainPageWindow::vchat_clicked_from_search_pg);
        }
        scroll_widget->show_search_chats();
    });
}


void MainPageWindow::handle_vchat_click(QString nickname)
{
    qDebug() << "Opening chat with: " << nickname;
    scroll_widget->clear_search_chats();
    scroll_widget->delete_search_chats();
    emit vchat_clicked_from_main_pg(nickname);
}

void MainPageWindow::vchat_clicked_from_search_pg(QString nickname)
{
    if (contacts_nicknames_to_get_account_info.contains(nickname)) {
        clear_matched_arrays();
        handle_vchat_click(nickname);
        get_contacts_info_and_show();
        return;
    }

    disconnect(client_main_page, &HttpClient::responseReceived, nullptr, nullptr);
    connect(client_main_page, &HttpClient::responseReceived, this, [=](QByteArray responseData){
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObject = jsonResponse.object();
        if (jsonObject.contains("message") && jsonObject["message"].toString() == "Contact added successfully to both users")
        {
            qDebug() << "Contact added successfully: " << nickname;
            contacts_nicknames_to_get_account_info.append(nickname);
            clear_matched_arrays();
            scroll_widget->clear_chats();
            scroll_widget->delete_all_chats();
            handle_vchat_click(nickname);
            get_contacts_info_and_show();
        }
        else
        {
            qDebug() << "Failed to add contact: " << jsonObject.value("detail").toString();
        }
    });

    QString addContactRequestLink = "https://synergy-iauu.onrender.com/addcontact/" + Globals::getInstance().getUserId() + "/" + nickname;
    // QJsonObject jsonData;
    // jsonData["user_id"] = Globals::getInstance().getUserId();
    // jsonData["contact_nickname"] = nickname;
    qDebug() << "Sending request to:" << addContactRequestLink;
    qDebug() << "Request body:" << nickname;
    client_main_page->postRequest(addContactRequestLink, {});
}

void MainPageWindow::handleProfileButton()
{
    qDebug() << "profile button";
    emit profile_button_signal();
}

//kanchvuma es funkcian vor stananq contactnery    //getRequest Profile Info
void MainPageWindow::handleIdReceiving()
{
    disconnect(client_main_page, &HttpClient::responseReceived, this, &MainPageWindow::handle_contact);

    connect(client_main_page, &HttpClient::responseReceived, this, &MainPageWindow::handle_contact);

    QString link("https://synergy-iauu.onrender.com/profile_info/");
    QUrl accountinfo(link + Globals::getInstance().getUserId());
    client_main_page->getRequest(accountinfo);
}

void MainPageWindow::clearDataOnLogout()
{
    clear_contact_array();
    clear_matched_arrays();

    contacts_nicknames_to_get_account_info.clear();
    scroll_widget->clear_chats();
    scroll_widget->clear_search_chats();
    scroll_widget->delete_all_chats();
    scroll_widget->delete_search_chats();
    disconnect(client_main_page, &HttpClient::responseReceived, nullptr, nullptr);
}


//anuny miqich sxala pti liner remove all data on log out
void MainPageWindow::handleContactReDonwnload()
{
    scroll_widget->clear_search_chats();
    scroll_widget->delete_search_chats();
    scroll_widget->clear_chats();
    scroll_widget->delete_all_chats();
    contacts_nicknames_to_get_account_info.clear();
    clear_contact_array();
    clear_matched_arrays();
}

void MainPageWindow::clear_matched_arrays()
{
    matched_contacts.clear();
    matched_other_users.clear();
    matched_contacts.resize(0);
    matched_other_users.resize(0);
}

void MainPageWindow::clear_contact_array()
{
    contacts.clear();
    contacts.resize(0);
}

// response stanaluc vercnum enq mer contactneri nickname ery ev gcum QStringList i mej vor heto avelacnenq mainpage um
void MainPageWindow::handle_contact(QByteArray responseData)
{
    // disconnect(client_main_page, &HttpClient::responseReceived, this, &MainPageWindow::handle_contact);

    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    if (jsonResponse.isNull() || !jsonResponse.isObject()) {
        qDebug() << "Invalid JSON response.";
        return;
    }

    QJsonObject jsonObject = jsonResponse.object();
    QJsonValue contactsValue = jsonObject.value("contacts");

    QByteArray contactsByteArray = contactsValue.toString().toUtf8();
    QJsonDocument contactsDoc = QJsonDocument::fromJson(contactsByteArray);

    if (contactsDoc.isNull() || !contactsDoc.isArray()) {
        qDebug() << "Failed to parse contacts as a JSON array!";
        return;
    }

    contacts_nicknames_to_get_account_info.clear();
    scroll_widget->clear_chats();

    QJsonArray contactsArray = contactsDoc.array();
    for (const QJsonValue &value : contactsArray) {
        if (value.isString()) {
            contacts_nicknames_to_get_account_info.append(value.toString());
        }
    }

    qDebug() << "Extracted nicknames: " << contacts_nicknames_to_get_account_info;

    disconnect(this, &MainPageWindow::received_contacts, this, &MainPageWindow::get_contacts_info_and_show);
    connect(this, &MainPageWindow::received_contacts, this, &MainPageWindow::get_contacts_info_and_show);
    emit received_contacts();
}

void MainPageWindow::get_contacts_info_and_show()
{
    int count_of_contacts = contacts_nicknames_to_get_account_info.size();
    qDebug() << "count_of_contacts = contacts_nicknames_to_get_account_info.size();" << count_of_contacts;
    for (int i = 0; i < count_of_contacts; ++i)
    {
        disconnect(client_main_page, &HttpClient::responseReceived, nullptr, nullptr);

        connect(client_main_page, &HttpClient::responseReceived, this, [this](QByteArray responseData) {
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            if (!jsonDoc.isObject()) {
                qDebug() << "Invalid JSON response in get_contacts_info_and_show";
                return;
            }

            QJsonObject jsonObject = jsonDoc.object();
            QString name = jsonObject.value("name").toString();
            QString surname = jsonObject.value("surname").toString();
            QString nickname = jsonObject.value("nickname").toString();

            qDebug() << "Received Contact Info - Name:" << name << ", Surname:" << surname << "Nickname:" << nickname;

            contacts.emplaceBack(new VChatWidget(name, nickname, surname));

            emit contact_successfully_added_to_scrollWidget();
        });

        QString contactInfoGetRequestLink = "https://synergy-iauu.onrender.com/getContactInfo/"
                                            + Globals::getInstance().getUserId() + "/" + contacts_nicknames_to_get_account_info[i];
        client_main_page->getRequest(contactInfoGetRequestLink);
    }
}
// void MainPageWindow::get_contacts_info_and_show()
// {
//     int count_of_contacts = contacts_nicknames_to_get_account_info.size();
//     qDebug() << "Fetching contact details for:" << count_of_contacts << " contacts";
//     scroll_widget->clear_chats();
//     scroll_widget->delete_all_chats();
//     scroll_widget->clear_search_chats();
//     scroll_widget->delete_search_chats();
//     clear_contact_array();
//     disconnect(client_main_page, &HttpClient::responseReceived, nullptr, nullptr);
//     connect(client_main_page, &HttpClient::responseReceived, this, [this](QByteArray responseData) {
//         QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
//         if (!jsonDoc.isObject()) {
//             qDebug() << "Invalid JSON response in get_contacts_info_and_show";
//             return;
//         }

//         QJsonObject jsonObject = jsonDoc.object();
//         QString name = jsonObject.value("name").toString();
//         QString surname = jsonObject.value("surname").toString();
//         QString nickname = jsonObject.value("nickname").toString();

//         qDebug() << "Received Contact Info - Name:" << name << ", Surname:" << surname << "Nickname:" << nickname;

//         contacts.emplaceBack(new VChatWidget(name, nickname, surname));

//         emit contact_successfully_added_to_scrollWidget();
//     });

//     for (int i = 0; i < count_of_contacts; ++i)
//     {
//         QString contactInfoGetRequestLink = "https://synergy-iauu.onrender.com/getContactInfo/"
//                                             + Globals::getInstance().getUserId() + "/" + contacts_nicknames_to_get_account_info[i];
//         client_main_page->getRequest(contactInfoGetRequestLink);
//     }
// }


void MainPageWindow::handle_search_data(QByteArray responseData)
{
    disconnect(client_main_page, &HttpClient::responseReceived, this, &MainPageWindow::handle_search_data);
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (!jsonDoc.isObject()) {
        qWarning() << "Invalid JSON response.";
        return;
    }
    QJsonObject jsonObject = jsonDoc.object();
    clear_matched_arrays();

    if (jsonObject.contains("matched_contacts") && jsonObject["matched_contacts"].isArray())
    {
        QJsonArray matchedContactsArray = jsonObject["matched_contacts"].toArray();
        for (const QJsonValue &val : matchedContactsArray)
        {
            if (val.isObject())
            {
                QJsonObject userObj = val.toObject();
                QString nickname = userObj.value("nickname").toString();
                QString name = userObj.value("name").toString();
                QString surname = userObj.value("surname").toString();

                VChatWidget* contactWidget = new VChatWidget(name,nickname, surname);
                matched_contacts.append(contactWidget);
                emit matched_contact_or_other_user_added_successfully(true);
            }
        }
    }

    if (jsonObject.contains("matched_other_users") && jsonObject["matched_other_users"].isArray())
    {
        QJsonArray matchedOtherUsersArray = jsonObject["matched_other_users"].toArray();
        for (const QJsonValue &val : matchedOtherUsersArray)
        {
            if (val.isObject())
            {
                QJsonObject userObj = val.toObject();
                QString nickname = userObj.value("nickname").toString();
                QString name = userObj.value("name").toString();
                QString surname = userObj.value("surname").toString();

                VChatWidget* userWidget = new VChatWidget(name,nickname, surname);
                matched_other_users.append(userWidget);
                emit matched_contact_or_other_user_added_successfully(false);
            }
        }
    }

    qDebug() << "Matched Contacts:" << matched_contacts.size();
    qDebug() << "Matched Other Users:" << matched_other_users.size();

    if (!(matched_contacts.isEmpty() && matched_other_users.isEmpty()))
    {

        searchBar->setText("");
        clear_contact_array();
        scroll_widget->clear_chats();
        scroll_widget->show_search_chats();
    }
}

// void MainPageWindow::handle_search_data(QByteArray responseData)
// {
//     disconnect(client_main_page, &HttpClient::responseReceived, this, &MainPageWindow::handle_search_data);
//     QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
//     if (!jsonDoc.isObject()) {
//         qWarning() << "Invalid JSON response.";
//         return;
//     }
//     QJsonObject jsonObject = jsonDoc.object();

//     // Clear matched arrays before adding new results
//     clear_matched_arrays();

//     if (jsonObject.contains("matched_contacts") && jsonObject["matched_contacts"].isArray())
//     {
//         QJsonArray matchedContactsArray = jsonObject["matched_contacts"].toArray();
//         for (const QJsonValue &val : matchedContactsArray)
//         {
//             if (val.isObject())
//             {
//                 QJsonObject userObj = val.toObject();
//                 QString nickname = userObj.value("nickname").toString();
//                 QString name = userObj.value("name").toString();
//                 QString surname = userObj.value("surname").toString();

//                 VChatWidget* contactWidget = new VChatWidget(name,nickname, surname);
//                 matched_contacts.append(contactWidget);
//                 emit matched_contact_or_other_user_added_successfully(true);
//             }
//         }
//     }

//     if (jsonObject.contains("matched_other_users") && jsonObject["matched_other_users"].isArray())
//     {
//         QJsonArray matchedOtherUsersArray = jsonObject["matched_other_users"].toArray();
//         for (const QJsonValue &val : matchedOtherUsersArray)
//         {
//             if (val.isObject())
//             {
//                 QJsonObject userObj = val.toObject();
//                 QString nickname = userObj.value("nickname").toString();
//                 QString name = userObj.value("name").toString();
//                 QString surname = userObj.value("surname").toString();

//                 VChatWidget* userWidget = new VChatWidget(name,nickname, surname);
//                 matched_other_users.append(userWidget);
//                 emit matched_contact_or_other_user_added_successfully(false);
//             }
//         }
//     }

//     qDebug() << "Matched Contacts:" << matched_contacts.size();
//     qDebug() << "Matched Other Users:" << matched_other_users.size();

//     if (!(matched_contacts.isEmpty() && matched_other_users.isEmpty()))
//     {
//         searchBar->setText("");
//     }
// }


void MainPageWindow::handleSearch()
{
    QString searchText = searchBar->text();
    if (!searchText.isEmpty())
    {
        qDebug() << "Search query:" << searchText;
        QString searchLink = "https://synergy-iauu.onrender.com/search/"
                             + Globals::getInstance().getUserId() + "/" + searchText;
        connect(client_main_page, &HttpClient::responseReceived, this, &MainPageWindow::handle_search_data, Qt::UniqueConnection);
        client_main_page->getRequest(searchLink);
    }
    else
    {
        qDebug() << "Search query is empty";
    }
}
// void MainPageWindow::handleSearch()
// {
//     QString searchText = searchBar->text();
//     if (!searchText.isEmpty())
//     {
//         qDebug() << "Search query:" << searchText;

//         // Clear previous search results before making a new request
//         scroll_widget->clear_search_chats();
//         scroll_widget->delete_search_chats();

//         QString searchLink = "https://synergy-iauu.onrender.com/search/"
//                              + Globals::getInstance().getUserId() + "/" + searchText;
//         connect(client_main_page, &HttpClient::responseReceived, this, &MainPageWindow::handle_search_data, Qt::UniqueConnection);
//         client_main_page->getRequest(searchLink);
//     }
//     else
//     {
//         qDebug() << "Search query is empty";
//     }
// }



void MainPageWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        searchBar->setFocus();
    }
}
