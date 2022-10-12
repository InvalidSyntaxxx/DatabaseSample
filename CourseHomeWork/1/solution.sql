-- 1 为 student表添加用户自定义约束条件；需要添加的约束条件有：学院ID，必须为学院表中存在的ID；性别必须为”男“或者”女“；
ALTER TABLE student ADD FOREIGN KEY(college_id) REFERENCES college(college_id)
ALTER TABLE student ADD CONSTRAINT stu_check CHECK(
student.sex IN ("男","女")
);

-- 2 写一条SQL语句，向student表中，插入一条不违背该表所有约束条件的记录；
INSERT INTO student VALUES(0727132,"王大侠","男",20,"18695481246",1013,"广东省东顾市置王路5987号倔憎小区7单元1398室")

-- 3 基于数据库中的基本表，创建一个视图，视图中，能显示一个社团编号，社团名称，参加人数以及该社团的负责学院和学院联系人姓名及电话；
CREATE VIEW club_view AS
SELECT club.club_number,club.club_name,club.StudentCount,college.college_name,college.contacts,college.office_telephone
FROM club LEFT JOIN college ON college.college_id=club.college_id;

-- 4 创建一个触发器，当某个学生退出某个社团后，在club表中，相应的社团会员人数-1，并将该学生俱乐部成员状态变为2（已经退社）；
DROP TRIGGER changeClub;
CREATE TRIGGER changeClub
AFTER  UPDATE ON student_club
FOR EACH ROW
BEGIN
	UPDATE club SET club.StudentCount=club.StudentCount-1 WHERE OLD.club_number=club.club_number;
	UPDATE student_club SET NEW.status=2;
END
;

-- 5 在club表上，创建一个触发器，能实现，当删除某个社团后，学生社团参加信息表student_club中，学生加入该社团的记录同时删除；
DROP TRIGGER deleteClub ;
CREATE TRIGGER deleteClub
AFTER  DELETE ON club
FOR EACH ROW
BEGIN
	DELETE FROM student_club WHERE student_club.club_number=OLD.club_number;
END
;

-- 6 查询统计，各学院的学生数量，按照数量从高到低排序；输出列表数据格式为：学院编号，学院名称，学生数量；
SELECT student.college_id,college_name,COUNT(*) as studentCount FROM student
LEFT JOIN college ON college.college_id=student.college_id
GROUP BY college_id
ORDER BY studentCount DESC;

-- 7 请查询统计每个学院主导的社团数量；输出学院名称，社团名字；
SELECT COUNT(*) as count,college.college_name from club
LEFT JOIN college ON club.college_id=college.college_id
GROUP BY college.college_id;

-- 8 请基于student_club表中的实际数据，统计正参加的各社团学生情况，输出：社团编号，社团名称，参加人数，男生人数，女生人数；（注意，不适用club表中的统计数据）
SELECT student_club.club_number  as '社团编号',
club.club_name as '社团名称',
COUNT(*) as '参加人数',
sum(case when student.sex='男' then 1 else 0 end) as '男生人数',
sum(case when student.sex='女' then 1 else 0 end) as '女生人数'
FROM student_club
LEFT JOIN club ON club.club_number=student_club.club_number
LEFT JOIN student ON student.student_number=student_club.student_number 
GROUP BY student_club.club_number;

-- 9 查询“电气与电子工程学院”并加入“华电吉他社”社团的学生，输出：学生学号，姓名，平均考核成绩；
SELECT student.student_number as 学生学号,student.`name` as 姓名,avg(student_club.score) as 平均考核成绩 
FROM student_club
LEFT JOIN student ON student.student_number=student_club.student_number

-- SELECT * from college where college.college_name="电气与电子工程学院" # ???? 这搜索出来的全为空??编码格式不对吗...
-- 后续：发现建表的时候电气与电子工程学院前面有'\r\n'，自行理解。。。

WHERE student.college_id=1001
AND student_club.club_number=(SELECT club.club_number FROM club WHERE club.club_name="华电吉他社")
GROUP BY student.student_number

-- 10 请基于student_club表中的数据，统计计算 每个社团的考核成绩分布；输出格式：社团编号，社团名称，不及格人数；70--80分人数；80--90分人数；大于等于90分人数；
SELECT 
club.club_number as '社团编号',
club.club_name as '社团名称',
-- COUNT(student_club.student_number) as '总人数',
sum(case when student_club.score<60 THEN 1 ELSE 0 END) as '不及格人数',
-- sum(case when (student_club.score>=60 AND student_club.score<70) THEN 1 END) as '60-70人数',
sum(case when (student_club.score>=70 AND student_club.score<80) THEN 1 END) as '70-80人数',
sum(case when (student_club.score>=80 AND student_club.score<90) THEN 1 END) as '80-90人数',
sum(case when student_club.score>90 THEN 1 ELSE 0 END) as '大于90人数'
from student_club
LEFT JOIN club ON club.club_number=student_club.club_number 	# 这里原本club和student_club的数据类型不一样，可能需转换
GROUP BY student_club.club_number 		# 我一直写成student_number。。。。。。