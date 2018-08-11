搞事用插件
---

命令列表：
* GSDetectObjectLod
    `GSDetectObjectLod`
    统计当前Level的所有`静态网格`，`骨骼网格`，`植被模型` 的 `使用数量`，`移动属性`，`阴影属性`，以及在各LOD下的`面数`，`Section数`和`ScreenSize`，并保存到Save/Log/[LevelName].csv下。

* GSFilteStaticMeshActor 
    `GSFilteStaticMeshActor [RegexMeshName]`
    编辑器模式下专用，在当前选中的所有Actor中，过滤掉所有的非StaticMeshActor，并过滤掉使用了[RegexMeshName]所不能完全匹配的StaticMesh的Actor。
    例如:
    `GSFilteStaticMeshActor SM_.+`

* GSFilterActorClassName
    `GSFilterActorClassName [RegexClassName]`
    同上，在当前选中的所有Actor中，根据类型名，过滤不匹配的Actor。

* GSFilterActorDisplayName
    `GSFilterActorDisplayName [RegexClassName]`
    同上，在当前选中的所有Actor中，根据显示名称，过滤不匹配的Actor。

* GSFilterActorIDName
    `GSFilterActorIDName [RegexClassName]`
    同上，在当前选中的所有Actor中，根据ID名称，过滤不匹配的Actor。

* GSCopySelectedActorName
    `GSCopySelectedActorName [FormatString]...`
    编辑器模式下专用，收集选中的Actor的类型名与显示名，然后根据格式化字符串进行格式化后复制到剪切板中。
    例如：
    `GSCopySelectCNAndDN < %CN, %DN, %IN>,`
    %CN将会被替换成Actor的类型名，%DN将会被替换成Actor的显示名。
    例如选中C1-D1-I1, C2-D2-I2两个Actor，格式化后的字符串为`<C1, D1, I1>, <C2, D2, I2>,`。
    支持CPP风格的转义序列，例如`\n`会被转换成一个换行符。
    另，转义支付串中的空格会被分隔，虽然在后来会手动链接，但多个空格只能被当成一个空格使用。

* GSCopyCurrentPosition
    `GSCopyCurrentPosition`
   编辑器模式下专用，将当前摄像机位置保存成`{X=??? Y=??? Z=??? P=??? Y=??? R=???}`的格式，并复制到剪切板中。

* GSMoveToPosition
    `GSMoveToPosition [LocationInfo]`
    编辑器下专用，将当前摄像机移动到这个位置，其接受类似`{X=??? Y=??? Z=??? P=??? Y=??? R=???}`形式的字符串。
    在IsGame()状态下无效，因为摄像机的位置有所限制。

* GSStoreSelectedActor
    `GSStoreSelectedActor`
    编辑器下专用，将选中的所有Actor储存，以供后面获取。使用该命令时，当前的选择将会覆盖上一次的选择。

* GSRestoreSelectedActor
    `GSRestoreSelectedActor`
    编辑器下专用，恢复储存的Actor，当前所选择的将会被覆盖。
