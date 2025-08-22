# MolCpp Core - C++20 Block and Frame Implementation

## 概述

成功实现了C++20增强的`Block`和`Frame`类，支持异构数据存储和现代C++特性。

## 架构特点

### Block类 (C++20增强版)
- **异构存储**: 使用`std::any`支持不同类型的`xt::xarray`
- **C++20概念**: 使用`XtensorNumericArray`概念进行类型约束
- **范围支持**: 提供`keys()`, `values()`, `items()`的ranges视图
- **类型安全**: 模板化访问器确保类型正确性
- **现代特性**: 使用`std::format`、`std::span`等C++20特性

### Frame类 (C++20增强版)
- **层次化容器**: 存储多个命名的Block实例
- **跨Block访问**: 直接访问任意Block中的变量
- **元数据支持**: 使用`std::any`存储任意类型的元数据
- **C++20概念**: 使用`FrameBlock`和`FrameMetadata`概念
- **范围支持**: 提供blocks的ranges视图

## 文件结构

```
molcpp/core/include/molcpp/core/
├── block.hpp              # Block类定义 (C++20)
├── block_impl.hpp         # Block实现 (C++20)
├── frame.hpp              # Frame类定义 (C++20)
├── frame_impl.hpp         # Frame实现 (C++20)
└── README.md              # 本文档
```

## 测试覆盖

✅ **18个断言通过，2个测试用例**

- **Block基础操作**: 创建、设置、访问、类型安全
- **Frame基础操作**: Block管理、跨Block变量访问
- **元数据操作**: 设置和获取不同类型的元数据

## C++20特性使用

1. **Concepts**: 类型约束和编译时检查
2. **Ranges**: 现代迭代和视图
3. **std::format**: 现代字符串格式化
4. **std::span**: 高效数组视图
5. **模板**: 模板参数包和完美转发

## 与Python兼容性

- **底层数据结构**: 使用`xtensor`确保与Python的兼容性
- **接口设计**: 参考Python `molpy`的设计
- **类型映射**: C++类型能够直接映射到Python类型

## 性能特点

- **零拷贝访问**: 使用`std::span`进行高效数据访问
- **移动语义**: 支持完美转发和移动操作
- **模板优化**: 编译时类型检查和优化

## 编译要求

- **C++标准**: C++20
- **编译器**: GCC 13+ 或 Clang 14+
- **依赖**: xtensor, std库

## 成功指标

✅ 编译成功无错误  
✅ 所有测试通过  
✅ C++20特性正确使用  
✅ 类型安全保证  
✅ 现代C++最佳实践  

## 下一步

可以在此基础上继续开发：
1. 添加更多数据类型支持
2. 实现序列化/反序列化
3. 添加更多C++20特性
4. 优化性能和内存使用
5. 扩展测试覆盖

