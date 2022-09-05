//
// Created by Vanka on 03.07.2022.
//

#ifndef MY_IDEAS_MYSTRING_H
#define MY_IDEAS_MYSTRING_H

#include <string>
#include <cstring>
#include <utility>
#include <cctype>
#include <sstream>
#include <functional>

using std::function;
using std::string;

class MyString {

private:

    std::size_t _length;
    std::size_t _array_size;
    char* _data;

    struct ArrayIndexException : public std::exception {
        const char *what() const throw() {
            return "Index out of bounds exception";
        }
    };
    
    struct ExpandException : public std::exception {
        const char *what() const throw() {
            return "Char array _length more then ULONG_LONG_MAX";
        }
    };

    struct ReserveException : public std::exception {
        const char *what() const throw() {
            return "Reserve len can't be less then _length";
        }
    };

    class CharSequence{
        const char* str;
        const size_t len;
        CharSequence(char* str): str(str), len(strlen(str)) {}
        CharSequence(const string &str): str(str.data()), len(str.length()) {}
        CharSequence(const MyString &str): str(str._data), len(str._length) {}
    };

    void init(const char* str, size_t length){
        _length = length;
        _array_size = _length+1;
        _data = new char[_array_size];
        memcpy(_data, str, sizeof(char)*(_array_size));
    }

    void init(const char* str1, size_t len1, const char* str2, size_t len2){
        _length = len1+len2;
        _array_size = _length+1;
        _data = new char[_array_size];
        memcpy(_data, str1, sizeof(char)*(len1));
        memcpy(_data+len1, str2, sizeof(char)*(len2+1));
    }

    /*void expand(size_t addingLen){
        if (addingLen) {
            if (_length + addingLen < _length)throw ExpandException();
            while (_length+addingLen+1>_array_size)expand();
            _length += addingLen;
            _data[_length] = '\0';
        }
    }*/

    void expand(size_t addingLen){
        if (addingLen) {
            if (_length + addingLen + 1 < _length)throw ExpandException();
            size_t new_size = _array_size;
            if (new_size==0)new_size = 1;
            while (_length + addingLen + 1 > new_size) {
                new_size <<= 1u;
                if (new_size == ULONG_LONG_MAX)throw ExpandException();
                else if (new_size < _array_size)new_size = ULONG_LONG_MAX;
            }
            if (new_size != _array_size) {
                char *temp = new char[new_size];
                memcpy(temp, _data, sizeof(char) * _length);
                delete[] _data;
                _data = temp;
                _array_size = new_size;
            }
        }
    }

    void addLen(size_t addingLen){
        expand(addingLen);
        _length += addingLen;
    }

    //str with \0, size without \0 char
    MyString& _append(const char* str, size_t size){
        expand(size);
        memcpy(_data+_length,str,sizeof(char)*(size+1));
        _length+=size;
//        strcat(_data, str);
        return *this;
    }

    bool _startWith(const char* main, const char* str, size_t len){
        for (size_t i = 0; i < len; ++i) {
            if (main[i]!=str[i])return false;
        }
        return true;
    }

    bool _startWith(const char* str, size_t len){
        if (len>_length)return false;
        for (size_t i = 0; i < len; ++i) {
            if (_data[i]!=str[i])return false;
        }
        return true;
    }

    bool _endsWith(const char* str, size_t len){
        if (len>_length)return false;
        for (size_t i = 0; i < len; ++i) {
            if (_data[_length-len+i]!=str[i])return false;
        }
        return true;
    }

    std::vector<MyString> _split(const char* str, size_t len){
        if (len==0)throw ArrayIndexException();
        std::vector<MyString> v;
        size_t k = 0,i;
        char* ptr = _data;

        for (i = 0; i < _length-len+1;) {
            if (_startWith(_data + i, str, len)){
                if (k) v.push_back(MyString(k+1).sappend(ptr,k));
                ptr+=k+len;
                k=0;
                i+=len;
            }else{
                ++k;
                ++i;
            }
        }
        if (_data+_length!=ptr)v.push_back(MyString(ptr));

        return v;
    }

