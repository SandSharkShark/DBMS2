#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include "UserManager.h"

class RegisterDialog : public QDialog {
    Q_OBJECT
    
public:
    RegisterDialog(UserManager& userManager, QWidget* parent = nullptr);
    QString getUsername() const;
    
private slots:
    void attemptRegister();
    
private:
    UserManager& userManager;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QLineEdit* confirmPasswordEdit;
    QComboBox* roleComboBox;
    
    void setupUi();
};

#endif 