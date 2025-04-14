#include "RegisterDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

RegisterDialog::RegisterDialog(UserManager& manager, QWidget* parent)
    : QDialog(parent), userManager(manager) {
    setupUi();
}

void RegisterDialog::setupUi() {
    setWindowTitle("用户注册");
    setModal(true);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 用户名输入
    QHBoxLayout* userLayout = new QHBoxLayout;
    userLayout->addWidget(new QLabel("用户名:"));
    usernameEdit = new QLineEdit(this);
    userLayout->addWidget(usernameEdit);
    mainLayout->addLayout(userLayout);
    
    // 密码输入
    QHBoxLayout* passLayout = new QHBoxLayout;
    passLayout->addWidget(new QLabel("密码:"));
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(passwordEdit);
    mainLayout->addLayout(passLayout);
    
    // 确认密码
    QHBoxLayout* confirmLayout = new QHBoxLayout;
    confirmLayout->addWidget(new QLabel("确认密码:"));
    confirmPasswordEdit = new QLineEdit(this);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmLayout->addWidget(confirmPasswordEdit);
    mainLayout->addLayout(confirmLayout);
    
    // 用户角色选择
    QHBoxLayout* roleLayout = new QHBoxLayout;
    roleLayout->addWidget(new QLabel("用户角色:"));
    roleComboBox = new QComboBox(this);
    roleComboBox->addItem("查看者", static_cast<int>(UserRole::VIEWER));
    roleComboBox->addItem("编辑者", static_cast<int>(UserRole::EDITOR));
    roleComboBox->addItem("管理员", static_cast<int>(UserRole::ADMIN));
    roleLayout->addWidget(roleComboBox);
    mainLayout->addLayout(roleLayout);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* registerButton = new QPushButton("注册", this);
    QPushButton* cancelButton = new QPushButton("取消", this);
    
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(registerButton, &QPushButton::clicked, this, &RegisterDialog::attemptRegister);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    resize(350, 200);
}

void RegisterDialog::attemptRegister() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "错误", "用户名和密码不能为空");
        return;
    }
    
    if (password != confirmPassword) {
        QMessageBox::warning(this, "错误", "两次输入的密码不一致");
        passwordEdit->clear();
        confirmPasswordEdit->clear();
        passwordEdit->setFocus();
        return;
    }
    
    UserRole role = static_cast<UserRole>(roleComboBox->currentData().toInt());
    
    if (userManager.registerUser(username.toStdString(), password.toStdString(), role)) {
        QMessageBox::information(this, "成功", "注册成功");
        accept();
    } else {
        QMessageBox::warning(this, "错误", "用户名已存在");
        usernameEdit->setFocus();
    }
}

QString RegisterDialog::getUsername() const {
    return usernameEdit->text();
} 