    long long _indexOf(const char* str, size_t len, long long startIndex){
        if (startIndex<0|| startIndex >= _length) throw ArrayIndexException();
        for (; startIndex <= _length-len; ++startIndex)
            if (_startWith(_data + startIndex, str, len))return startIndex;
        return -1;
    }

    long long _lastIndexOf(const char* str, size_t len, long long startIndex){
        if (startIndex<0|| startIndex >= _length) throw ArrayIndexException();
        if (startIndex+len>_length)startIndex = _length-len;
        for (; startIndex >=0; --startIndex)
            if (_startWith(_data + startIndex, str, len))return startIndex;
        return -1;
    }

    long long _lastIndexOf(const char* str, size_t len){
        return _lastIndexOf(str,len,_length-1);
    }

    MyString& _replaceAll(const char* oldString, size_t len1, const char* newString, size_t len2){
        if (len1) {
            size_t new_len, i = 0;
            std::vector<size_t> indexes;
            while (i + len1 <= _length) {
                if (_startWith(_data + i, oldString, len1)) {
                    indexes.push_back(i);
                    i += len1;
                } else ++i;
            }

            if (!indexes.empty()) {
                new_len = _length + (len2 - len1) * indexes.size();
                if (_array_size < new_len + 1)_array_size = new_len + 1;
                char *temp = new char[_array_size];
                size_t index = indexes[0], range;
                memcpy(temp, _data, sizeof(char) * index);
                for (size_t j = 0; j < indexes.size() - 1; ++j) {
                    memcpy(temp + index, newString, sizeof(char) * len2);
                    index += len2;
                    range = indexes[j + 1] - indexes[j] - len1;
                    memcpy(temp + index, _data + indexes[j] + len1, sizeof(char) * range);
                    index += range;
                }
                memcpy(temp + index, newString, sizeof(char) * len2);
                index += len2;
                size_t last = indexes.back();
                memcpy(temp + index, _data + last + len1, sizeof(char) * (_length - last - len1));

                _length = new_len;
                temp[_length] = '\0';
                delete[] _data;
                _data = temp;
            }
        }
        return *this;
    }

    MyString& _replace(const char* oldString, size_t len1, const char* newString, size_t len2){
        if (len1) {
            for (size_t i = 0; i + len1 <= _length; ++i) {
                if (_startWith(_data + i, oldString, len1)) {
                    if (len2 > len1)expand(len2 - len1);
                    memcpy(_data + i + len2, _data + i + len1, sizeof(char) * (_length - i - len1));
                    memcpy(_data + i, newString, sizeof(char) * len2);
                    _length = _length - len1 + len2;
                    _data[_length] = '\0';
                    break;
                }
            }
        }
        return *this;
    }

    MyString& _replaceLast(const char* oldString, size_t len1, const char* newString, size_t len2){
        if (len1>0 && len1 <= _length) {
            size_t i = _length-len1+1;
            while ( i>0 ){
                --i;
                if (_startWith(_data + i, oldString, len1)) {
                    if (len2>len1)expand(len2 - len1);
                    memcpy(_data + i + len2, _data + i + len1, sizeof(char) * (_length - i - len1));
                    memcpy(_data + i, newString, sizeof(char) * len2);
                    _length=_length-len1+len2;
                    _data[_length] = '\0';
                    break;
                }
            }
        }
        return *this;
    }

public:
    static char* stringToChars(const string &s) {
//        return strdup(s.c_str());
        char* chars = new char[s.size() + 1];
        memcpy(chars, s.data(), sizeof(char) * (s.size() + 1));
        return chars;
    }

    static string charsToString(const char *chars) {
        return string(chars);
    }

    template<class T>
    static string toString(const T& object) {
        std::ostringstream stringStream;
        stringStream << object;
        return stringStream.str();
    }

