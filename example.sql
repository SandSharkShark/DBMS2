CREATE TABLE Students (
    ID INTEGER PRIMARY KEY,
    Name TEXT NOT NULL,
    Age INTEGER,
    Gender TEXT,
    Department TEXT,
    EnrollmentDate TEXT
);

CREATE TABLE Courses (
    CourseID INTEGER PRIMARY KEY,
    CourseName TEXT NOT NULL,
    Credits INTEGER,
    TeacherName TEXT
);

CREATE TABLE StudentCourses (
    StudentID INTEGER,
    CourseID INTEGER,
    Grade FLOAT,
    FOREIGN KEY (StudentID) REFERENCES Students(ID),
    FOREIGN KEY (CourseID) REFERENCES Courses(CourseID)
);

INSERT INTO Students (ID, Name, Age, Gender, Department, EnrollmentDate)
VALUES (1, 张三, 20, 男, 计算机系, 2022-09-01);

INSERT INTO Students (ID, Name, Age, Gender, Department, EnrollmentDate)
VALUES (2, 李四, 19, 女, 数学系, 2022-09-01);

INSERT INTO Students (ID, Name, Age, Gender, Department, EnrollmentDate)
VALUES (3, 王五, 21, 男, 物理系, 2021-09-01);


INSERT INTO Courses (CourseID, CourseName, Credits, TeacherName)
VALUES (101, 数据库原理, 3, 陈教授);

INSERT INTO Courses (CourseID, CourseName, Credits, TeacherName)
VALUES (102, 操作系统, 4, 李教授);

INSERT INTO Courses (CourseID, CourseName, Credits, TeacherName)
VALUES (103, 计算机网络, 3, 王教授);


INSERT INTO StudentCourses (StudentID, CourseID, Grade)
VALUES (1, 101, 85.5);

INSERT INTO StudentCourses (StudentID, CourseID, Grade)
VALUES (1, 102, 92.0);

INSERT INTO StudentCourses (StudentID, CourseID, Grade)
VALUES (2, 101, 88.5);

INSERT INTO StudentCourses (StudentID, CourseID, Grade)
VALUES (3, 103, 90.0);


SELECT * FROM Students;


UPDATE Courses SET Credits = 4
WHERE CourseName = '数据库原理';


DELETE FROM StudentCourses
WHERE Grade < 90;


SELECT Department, COUNT(*) as StudentCount
FROM Students
GROUP BY Department;


SELECT s.Name, AVG(sc.Grade) as AvgGrade
FROM Students s
JOIN StudentCourses sc ON s.ID = sc.StudentID
GROUP BY s.ID, s.Name
HAVING AVG(sc.Grade) > 85;


SELECT s.Name, COUNT(sc.CourseID) as CourseCount
FROM Students s
JOIN StudentCourses sc ON s.ID = sc.StudentID
GROUP BY s.ID, s.Name
ORDER BY CourseCount DESC;
