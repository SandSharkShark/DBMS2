#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include "UserManager.h"

class LoginDialog : public QDialog {
    Q_OBJECT
    
public:
    LoginDialog(UserManager& userManager, QWidget* parent = nullptr, bool allowCancel = true);
    
private slots:
    void attemptLogin();
    void showRegisterDialog();
    
private:
    UserManager& userManager;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginButton;
    QPushButton* registerButton;
    
    void setupUi(bool allowCancel);
};

#endif 