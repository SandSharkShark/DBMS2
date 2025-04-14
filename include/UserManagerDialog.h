#ifndef USERMANAGERDIALOG_H
#define USERMANAGERDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include "UserManager.h"

class UserManagerDialog : public QDialog {
    Q_OBJECT
    
public:
    UserManagerDialog(UserManager& userManager, QWidget* parent = nullptr);
    
private slots:
    void addUser();
    void deleteUser();
    void changeRole();
    void resetPassword();
    void refreshUserList();
    
private:
    UserManager& userManager;
    QTableWidget* userTable;
    QPushButton* addBtn;
    QPushButton* deleteBtn;
    QPushButton* changeRoleBtn;
    QPushButton* resetPasswordBtn;
    
    void setupUi();
    QString getRoleString(UserRole role) const;
    UserRole getRoleFromString(const QString& roleStr) const;
};

#endif 