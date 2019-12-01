#include "kernel/types.h"
#include "kernel/proc.h"

// Shamlessly stolen from https://www.geeksforgeeks.org/quick-sort/
void swap(struct proc *arr[], int low, int high){
  struct proc *hold  = arr[low];
  arr[low] = arr[high];
  arr[high] = hold;
}

int partition (struct proc* arr[], int low, int high) 
{ 
  int pivot= arr[high]->vruntime;    // pivot 
  int i = (low - 1);  // Index of smaller element 

  for (int j = low; j <= high- 1; j++) 
  { 
    // If current element is smaller than the pivot 
    if (arr[j]->vruntime < pivot) 
    { 
      i++;    // increment index of smaller element 
      swap(arr, i, j); 
    } 
  } 
  swap(arr, i + 1, high); 
  return (i + 1); 
} 

void sortProc(struct proc* arr[], int low, int high) 
{ 
  if (low < high) 
  { 
    /* pi is partitioning index, arr[p] is now 
     *            at right place */
    int pi = partition(arr, low, high); 

    // Separately sort elements before 
    //         // partition and after partition 
    sortProc(arr, low, pi - 1); 
    sortProc(arr, pi + 1, high); 
  } 
} 
