#ifndef STRING_EX_H
#define STRING_EX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief      { Get sub-string from a string }
 *
 * @param[in]  s     { Original string }
 * @param[out] ss    { Output sub-string }
 * @param[in]  pos   The beginning position
 * @param[in]  l     { sub-string length }
 */
void strex_substr(char *s, char *ss, int pos, int l);

/**
 * @brief Splitting a string into several sub-strings by a dedicated delimeter.
 *        The splitted sub-strings will be stored in a newly allocated array, 
 *        which will be finally returned.
 * 
 * @param str Original string
 * @param delim The splitting delimeter character
 * @param count The number of the splitted sub-strings
 * @return The array that stores the splitted sub-strings
 */
char **strex_split(const char *str, char delim, int *count);

#endif