    static string toString(long long num, int base){
        string result;
        long long remainder;
        bool minus = false;
        if (num<0){
            num=-num;
            minus = true;
        }
        while(num){
            remainder = num%base;
            result.push_back(remainder>9 ? (char)(55+remainder) : (char)(48+remainder));
            num/=base;
        }
        if (minus)result.push_back('-');
        char temp;
        size_t index, end = result.size() >> 1u;
        for (size_t i = 0; i < end; ++i) {
            index = result.size() - i - 1;
            temp = result[i];
            result[i] = result[index];
            result[index] = temp;
        }
        return result.length()?result:"0";
    }

    MyString() {
        _data = nullptr;
        _length = 0;
        _array_size = 0;
    }

    MyString(std::size_t reserve_len) {
        _length = 0;
        _array_size = reserve_len;
        _data = new char[reserve_len];
    }

  /*  MyString(char *chars, size_t reserve_len) {
        _length=strlen(chars);
        if (_length>reserve_len)throw ReserveException();
        _array_size = reserve_len;
        _data = new char[_array_size];
        memcpy(_data, chars, sizeof(char)*(_array_size));
    }

    MyString(const string &str, size_t reserve_len) {
        _length=str.length();
        if (_length>reserve_len)throw ReserveException();
        _array_size = reserve_len;
        _data = new char[_array_size];
        memcpy(_data, str.data(), sizeof(char)*(_array_size));
    }*/

    MyString(const string &str) {
        init(str.data(), str.length());
//        std::cout<<"str\n";
    }

    MyString(const char *chars) {
//        std::cout<<"chars\n";
        init(chars,strlen(chars));
    }

    //copy constructor
    MyString(const MyString &str){
        init(str._data,str._length);
    }

    MyString(const MyString& str, const char *str2) {
        init(str._data, str._length, str2, strlen(str2));
    }

    MyString(const MyString& str, const string &str2) {
        init(str._data, str._length, str2.data(), str2.length());
    }

    MyString(const MyString& str, const MyString& str2) {
        init(str._data, str._length, str2._data, str2._length);
    }

    MyString(const char *str, const char *str2) {
        init(str, strlen(str), str2, strlen(str2));
    }

    MyString(const string &str, const string &str2) {
        init(str.data(), str.length(), str2.data(), str2.length());
    }

    ~MyString(){
        delete[] _data;
    }

    size_t len(){
        return _length;
    }

    //change _array_size
    void reserve(size_t new_size){
        if (new_size > _array_size){
            char* temp = new char[new_size];
            memcpy(temp,_data,sizeof(char)*(_length+1));
            delete[] _data;
            _data = temp;
            _array_size = new_size;
        }
    }

    //change _length (_array_size)
    void resize(size_t new_len){
        if (_length < new_len)addLen(new_len - _length);
        else _length = new_len;
        _data[_length] = '\0';
    }

    //support negative indexes
    char& operator [](long long index){
        size_t u_index;
        if (index<0)u_index=_length+index;
        else u_index = index;
        if (u_index >= _length)throw ArrayIndexException();
        return _data[u_index];
    }

    //do not support negative indexes
    char& get(unsigned long long index){
        if (index >= _length)throw ArrayIndexException();
        return _data[index];
    }

    //cout print
    friend std::ostream& operator << (std::ostream &out, const MyString &str);

    MyString operator +(const MyString& str){
        return MyString(*this,str);
    }

    MyString operator +(const char* str){
        return MyString(*this, str);
    }

    MyString operator +(const string &str){
        return MyString(*this, str);
    }

    template<class T>
    MyString operator +(const T &obj){
        return MyString(*this).append(obj);
    }

    MyString operator +(char c){
        return MyString(*this).append(c);
    }

    template<class T>
    MyString& operator +=(const T &str){
      return append(str);
    }

    MyString operator *(unsigned int times){
        return MyString(*this).repeat(times);
    }

