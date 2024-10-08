# 如何为 LuaSTG Sub 引擎出谋献策

> 下文中的“我们”指 LuaSTG Sub 引擎开发团队

## 发现并报告缺陷

无论你是玩家，还是游戏开发者，发现引擎的缺陷（bug）时，都向我们报告缺陷。

我们提供了多种报告缺陷的途径：

* GitHub Issues：https://github.com/Legacy-LuaSTG-Engine/LuaSTG-Sub/issues
* 邮箱：contact@luastg-sub.com
* LuaSTG 交流群（QQ群）：230927410
* LuaSTG 寒流群（QQ群）：563868788

每一个缺陷报告都会帮助改善引擎代码质量，即使你并未编写一行代码。

## 提出功能需求

LuaSTG Sub 并不是一款大而全的游戏引擎，在过去的 10 多年里，LuaSTG 系列引擎一直专精于“东方风弹幕射击游戏（东方STG）”这个小众领域，引擎提供的主要功能很少发生重大改变。

此外，我们是一群用爱发电的开发者，利用业余时间开发 LuaSTG Sub 引擎，时间、精力、技术都十分紧缺。虽然有很多“棒极了”的想法，但我们却无力开发。

尽管如此，我们仍然欢迎游戏开发者们提出功能需求，因为这会给东方STG带来新的可能性。比如，我们将引擎迁移到更加现代化的平台（Direct3D11、XAudio2等），提高了引擎对Windows 10/11系统的兼容性；我们开发了基本的模型渲染功能，而该功能已经被一些作品使用，关卡背景画面因此变得更加丰富。

> 用开玩笑的方式来说，我们每次开发新功能都是被迫的——“屎都到屁眼了才知道急”

## 改进手册、API文档、翻译

> 程序员最讨厌的是编写手册和API文档、他人的项目不提供手册和API文档

如果你发现手册和API文档有错、漏之处，欢迎向我们反馈。

当然，如果你非常熟悉 Git 和 GitHub 的使用，也可以发起 Pull Request。

## 提交代码

> 程序员最讨厌的是阅读他人的代码

当你非常熟悉 Git 和 GitHub 的使用，且一些功能改进和缺陷已经自行开发、修复完成时，可以考虑发起 Pull Request。

发起 Pull Request 时，需要注意：

* 及时合并主分支最新提交
* 及时处理合并冲突
* 存在破坏兼容性的修改时，可能不会合并到主分支，而是合并到单独的特性分支
* 修改内容过多时，审阅代码可能需要花费很长的时间
