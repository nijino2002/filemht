/* 
 * File:   ExString.h
 * Author: root
 * Implements some functions not included in standard library
 * Created on January 10, 2010, 2:04 PM
 */

#ifndef _EXSTRING_H
#define	_EXSTRING_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <iostream>

typedef std::basic_string<char>::size_type S_T;
static const S_T npos = -1; 

class ExString {
public:
    ExString();
    ExString(const ExString& orig);
    virtual ~ExString();
private:
public:
    static void esToUpper(char *s);
    static void esToUpper(std::string& s);
    static void esToLower(char *s);
    static void esToLower(std::string& s);
    static void esReverse(std::string& s, int pos, int num);
    static std::vector<std::string> esTokenize(const std::string& src, std::string tok, bool trim=false, std::string null_subst="");
};

#endif	/* _EXSTRING_H */

