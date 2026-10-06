/*
 * week4_1_dynamic_array.c
 * Author: [SHASHANK SAGAR NARWA]
 * Student ID: [251ADB118]
 * Description:
 *   Demonstrates creation and usage of a dynamic array using malloc.
 *   Allocate memory for n integers, read them from the user,
 *   print their sum and average, and then free the memory.
 *
 *   Output must match the format in the Week 4 instructions exactly
 *   (it is checked by the autograder).
 */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  int n;
  int* arr = NULL;

  printf("Enter number of elements: ");
  if (scanf("%d", &n) != 1 || n <= 0) {
    printf("Invalid size.\n");
    return 1;
  }

  // Allocate memory for n integers; sizeof(int) keeps it portable
  arr = malloc((size_t)n * sizeof(int));

  // Check allocation success: malloc returns NULL on failure
  if (arr == NULL) {
    printf("Memory allocation failed.\n");
    return 1;
  }

  // Prompt, then read n integers into the array
  printf("Enter %d integers: ", n);
  long sum = 0;
  for (int i = 0; i < n; i++) {
    if (scanf("%d", &arr[i]) != 1) {
      printf("Invalid input.\n");
      free(arr);  // free before exiting so nothing leaks
      return 1;
    }
    sum += arr[i];
  }

  // Cast to double so the average is not truncated (7 8 -> 7.50)
  double avg = (double)sum / n;

  printf("Sum = %ld\n", sum);
  printf("Average = %.2f\n", avg);

  // Every successful malloc needs a matching free
  free(arr);

  return 0;
}