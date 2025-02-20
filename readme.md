# 🎮 2048 Game in C++ with SDL2

**个人的C++程序设计作业**，重写自原先的C语言版本 <br><br>
一个基于SDL2库实现的经典2048游戏，支持图形界面、音效和流畅的动画效果✨<br><br>
<img alt="Snipaste_2025-02-20_11-44-25" src="https://github.com/user-attachments/assets/8d02d011-07bd-47b8-ad56-ee2674feac80" />


## 🚀 功能特性

- **🎯 基本游戏逻辑**
  - ⬆️⬇️⬅️➡️ 方块移动与合并
  - 🆕 新方块生成（2或4）
  - 🏆 游戏胜利（达到2048）与💀失败判断
- **🌈 视觉特效**
  - 🌀 平滑的滑动动画
  - 💥 合并时的膨胀动画
  - ✨ 新方块生成动画
- **📊 分数系统**
  - 🏅 实时分数显示
  - 🏆 最高分记录（持久化存储）
- **🖱️ 交互功能**
  - ⌨️ 键盘控制（WASD/方向键）
  - 🔄 重新开始按钮
  - 🎮 游戏结束/胜利界面
- **🎵 音效支持**
  - 🔊 方块合并音效
  - 🎶 游戏结束/胜利音效

## 📦 依赖项

- SDL2
- SDL2_image
- SDL2_ttf
- SDL2_mixer

### 安装依赖

由于项目目前使用预编译的64位 SDL2 动态链接库，故无需安装依赖
仅支持windows

## 🛠️ 编译与运行

1. 使用提供的编译脚本
```bash
bulid.bat
```
将在/dist文件夹下生成游戏

2. cmake 编译

可以使用任何支持cmake的IDE打开项目，如Clion，vscode等

## 🎮 游戏控制

- **方向键** ⬆️⬇️⬅️➡️: 移动方块
- **WASD**：由于中文输入法等原因，请在切换到英文时在进入游戏界面，否者会导致无法输入，此方式不建议
- **🖱️ 鼠标点击**: 游戏结束时点击"Try Again"重新开始
- **⌨️ P键 （请先切换到英文输入法）**: 生成1024方块（测试用）

## 🏗️ 项目结构

| 类名            | 职责                          |
|-----------------|-------------------------------|
| `Grid`         | 🧩 处理游戏逻辑和状态管理     |
| `Renderer`     | 🎨 图形渲染和动画             |
| `AudioManager` | 🔊 音效播放管理               |
| `InputManager` | ⌨️ 输入处理                   |
| `ScoreManager` | 📈 分数管理                   |

## 🗃️ 资源文件

- 📝 字体: `ttf/font.ttf`
- 🖼️ 图片资源: `img/` 目录
- 🔊 音效文件: `audio/` 目录
- 🏆 高分记录: `img/score.txt`

## ⚠️ 限制

- 📏 目前仅支持4X4棋盘
- 🎮 仅支持基础游戏模式
