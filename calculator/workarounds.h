/*
 * workarounds.h
 *
 *  Created on: Mar 23, 2018
 *      Author: tleonard
 */

#ifndef WORKAROUNDS_H_
#define WORKAROUNDS_H_

__extension__ typedef int __guard __attribute__((mode (__DI__)));

extern "C" int __cxa_guard_acquire(__guard *);
extern "C" void __cxa_guard_release (__guard *);
extern "C" void __cxa_guard_abort (__guard *);
extern "C" void __cxa_pure_virtual(void);

#endif /* WORKAROUNDS_H_ */
