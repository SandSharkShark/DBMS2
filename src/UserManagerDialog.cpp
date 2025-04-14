#include "UserManagerDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include "RegisterDialog.h"

UserManagerDialog::UserManagerDialog(UserManager& manager, QWidget* parent)
    : QDialog(parent), userManager(manager) {
    setupUi();
    refreshUserList();
}

void UserManagerDialog::setupUi() {
    setWindowTitle("用户管理");
    setModal(true);
    resize(600, 400);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建用户表格
    userTable = new QTableWidget(this);
    userTable->setColumnCount(3);
    userTable->setHorizontalHeaderLabels({"用户名", "角色", "操作"});
    userTable->horizontalHeader()->setStretchLastSection(true);
    userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(userTable);
    
    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    addBtn = new QPushButton("添加用户", this);
    deleteBtn = new QPushButton("删除用户", this);
    changeRoleBtn = new QPushButton("修改角色", this);
    resetPasswordBtn = new QPushButton("重置密码", this);
    
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addWidget(changeRoleBtn);
    buttonLayout->addWidget(resetPasswordBtn);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(addBtn, &QPushButton::clicked, this, &UserManagerDialog::addUser);
    connect(deleteBtn, &QPushButton::clicked, this, &UserManagerDialog::deleteUser);
    connect(changeRoleBtn, &QPushButton::clicked, this, &UserManagerDialog::changeRole);
    connect(resetPasswordBtn, &QPushButton::clicked, this, &UserManagerDialog::resetPassword);
}

void UserManagerDialog::refreshUserList() {
    userTable->setRowCount(0);
    
    auto users = userManager.getUserList();
    for (const auto& username : users) {
        int row = userTable->rowCount();
        userTable->insertRow(row);
        
        // 用户名
        userTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(username)));
        
        // 角色
        const User* user = userManager.getUser(username);
        if (user) {
            userTable->setItem(row, 1, new QTableWidgetItem(getRoleString(user->getRole())));
        }
    }
    
    userTable->resizeColumnsToContents();
}

void UserManagerDialog::addUser() {
    RegisterDialog dialog(userManager, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshUserList();
    }
}

void UserManagerDialog::deleteUser() {
    auto items = userTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择要删除的用户");
        return;
    }
    
    QString username = items[0]->text();
    
    // 不能删除当前登录的用户
    if (username == QString::fromStdString(userManager.getCurrentUser()->getUsername())) {
        QMessageBox::warning(this, "警告", "不能删除当前登录的用户");
        return;
    }
    
    if (QMessageBox::question(this, "确认",
        QString("确定要删除用户 %1 吗？").arg(username),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        
        if (userManager.deleteUser(username.toStdString())) {
            refreshUserList();
            QMessageBox::information(this, "成功", "用户删除成功");
        } else {
            QMessageBox::warning(this, "错误", "删除用户失败");
        }
    }
}

void UserManagerDialog::changeRole() {
    auto items = userTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择要修改的用户");
        return;
    }
    
    QString username = items[0]->text();
    
    // 不能修改当前登录用户的角色
    if (username == QString::fromStdString(userManager.getCurrentUser()->getUsername())) {
        QMessageBox::warning(this, "警告", "不能修改当前登录用户的角色");
        return;
    }
    
    QStringList roles = {"查看者", "编辑者", "管理员"};
    QString currentRole = items[1]->text();
    
    bool ok;
    QString newRole = QInputDialog::getItem(this, "修改角色",
        "选择新角色:", roles, roles.indexOf(currentRole), false, &ok);
    
    if (ok && !newRole.isEmpty()) {
        if (userManager.changeUserRole(username.toStdString(), getRoleFromString(newRole))) {
            refreshUserList();
            QMessageBox::information(this, "成功", "角色修改成功");
        } else {
            QMessageBox::warning(this, "错误", "修改角色失败");
        }
    }
}

void UserManagerDialog::resetPassword() {
    auto items = userTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择要重置密码的用户");
        return;
    }
    
    QString username = items[0]->text();
    
    bool ok;
    QString newPassword = QInputDialog::getText(this, "重置密码",
        "输入新密码:", QLineEdit::Password, "", &ok);
    
    if (ok && !newPassword.isEmpty()) {
        if (userManager.resetPassword(username.toStdString(), newPassword.toStdString())) {
            QMessageBox::information(this, "成功", "密码重置成功");
        } else {
            QMessageBox::warning(this, "错误", "密码重置失败");
        }
    }
}

QString UserManagerDialog::getRoleString(UserRole role) const {
    switch (role) {
        case UserRole::ADMIN: return "管理员";
        case UserRole::EDITOR: return "编辑者";
        case UserRole::VIEWER: return "查看者";
        default: return "未知";
    }
}

UserRole UserManagerDialog::getRoleFromString(const QString& roleStr) const {
    if (roleStr == "管理员") return UserRole::ADMIN;
    if (roleStr == "编辑者") return UserRole::EDITOR;
    return UserRole::VIEWER;
} 