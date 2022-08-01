# chisato
## Introduction
`chisato`是一个简单的**配置文件解析器**(parser)。
支持的字段类型：
* 整型(integer)
* 浮点数(floating-point number)
* 字符串(string)
* 用户自定义(user-defined)

## Format
配置文件的格式是`字段-值对`(field-value pair)：`field: value`，冒号到`value`之间有无空格不影响解析。

`注释`(comment)支持单行注释，`#`后面的内容会被忽视。

```
# comment line
field1: value1 # comment after field-value pair
...
field-N: value-N
```

## Usage
对于整型，浮点数，字符串没有特殊定制需求，直接绑定配置字段：
```cpp
int i = 0; // default: 0
chisato::AddConfig("int", &i);
// double, string same with int
```
对于自定义字段类型，比如枚举类型，可以通过注册`回调`(callback)的方式解析并设置，其函数签名(signature)：
```cpp
/* \param value Value of field
 * \return
 *  true -- success
 */
bool(StrSlice value)
```
`StrSlice`是`std::string_view`(c++17)的一个不完整实现，仅满足该库需求，具体参见`str_slice.h`。

`chisato`支持两种风格的回调：
* C风格回调(C-style callback)
```cpp
typedef bool(*ConfigCallback)(StrSlice, void*);
```
`void*`是**泛型参数**(generic parameter)，用它捕获变量（模拟lambda，不过是具名函数）。

* Cpp风格回调(Cpp-style callback)
```cpp
typedef std::function<bool(StrSlice)> ConfigFunction;
```
cpp风格是采用`std::function<>`接受各种可调用对象（包括函数指针，lambda和函数对象）。

Example:
```cpp
inline void str2enum(StrSlice value, void *args) noexcept {
  Enum *e = (Enum*)args;
  if (value == "xxxx") {
    *e = Enum::xxx;
  }
}

Enum e

// C-style callback
chisato::AddConfig("enum", &e, &str2enum);

// Cpp-styple callback
chisato::AddConfig("enum", [&e](StrSlice value) {
  str2enum(value, &e);
});
```
当配置字段全部添加完成之后，调用`Parse`解析即可：
```cpp
std::string errmsg;
// Register Config fields
chisato::AddConfig("...", &...);
// ...

auto success = chisato::Parse(path-name, errmsg);
if (success) {
  // ...
  // do something
} else {
  // do something
  ::fprintf(stderr, "Failed to parse the config: \n%s\n", errmsg.c_str());
  // e.g. exit()
}
```

