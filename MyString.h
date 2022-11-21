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
#include <vector>
using std::function;
using std::string;

class MyString {
typedef unsigned long long size;
private:
	
    size _length;
    size _array_size;
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

    //temporary objects class
    class CharSequence{
    public:
        const char* data;
        const size len;
        CharSequence(const char* str): data(str), len(strlen(str)) {}
        CharSequence(const string &str): data(str.data()), len(str.length()) {}
        CharSequence(const MyString &str): data(str._data), len(str._length) {}
        CharSequence(const char &c): data(&c), len(1) {}
    };

    void init(const char* str, size length){
        _length = length;
        _array_size = _length+1;
        _data = new char[_array_size];
        memcpy(_data, str, sizeof(char)*(_array_size));
    }

    void init(const char* str1, size len1, const char* str2, size len2){
        _length = len1+len2;
        _array_size = _length+1;
        _data = new char[_array_size];
        memcpy(_data, str1, sizeof(char)*len1);
        memcpy(_data+len1, str2, sizeof(char)*len2);
        _data[_length]='\0';
    }

    void expand(size addingLen){
        if (addingLen) {
            if (_length + addingLen + 1 < _length)throw ExpandException();
            size new_size = _array_size;
            if (new_size==0)new_size = 1;
            while (_length + addingLen + 1 > new_size) {
                new_size <<= 1u;
                if (new_size == ULLONG_MAX)throw ExpandException();
                else if (new_size < _array_size)new_size = ULLONG_MAX;
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

    void addLen(size addingLen){
        expand(addingLen);
        _length += addingLen;
    }

    //size without \0 char
    MyString& _append(const char* str, size size){
        expand(size);
        memcpy(_data+_length,str,sizeof(char)*size);
        _data[_length+=size]='\0';
        return *this;
    }

    bool _startWith(const char* main, const char* str, size len){
        for (size i = 0; i < len; ++i) {
            if (main[i]!=str[i])return false;
        }
        return true;
    }

    bool _startWith(const char* str, size len){
        if (len>_length)return false;
        for (size i = 0; i < len; ++i) {
            if (_data[i]!=str[i])return false;
        }
        return true;
    }

    bool _endsWith(const char* str, size len){
        if (len>_length)return false;
        for (size i = 0; i < len; ++i) {
            if (_data[_length-len+i]!=str[i])return false;
        }
        return true;
    }

    std::vector<MyString> _split(const char* str, size len){
        if (len==0)throw ArrayIndexException();
        std::vector<MyString> v;
        size k = 0,i;
        char* ptr = _data;

        for (i = 0; i < _length-len+1;) {
            if (_startWith(_data + i, str, len)){
                if (k) v.push_back(MyString(k+1)._append(ptr,k));
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

    long long _indexOf(const char* str, size len, long long startIndex){
        if (startIndex<0|| startIndex >= _length) throw ArrayIndexException();
        for (; startIndex+len <= _length; ++startIndex)
            if (_startWith(_data + startIndex, str, len))return startIndex;
        return -1;
    }

    long long _lastIndexOf(const char* str, size len, long long startIndex){
        if (startIndex<0|| startIndex >= _length) throw ArrayIndexException();
        if (startIndex+len>_length)startIndex = _length-len;
        for (; startIndex >=0; --startIndex)
            if (_startWith(_data + startIndex, str, len))return startIndex;
        return -1;
    }

    long long _lastIndexOf(const char* str, size len){
        return _lastIndexOf(str,len,_length-1);
    }

    MyString& _replaceAll(const char* oldString, size len1, const char* newString, size len2){
        if (len1) {
            size new_len, i = 0;
            std::vector<size> indexes;
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
                size index = indexes[0], range;
                memcpy(temp, _data, sizeof(char) * index);
                for (size j = 0; j < indexes.size() - 1; ++j) {
                    memcpy(temp + index, newString, sizeof(char) * len2);
                    index += len2;
                    range = indexes[j + 1] - indexes[j] - len1;
                    memcpy(temp + index, _data + indexes[j] + len1, sizeof(char) * range);
                    index += range;
                }
                memcpy(temp + index, newString, sizeof(char) * len2);
                index += len2;
                size last = indexes.back();
                memcpy(temp + index, _data + last + len1, sizeof(char) * (_length - last - len1));

                _length = new_len;
                temp[_length] = '\0';
                delete[] _data;
                _data = temp;
            }
        }
        return *this;
    }

    MyString& _replace(const char* oldString, size len1, const char* newString, size len2){
        if (len1) {
            for (size i = 0; i + len1 <= _length; ++i) {
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

    MyString& _replaceLast(const char* oldString, size len1, const char* newString, size len2){
        if (len1>0 && len1 <= _length) {
            size i = _length-len1+1;
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
        size index, end = result.size() >> 1u;
        for (size i = 0; i < end; ++i) {
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

    MyString(size reserve_len) {
        _length = 0;
        _array_size = reserve_len;
        _data = new char[reserve_len];
    }

    //copy constructor
    MyString(const MyString &str){
        init(str._data,str._length);
    }

    MyString(const char *chars) {
        init(chars,strlen(chars));
    }

    MyString(const string &str) {
        init(str.data(), str.length());
    }

    MyString(const CharSequence& str, const CharSequence &str2) {
        init(str.data, str.len, str2.data, str2.len);
    }

    ~MyString(){
        delete[] _data;
    }

    inline size length(){
        return _length;
    }

    //change _array_size
    void reserve(size new_size){
        if (new_size > _array_size){
            char* temp = new char[new_size];
            memcpy(temp,_data,sizeof(char)*(_length+1));
            delete[] _data;
            _data = temp;
            _array_size = new_size;
        }
    }

    //change _length (_array_size)
    void resize(size new_len){
        if (_length < new_len)addLen(new_len - _length);
        else _length = new_len;
        _data[_length] = '\0';
    }

    //support negative indexes
    char& operator [](long long index){
        size u_index;
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

    //cin
    friend std::istream& operator >> (std::istream &in, MyString &str);

    friend std::istream& getline(std::istream& in, MyString &str);



    template<class T>
    MyString operator +(const T &obj){
        return MyString(*this).append(obj);
    }

    template<class T>
    MyString& operator +=(const T &obj){
      return append(obj);
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

    MyString& operator =(const MyString& str) {
        if (&str!=this){
            delete[] _data;
            init(str._data, str._length);
        }
        return *this;
    }

    bool operator ==(const MyString& str) const{
        return equals(str);
    }
    
    bool equals(const MyString& str) const{
        if (&str==this)return true;
        for (size i = 0; i < _length; ++i) {
            if (_data[i]!=str._data[i])return false;
        }
        return true;
    }

    MyString& append(const char& c){
        return _append(&c,1);
    }

    MyString& append(const char* str){
        return _append(str, strlen(str));
    }

    MyString& append(const MyString& str){
        return _append(str._data, str._length);
    }
    
    MyString& append(const string &str){
        return _append(str.data(), str.length());
    }

    template<class T>
    MyString& append(const T& obj){
        return append(toString(obj));
    }

    string toString() {
        return string(_data);
    }

    char* toChars(){
        return _data;
    }

    char* getData(){
        return _data;
    }

    MyString& repeat(unsigned int times){
        size copy_len = _length;
        char* copy = new char[copy_len+1];
        memcpy(copy,_data,sizeof(char)*(copy_len+1));
        for (unsigned int i = 1; i < times; ++i) {
            _append(copy, copy_len);
        }
        delete[] copy;
        return *this;
    }

    std::vector<MyString> split(const CharSequence &str){
        return _split(str.data, str.len);
    }

    std::vector<MyString> split(){
        return split(" ");
    }

    MyString substring(long long start, long long end){
        if (start<0)start= _length + start;
        if (end<0)end= _length + end;
        if (start>end || end > _length)throw ArrayIndexException();

        return MyString(end-start+1)._append(_data+start,end-start);
    }

    MyString substring(long long start){
        return substring(start, _length);
    }

    MyString& reverse(){
        char temp;
        size index, end = _length >> 1u;
        for (size i = 0; i < end; ++i) {
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

    long long indexOf(const CharSequence& str, long long startIndex = 0){
        return _indexOf(str.data, str.len, startIndex);
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

    long long lastIndexOf(const CharSequence& str){
        return _lastIndexOf(str.data, str.len);
    }

    long long lastIndexOf(const CharSequence& str, long long startIndex){
        return _lastIndexOf(str.data, str.len, startIndex);
    }

    bool contains(char c){
        for (size i = 0; i < _length; ++i) {
            if (_data[i]==c)return true;
        }
        return false;
    }

    bool contains(const CharSequence& str){
        return indexOf(str)!=-1;
    }

    MyString& leftStrip(const char c){
        size shift = 0;
        while(shift < _length && _data[shift]==c) ++shift;
        _length-=shift;
        for (size i = 0; i <= _length; ++i) {
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
        for (size i = 0; i < _length; ++i) {
            _data[i] = (char)std::tolower(_data[i]);
        }
        return *this;
    }

    MyString& toUpperCase(){
        for (size i = 0; i < _length; ++i) {
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

    bool startWith(const CharSequence& str){
        return _startWith(str.data, str.len);
    }

    bool endsWith(const CharSequence& str){
        return _endsWith(str.data, str.len);
    }

    template<class T>
    MyString join(const T* arr, size len){
        if (len==0)return MyString();
        MyString result(toString(arr[0]));
        for (size i = 1; i < len; ++i) {
            result.append(*this).append(toString(arr[i]));
        }
        return result;
    }

    MyString& replaceAll(const CharSequence& oldString, const CharSequence& newString){
        return _replaceAll(oldString.data,oldString.len,newString.data,newString.len);
    }

    MyString& replace(const CharSequence& oldString, const CharSequence& newString){
        return _replace(oldString.data,oldString.len,newString.data,newString.len);
    }

    MyString& replaceLast(const CharSequence& oldString, const CharSequence& newString){
        return _replaceLast(oldString.data,oldString.len,newString.data,newString.len);
    }

    MyString& fill(char c){
        for (size i = 0; i < _length; ++i) {
            _data[i]=c;
        }
        return *this;
    }

    MyString& clear(){
        _length = 0;
        if (_data != nullptr)_data[0] = '\0';
        return *this;
    }

    MyString& set(const CharSequence& str) {
        if (str.data!=_data){
            delete[] _data;
            init(str.data, str.len);
        }
        return *this;
    }

    void forEach(const function<void(char c)> &func){
        for (size i = 0; i < _length; ++i)
            func(_data[i]);
    }

    void forEach(const function<void(char c, size index)>& func){
        for (size i = 0; i < _length; ++i)
            func(_data[i],i);
    }

    template<typename U>
    void forEach(const function<U(char c)> &func){
        for (size i = 0; i < _length; ++i)
            func(_data[i]);
    }

    template<typename U>
    void forEach(const function<U(char c, size index)> &func){
        for (size i = 0; i < _length; ++i)
            func(_data[i],i);
    }

    size count(const function<bool(char c)> &func){
        size k = 0;
        for (size i = 0; i < _length; ++i)
            if (func(_data[i]))++k;
        return k;
    }

    size count(const function<bool(char c, size index)> &func){
        size k = 0;
        for (size i = 0; i < _length; ++i)
            if (func(_data[i], i))++k;
        return k;
    }

    size count(char c){
        size k = 0;
        for (size i = 0; i < _length; ++i)
            if (_data[i]==c)++k;
        return k;
    }

    bool any(const function<bool(char c)> &func){
        for (size i = 0; i < _length; ++i) {
            if (func(_data[i]))return true;
        }
        return false;
    }

    bool all(const function<bool(char c)> &func){
        for (size i = 0; i < _length; ++i) {
            if (!func(_data[i]))return false;
        }
        return true;
    }

    int toInt(){
        return std::stoi(_data);
    }

    long long toLong(){
        return std::stoul(_data);
    }

    unsigned long long toULong(){
        return std::stoull(_data);
    }

    float toFloat(){
        return std::stof(_data);
    }

    double toDouble(){
        return std::stod(_data);
    }

    long double toLongDouble(){
        return std::stold(_data);
    }

    //iterator
    template<typename ValueType>
    class MyIterator: public std::iterator<std::input_iterator_tag, ValueType>{
        friend class MyString;//for private constructor
    private:
        ValueType* p;
        MyIterator(ValueType* p):p(p) {}

    public:
        MyIterator(const MyIterator &it):p(it.p){}

        bool operator!=(MyIterator const& other) const {
            return p != other.p;;
        }

        bool operator==(MyIterator const& other) const{
            return p == other.p;
        }


        MyIterator& operator++() {
            ++p;
            return *this;
        }
        MyIterator& operator--() {
            --p;
            return *this;
        }

        MyIterator& operator+=(int num) {
            p+=num;
            return *this;
        }

        MyIterator& operator-=(int num) {
            p-=num;
            return *this;
        }

        typename MyIterator<ValueType>::reference operator*() const{
            return *p;
        };
    };

    typedef MyIterator<char> iterator;
    typedef MyIterator<const char> const_iterator;

    iterator begin(){
        return iterator(_data);
    }

    iterator end(){
        return iterator(_data + _length);
    }

    const_iterator begin() const{
        return const_iterator(_data);
    }

    const_iterator end() const{
        return const_iterator(_data + _length);
    }

};
std::ostream& operator <<(std::ostream &out, const MyString &str) {
    return out<<str._data;
}

std::istream& operator >>(std::istream &in, MyString &str) {
    string temp;
    in >> temp;
    str.set(temp);
    return in;
}

std::istream& getline(std::istream& in, MyString &str){
    string temp;
    std::getline(in, temp);
    str.set(temp);
    return in;
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

//CharSequence

//delete cout comments

