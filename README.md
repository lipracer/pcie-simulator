# pcie-qemu

pcie-qemu 是一个qemu跟linux驱动开发的学习项目，最后完成一个pcie设备的模拟，以及基础的驱动功能。

## 功能特点

- 简单的驱动程序开发教程
- pcie设备的模拟


## 安装指南

1. 克隆仓库：

   ```bash
   git clone https://github.com/lipracer/pcie-simulator.git
   ```
2. 更新submodule
    ```
    make update_3party
    ```
3. 安装系统依赖
    ```
    apt install bison flex bc libglib2.0-dev cpio ninja-build
    ```
4. 编译submodule
    ```
    make all -j8
    ```


## 使用示例

```shell
make test_char_dev
```

## 贡献指南
欢迎任何形式的贡献！请阅读 [CONTRIBUTING.md](./CONTRIBUTING.md) 了解详细的贡献流程。


## 许可证

本项目采用 MIT 许可证，详情请参阅 [LICENSE](./LICENSE) 文件。