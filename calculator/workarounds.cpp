/*
 * workarounds.c
 *
 *  Created on: Mar 23, 2018
 *      Author: tleonard
 */


// This file contains implementations of functions needed to satisfy the linker for C++


#include "workarounds.h"


int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);};
void __cxa_guard_release (__guard *g) {*(char *)g = 1;};
void __cxa_guard_abort (__guard *) {};
void __cxa_pure_virtual(void) {};