    MyString& operator *=(unsigned int times){
        return repeat(times);
    }

    MyString operator ()(long long start, long long end){
        return substring(start,end);
    }

    MyString operator ()(long long end){
        return substring(end);
    }

    MyString& operator =(const MyString& str){
        if (&str!=this){
            delete[] _data;
            init(str._data, str._length);
        }
        return *this;
    }

    bool operator ==(const MyString& str){
        return equals(str);
    }
    
    bool equals(const MyString& str){
        if (&str==this)return true;
        for (size_t i = 0; i < _length; ++i) {
            if (_data[i]!=str._data[i])return false;
        }
        return true;
    }

    MyString& append(char c){
        return sappend(&c,1);
    }

    MyString& append(const char* str){
        return _append(str, strlen(str));
    }

    MyString& append(const MyString& str){
        return _append(str._data, str._length);
    }
    
    MyString& append(const string &str){
//        std::cout<<"append str\n";
        return _append(str.data(), str.length());
    }

    template<class T>
    MyString& append(const T& obj){
        return append(toString(obj));
    }

    //save append: str without \0, size without \0 char
    MyString& sappend(const char* str, size_t size){
        expand(size);
        memcpy(_data+_length, str,sizeof(char)*size);
        _data[_length+=size]='\0';
        return *this;
    }

    string getString() {
        return string(_data);
    }

    char* getChars(){
        return _data;
    }

    char* getData(){
        return _data;
    }

    MyString& repeat(unsigned int times){
        size_t copy_len = _length;
        char* copy = new char[copy_len+1];
        memcpy(copy,_data,sizeof(char)*(copy_len+1));
        for (unsigned int i = 1; i < times; ++i) {
            _append(copy, copy_len);
        }
        delete[] copy;
        return *this;
    }

    std::vector<MyString> split(const char* str){
        return _split(str, strlen(str));
    }

    std::vector<MyString> split(const string &str){
        return _split(str.data(), str.size());
    }

    std::vector<MyString> split(const MyString& str){
        return _split(str._data, str._length);
    }

    std::vector<MyString> split(){
        return split(" ");
    }

    MyString substring(long long start, long long end){
        if (start<0)start= _length + start;
        if (end<0)end= _length + end;
        if (start>end || end > _length)throw ArrayIndexException();

        return MyString(end-start+1).sappend(_data+start,end-start);
    }

    MyString substring(long long end){
        return substring(0, end);
    }

    bool isEmpty(){
        return _length==0;
    }

    MyString& reverse(){
        char temp;
        size_t index, end = _length >> 1u;
        for (size_t i = 0; i < end; ++i) {
            index = _length - i - 1;
            temp = _data[i];
            _data[i] = _data[index];
            _data[index] = temp;
        }
        return *this;
    }

    long long indexOf(char c, long long startIndex = 0){
        if (startIndex<0|| startIndex >= _length) throw ArrayIndexException();
        for (; startIndex < _length; ++startIndex)
            if (_data[startIndex] == c)return startIndex;
        return -1;
    }

    long long indexOf(const char* str, long long startIndex = 0){
        return _indexOf((char *) str, strlen(str), startIndex);
    }

    long long indexOf(const string &str, long long startIndex = 0){
        return _indexOf(str.data(), str.size(), startIndex);
    }

    long long indexOf(const MyString& str, long long startIndex = 0){
        return _indexOf(str._data, str._length, startIndex);
    }


    long long lastIndexOf(char c, long long startIndex){
        if (startIndex<0 || startIndex >= _length) throw ArrayIndexException();
        for (;startIndex >= 0; --startIndex)
            if (_data[startIndex] == c)return startIndex;
        return -1;
    }

    long long lastIndexOf(char c){
        return lastIndexOf(c, _length - 1);
    }

    long long lastIndexOf(const char* str){
        return _lastIndexOf(str, strlen(str));
    }

