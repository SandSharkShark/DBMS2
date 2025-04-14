#include "LoginDialog.h"
#include "RegisterDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

LoginDialog::LoginDialog(UserManager& manager, QWidget* parent, bool allowCancel)
    : QDialog(parent), userManager(manager) {
    setupUi(allowCancel);
}

void LoginDialog::setupUi(bool allowCancel) {
    setWindowTitle("用户登录");
    setModal(true);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 添加提示标签
    QLabel* tipLabel = new QLabel("默认管理员账户：\n用户名：root\n密码：123456", this);
    tipLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(tipLabel);
    
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
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    loginButton = new QPushButton("登录", this);
    registerButton = new QPushButton("注册", this);
    QPushButton* cancelButton = new QPushButton("取消", this);
    
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);
    if (allowCancel) {
        buttonLayout->addWidget(cancelButton);
    } else {
        cancelButton->deleteLater();
    }
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::attemptLogin);
    connect(registerButton, &QPushButton::clicked, this, &LoginDialog::showRegisterDialog);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // 设置默认按钮
    loginButton->setDefault(true);
    
    // 设置Tab顺序
    setTabOrder(usernameEdit, passwordEdit);
    setTabOrder(passwordEdit, loginButton);
    
    resize(300, 150);
    
    // 如果不允许取消，设置窗口标志
    if (!allowCancel) {
        setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    }
}

void LoginDialog::attemptLogin() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "错误", "用户名和密码不能为空");
        return;
    }
    
    if (userManager.login(username.toStdString(), password.toStdString())) {
        accept();
    } else {
        QMessageBox::warning(this, "错误", "用户名或密码错误");
        passwordEdit->clear();
        passwordEdit->setFocus();
    }
}

void LoginDialog::showRegisterDialog() {
    RegisterDialog dialog(userManager, this);
    if (dialog.exec() == QDialog::Accepted) {
        usernameEdit->setText(dialog.getUsername());
        passwordEdit->clear();
        passwordEdit->setFocus();
    }
} 