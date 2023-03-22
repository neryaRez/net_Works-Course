import sqlite3

'''@Creator_DB
Creates the database and the table of the students.
'''


class Creator_DB:
    def __init__(self):
        self.connector = sqlite3.connect("My_Class.db")
        self.cursor = self.connector.cursor()

    def create_table(self):
        self.cursor.execute('''CREATE Table Students (Name TEXT, Major TEXT, Year TEXT, Average REAL,
         Degree_Level Text)''')

    def add_objects(self):
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Maria', 'Computer Science', 'year of study: 2', 80, 'Bachelor')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Mor',   'Psychology',       'year of study: 3', 79, 'Bachelor')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Mario', 'Psychology',       'year of study: 2', 90, 'Master')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Luigi', 'Economy',          'year of study: 2', 67.97, 'Bachelor')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Sasha', 'Economy',          'year of study: 3', 88, 'Bachelor')''')

        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Nathan', 'Computer Science', 'year of study: 1', 77, 'Bachelor')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Shimi',  'Psychology',       'year of study: 3', 95, 'Bachelor')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Sharif', 'Computer Science', 'year of study: 2', 89, 'Master')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Sandra', 'Economy',          'year of study: 1', 90, 'Bachelor')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Sakura', 'Computer Science', 'year of study: 1', 92, 'Bachelor')''')

        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Ido',    'Economy',           'year of study: 1', 70, 'Bachelor')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Ori',    'Psychology',        'year of study: 4', 82, 'Bachelor')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Alice',  'Economy',           'year of study: 2', 69, 'Master')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('James',  'Computer Science',  'year of study: 1' ,86, 'Master')''')
        self.cursor.execute(
            '''INSERT INTO Students VALUES ('Dave',   'Computer Science',  'year of study: 1', 81, 'Bachelor')''')

        self.connector.commit()
        self.connector.close()


'''@queries_Factory
Has 10 functions in order to supply answers on 10 queries.
'''


class queries_Factory:
    def __init__(self):
        self.connector = sqlite3.connect("My_Class.db")
        self.cursor = self.connector.cursor()

    def q1(self):
        s = '''My Class:\n'''
        self.cursor.execute('SELECT * FROM Students')
        for cur in self.cursor:
            s += str(cur)
            s += '\n'
        return s

    def q2(self):
        s = '''The Excellent Students:\n '''
        self.cursor.execute('SELECT * FROM Students WHERE Average>=85')
        for cur in self.cursor:
            s += str(cur)
            s += '\n'
        return s

    def q3(self):
        s = '''The B.A Students :\n'''
        self.cursor.execute('''SELECT * FROM Students WHERE Degree_Level = 'Bachelor' ''')
        for cur in self.cursor:
            s += str(cur)
            s += '\n'
        return s

    def q4(self):
        s = '''The First Year B.A Students :\n'''
        self.cursor.execute('''SELECT * FROM Students WHERE Year = 'year of study: 1' AND Degree_Level = 'Bachelor' ''')
        for cur in self.cursor:
            s += str(cur)
            s += '\n'
        return s

    def q5(self):
        s = '''The M.A Students\n'''
        self.cursor.execute('''SELECT * FROM Students WHERE Degree_Level = 'Master' ''')
        for cur in self.cursor:
            s += str(cur)
            s += '\n'
        return s

    def q6(self):
        s = '''The Students of Computer Science:\n'''
        self.cursor.execute('''SELECT * FROM Students WHERE Major = 'Computer Science' ''')
        for cur in self.cursor:
            s += str(cur)
            s += '\n'
        return s

    def q7(self):
        s = '''The Excellent Students of Computer Science:\n'''
        self.cursor.execute('''SELECT * FROM Students WHERE Major = 'Computer Science' AND Average >=85''')
        for cur in self.cursor:
            s += str(cur)
            s += '\n'
        return s

    def q8(self):
        s = '''The Students of Economy:\n'''
        self.cursor.execute('''SELECT * FROM Students WHERE Major = 'Economy' ''')
        for cur in self.cursor:
            s += str(cur)
            s += '\n'
        return s

    def q9(self):
        s = '''The Students of Psychology:\n'''
        self.cursor.execute('''SELECT * FROM Students WHERE Major = 'Psychology' ''')
        for cur in self.cursor:
            s += str(cur)
            s += '\n'
        return s

    def q10(self):
        s = '''The Distribution of the Grades:\n'''
        counter60, counter70, counter80, counter90 = 0, 0, 0, 0

        self.cursor.execute('''SELECT * FROM Students WHERE Average>=60 AND Average<70''')
        for cur in self.cursor:
            counter60 += 1

        self.cursor.execute('''SELECT * FROM Students WHERE Average>=70 And Average<80''')
        for cur in self.cursor:
            counter70 += 1

        self.cursor.execute('''SELECT * FROM Students WHERE Average>=80 And Average<90''')
        for cur in self.cursor:
            counter80 += 1

        self.cursor.execute('''SELECT * FROM Students WHERE Average>=90 And Average<100''')
        for cur in self.cursor:
            counter90 += 1

        dct = {'60 - 70:': "{:.2f}".format((counter60 / 15) * 100),
               '70 - 80:': "{:.2f}".format((counter70 / 15) * 100),
               '80 - 90:': "{:.2f}".format((counter80 / 15) * 100),
               '90 - 100:': "{:.2f}".format((counter90 / 15) * 100)
               }
        for key, value in dct.items():
            s += str((key, value))
            s += '\n'
        return s


'''@DB_applicationStr
The opening String which is a list of 10 queries , to let the user to choose one of them.
'''

DB_applicationStr = '''Hello client! 
You have at your disposal, a data-base which represents a class of students, who learn 
3 majors: Computer Science, Economy, and Psychology.
At your service, a list of 10 queries for the data-base:

1) Represent all the students in the class.
2) Represent the excellent students in the class.
3) Represent the B.A students. 
4) Show the first year B.A students.
5) Show the M.A students.
6) Represent the students of Computer Science.
7) Represent the excellent students of Computer Science.
8) Represent the students of Economy.
9) Represent the students of Psychology.
10) Represent the Distribution's grades of the class.

In order to choose the ith query of the list, type i, while 1 <= i <= 10.
'''
# The running of the program
if __name__ == "__main__":
    creator = Creator_DB()
    creator.create_table()
    creator.add_objects()
