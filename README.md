# Apple-style Calculator for 3DS

苹果风格计算器，上屏显示结果，下屏触屏输入。

## 怎么编译（GitHub Actions 云端编译）

1. 在 GitHub 上新建一个仓库（比如叫 `apple-calc`）
2. 把这个文件夹里的所有文件上传到仓库根目录
3. GitHub Actions 会自动开始编译
4. 编译完成后，在 Actions 页面的 Artifacts 里下载 `3ds-calculator.zip`
5. 解压后得到 `apple-calc.3dsx` 文件

## 怎么在 3DS 上运行

1. 把 `apple-calc.3dsx` 拷到 SD 卡的 `/3ds/` 文件夹
2. 打开 Homebrew Launcher
3. 找到 Apple Calculator 运行

## 功能

- 上屏：大字号显示计算结果
- 下屏：触屏键盘输入
- 苹果风格：黑底 + 白字 + 橙色运算符
- 支持：四则运算、小数点、正负号、百分号、清除
