/* 
 * File:   ExString.cpp
 * Author: root
 * 
 * Created on January 10, 2010, 2:04 PM
 */

#include "ExString.h"

ExString::ExString() {
}

ExString::ExString(const ExString& orig) {
}

ExString::~ExString() {
}

void ExString::esToUpper(char *s)
{
    
}

void ExString::esToUpper(std::string& s)
{
    char *p = NULL;
    int p_len = s.length() + 1;

    p = new char[p_len];
    memset(p, 0, p_len);
    strcpy(p, s.c_str());

    for(int i=0; i<p_len; i++)
    {
        if(isalpha(p[i]) != 0)
        {
            p[i] = toupper(p[i]);
        }
    }

    s.clear();
    s.assign(p);
    delete p;
}

void ExString::esToLower(char *s)
{
    
}

void ExString::esToLower(std::string& s)
{
    char *p = NULL;
    int p_len = s.length() + 1;

    p = new char[p_len];
    memset(p, 0, p_len);
    strcpy(p, s.c_str());

    for(int i=0; i<p_len; i++)
    {
        if(isalpha(p[i]) != 0)
        {
            p[i] = tolower(p[i]);
        }
    }

    s.clear();
    s.assign(p);
    delete p;
}

void ExString::esReverse(std::string& s, int pos, int num)
{
    std::string tmp = s;

    for(int i=pos; i<pos+num; i++)
    {
        s[i] = tmp[pos+num-1-i];
    }
}

////trim指示是否保留空串，默认为保留。
std::vector<std::string> ExString::esTokenize(const std::string& src, std::string tok, bool trim, std::string null_subst)
{
    if( src.empty() || tok.empty() ) throw "tokenize: empty string\0";
    std::vector<std::string> v;
    S_T pre_index = 0, index = 0, len = 0;
    while( (index = src.find_first_of(tok, pre_index)) != npos )
    {
        if( (len = index-pre_index)!=0 )
            v.push_back(src.substr(pre_index, len));
        else if(trim==false)
            v.push_back(null_subst);
        pre_index = index+1;
    }
    
    std::string endstr = src.substr(pre_index);

    if( trim==false )
        v.push_back(endstr.empty() ? null_subst:endstr);
    else if(!endstr.empty())
        v.push_back(endstr);
    
    return v;
}  
