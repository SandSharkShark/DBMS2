#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QSettings>
#include <QGroupBox>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setupUi();
    loadSettings();
}

void SettingsDialog::setupUi() {
    setWindowTitle("设置");
    resize(600, 400);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建选项卡
    tabWidget = new QTabWidget(this);
    
    // 创建各个选项卡页面
    QWidget* generalTab = new QWidget;
    QWidget* editorTab = new QWidget;
    QWidget* autoSaveTab = new QWidget;
    QWidget* performanceTab = new QWidget;
    
    createGeneralTab(generalTab);
    createEditorTab(editorTab);
    createAutoSaveTab(autoSaveTab);
    createPerformanceTab(performanceTab);
    
    tabWidget->addTab(generalTab, "常规");
    tabWidget->addTab(editorTab, "编辑器");
    tabWidget->addTab(autoSaveTab, "自动保存");
    tabWidget->addTab(performanceTab, "性能");
    
    mainLayout->addWidget(tabWidget);
    
    // 创建按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* okButton = new QPushButton("确定", this);
    QPushButton* applyButton = new QPushButton("应用", this);
    QPushButton* cancelButton = new QPushButton("取消", this);
    QPushButton* defaultsButton = new QPushButton("恢复默认", this);
    
    buttonLayout->addWidget(defaultsButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(okButton, &QPushButton::clicked, this, [this]() {
        applySettings();
        accept();
    });
    connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::applySettings);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(defaultsButton, &QPushButton::clicked, this, &SettingsDialog::restoreDefaults);
}

void SettingsDialog::createGeneralTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    // 数据库路径设置
    QGroupBox* pathGroup = new QGroupBox("数据库路径", tab);
    QHBoxLayout* pathLayout = new QHBoxLayout(pathGroup);
    dbPathEdit = new QLineEdit(pathGroup);
    QPushButton* browseBtn = new QPushButton("浏览", pathGroup);
    pathLayout->addWidget(dbPathEdit);
    pathLayout->addWidget(browseBtn);
    
    // 主题设置
    QGroupBox* themeGroup = new QGroupBox("外观", tab);
    QGridLayout* themeLayout = new QGridLayout(themeGroup);
    themeCombo = new QComboBox(themeGroup);
    themeCombo->addItems({"系统默认", "亮色", "暗色"});
    languageCombo = new QComboBox(themeGroup);
    languageCombo->addItems({"简体中文", "English"});
    
    themeLayout->addWidget(new QLabel("主题:"), 0, 0);
    themeLayout->addWidget(themeCombo, 0, 1);
    themeLayout->addWidget(new QLabel("语言:"), 1, 0);
    themeLayout->addWidget(languageCombo, 1, 1);
    
    // 历史记录设置
    QGroupBox* historyGroup = new QGroupBox("历史记录", tab);
    QHBoxLayout* historyLayout = new QHBoxLayout(historyGroup);
    historySizeBox = new QSpinBox(historyGroup);
    historySizeBox->setRange(0, 1000);
    historyLayout->addWidget(new QLabel("保留历史记录数:"));
    historyLayout->addWidget(historySizeBox);
    
    layout->addWidget(pathGroup);
    layout->addWidget(themeGroup);
    layout->addWidget(historyGroup);
    layout->addStretch();
    
    connect(browseBtn, &QPushButton::clicked, this, &SettingsDialog::browseDatabasePath);
}

void SettingsDialog::createEditorTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    // 编辑器功能
    QGroupBox* featuresGroup = new QGroupBox("功能", tab);
    QVBoxLayout* featuresLayout = new QVBoxLayout(featuresGroup);
    autoCompleteCheck = new QCheckBox("启用自动完成", featuresGroup);
    syntaxHighlightCheck = new QCheckBox("启用语法高亮", featuresGroup);
    autoIndentCheck = new QCheckBox("自动缩进", featuresGroup);
    lineNumbersCheck = new QCheckBox("显示行号", featuresGroup);
    
    featuresLayout->addWidget(autoCompleteCheck);
    featuresLayout->addWidget(syntaxHighlightCheck);
    featuresLayout->addWidget(autoIndentCheck);
    featuresLayout->addWidget(lineNumbersCheck);
    
    // 字体设置
    QGroupBox* fontGroup = new QGroupBox("字体", tab);
    QGridLayout* fontLayout = new QGridLayout(fontGroup);
    fontCombo = new QFontComboBox(fontGroup);
    fontSizeBox = new QSpinBox(fontGroup);
    fontSizeBox->setRange(8, 72);
    
    fontLayout->addWidget(new QLabel("字体:"), 0, 0);
    fontLayout->addWidget(fontCombo, 0, 1);
    fontLayout->addWidget(new QLabel("大小:"), 1, 0);
    fontLayout->addWidget(fontSizeBox, 1, 1);
    
    // 缩进设置
    QGroupBox* indentGroup = new QGroupBox("缩进", tab);
    QHBoxLayout* indentLayout = new QHBoxLayout(indentGroup);
    tabSizeBox = new QSpinBox(indentGroup);
    tabSizeBox->setRange(1, 8);
    indentLayout->addWidget(new QLabel("Tab 宽度:"));
    indentLayout->addWidget(tabSizeBox);
    
    layout->addWidget(featuresGroup);
    layout->addWidget(fontGroup);
    layout->addWidget(indentGroup);
    layout->addStretch();
}

