// FAT16 core functionality: https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23985072
// FAT16 structures: https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23985264
// FAT16 resolver: https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/24023820
#pragma once
#include "file.h"

// functions
struct filesystem* fat16_init();
