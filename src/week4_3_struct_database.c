/*
 * week4_3_struct_database.c
 * Author: [SHASHANK SAGAR NARWA]
 * Student ID: [251ADB118]
 * Description:
 *   Simple in-memory "database" using an array of structs.
 *   Use malloc to allocate space for n Student records,
 *   read each record from the user, print them as a table,
 *   and then free the memory.
 *
 *   Output must match the format in the Week 4 instructions exactly
 *   (it is checked by the autograder).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Student record (same definition as in Task 2)
struct Student {
  char name[50];
  int id;
  float grade;
};

int main(void) {
  int n;
  struct Student* students = NULL;

  printf("Enter number of students: ");
  if (scanf("%d", &n) != 1 || n <= 0) {
    printf("Invalid number.\n");
    return 1;
  }

  // Allocate space for n Student records
  students = malloc((size_t)n * sizeof(struct Student));

  // Check allocation success: malloc returns NULL on failure
  if (students == NULL) {
    printf("Memory allocation failed.\n");
    return 1;
  }

  // Read each student: name, id, grade
  for (int i = 0; i < n; i++) {
    printf("Enter data for student %d: ", i + 1);
    // %49s leaves room for '\0' and prevents a buffer overflow
    if (scanf("%49s %d %f", students[i].name, &students[i].id,
              &students[i].grade) != 3) {
      printf("Invalid input.\n");
      free(students);  // free before exiting so nothing leaks
      return 1;
    }
  }

  // One empty line, then the table
  printf("\n");
  printf("%-6s %-11s %s\n", "ID", "Name", "Grade");
  for (int i = 0; i < n; i++) {
    printf("%-6d %-11s %.1f\n", students[i].id, students[i].name,
           students[i].grade);
  }

  // Every successful malloc needs a matching free
  free(students);

  return 0;
}