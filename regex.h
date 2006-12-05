/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * regex.c - regex compiler and matcher.
 *
 * Copyright (C) 1999-2001 Steven Phillips
 * Copyright (C) 2002-2006 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Created:     Wed Aug 11 1999
 * Synopsis:    regex compiler and matcher.
 * Authors:     Steven Phillips
 * Notes:
 *      If using elsewhere extract the code within the 2 "Header for regex"
 *      comments and place into a regex.h header file.
 * 
 *  To compile a small test harness run
 *      gcc -D_TEST_RIG regex.c
 */

#ifndef __REGEX_H
#define __REGEX_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define meREGEXCLASS_SIZE 32
typedef unsigned char meRegexClass[meREGEXCLASS_SIZE] ;

typedef struct meRegexItem {
    /* linked list of all malloced items */
    struct meRegexItem *lnext ;   /* next item in the global list */
    /* current regex item linkage */
    struct meRegexItem *next ;    /* next item in the regex */
    struct meRegexItem *child ;   /* Groups sub-expression list */
    struct meRegexItem *alt ;     /* Group alternative sub expression */
    int matchMin ;                /* minimum number of matches */
    int matchMax ;                /* maximum number of matches */
    union {
        unsigned char cc ;        /* character data */
        unsigned char dd[2] ;     /* double character data */
        int group ;               /* group number */
        meRegexClass cclass ;     /* Class bit mask */
    } data ;
    unsigned char type ;          /* item type */
} meRegexItem ;    

typedef struct {
    int start ;
    int end ;
} meRegexGroup ;

typedef struct meRegex {
    /* Public elements */
    unsigned char *regex ;        /* the regex string */
    int groupNo ;                 /* the number of groups in regex */
    int newlNo ;                  /* the number of \n char found */
    int flags ;                   /* compile + okay flag - set to 0 to force recompile */
    meRegexGroup *group ;         /* the group start and end offsets */
    
    /* Private elements */
    int regexSz ;                 /* malloced size of regex string */
    int groupSz ;                 /* malloced size of group array */
    meRegexItem  *lhead ;         /* link list of all malloced items */
    meRegexItem  *lnext ;         /* the next free item */
    meRegexItem  *head ;          /* pointer to the regex first item */
    meRegexClass  start ;         /* first character cclass bit mask */
} meRegex ;

/* meRegexComp return values */
#define meREGEX_OKAY               0
/* First set of errors can be cause by the regex being incomplete - 
 * useful for isearch - ME uses UNKNOWN for gnu regex */
#define meREGEX_ERROR_UNKNOWN      1
#define meREGEX_ERROR_TRAIL_BKSSH  2
#define meREGEX_ERROR_OCLASS       3
#define meREGEX_ERROR_OGROUP       4
/* the next set are regex errors which aren't caused by being incomplete */
#define meREGEX_ERROR_CLASS        5
#define meREGEX_ERROR_UNSUPCLASS   6
#define meREGEX_ERROR_BKSSH_CHAR   7
#define meREGEX_ERROR_CGROUP       8
#define meREGEX_ERROR_INTERVAL     9
#define meREGEX_ERROR_BACK_REF     10
#define meREGEX_ERROR_NESTGROUP    11
/* the next set are internal errors */
#define meREGEX_ERROR_MALLOC       12

/* meRegexComp internal return value - never actually returned */
#define meREGEX_ALT               -1

/* meRegexComp flags - only ICASE should be passed by the user */
#define meREGEX_ICASE           0x01
#define meREGEX_VALID           0x02
#define meREGEX_MAYBEEMPTY      0x04
#define meREGEX_BEGLINE         0x08
#define meREGEX_MAYENDBUFF      0x10
#define meREGEX_CLASSCHANGE     0x20
#define meREGEX_CLASSUSED       0x40

#define meRegexInvalidate(regex)   (regex->flags = 0)
#define meRegexClassChanged(regex) (regex->flags |= meREGEX_CLASSCHANGE)

/* meRegexMatch flags */
#define meREGEX_BACKWARD        0x01
#define meREGEX_BEGBUFF         0x02
#define meREGEX_ENDBUFF         0x04
#define meREGEX_MATCHWHOLE      0x08

int
meRegexComp(meRegex *regex, unsigned char *regStr, int flags) ;
int
meRegexMatch(meRegex *regex, unsigned char *string, int len, 
             int offsetS, int offsetE, int flags) ;
void
meRegexFree(meRegex *regex) ;

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __REGEX_H */