void SettingsDialog::createAutoSaveTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    // 自动保存设置
    QGroupBox* autoSaveGroup = new QGroupBox("自动保存", tab);
    QGridLayout* autoSaveLayout = new QGridLayout(autoSaveGroup);
    autoSaveCheck = new QCheckBox("启用自动保存", autoSaveGroup);
    autoSaveIntervalBox = new QSpinBox(autoSaveGroup);
    autoSaveIntervalBox->setRange(1, 60);
    autoSaveIntervalBox->setSuffix(" 分钟");
    
    autoSaveLayout->addWidget(autoSaveCheck, 0, 0, 1, 2);
    autoSaveLayout->addWidget(new QLabel("保存间隔:"), 1, 0);
    autoSaveLayout->addWidget(autoSaveIntervalBox, 1, 1);
    
    // 自动备份设置
    QGroupBox* backupGroup = new QGroupBox("自动备份", tab);
    QVBoxLayout* backupLayout = new QVBoxLayout(backupGroup);
    autoBackupCheck = new QCheckBox("启用自动备份", backupGroup);
    
    QHBoxLayout* pathLayout = new QHBoxLayout;
    backupPathEdit = new QLineEdit(backupGroup);
    QPushButton* browseBackupBtn = new QPushButton("浏览", backupGroup);
    pathLayout->addWidget(new QLabel("备份路径:"));
    pathLayout->addWidget(backupPathEdit);
    pathLayout->addWidget(browseBackupBtn);
    
    backupLayout->addWidget(autoBackupCheck);
    backupLayout->addLayout(pathLayout);
    
    layout->addWidget(autoSaveGroup);
    layout->addWidget(backupGroup);
    layout->addStretch();
    
    connect(browseBackupBtn, &QPushButton::clicked, this, &SettingsDialog::browseBackupPath);
}

void SettingsDialog::createPerformanceTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    // 连接设置
    QGroupBox* connectionGroup = new QGroupBox("连接", tab);
    QGridLayout* connectionLayout = new QGridLayout(connectionGroup);
    maxConnectionsBox = new QSpinBox(connectionGroup);
    maxConnectionsBox->setRange(1, 100);
    queryTimeoutBox = new QSpinBox(connectionGroup);
    queryTimeoutBox->setRange(0, 3600);
    queryTimeoutBox->setSuffix(" 秒");
    
    connectionLayout->addWidget(new QLabel("最大连接数:"), 0, 0);
    connectionLayout->addWidget(maxConnectionsBox, 0, 1);
    connectionLayout->addWidget(new QLabel("查询超时:"), 1, 0);
    connectionLayout->addWidget(queryTimeoutBox, 1, 1);
    
    // 编码设置
    QGroupBox* encodingGroup = new QGroupBox("编码", tab);
    QHBoxLayout* encodingLayout = new QHBoxLayout(encodingGroup);
    encodingCombo = new QComboBox(encodingGroup);
    encodingCombo->addItems({"UTF-8", "GBK", "ASCII"});
    encodingLayout->addWidget(new QLabel("默认编码:"));
    encodingLayout->addWidget(encodingCombo);
    
    // 事务设置
    QGroupBox* transactionGroup = new QGroupBox("事务", tab);
    QVBoxLayout* transactionLayout = new QVBoxLayout(transactionGroup);
    useTransactionsCheck = new QCheckBox("启用事务支持", transactionGroup);
    transactionLayout->addWidget(useTransactionsCheck);
    
    layout->addWidget(connectionGroup);
    layout->addWidget(encodingGroup);
    layout->addWidget(transactionGroup);
    layout->addStretch();
}

