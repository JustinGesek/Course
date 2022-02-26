#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* ARGV0 = "assignment1";
typedef struct Test Test; //gives a name to a type
typedef struct Course Course; //gives a name to a type
typedef struct Section Section; //gives a name to a type
typedef struct Student Student;//gives a name to a type

struct Test
{
	Course** courses;
	int ncourses;
};

struct Course
{
	char* courseName;
	int nsections;
	Student** sections;
	int* num_students;
	int* num_scores;
};

struct Section
{
	Student** students;
	int nstudents;
	int nassignments;
};
struct Student
{
	int id;
	char* lastName;
	float* scores;
	float std_average;
};
void process_course(Course* course)
{
	Student* bestStudent = course->sections[0];
	int iStudent = 0;
	int passingStudents = 0;
	float* section_averages = (float*)malloc(sizeof(float) * course->nsections);
	for (int s = 0; s < course->nsections; ++s)
	{
		section_averages[s] = 0.0;
		for (int n = 0; n < course->num_students[s]; ++n)
		{
			Student* thisStudent = course->sections[iStudent++];
			if (bestStudent->std_average < thisStudent->std_average)
			{
				bestStudent = thisStudent;
			}
			passingStudents += thisStudent->std_average >= 70.0;
			section_averages[s] += thisStudent->std_average;
		}
		section_averages[s] /= course->num_students[s];
	}
	printf("%s %d", course->courseName, passingStudents);
	for (int s = 0; s < course->nsections; ++s)
	{
		printf(" %.2f", section_averages[s]);
	}
	printf(" %d %s %.2f\n", bestStudent->id, bestStudent->lastName, bestStudent->std_average);
}
void process_courses(Course** courses, int num_courses)
{
	for (int i = 0; i < num_courses; ++i)
	{
		process_course(courses[i]);
	}
	printf("\n");
}

void release_students(Student** students, int num_students)
{
	for (int i = 0; i < num_students; i++)
	{
		free(students[i]->scores);
		free(students[i]->lastName);
		free(students[i]);
	}
	free(students);
}

void release_sections(Student** sections, int num_sections, int num_students[], int num_scores[])
{
	int total_students = 0;
	for (int i = 0; i < num_sections; i++)
	{
		total_students += num_students[i];
	}
	release_students(sections, total_students);
	free(num_students);
	free(num_scores);
}

