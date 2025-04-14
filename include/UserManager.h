#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <string>
#include <map>
#include "User.h"

class UserManager {
public:
    UserManager(const std::string& dbPath);
    
    // 用户管理
    bool registerUser(const std::string& username, 
                     const std::string& password,
                     UserRole role = UserRole::VIEWER);
    bool login(const std::string& username, const std::string& password);
    void logout();
    
    // 用户操作
    bool changePassword(const std::string& username, 
                       const std::string& oldPassword,
                       const std::string& newPassword);
    bool changeUserRole(const std::string& username, UserRole newRole);
    bool deleteUser(const std::string& username);
    
    // 状态查询
    bool isLoggedIn() const { return currentUser != nullptr; }
    const User* getCurrentUser() const { return currentUser; }
    std::vector<std::string> getUserList() const;
    const User* getUser(const std::string& username) const;
    bool resetPassword(const std::string& username, const std::string& newPassword);
    
private:
    std::string dbPath;
    std::map<std::string, User> users;
    User* currentUser;
    
    // 辅助方法
    std::string hashPassword(const std::string& password);
    bool loadUsers();
    bool saveUsers();
};

#endif 