    long long lastIndexOf(const char* str, long long startIndex){
        return _lastIndexOf(str, strlen(str), startIndex);
    }

    long long lastIndexOf(const string &str){
        return _lastIndexOf(str.data(), str.size());
    }

    long long lastIndexOf(const string &str, long long startIndex){
        return _lastIndexOf(str.data(), str.size(), startIndex);
    }

    long long lastIndexOf(const MyString& str){
        return _lastIndexOf(str._data, str._length);
    }

    long long lastIndexOf(const MyString& str, long long startIndex){
        return _lastIndexOf(str._data, str._length, startIndex);
    }

    bool contains(char c){
        for (size_t i = 0; i < _length; ++i) {
            if (_data[i]==c)return true;
        }
        return false;
    }

    bool contains(const char* str){
        return indexOf(str)!=-1;
    }

    bool contains(const string &str){
        return indexOf(str)!=-1;
    }

    bool contains(const MyString& str){
        return indexOf(str)!=-1;
    }

    MyString& leftStrip(const char c){
        size_t shift = 0;
        while(shift < _length && _data[shift]==c) ++shift;
        _length-=shift;
        for (size_t i = 0; i <= _length; ++i) {
            _data[i]=_data[i+shift];
        }
        return *this;
    }

    MyString& rightStrip(const char c){
        while(_length>0 && _data[_length-1]==c) --_length;
        _data[_length]='\0';
        return *this;
    }

    MyString& strip(const char c){
        return leftStrip(c).rightStrip(c);
    }

    MyString& trim(){
        return strip(' ');
    }

    MyString& toLowerCase(){
        for (size_t i = 0; i < _length; ++i) {
            _data[i] = (char)std::tolower(_data[i]);
        }
        return *this;
    }

    MyString& toUpperCase(){
        for (size_t i = 0; i < _length; ++i) {
            _data[i] = (char)std::toupper(_data[i]);
        }
        return *this;
    }

    static char toLowerCase(char c){
        return (char)std::tolower(c);
    }

    static char toUpperCase(char c){
        return (char)std::toupper(c);
    }

    static bool isLowerCase(char c){
        return std::islower(c);
    }

    static bool isUpperCase(char c){
        return std::isupper(c);
    }

    static bool isAlpha(char c){//alphabet
        return std::isalpha(c);
    }

    static bool isDigit(char c){
        return std::isdigit(c);
    }

    static bool isSpace(char c){
        return std::isspace(c);
    }

    static bool isPunct(char c){//punctuation
        return std::ispunct(c);
    }

    bool startWith(const char* str){
        return _startWith( str, strlen(str));
    }

    bool startWith(const string &str){
        return _startWith(str.data(), str.size());
    }

    bool startWith(const MyString& str){
        return _startWith(str._data, str._length);
    }

    bool endsWith(const char* str){
        return _endsWith( str, strlen(str));
    }

    bool endsWith(const string &str){
        return _endsWith(str.data(), str.size());
    }

    bool endsWith(const MyString& str){
        return _endsWith(str._data, str._length);
    }

    template<class T>
    MyString join(const T* arr, size_t len){
        if (len==0)return MyString();
        MyString result(toString(arr[0]));
        for (size_t i = 1; i < len; ++i) {
            result.append(*this).append(toString(arr[i]));
        }
        return result;
    }

    MyString& replaceAll(const char* oldString, const char* newString){
        return _replaceAll(oldString,strlen(oldString),newString,strlen(newString));
    }

    MyString& replaceAll(const string &oldString, const char* newString){
        return _replaceAll(oldString.data(),oldString.size(), newString,strlen(newString));
    }

    MyString& replaceAll(const MyString& oldString, const char* newString){
        return _replaceAll(oldString._data,oldString._length, newString,strlen(newString));
    }

    MyString& replaceAll(const MyString& oldString, const MyString& newString){
        return _replaceAll(oldString._data,oldString._length, newString._data, newString._length);
    }

