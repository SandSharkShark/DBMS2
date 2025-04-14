#ifndef USER_H
#define USER_H

#include <string>
#include <vector>

enum class UserRole {
    ADMIN,      // 管理员：所有权限
    EDITOR,     // 编辑者：可以修改数据，但不能管理用户
    VIEWER      // 查看者：只能查询数据
};

class User {
public:
    User() = default;
    User(const std::string& username, 
         const std::string& passwordHash,
         UserRole role = UserRole::VIEWER)
        : username(username), passwordHash(passwordHash), role(role) {}
    
    // Getters
    const std::string& getUsername() const { return username; }
    const std::string& getPasswordHash() const { return passwordHash; }
    UserRole getRole() const { return role; }
    
    // 权限检查
    bool canModifyData() const { 
        return role == UserRole::ADMIN || role == UserRole::EDITOR; 
    }
    bool canManageUsers() const { 
        return role == UserRole::ADMIN; 
    }
    
private:
    std::string username;
    std::string passwordHash;
    UserRole role;
};

#endif 