#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>

class SettingsDialog : public QDialog {
    Q_OBJECT
    
public:
    SettingsDialog(QWidget* parent = nullptr);
    
    // 获取设置值
    QString getDbPath() const;
    int getHistorySize() const;
    bool isAutoCompleteEnabled() const;
    bool isSyntaxHighlightEnabled() const;
    QString getTheme() const;
    QFont getEditorFont() const;
    int getAutoSaveInterval() const;
    bool isAutoBackupEnabled() const;
    QString getBackupPath() const;
    int getMaxConnectionCount() const;
    QString getDefaultEncoding() const;
    
signals:
    void settingsChanged();
    
private slots:
    void browseDatabasePath();
    void browseBackupPath();
    void applySettings();
    void restoreDefaults();
    
private:
    // UI组件
    QTabWidget* tabWidget;
    
    // 常规设置
    QLineEdit* dbPathEdit;
    QSpinBox* historySizeBox;
    QComboBox* themeCombo;
    QComboBox* languageCombo;
    
    // 编辑器设置
    QCheckBox* autoCompleteCheck;
    QCheckBox* syntaxHighlightCheck;
    QFontComboBox* fontCombo;
    QSpinBox* fontSizeBox;
    QSpinBox* tabSizeBox;
    QCheckBox* autoIndentCheck;
    QCheckBox* lineNumbersCheck;
    
    // 自动保存设置
    QCheckBox* autoSaveCheck;
    QSpinBox* autoSaveIntervalBox;
    QCheckBox* autoBackupCheck;
    QLineEdit* backupPathEdit;
    
    // 性能设置
    QSpinBox* maxConnectionsBox;
    QSpinBox* queryTimeoutBox;
    QComboBox* encodingCombo;
    QCheckBox* useTransactionsCheck;
    
    void setupUi();
    void createGeneralTab(QWidget* tab);
    void createEditorTab(QWidget* tab);
    void createAutoSaveTab(QWidget* tab);
    void createPerformanceTab(QWidget* tab);
    void loadSettings();
    void saveSettings();
};

#endif 