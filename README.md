# chisato
## Introduction
`chisato`是一个简单的**配置文件解析器**(parser)。

暂时只支持`字符串`值。

该库是`header-only`的，其中的全局变量定义为`static`变量(算是一个比较tricky的做法)，因此最好只包含在一个翻译单元(translation unit)中。

## Format
```
# comment line
field1: value1
...
```

## Usage
```cpp
std::string errmsg;
auto success = chisato::Parse(path-name, errmsg);
if (success) {
  auto value1 = chisato::GetField(field-name);
  // ...
  // do something
} else {
  // do something
  ::fprintf(stderr, "Failed to parse the config: %s\n", errmsg.c_str());
  // e.g. exit()
}
```