void SettingsDialog::loadSettings() {
    QSettings settings("MyCompany", "DatabaseSystem");
    
    // 加载常规设置
    dbPathEdit->setText(settings.value("dbPath", "./data").toString());
    historySizeBox->setValue(settings.value("historySize", 100).toInt());
    themeCombo->setCurrentText(settings.value("theme", "系统默认").toString());
    languageCombo->setCurrentText(settings.value("language", "简体中文").toString());
    
    // 加载编辑器设置
    autoCompleteCheck->setChecked(settings.value("autoComplete", true).toBool());
    syntaxHighlightCheck->setChecked(settings.value("syntaxHighlight", true).toBool());
    fontCombo->setCurrentFont(settings.value("editorFont", QFont("Consolas")).value<QFont>());
    fontSizeBox->setValue(settings.value("fontSize", 12).toInt());
    tabSizeBox->setValue(settings.value("tabSize", 4).toInt());
    autoIndentCheck->setChecked(settings.value("autoIndent", true).toBool());
    lineNumbersCheck->setChecked(settings.value("lineNumbers", true).toBool());
    
    // 加载自动保存设置
    autoSaveCheck->setChecked(settings.value("autoSave", false).toBool());
    autoSaveIntervalBox->setValue(settings.value("autoSaveInterval", 5).toInt());
    autoBackupCheck->setChecked(settings.value("autoBackup", false).toBool());
    backupPathEdit->setText(settings.value("backupPath", "./backup").toString());
    
    // 加载性能设置
    maxConnectionsBox->setValue(settings.value("maxConnections", 10).toInt());
    queryTimeoutBox->setValue(settings.value("queryTimeout", 30).toInt());
    encodingCombo->setCurrentText(settings.value("defaultEncoding", "UTF-8").toString());
    useTransactionsCheck->setChecked(settings.value("useTransactions", true).toBool());
}

void SettingsDialog::saveSettings() {
    QSettings settings("MyCompany", "DatabaseSystem");
    
    // 保存常规设置
    settings.setValue("dbPath", dbPathEdit->text());
    settings.setValue("historySize", historySizeBox->value());
    settings.setValue("theme", themeCombo->currentText());
    settings.setValue("language", languageCombo->currentText());
    
    // 保存编辑器设置
    settings.setValue("autoComplete", autoCompleteCheck->isChecked());
    settings.setValue("syntaxHighlight", syntaxHighlightCheck->isChecked());
    settings.setValue("editorFont", fontCombo->currentFont());
    settings.setValue("fontSize", fontSizeBox->value());
    settings.setValue("tabSize", tabSizeBox->value());
    settings.setValue("autoIndent", autoIndentCheck->isChecked());
    settings.setValue("lineNumbers", lineNumbersCheck->isChecked());
    
    // 保存自动保存设置
    settings.setValue("autoSave", autoSaveCheck->isChecked());
    settings.setValue("autoSaveInterval", autoSaveIntervalBox->value());
    settings.setValue("autoBackup", autoBackupCheck->isChecked());
    settings.setValue("backupPath", backupPathEdit->text());
    
    // 保存性能设置
    settings.setValue("maxConnections", maxConnectionsBox->value());
    settings.setValue("queryTimeout", queryTimeoutBox->value());
    settings.setValue("defaultEncoding", encodingCombo->currentText());
    settings.setValue("useTransactions", useTransactionsCheck->isChecked());
}

void SettingsDialog::browseDatabasePath() {
    QString dir = QFileDialog::getExistingDirectory(this,
        "选择数据库目录", dbPathEdit->text());
    if (!dir.isEmpty()) {
        dbPathEdit->setText(dir);
    }
}

void SettingsDialog::browseBackupPath() {
    QString dir = QFileDialog::getExistingDirectory(this,
        "选择备份目录", backupPathEdit->text());
    if (!dir.isEmpty()) {
        backupPathEdit->setText(dir);
    }
}

void SettingsDialog::applySettings() {
    saveSettings();
    emit settingsChanged();  // 发送设置改变信号
}

void SettingsDialog::restoreDefaults() {
    if (QMessageBox::question(this, "确认",
        "确定要恢复默认设置吗？\n这将丢失所有自定义设置。",
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        
        QSettings settings("MyCompany", "DatabaseSystem");
        settings.clear();
        loadSettings();
    }
} 