    MyString& replaceAll(const MyString& oldString, const string &newString){
        return _replaceAll(oldString._data,oldString._length, newString.data(), newString.length());
    }

    MyString& replace(const char* oldString, const char* newString){
        return _replace(oldString,strlen(oldString),newString,strlen(newString));
    }

    MyString& replace(const string &oldString, const char* newString){
        return _replace(oldString.data(),oldString.size(), newString,strlen(newString));
    }

    MyString& replace(const MyString& oldString, const char* newString){
        return _replace(oldString._data,oldString._length, newString,strlen(newString));
    }

    MyString& replace(const MyString& oldString, const MyString& newString){
        return _replace(oldString._data,oldString._length, newString._data, newString._length);
    }

    MyString& replace(const MyString& oldString, const string &newString){
        return _replace(oldString._data,oldString._length, newString.data(), newString.length());
    }

    MyString& replaceLast(const char* oldString, const char* newString){
        return _replaceLast(oldString,strlen(oldString),newString,strlen(newString));
    }

    MyString& replaceLast(const string &oldString, const char* newString){
        return _replaceLast(oldString.data(),oldString.size(), newString,strlen(newString));
    }

    MyString& replaceLast(const MyString& oldString, const char* newString){
        return _replaceLast(oldString._data,oldString._length, newString,strlen(newString));
    }

    MyString& replaceLast(const MyString& oldString, const MyString& newString){
        return _replaceLast(oldString._data,oldString._length, newString._data, newString._length);
    }

    MyString& replaceLast(const MyString& oldString, const string &newString){
        return _replaceLast(oldString._data,oldString._length, newString.data(), newString.length());
    }

    MyString& fill(char c){
        for (size_t i = 0; i < _length; ++i) {
            _data[i]=c;
        }
        return *this;
    }

    MyString& clear(){
        _length = 0;
        if (_data != nullptr)_data[0] = '\0';
        return *this;
    }

    void forEach(const function<void(char c)> &func){
        for (size_t i = 0; i < _length; ++i)
            func(_data[i]);
    }

    void forEach(const function<void(char c, size_t index)>& func){
        for (size_t i = 0; i < _length; ++i)
            func(_data[i],i);
    }

    template<typename U>
    void forEach(const function<U(char c)> &func){
        for (size_t i = 0; i < _length; ++i)
            func(_data[i]);
    }

    template<typename U>
    void forEach(const function<U(char c, size_t index)> &func){
        for (size_t i = 0; i < _length; ++i)
            func(_data[i],i);
    }

    int count(const function<bool(char c)> &func){
        int k = 0;
        for (size_t i = 0; i < _length; ++i)
            if (func(_data[i]))k++;
        return k;
    }

    int count(const function<bool(char c, size_t index)> &func){
        int k = 0;
        for (size_t i = 0; i < _length; ++i)
            if (func(_data[i], i))k++;
        return k;
    }

    size_t count(char c){
        size_t k = 0;
        for (size_t i = 0; i < _length; ++i)
            if (_data[i]==c)k++;
        return k;
    }

    bool any(const function<bool(char c)> &func){
        for (size_t i = 0; i < _length; ++i) {
            if (func(_data[i]))return true;
        }
        return false;
    }

    bool all(const function<bool(char c)> &func){
        for (size_t i = 0; i < _length; ++i) {
            if (!func(_data[i]))return false;
        }
        return true;
    }



};

std::ostream &operator<<(std::ostream &out, const MyString &str) {
    return out<<str._data;
}

template<class T>
MyString operator +(const T& obj, const MyString& str){
    return MyString(MyString::toString(obj)).append(str);
}

#endif //MY_IDEAS_MYSTRING_H


//repeat
//split
//substring

//startWith
//endsWith

//leftStrip
//rightStrip
//strip
//trim

//lower
//upper
//codePointAt?

//isDigit
//join

//replace
//replaceAll
//replaceLast

//forEach
//count

//append int

//SimpleString

//delete cout comments

