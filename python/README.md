# molcpp Python Bindings

这个目录包含molcpp的Python绑定，使用pybind11和xtensor-python构建。

## 架构特点

### ECS (Entity-Component-System) 设计
- **Entity**: 基础实体类，提供组件管理功能
- **Component**: 所有组件的基类，支持类型信息
- **System**: 单例注册表，管理所有实体和组件
- **Atom**: 继承自Entity的原子实体
- **Bond**: 继承自Entity的键实体

### 继承关系
```
Entity (基类)
├── Atom (原子实体)
└── Bond (键实体)

Component (基类)
├── Position (位置)
├── Element (元素)
├── Mass (质量)
├── Radius (半径)
├── Velocity (速度)
├── Charge (电荷)
└── BondInfo (键信息)
```

## 编译

### 1. 配置CMake
```bash
cd molcpp
mkdir -p build
cd build
cmake .. -DBUILD_PYTHON=ON
```

### 2. 编译Python绑定
```bash
make -j$(nproc)
```

### 3. 安装（可选）
```bash
make install
```

## 使用方式

### 基本用法

```python
import molcpp

# 创建原子
atom = molcpp.atom.Atom()

# 通过继承的Entity方法添加组件
atom.add_component(molcpp.ecs.Position, 0.0, 0.0, 0.0)
atom.add_component(molcpp.ecs.Element, "C", 6)
atom.add_component(molcpp.ecs.Mass, 12.011)

# 使用便利方法
atom.set_position(1.0, 2.0, 3.0)
atom.set_element("O", 8)

# 创建键
atom1 = molcpp.atom.Atom()
atom2 = molcpp.atom.Atom()
bond = molcpp.bond.Bond(atom1, atom2)

# 系统查询
system = molcpp.ecs.System.get_instance()
entity_count = system.get_entity_count()
position_count = system.get_component_number(molcpp.ecs.Position)
```

### 组件管理

所有Entity（包括Atom和Bond）都继承了以下方法：
- `add_component(component_type, *args)` - 添加组件
- `get_component(component_type)` - 获取组件
- `has_component(component_type)` - 检查组件存在
- `remove_component(component_type)` - 移除组件
- `get_id()` - 获取实体ID

### 系统查询

```python
system = molcpp.ecs.System.get_instance()

# 统计信息
total_entities = system.get_entity_count()
position_components = system.get_component_number(molcpp.ecs.Position)

# 查找实体
entities_with_position = system.get_entities_with_component(molcpp.ecs.Position)
```

## 测试

```bash
# 编译后运行测试
cd molcpp/python
python tests/test_bindings.py

# 运行示例
python examples/water_molecule.py
```

## 设计原则

1. **继承优于重复**: Atom和Bond继承Entity的方法，而不是重新绑定
2. **抽象化**: Atom不包含具体的化学元素创建方法，保持抽象性
3. **组合优于继承**: 使用ECS模式，实体通过组件组合来获得功能
4. **类型安全**: 所有组件都有类型信息，支持运行时类型查询

## 文件结构

```
python/
├── bindings/
│   ├── atom.cpp          # Atom类绑定
│   ├── bond.cpp          # Bond类绑定
│   ├── ecs.cpp           # ECS系统绑定
│   └── molcpp.cpp        # 主模块绑定
├── tests/
│   └── test_bindings.py  # 绑定测试
├── examples/
│   └── water_molecule.py # 使用示例
└── README.md             # 本文件
```