void release_courses(Course** courses, int num_courses)
{
	for (int i = 0; i < num_courses; i++)
	{
		release_sections(courses[i]->sections, courses[i]->nsections, courses[i]->num_students, courses[i]->num_scores);
		free(courses[i]->courseName);
	}
	free(courses);
}
void release_tests(Test** tests, int num_tests)
{
	for (int i = 0; i < num_tests; i++)
	{
		release_courses(tests[i]->courses, tests[i]->ncourses);
	}
	free(tests);
}
Student** read_sections(FILE* input, int num_students[], int num_scores[], int num_sections)
{
	char buffer[1024];
	Student** sections = NULL;//(Section**)malloc(sizeof(Section*) * nsections);
	int total_students = 0;
	int next_student = 0;
	for (int s = 0; s < num_sections; ++s)
	{
		int nstudents = 0;
		int nassignments = 0;
		fgets(buffer, sizeof(buffer), input);// read a single line into our buffer, reads a max of 1023 characters plus 0.

		if (sscanf(buffer, " %u %u", &nstudents, &nassignments) != 2) //returns the # of variables it was able to convert.
		{
			fprintf(stderr, "%s: Malformed number of students and assignemnts; '%s'; exiting!\n", ARGV0, buffer); //error message
			exit(1);
		}
		total_students += nstudents;
		sections = (Student**)realloc(sections, sizeof(Student*) * total_students);
		num_students[s] = nstudents;
		num_scores[s] = nassignments;
		Student** students = (Student**)malloc(sizeof(Student*) * nstudents);
		for (int student = 0; student < nstudents; ++student)
		{
			Student* s = (Student*)malloc(sizeof(Student));
			int id = 0; // id
			char nl[32]; //last name
			double score = 0.0;
			int n = 0;
			int m = 0;

			fgets(buffer, sizeof(buffer), input);// read a single line into our buffer, reads a max of 1023 characters plus 0.

			if (sscanf(buffer, " %u %s %n", &id, nl, &m) != 2) //returns the # of variables it was able to convert; find one or more characters that are not spaces and put them in the address; tell me how many bytes you see during the exicution.
			{
				fprintf(stderr, "%s: Malformed student id and name; '%s'; exiting!\n", ARGV0, buffer); //error message
				exit(1);
			}
			n = m;
			s->id = id;
			s->lastName = strdup(nl);
			s->scores = (float*)malloc(sizeof(float) * nassignments);
			s->std_average = 0.0;
			for (int a = 0; a < nassignments; ++a)
			{
				if (sscanf(&buffer[n], " %lf %n", &score, &m) != 1) //looking for test score that is a floating point number, lf stores as a double.
				{
					fprintf(stderr, "%s: Malformed test score; '%s'; exiting!\n", ARGV0, buffer); //error message
					exit(1);
				}
				s->scores[a] = score;
				s->std_average += score;
				n += m;
			}
			s->std_average /= nassignments;
			sections[next_student++] = s;
		}
	}
	return sections;
}
Course** read_courses(FILE* input, int* num_courses)
{
	char buffer[1024];
	fgets(buffer, sizeof(buffer), input);// read a single line into our buffer, reads a max of 1023 characters plus 0.
	int ncourses;
	if (sscanf(buffer, " %u", &ncourses) != 1) //returns the # of variables it was able to convert.
	{
		fprintf(stderr, "%s: Malformed number of courses; '%s'; exiting!\n", ARGV0, buffer); //error message
		exit(1);
	}
	Course** courses = (Course**)malloc(sizeof(Course*) * ncourses);
	for (int c = 0; c < ncourses; ++c)
	{
		Course* course = (Course*)malloc(sizeof(Course));
		char coursename[32]; // course name
		fgets(buffer, sizeof(buffer), input);// read a single line into our buffer, reads a max of 1023 characters plus 0.

		if (sscanf(buffer, " %s", coursename) != 1)
		{
			fprintf(stderr, "%s: Malformed number of course name; '%s'; exiting!\n", ARGV0, buffer); //error message
			exit(1);
		}
		int nsections = 0;
		fgets(buffer, sizeof(buffer), input);// read a single line into our buffer, reads a max of 1023 characters plus 0.

		if (sscanf(buffer, " %u", &nsections) != 1) //returns the # of variables it was able to convert.
		{
			fprintf(stderr, "%s: Malformed number of sections; '%s'; exiting!\n", ARGV0, buffer); //error message
			exit(1);
		}
		course->courseName = strdup(coursename);
		course->nsections = nsections;
		course->num_students = (int*)malloc(sizeof(int) * nsections);
		course->num_scores = (int*)malloc(sizeof(int) * nsections);
		course->sections = read_sections(input, course->num_students, course->num_scores, nsections);
		courses[c] = course;
	}
	*num_courses = ncourses;
	return courses;
}

int main(int argc, char* const argv[])
{
	ARGV0 = argv[0];
	FILE* input = fopen("assignment1input.txt", "r");// opens file
	if (input == NULL)
	{
		fprintf(stderr, "%s: Unable to open assignment1input.txt; exiting!\n", ARGV0); //error message
		perror(ARGV0);
		exit(1);
	}
	//read file
	char buffer[1024];
	fgets(buffer, sizeof(buffer), input);// read a single line into our buffer, reads a max of 1023 characters plus 0.
	int ntests = 0;
	if (sscanf(buffer, " %u", &ntests) != 1) //returns the # of variables it was able to convert.
	{
		fprintf(stderr, "%s: Malformed number of tests; '%s'; exiting!\n", ARGV0, buffer); //error message
		exit(1);
	}
	Test** tests = (Test**)malloc(sizeof(Test*) * ntests);
	for (int t = 0; t < ntests; ++t)// time to read the tests
	{
		tests[t] = (Test*)malloc(sizeof(Test));
		tests[t]->ncourses = 0;
		tests[t]->courses = read_courses(input, &tests[t]->ncourses);
		printf("test case %d\n", t + 1);
		process_courses(tests[t]->courses, tests[t]->ncourses);
	}
	release_tests(tests, ntests);
}