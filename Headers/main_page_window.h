#ifndef MAIN_PAGE_WINDOW_H
#define MAIN_PAGE_WINDOW_H

#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QStringList>
#include <QVector>
#include <QWidget>
#include "httpclient.h"
#include "scroll_widget.h"
#include "v_chat_widget.h"

class MainPageWindow : public QWidget
{
    Q_OBJECT

public:
    MainPageWindow(QWidget *parent = nullptr);
    ~MainPageWindow() = default;
    void setLanguage();
private:
    void connections();


private:
    ScrollWidget *scroll_widget;
    QPushButton *SearchButton;
    QPushButton *ProfileButton;
    QLineEdit *searchBar;    
    QLabel *chat;
    HttpClient *client_main_page;

//for search box
    QVector<VChatWidget*> matched_contacts;
    QVector<VChatWidget*> matched_other_users;

// esqany login ylneluc contactnery stanalu u cuyc talu hamar
private:
    QVector<VChatWidget*> contacts;
    QStringList contacts_nicknames_to_get_account_info;
private slots:
    void handle_contact(QByteArray responseData);
    void get_contacts_info_and_show();
public slots:
    void handleIdReceiving();
// /////////////////////////////////////////////////////////
public slots:
    void clearDataOnLogout();
    void handleContactReDonwnload();

private:
    void clear_matched_arrays();
    void clear_contact_array();

private slots:
    void handleSearch();
    void handle_search_data(QByteArray responseData);
signals:
    void found_users_by_search(bool yes_or_not);
    void contact_successfully_added_to_scrollWidget();
    void matched_contact_or_other_user_added_successfully(bool contact_or_otherUser);
private slots:
    void vchat_clicked_from_search_pg(QString nickname);

public slots:
    void handle_vchat_click(QString);
    void keyPressEvent(QKeyEvent *event);
    void handleProfileButton();

signals:
    void vchat_clicked_from_main_pg(QString nickname);
    void profile_button_signal();
    void received_contacts();
};

#endif // MAIN_PAGE_WINDOW_H
