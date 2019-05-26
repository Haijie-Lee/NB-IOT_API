# NBIoT API v1.5

帮助用户在大多数单片机上连接到NB-IOT平台。仅需几步，用户即可完成任何想要与IOT平台交换的数据的传输。

对于不同的IOT平台的通信数据格式，用户可以通过修改<coap.c>中的内容去适配不同的数据格式。

对于不同NB模组，用户可以通过修改<nb_cmd.c>，和其它与该文件有关联的文件，去适配不同的NB模组。

---

前提条件，你需要知道 IOT平台 的通信数据格式并且在IOT平台上注册一个产品，这对于任何一个程序员都是不困难的。并且你需要知道一些关于（在物理层通过无线通信提供与IOT平台通信的能力的）NB模组的AT指令的知识。



- 第一步，你可以尝试阅读<user_example>去了解如何使用这个API。

- 第二步，在<HardwareInterface>中编写当前单片机的硬件驱动功能，。

- 第三步，你需要检查当前API中的用户设置与用户数据格式与你的目标产品是否一致，如果一致则可以进行下一步。

- 第四步，根据<user_example>的示范开始编写自己的程序。

... 

其他的描述正在等待补充。

![API结构图][scenery]

[scenery]:https://github.com/Haijie-Lee/NB-IOT_API/blob/NB-IOT_API-V1.5/Components/API%E7%BB%93%E6%9E%84%E5%9B%BE.jpg "API的层次图"
